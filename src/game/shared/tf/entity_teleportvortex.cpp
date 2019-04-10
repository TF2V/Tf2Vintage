#include "cbase.h"
#include "tf_gamerules.h"
#include "entity_teleportvortex.h"

#ifdef CLIENT_DLL
#include "c_tf_player.h"
#else
#include "tf_player.h"
#include "team.h"
#endif

#ifdef CLIENT_DLL
ConVar vortex_float_osc_speed( "vortex_float_osc_speed", "2.0", FCVAR_CHEAT|FCVAR_REPLICATED|FCVAR_HIDDEN );
ConVar vortex_float_amp( "vortex_float_amp", "5.0", FCVAR_CHEAT|FCVAR_REPLICATED|FCVAR_HIDDEN );
ConVar vortex_fade_fraction_denom( "vortex_fade_fraction_denom", "10.0", FCVAR_CHEAT|FCVAR_REPLICATED|FCVAR_HIDDEN );
ConVar vortex_book_offset( "vortex_book_offset", "5.0", FCVAR_CHEAT|FCVAR_REPLICATED|FCVAR_HIDDEN );
#endif

IMPLEMENT_NETWORKCLASS_ALIASED( TeleportVortex, DT_TeleportVortex )
BEGIN_NETWORK_TABLE( CTeleportVortex, DT_TeleportVortex )
#ifdef CLIENT_DLL
	RecvPropInt( RECVINFO( m_iState ) ),
#else
	SendPropInt( SENDINFO( m_iState ), 4, SPROP_CHANGES_OFTEN|SPROP_UNSIGNED ),
#endif
END_NETWORK_TABLE()

#ifdef GAME_DLL
BEGIN_DATADESC( CTeleportVortex )
	DEFINE_THINKFUNC( VortexThink ),
END_DATADESC()
#endif

LINK_ENTITY_TO_CLASS( teleport_vortex, CTeleportVortex );

#ifdef GAME_DLL
//-----------------------------------------------------------------------------
// Purpose: Teleport to another section of map for additional AI logic
//-----------------------------------------------------------------------------
static void SendPlayerToTheUnderworld( CTFPlayer *pPlayer, const char *pszTargetName )
{
	if ( pPlayer )
	{
		CUtlVector<CBaseEntity *> targets;
		for ( CBaseEntity *pEnt = gEntList.FirstEnt(); pEnt; pEnt = gEntList.NextEnt( pEnt ) )
		{
			if ( pEnt->ClassMatches( "info_target" ) && FStrEq( STRING( pEnt->GetEntityName() ), pszTargetName ) )
				targets.AddToTail( pEnt );
		}

		if ( targets.IsEmpty() )
		{
			Warning( "SendPlayerToTheUnderworld: No info_target entities named '%s' found!\n", pszTargetName );
			return;
		}

		CUtlVector<CTFPlayer *> players;
		CUtlVector<CBaseEntity *> blocked;
		
		CollectPlayers( &players, GetEnemyTeam( pPlayer ), true );
		for ( int i=0; i<players.Count(); ++i )
		{
			CTFPlayer *player = players[i];

			for ( int j=0; j<targets.Count(); ++j )
			{
				CBaseEntity *target = targets[j];

				if ( ( target->GetAbsOrigin() - player->GetAbsOrigin() ).LengthSqr() < Square( 25.0f ) )
				{
					blocked.AddToTail( target );
					break;
				}
			}
		}

		CBaseEntity *pTarget = nullptr;
		if ( blocked.IsEmpty() )
		{
			pTarget = targets[RandomInt( 0, targets.Count()-1 )];
		}
		else
		{
			pTarget = blocked[RandomInt( 0, blocked.Count()-1 )];
			for ( int i=0; i<players.Count(); ++i )
			{
				CTFPlayer *player = players[i];
				if ( ( pTarget->GetAbsOrigin() - player->GetAbsOrigin() ).LengthSqr() < Square( 25.0f ) )
				{
					CTakeDamageInfo info( pPlayer, pPlayer, 1000.0f, DMG_CRUSH, TF_DMG_CUSTOM_TELEFRAG );
					player->TakeDamage( info );
				}
			}
		}

		if ( !pTarget )
			return;

		if ( pPlayer->GetTeam() )
		{
			UTIL_LogPrintf( "HALLOWEEN: \"%s<%i><%s><%s>\" purgatory_teleport \"%s\"\n",
							pPlayer->GetPlayerName(),
							pPlayer->GetUserID(),
							pPlayer->GetNetworkIDString(),
							pPlayer->GetTeam()->GetName(),
							pszTargetName );
		}

		if ( FStrEq( pszTargetName, "spawn_loot" ) )
		{
			CReliableBroadcastRecipientFilter filter;
			UTIL_SayText2Filter( filter, pPlayer, "#TF_Halloween_Loot_Island", pPlayer->GetPlayerName() );
		}

		if ( pPlayer->IsPlayerClass( TF_CLASS_SNIPER ) && pPlayer->m_Shared.InCond( TF_COND_AIMING ) )
		{
			CTFWeaponBaseGun *pWeapon = (CTFWeaponBaseGun *)pPlayer->GetActiveTFWeapon();
			if ( pWeapon && WeaponID_IsSniperRifle( pWeapon->GetWeaponID() ) )
				pWeapon->ToggleZoom();
		}

		pPlayer->Teleport( &pTarget->GetAbsOrigin(), &pTarget->GetAbsAngles(), &vec3_origin );

		if ( TFGameRules() && TFGameRules()->IsHalloweenScenario( CTFGameRules::HALLOWEEN_SCENARIO_LAKESIDE ) && pPlayer->IsAlive() )
		{
			pPlayer->TakeHealth( pPlayer->GetMaxHealth(), DMG_GENERIC );
			pPlayer->m_Shared.HealthKitPickupEffects( 0 );
			pPlayer->m_Shared.RemoveCond( TF_COND_HALLOWEEN_BOMB_HEAD );
		}

		UTIL_ScreenFade( pPlayer, { 0x64, 0xFF, 0xFF, 0xFF }, 0.25f, 0.4f, FFADE_IN );
	}
}

