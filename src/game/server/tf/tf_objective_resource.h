//========= Copyright � 1996-2006, Valve Corporation, All rights reserved. ============//
//
// Purpose: DOD's objective resource, transmits all objective states to players
//
// $NoKeywords: $
//=============================================================================//

#ifndef TF_OBJECTIVE_RESOURCE_H
#define TF_OBJECTIVE_RESOURCE_H
#ifdef _WIN32
#pragma once
#endif

#include "tf_shareddefs.h"
#include "team_objectiveresource.h"

class CTFObjectiveResource : public CBaseTeamObjectiveResource
{
	DECLARE_CLASS( CTFObjectiveResource, CBaseTeamObjectiveResource );
public:
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	CTFObjectiveResource();
	virtual ~CTFObjectiveResource();

	virtual void Spawn( void );

	void SetMannVsMachineMaxWaveCount( int nCount )			{ m_nMannVsMachineMaxWaveCount = nCount; }
	int GetMannVsMachineMaxWaveCount( void )				{ return m_nMannVsMachineMaxWaveCount; }
	void SetMannVsMachineWaveCount( int nCount )			{ m_nMannVsMachineWaveCount = nCount; }
	int GetMannVsMachineWaveCount( void )					{ return m_nMannVsMachineWaveCount; }

	void SetMannVsMachineWaveEnemyCount( int nCount )		{ m_nMannVsMachineWaveEnemyCount = nCount; }
	int	 GetMannVsMachineWaveEnemyCount( void )				{ return m_nMannVsMachineWaveEnemyCount.Get(); }

	void AddMvMWorldMoney( int nCurrency )					{ m_nMvMWorldMoney += nCurrency; }

	void SetMannVsMachineNextWaveTime( float flTime )		{ m_flMannVsMachineNextWaveTime = flTime; }
	void SetMannVsMachineBetweenWaves( bool bVal )			{ m_bMannVsMachineBetweenWaves = bVal; }
	bool GetMannVsMachineIsBetweenWaves( void )				{ return m_bMannVsMachineBetweenWaves; }

	void SetMannVsMachineWaveClassCount( int nIndex, int nCount ) { m_nMannVsMachineWaveClassCounts.Set( nIndex, nCount ); }
	void SetMannVsMachineWaveClassName( int nIndex, string_t iszClassIconName ) { m_iszMannVsMachineWaveClassNames.Set( nIndex, iszClassIconName ); }
	string_t GetMannVsMachineWaveClassName( int nIndex )	{ return m_iszMannVsMachineWaveClassNames[ nIndex ]; }
	void IncrementMannVsMachineWaveClassCount( string_t iszClassIconName, unsigned int iFlags );
	void DecrementMannVsMachineWaveClassCount( string_t iszClassIconName, unsigned int iFlags );
	void IncrementTeleporterCount()							{ IncrementMannVsMachineWaveClassCount( m_iszTeleporter, MVM_CLASS_FLAG_MISSION ); }
	void DecrementTeleporterCount()							{ DecrementMannVsMachineWaveClassCount( m_iszTeleporter, MVM_CLASS_FLAG_MISSION ); }
	int GetMannVsMachineWaveClassCount( int nIndex )		{ return m_nMannVsMachineWaveClassCounts[ nIndex ]; }
	void SetMannVsMachineWaveClassActive( string_t iszClassIconName, bool bActive = true );

	void ClearMannVsMachineWaveClassFlags( void );
	void AddMannVsMachineWaveClassFlags( int nIndex, unsigned int iFlags );
	unsigned int GetMannVsMachineWaveClassFlags( int nIndex ) { return m_nMannVsMachineWaveClassFlags[ nIndex ]; }

	void SetFlagCarrierUpgradeLevel( int nLevel )			{ m_nFlagCarrierUpgradeLevel = nLevel; }
	int GetFlagCarrierUpgradeLevel( void )					{ return m_nFlagCarrierUpgradeLevel; }
	void SetBaseMvMBombUpgradeTime( float nTime )			{ m_flMvMBaseBombUpgradeTime = nTime; }
	float GetBaseMvMBombUpgradeTime( void )					{ return m_flMvMBaseBombUpgradeTime; }
	void SetNextMvMBombUpgradeTime( float nTime )			{ m_flMvMNextBombUpgradeTime = nTime; }
	float GetNextMvMBombUpgradeTime( void )					{ return m_flMvMNextBombUpgradeTime; }

	void SetMannVsMachineChallengeIndex( int iIndex )		{ m_iChallengeIndex = iIndex; }
	int	 GetMannVsMachineChallengeIndex( void )				{ return m_iChallengeIndex; }
	void SetMvMPopfileName( string_t iszMvMPopfileName )	{ m_iszMvMPopfileName = iszMvMPopfileName; }
	string_t GetMvMPopfileName( void ) const				{ return m_iszMvMPopfileName.Get(); }
	void SetMannVsMachineEventPopfileType( int nType )		{ m_nMvMEventPopfileType = nType; }

private:
	CNetworkVar( int, m_nMannVsMachineMaxWaveCount );
	CNetworkVar( int, m_nMannVsMachineWaveCount );
	CNetworkVar( int, m_nMannVsMachineWaveEnemyCount );

	CNetworkVar( int, m_nMvMWorldMoney );

	CNetworkVar( float, m_flMannVsMachineNextWaveTime );
	CNetworkVar( bool, m_bMannVsMachineBetweenWaves );

	CNetworkArray( int, m_nMannVsMachineWaveClassCounts, MVM_CLASS_TYPES_PER_WAVE_MAX );
	CNetworkArray( string_t, m_iszMannVsMachineWaveClassNames, MVM_CLASS_TYPES_PER_WAVE_MAX );
	CNetworkArray( unsigned int, m_nMannVsMachineWaveClassFlags, MVM_CLASS_TYPES_PER_WAVE_MAX );
	CNetworkArray( bool, m_bMannVsMachineWaveClassActive, MVM_CLASS_TYPES_PER_WAVE_MAX );

	CNetworkVar( int, m_nFlagCarrierUpgradeLevel );
	CNetworkVar( float, m_flMvMBaseBombUpgradeTime );
	CNetworkVar( float, m_flMvMNextBombUpgradeTime );

	CNetworkVar( int, m_iChallengeIndex );
	CNetworkVar( string_t, m_iszMvMPopfileName ) ;
	CNetworkVar( int, m_nMvMEventPopfileType );

	string_t m_iszTeleporter;
};

inline CTFObjectiveResource *TFObjectiveResource()
{
	return static_cast< CTFObjectiveResource *>( g_pObjectiveResource );
}

#endif	// TF_OBJECTIVE_RESOURCE_H

