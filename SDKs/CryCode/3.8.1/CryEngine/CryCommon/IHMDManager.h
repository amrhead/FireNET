////////////////////////////////////////////////////////////////////////////
//
//  CryEngine Source File.
//  Copyright (C), Crytek, 1999-2015.
// -------------------------------------------------------------------------
//  File name:   IHMDManager.h
//  Version:     v1.00
//  Description: Head Mounted Display Manager Interface
// -------------------------------------------------------------------------
//  History: Dario Sancho
// -------------------------------------------------------------------------
// Summary
//   Head Mounted Display Engine Manager
// Description
//	 So far we only support Oculus and once instance only of it so it will 
//   take some iterations and new devices to figure out the proper 
//   structure for this interface
////////////////////////////////////////////////////////////////////////////

#ifndef __IHMDMANAGER_H__
#define __IHMDMANAGER_H__

#pragma once

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

struct IHMDDevice;

struct IHMDManager
{
public:

	virtual ~IHMDManager() {}

	enum EHmdSetupAction
	{
		eHSA_CreateCvars = 0,
		eHSA_PreInit,
		eHSA_Init,
	};

	enum EHmdAction
	{
		eHA_DrawInfo = 0,
		eHA_UpdateTracking,
	};

	struct SAsymmetricCameraSetupInfo
	{
		float fov, aspectRatio, asymH, asymV, eyeDist;
	};

	// basic functionality needed to setup and destroy an HMD during system init / system shutdown
	virtual void SetupAction(EHmdSetupAction cmd) = 0;

	// trigger an action on the current HMD
	virtual void Action(EHmdAction action) = 0;

	// returns the active HMD (or a HMDNullDevice if none has been activated)
	virtual IHMDDevice* GetHMDDevice() const = 0;

	// returns true if we have an HMD device recognized and r_stereodevice, r_stereooutput and r_stereomode are properly set for stereo rendering
	virtual bool IsStereoSetupOk() const = 0;

	// populates o_info with the asymmetric camera information returned by the current HMD device
	virtual bool GetAsymmetricCameraSetupInfo(int nEye, SAsymmetricCameraSetupInfo & outInfo) const = 0;
};
#endif // __IHMDMANAGER_H__
