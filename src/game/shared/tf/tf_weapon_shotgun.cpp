//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================

#include "cbase.h"
#include "props_shared.h"
#include "tf_weapon_shotgun.h"
#include "decals.h"
#include "tf_fx_shared.h"
#include "tf_gamerules.h"

// Client specific.
#if defined( CLIENT_DLL )
#include "c_tf_player.h"
// Server specific.
#else
#include "tf_player.h"
#include "tf_obj_sentrygun.h"
#endif


float AirBurstDamageForce( Vector const &vecSize, float damage, float scale )
{
	const float flSizeMag = vecSize.x * vecSize.y * vecSize.z;
	const float flHullMag = 48 * 48 * 82.0;

	const float flDamageForce = damage * ( flHullMag / flSizeMag ) * scale;

	return Min( flDamageForce, 1000.0f );
}

//=============================================================================
//
// Weapon Shotgun tables.
//

CREATE_SIMPLE_WEAPON_TABLE( TFShotgun, tf_weapon_shotgun_primary )
CREATE_SIMPLE_WEAPON_TABLE( TFShotgun_Soldier, tf_weapon_shotgun_soldier )
CREATE_SIMPLE_WEAPON_TABLE( TFShotgun_HWG, tf_weapon_shotgun_hwg )
CREATE_SIMPLE_WEAPON_TABLE( TFShotgun_Pyro, tf_weapon_shotgun_pyro )
CREATE_SIMPLE_WEAPON_TABLE( TFScatterGun, tf_weapon_scattergun )

