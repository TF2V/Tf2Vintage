#ifndef TF_ZOMBIE_BEHAVIOR_H
#define TF_ZOMBIE_BEHAVIOR_H
#ifdef _WIN32
#pragma once
#endif


#include "NextBotBehavior.h"

class CZombieBehavior : public Action<CZombie>
{
	DECLARE_CLASS( CZombieBehavior, Action<CZombie> );
public:

	virtual char const *GetName( void ) const;

	virtual ActionResult<CZombie> OnStart( CZombie *me, Action <CZombie> *priorAction ) override;
	virtual ActionResult<CZombie> Update( CZombie *me, float dt ) override;

	virtual EventDesiredResult<CZombie> OnKilled( CZombie *me, CTakeDamageInfo const& info ) override;

	virtual Action<CZombie> *InitialContainedAction( CZombie *me ) override;

private:
	bool ShouldLaugh( CZombie *actor );
};

#endif
