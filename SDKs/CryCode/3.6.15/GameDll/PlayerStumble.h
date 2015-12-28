/*************************************************************************
Crytek Source File.
Copyright (C), Crytek Studios, 2001-2010.

-------------------------------------------------------------------------
History:
- 21:07:2010  Created by Jens Sch�bel

*************************************************************************/

#ifndef PLAYERSTUMBLE_H
#define PLAYERSTUMBLE_H

#pragma once

namespace PlayerActor
{
  namespace Stumble
  {
    struct StumbleParameters
    {
      float minDamping;
      float maxDamping;
      float periodicTime;
      float strengthX;
      float strengthY;
      float randomness;
    };

    class CPlayerStumble
    {
    public:
      CPlayerStumble();
      void Disable();
      bool Update(float deltaTime, const pe_status_dynamics &dynamics);
      void Start(StumbleParameters* stumbleParameters);

      inline const Vec3& GetCurrentActionImpulse() const { return m_finalActionImpulse; }
    protected:
    private:
    public:
    protected:
    private:
      StumbleParameters m_stumbleParameter;
      Vec3	m_finalActionImpulse;
      float m_currentPeriodicScaleX;
      float m_currentPeriodicScaleY;
      float m_currentPastTime;
    };
  } // namespace Stumble
} // namespace Player















#endif

