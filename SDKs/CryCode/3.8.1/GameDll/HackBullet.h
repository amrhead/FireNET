#ifndef _HACK_BULLET_H_
#define _HACK_BULLET_H_

#include "Projectile.h"



class CHackBullet : public CProjectile
{
private:
	virtual void HandleEvent(const SGameObjectEvent& event);
};



#endif
