/*************************************************************************
Crytek Source File.
Copyright (C), Crytek Studios, 2001-2005.
-------------------------------------------------------------------------
$Id$
$DateTime$
Description: Implements a third person view for vehicles

-------------------------------------------------------------------------
History:
- Created by Rich Semmens/Stan Fichele

*************************************************************************/
#ifndef __VEHICLEVIEWSTEER_H__
#define __VEHICLEVIEWSTEER_H__

#include "VehicleViewGameBase.h"

enum EVehicleViewSteerFlags
{
	eVCam_rotationClamp      = 1 << 0,      // Clamp any rotation
	eVCam_rotationSpring     = 1 << 1,      // Allow spring relaxation to default position
	eVCam_goingForwards      = 1 << 2,      // Direction of travel
	eVCam_canRotate          = 1 << 3,		// Use input to rotate camera
};


class CVehicleViewSteer
	: public CVehicleViewGameBase
{
	IMPLEMENT_VEHICLEOBJECT;
public:

	CVehicleViewSteer();
	virtual ~CVehicleViewSteer();

	// IVehicleView
	virtual bool Init(IVehicleSeat* pSeat, const CVehicleParams& table);
	virtual void Reset();
	virtual void ResetPosition()
	{
		m_position = m_pVehicle->GetEntity()->GetWorldPos();
	}

	virtual const char* GetName() { return m_name; }
	virtual bool IsThirdPerson() { return true; }
	virtual bool IsPassengerHidden() { return false; }

	virtual void OnAction(const TVehicleActionId actionId, int activationMode, float value);
	virtual void UpdateView(SViewParams &viewParams, EntityId playerId);

	virtual void OnStartUsing(EntityId passengerId);
	virtual void OnStopUsing();

	virtual void Update(const float frameTime);
	virtual void Serialize(TSerialize serialize, EEntityAspects);

	virtual bool ShootToCrosshair() { return false; }
	virtual void OffsetPosition(const Vec3 &delta);
	// ~IVehicleView

	bool Init(IVehicleSeat* pSeat);
	void CalcLookAt(const Matrix34& entityTM);

	void GetMemoryUsage(ICrySizer * s) const { s->Add(*this); }

protected:

	Vec3 m_rotation;
	Vec3 m_rotatingAction;
	Vec3 m_maxRotation;
	Vec3 m_maxRotation2;
	Vec3 m_stickSensitivity;
	Vec3 m_stickSensitivity2;

	static const char* m_name;

	Vec3 m_position;
	Vec3 m_localSpaceCameraOffset;
	Vec3 m_lastOffsetBeforeElev;
	Vec3 m_lastOffset;
	Vec3 m_lookAt0;
	Vec3 m_lookAt;
	Vec3 m_lastVehVel;

	int m_flags;             /* EVehicleViewSteerFlags */
	int m_forwardFlags;			
	int m_backwardsFlags;

	float m_angReturnSpeed1;
	float m_angReturnSpeed2;
	float m_angReturnSpeed;
	float m_curYaw;

	float m_angSpeedCorrection0;
	float m_angSpeedCorrection;
	float m_radius;
	float m_radiusVel;              // Velocity change of radius
	float m_radiusRelaxRate;        // Relax spring to default radius
	float m_radiusDampRate;         // Damp the velocity change of the radius
	float m_radiusVelInfluence;     // How much the vehicle velocity influences the camera distance/radius
	float m_radiusMin;
	float m_radiusMax;
	float m_inheritedElev;
	float m_pivotOffset;
};

#endif