//=============================================================================
//
// Weapon Shotgun functions.
//

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CTFShotgun::CTFShotgun()
{
	m_bReloadsSingly = true;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFShotgun::PrimaryAttack()
{
	if ( !CanAttack() )
		return;

	// Set the weapon mode.
	m_iWeaponMode = TF_WEAPON_PRIMARY_MODE;

	BaseClass::PrimaryAttack();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFShotgun::UpdatePunchAngles( CTFPlayer *pPlayer )
{
	// Update the player's punch angle.
	QAngle angle = pPlayer->GetPunchAngle();
	float flPunchAngle = m_pWeaponInfo->GetWeaponData( m_iWeaponMode ).m_flPunchAngle;
	angle.x -= SharedRandomInt( "ShotgunPunchAngle", ( flPunchAngle - 1 ), ( flPunchAngle + 1 ) );
	pPlayer->SetPunchAngle( angle );
}

//=============================================================================
//
// Weapon Scatter Gun functions.
//

void CTFScatterGun::FireBullet( CTFPlayer *pPlayer )
{
	if ( !HasKnockback() || ( TFGameRules() && TFGameRules()->State_Get() == GR_STATE_PREROUND ) )
	{
		BaseClass::FireBullet( pPlayer );
		return;
	}

	CTFPlayer *pOwner = ToTFPlayer( GetOwner() );
	if ( !pOwner )
		return;

	if ( !( ( pOwner->GetFlags() & FL_ONGROUND ) || pOwner->m_Shared.HasRecoiled() ) )
	{
		pOwner->m_Shared.SetHasRecoiled( true );

		pOwner->m_Shared.StunPlayer( 0.3f, 1.0f, 1.0f, TF_STUNFLAG_LIMITMOVEMENT | TF_STUNFLAG_SLOWDOWN, NULL );

	#ifdef GAME_DLL
		EntityMatrix matrix;
		matrix.InitFromEntity( pOwner );

		Vector vecLocalTranslation = pOwner->GetAbsOrigin() + pOwner->GetAbsVelocity();

		Vector vecLocal = matrix.WorldToLocal( vecLocalTranslation );
		vecLocal.x = -300.0f;

		Vector vecVelocity = matrix.LocalToWorld( vecLocal );
		vecVelocity -= pOwner->GetAbsOrigin();

		pOwner->SetAbsVelocity( vecVelocity );

		pOwner->ApplyAbsVelocityImpulse( Vector( 0, 0, 50 ) );
		pOwner->RemoveFlag( FL_ONGROUND );
	#endif
	}

	BaseClass::FireBullet( pPlayer );
}

void CTFScatterGun::Equip( CBaseCombatCharacter *pEquipTo )
{
	if ( pEquipTo )
	{
		CTFPlayer *pOwner = ToTFPlayer( pEquipTo );
		if ( pOwner )
		{
			// CTFPlayerShared::SetScoutHypeMeter
		}
	}

	BaseClass::Equip( pEquipTo );
}

bool CTFScatterGun::Reload()
{
	int nScatterGunNoReloadSingle = 0;
	CALL_ATTRIB_HOOK_INT( nScatterGunNoReloadSingle, set_scattergun_no_reload_single );
	if ( nScatterGunNoReloadSingle == 1 )
		m_bReloadsSingly = false;

	return BaseClass::Reload();
}

void CTFScatterGun::FinishReload()
{
	CTFPlayer *pOwner = ToTFPlayer( GetOwner() );
	if ( !pOwner )
		return;

	if ( !UsesClipsForAmmo1() )
		return;

	if ( ReloadsSingly() )
		return;

	m_iClip1 += Min( GetMaxClip1() - m_iClip1, pOwner->GetAmmoCount( m_iPrimaryAmmoType ) );

	pOwner->RemoveAmmo( GetMaxClip1(), m_iPrimaryAmmoType );

	BaseClass::FinishReload();
}

bool CTFScatterGun::SendWeaponAnim( int iActivity )
{
	if ( GetTFPlayerOwner() && HasKnockback() )
	{
		switch ( iActivity )
		{
			case ACT_VM_DRAW:
				iActivity = ACT_ITEM2_VM_DRAW;
				break;
			case ACT_VM_HOLSTER:
				iActivity = ACT_ITEM2_VM_HOLSTER;
				break;
			case ACT_VM_IDLE:
				iActivity = ACT_ITEM2_VM_IDLE;
				break;
			case ACT_VM_PULLBACK:
				iActivity = ACT_ITEM2_VM_PULLBACK;
				break;
			case ACT_VM_PRIMARYATTACK:
				iActivity = ACT_ITEM2_VM_PRIMARYATTACK;
				break;
			case ACT_VM_SECONDARYATTACK:
				iActivity = ACT_ITEM2_VM_SECONDARYATTACK;
				break;
			case ACT_VM_RELOAD:
				iActivity = ACT_ITEM2_VM_RELOAD;
				break;
			case ACT_VM_DRYFIRE:
				iActivity = ACT_ITEM2_VM_DRYFIRE;
				break;
			case ACT_VM_IDLE_TO_LOWERED:
				iActivity = ACT_ITEM2_VM_IDLE_TO_LOWERED;
				break;
			case ACT_VM_IDLE_LOWERED:
				iActivity = ACT_ITEM2_VM_IDLE_LOWERED;
				break;
			case ACT_VM_LOWERED_TO_IDLE:
				iActivity = ACT_ITEM2_VM_LOWERED_TO_IDLE;
				break;
			default:
				return BaseClass::SendWeaponAnim( iActivity );
		}
	}

	return BaseClass::SendWeaponAnim( iActivity );
}

bool CTFScatterGun::HasKnockback() const
{
	int nScatterGunHasKnockback = 0;
	CALL_ATTRIB_HOOK_INT( nScatterGunHasKnockback, set_scattergun_has_knockback );
	return nScatterGunHasKnockback == 1;
}



IMPLEMENT_NETWORKCLASS_ALIASED( TFShotgun_Revenge, DT_TFShotgun_Revenge )

BEGIN_NETWORK_TABLE( CTFShotgun_Revenge, DT_TFShotgun_Revenge )
#if !defined( CLIENT_DLL )
	SendPropFloat( SENDINFO( m_iRevengeCrits ), 0, SPROP_NOSCALE | SPROP_CHANGES_OFTEN ),
#else
	RecvPropFloat( RECVINFO( m_iRevengeCrits ) ),
#endif
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CTFShotgun_Revenge )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( tf_weapon_sentry_revenge, CTFShotgun_Revenge );
PRECACHE_WEAPON_REGISTER( tf_weapon_sentry_revenge );


CTFShotgun_Revenge::CTFShotgun_Revenge()
{
	m_bReloadsSingly = true;
	m_iRevengeCrits = 0;
}

#ifdef CLIENT_DLL
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CTFShotgun_Revenge::GetWorldModelIndex( void )
{
	CTFPlayer *pOwner = GetTFPlayerOwner();
	if ( pOwner && pOwner->IsAlive() )
	{
		if ( pOwner->IsPlayerClass( TF_CLASS_ENGINEER ) && pOwner->m_Shared.InCond( TF_COND_TAUNTING ) )
			return modelinfo->GetModelIndex( "models/player/items/engineer/guitar.mdl" );
	}

	return BaseClass::GetWorldModelIndex();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFShotgun_Revenge::SetWeaponVisible( bool visible )
{
	if ( !visible )
	{
		CTFPlayer *pOwner = GetTFPlayerOwner();
		if ( pOwner && pOwner->IsAlive() )
		{
			if ( pOwner->IsPlayerClass( TF_CLASS_ENGINEER ) && pOwner->m_Shared.InCond( TF_COND_TAUNTING ) )
			{
				const int iModelIndex = modelinfo->GetModelIndex( "models/player/items/engineer/guitar.mdl" );

				CUtlVector<breakmodel_t> list;

				BuildGibList( list, iModelIndex, 1.0f, COLLISION_GROUP_NONE );
				if ( !list.IsEmpty() )
				{
					QAngle vecAngles = CollisionProp()->GetCollisionAngles();

					Vector vecFwd, vecRight, vecUp;
					AngleVectors( vecAngles, &vecFwd, &vecRight, &vecUp );

					Vector vecOrigin = CollisionProp()->GetCollisionOrigin();
					vecOrigin = vecOrigin + vecFwd * 70.0f + vecUp * 10.0f;

					AngularImpulse angularImpulse( RandomFloat( 0.0f, 120.0f ), RandomFloat( 0.0f, 120.0f ), 0.0 );

					breakablepropparams_t params( vecOrigin, vecAngles, Vector( 0.0f, 0.0f, 200.0f ), angularImpulse );

					CreateGibsFromList( list, iModelIndex, NULL, params, NULL, -1, false, true );
				}
			}

		}
	}
	BaseClass::SetWeaponVisible( visible );
}
#endif

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFShotgun_Revenge::PrimaryAttack( void )
{
	if ( !CanAttack() )
		return;

	BaseClass::PrimaryAttack();

	m_iRevengeCrits = Max( m_iRevengeCrits - 1, 0 );

	CTFPlayer *pOwner = GetTFPlayerOwner();
	if ( pOwner && pOwner->IsAlive() )
	{
		if ( m_iRevengeCrits == 0 )
			pOwner->m_Shared.RemoveCond( TF_COND_CRITBOOSTED );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CTFShotgun_Revenge::GetCount( void ) const
{
	return m_iRevengeCrits;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CTFShotgun_Revenge::GetCustomDamageType( void ) const
{
	if ( m_iRevengeCrits > 0 )
		return TF_DMG_CUSTOM_SHOTGUN_REVENGE_CRIT;

	return TF_DMG_CUSTOM_NONE;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFShotgun_Revenge::Deploy( void )
{
	CTFPlayer *pOwner = GetTFPlayerOwner();
	if ( pOwner && BaseClass::Deploy() )
	{
		if ( m_iRevengeCrits > 0 )
			pOwner->m_Shared.AddCond( TF_COND_CRITBOOSTED );

		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFShotgun_Revenge::Holster( CBaseCombatWeapon *pSwitchTo )
{
	CTFPlayer *pOwner = GetTFPlayerOwner();
	if ( pOwner && BaseClass::Holster( pSwitchTo ) )
	{
		pOwner->m_Shared.RemoveCond( TF_COND_CRITBOOSTED );

		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFShotgun_Revenge::Detach( void )
{
	m_iRevengeCrits = 0;
	BaseClass::Detach();
}

#ifdef GAME_DLL
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFShotgun_Revenge::OnSentryKilled( CObjectSentrygun *pSentry )
{
	if ( CanGetRevengeCrits() )
	{
		m_iRevengeCrits = Min( m_iRevengeCrits + pSentry->GetAssists() + ( pSentry->GetKills() * 2 ), TF_WEAPON_MAX_REVENGE );

		CTFPlayer *pOwner = GetTFPlayerOwner();

		if ( pOwner && pOwner->GetActiveWeapon() == this )
		{
			if ( m_iRevengeCrits > 0 )
			{
				if ( !pOwner->m_Shared.InCond( TF_COND_CRITBOOSTED ) )
					pOwner->m_Shared.AddCond( TF_COND_CRITBOOSTED );
			}
			else
			{
				if ( pOwner->m_Shared.InCond( TF_COND_CRITBOOSTED ) )
					pOwner->m_Shared.RemoveCond( TF_COND_CRITBOOSTED );
			}
		}
	}
}
#endif

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFShotgun_Revenge::Precache( void )
{
	int iMdlIndex = PrecacheModel( "models/player/items/engineer/guitar.mdl" );
	PrecacheGibsForModel( iMdlIndex );

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFShotgun_Revenge::CanGetRevengeCrits( void ) const
{
	int nSentryRevenge = 0;
	CALL_ATTRIB_HOOK_INT( nSentryRevenge, sentry_killed_revenge );
	return nSentryRevenge == 1;
}
