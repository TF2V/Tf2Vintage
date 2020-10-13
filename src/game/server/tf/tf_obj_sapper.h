//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Upgrade that damages the object over time
//
//=============================================================================//

#ifndef TF_OBJ_SAPPER_H
#define TF_OBJ_SAPPER_H
#ifdef _WIN32
#pragma once
#endif

#include "tf_obj_baseupgrade_shared.h"

enum SapperModel_t
{
	SAPPER_MODEL_PLACED = 0,
	SAPPER_MODEL_PLACEMENT
};

// ------------------------------------------------------------------------ //
// Sapper upgrade
// ------------------------------------------------------------------------ //
class CObjectSapper : public CBaseObjectUpgrade
{
	DECLARE_CLASS( CObjectSapper, CBaseObjectUpgrade );

public:
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();

	CObjectSapper();

	virtual void	Spawn();
	virtual void	Precache();
	virtual bool	IsHostileUpgrade( void ) { return true; }
	virtual void	FinishedBuilding( void );
	virtual void	SetupAttachedVersion( void );
	char const		*GetSapperModelName( SapperModel_t iModelType );
	char const		*GetSapperSoundName( void );
	virtual void	DetachObjectFromObject( void );
	virtual void	UpdateOnRemove( void );
	virtual void	OnGoActive( void );

	virtual void	SapperThink( void );
	virtual int		GetBaseHealth( void );
	virtual int		OnTakeDamage( const CTakeDamageInfo &info );
	virtual void	Killed(const CTakeDamageInfo &info);

	virtual float	ReverseBuildingConstruction(void);
private:
	float m_flSapperDamageAccumulator;
	float m_flLastThinkTime;
	float m_flSappingStartTime;

	string_t m_iszPlacementModel;
	string_t m_iszPlacedModel;
	string_t m_iszSapperSound;
};

#endif // TF_OBJ_SAPPER_H