#endif


CTeleportVortex::CTeleportVortex()
{
}

CTeleportVortex::~CTeleportVortex()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTeleportVortex::Precache( void )
{
	BaseClass::Precache();

	PrecacheModel( "models/props_halloween/bombonomicon.mdl" );

	bool bAllowPrecache = CBaseEntity::IsPrecacheAllowed();
	CBaseEntity::SetAllowPrecache( true );

	PrecacheScriptSound( "Halloween.TeleportVortex.EyeballMovedVortex" );
	PrecacheScriptSound( "Halloween.TeleportVortex.EyeballDiedVortex" );
	PrecacheScriptSound( "Halloween.TeleportVortex.BookSpawn" );
	PrecacheScriptSound( "Halloween.TeleportVortex.BookExit" );

	PrecacheParticleSystem( "eyeboss_tp_vortex" );
	PrecacheParticleSystem( "eyeboss_aura_angry" );

	CBaseEntity::SetAllowPrecache( bAllowPrecache );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTeleportVortex::Spawn( void )
{
	Precache();
	BaseClass::Spawn();

	SetMoveType( MOVETYPE_NONE );

	Vector vecMins( -50.0f ), vecMaxs( 50.0f );
	CollisionProp()->SetSolid( SOLID_BBOX );
	CollisionProp()->SetCollisionBounds( vecMins, vecMaxs );
	CollisionProp()->SetSolidFlags( FSOLID_TRIGGER );

	SetCollisionGroup( COLLISION_GROUP_WEAPON );

	AddEffects( EF_NODRAW );

	UseClientSideAnimation();

	SetRenderMode( kRenderTransAlpha );
	SetRenderColor( 255, 255, 255, 255 );

	m_lifeTimeDuration.Start( 5.0f );

#ifndef CLIENT_DLL
	m_pszTeleTarget = "spawn_purgatory";

	SetThink( &CTeleportVortex::VortexThink );
	SetNextThink( gpGlobals->curtime );

	m_bUseTeamSpawns = false;
#else
	SetNextClientThink( CLIENT_THINK_ALWAYS );

	AddToLeafSystem( RENDER_GROUP_TWOPASS );
#endif
}

#ifndef CLIENT_DLL
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------s
int CTeleportVortex::UpdateTransmitState( void )
{
	return SetTransmitState( FL_EDICT_PVSCHECK );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTeleportVortex::StartTouch( CBaseEntity *pOther )
{
	BaseClass::StartTouch( pOther );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTeleportVortex::Touch( CBaseEntity *pOther )
{
	BaseClass::Touch( pOther );
	if ( pOther )
	{
		if ( m_bUseTeamSpawns )
		{
			CFmtStr str( "%s%s", m_pszTeleTarget, pOther->GetTeamNumber() == TF_TEAM_RED ? "_red" : "_blue" );
			SendPlayerToTheUnderworld( ToTFPlayer( pOther ), str );
		}
		else
		{
			SendPlayerToTheUnderworld( ToTFPlayer( pOther ), m_pszTeleTarget );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTeleportVortex::SetupVortex( bool bGotoLoot, bool b2 )
{
	m_bUseTeamSpawns = b2;
	if ( bGotoLoot )
	{
		m_iState = 2;

		m_pszTeleTarget = "spawn_loot";

		RemoveEffects( EF_NODRAW );
		SetModel( "models/props_halloween/bombonomicon.mdl" );

		return;
	}

	m_iState = 1;

	m_pszTeleTarget = "spawn_purgatory";

	AddEffects( EF_NODRAW );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTeleportVortex::VortexThink( void )
{
	StudioFrameAdvance();

	CUtlVector<CTFPlayer *> players;
	CollectPlayers( &players, TF_TEAM_RED, true );
	CollectPlayers( &players, TF_TEAM_BLUE, true, true );

	for ( int i=0; i<players.Count(); ++i )
	{
		CTFPlayer *pPlayer = players[i];
		if ( !(pPlayer->GetFlags() & FL_ONGROUND) )
		{
			Vector vecToMe = WorldSpaceCenter() - pPlayer->WorldSpaceCenter();
			float flLength = vecToMe.NormalizeInPlace();
			if ( flLength <= 500.0f )
			{
				if ( !pPlayer->IsLookingTowards( WorldSpaceCenter() ) || !pPlayer->IsLineOfSightClear( WorldSpaceCenter(), CBaseCombatCharacter::IGNORE_NOTHING, pPlayer ) )
					continue;

				Vector vecVel = ( vecToMe * 30.0f );
				pPlayer->ApplyAbsVelocityImpulse( vecVel );
			}
		}
	}

	if ( !m_lifeTimeDuration.IsElapsed() )
		SetNextThink( gpGlobals->curtime );
	else
		UTIL_Remove( this );
}

#else
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTeleportVortex::OnDataChanged( DataUpdateType_t updateType )
{
	if ( m_iState != m_iStateParity )
	{
		if ( m_iState != 0 )
		{
			if ( m_iState == 1 )
			{
				m_pGlowEffect = ParticleProp()->Create( "eyeboss_tp_vortex", PATTACH_ABSORIGIN );
				EmitSound( "Halloween.TeleportVortex.EyeballMovedVortex" );
			}
			else
			{
				int iSequence = LookupSequence( "flip_stimulated" );
				if ( iSequence )
				{
					SetSequence( iSequence );
					SetPlaybackRate( 1.0f );
					SetCycle( 0.0f );
					ResetSequenceInfo();
				}

				m_pGlowEffect = ParticleProp()->Create( "eyeboss_aura_angry", PATTACH_ABSORIGIN );
				EmitSound( "Halloween.TeleportVortex.EyeballDiedVortex" );
			}
		}

		m_iStateParity = m_iState;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTeleportVortex::ClientThink( void )
{
	float flLifeTime = m_lifeTimeDuration.GetCountdownDuration();
	float flElapsedTime = m_lifeTimeDuration.GetElapsedTime();
	float flDenom = vortex_fade_fraction_denom.GetFloat();

	if ( flElapsedTime <= ( flLifeTime / flDenom ) )
	{
		float flFraction = Clamp( flElapsedTime / ( flLifeTime / flDenom ), 0.0f, 1.0f );
		m_flFadeFraction = ( ( flFraction * -2.0f ) + 3.0f ) * Square( flFraction );
	}
	else if ( flElapsedTime >= ( flLifeTime - ( flLifeTime / flDenom ) ) )
	{
		float flFraction = Clamp( ( ( ( flLifeTime - flElapsedTime ) - flLifeTime  / flDenom ) / ( flLifeTime / flDenom ) ) + 1.0f, 0.0f, 1.0f );
		m_flFadeFraction = ( ( flFraction * -2.0f ) + 3.0f ) * Square( flFraction );
	}
	else
	{
		m_flFadeFraction = 1.0f;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTeleportVortex::BuildTransformations( CStudioHdr *pStudioHdr, Vector *pos, Quaternion *q, const matrix3x4_t& cameraTransform, int boneMask, CBoneBitList& boneComputed )
{
	q[0].x = vortex_book_offset.GetFloat();
	q[0].y = 0;
	q[0].z = ( vortex_float_amp.GetFloat() * sin( vortex_float_osc_speed.GetFloat() * gpGlobals->absoluteframetime ) ) + ( ( m_flFadeFraction * 500.0f ) + -500.0f );

	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	matrix3x4_t matrix;
	if ( pPlayer && pPlayer->IsAlive() )
	{
		Vector vecToPlayer = pPlayer->GetAbsOrigin() - GetAbsOrigin();
		vecToPlayer.NormalizeInPlace();

		if ( vecToPlayer.LengthSqr() > 0.1f )
			VectorMatrix( vecToPlayer, matrix );
	}
	else
	{
		QuaternionMatrix( *q, matrix );
	}

	MatrixQuaternion( matrix, *q );

	C_BaseAnimating::BuildTransformations( pStudioHdr, pos, q, cameraTransform, boneMask, boneComputed );
}

#endif