////////////////////////////////////////////////////////////////////////////
//
//  CryEngine Source File.
//  Copyright (C), Crytek, 1999-2015.
// -------------------------------------------------------------------------
//  File name:   IHMDDevice.h
// -------------------------------------------------------------------------
//  Description: Common interface for head mounted devices.
//               It provides a (manual) ref-counted interface
//               This could be probably better off being a standard smart pointer
//
//               Vendor specific interfaces are also defined in this file
// -------------------------------------------------------------------------
//  History: CarstenW - Original Oculus prototype
//           Dario & Valerio - Clean up and refactor for Oculus SDK 0.6.0.0
//
////////////////////////////////////////////////////////////////////////////

#ifndef _I_HMD_DEVICE_H_
#define _I_HMD_DEVICE_H_

#pragma once

enum HMDClass
{
	HMDCls_Null,
	HMDCls_Oculus
};
	
enum HMDType
{
	HMD_Unknown,
	// OCULUS
	HMD_DK1,
	HMD_DKHD,
	HMD_DK2,
	HMD_CB
};

enum HMDStatus
{
	HS_OrientationTracked = 0x0001,
	HS_PositionTracked = 0x0002,
	HS_CameraPoseTracked = 0x0004,
	HS_PositionConnected = 0x0008,
	HS_HmdConnected = 0x0010,
	//HS_ValidReferencePosition = 0x0020,

	HS_IsUsable = HS_HmdConnected | HS_OrientationTracked
};

struct HMDDeviceInfo
{
	HMDDeviceInfo()
		: type(HMD_Unknown)
		, productName(0)
		, manufacturer(0)
		, screenWidth(0)
		, screenHeight(0)
		, fovH(0)
		, fovV(0)
	{
	}

	HMDType type;

	const char* productName;
	const char* manufacturer;

	unsigned int screenWidth;
	unsigned int screenHeight;

	float fovH;
	float fovV;
};

struct HMDPoseState
{
	HMDPoseState()
		: orientation(Quat::CreateIdentity())
		, position(ZERO)
		//, referencePosition(ZERO)
		, angularVelocity(ZERO)
		, linearVelocity(ZERO)
		, angularAcceleration(ZERO)
		, linearAcceleration(ZERO)
	{
	}

	Quat orientation;
	Vec3 position;
	//Vec3 referencePosition;

	Vec3 angularVelocity;
	Vec3 linearVelocity;
	Vec3 angularAcceleration;
	Vec3 linearAcceleration;
};

struct HMDTrackingState
{
	HMDTrackingState()
		: pose()
		, statusFlags(0)
	{
	}

	bool CheckStatusFlags(unsigned int checked, unsigned int wanted) const { return (statusFlags & checked) == wanted; }
	bool CheckStatusFlags(unsigned int checkedAndWanted) const { return CheckStatusFlags(checkedAndWanted, checkedAndWanted); }

	HMDPoseState pose;

	unsigned int statusFlags;
};

struct IHMDDevice
{
	enum EInternalUpdate
	{
		e_DebugInfo = 0,
	};

	virtual void AddRef() = 0;
	virtual void Release() = 0;

	virtual HMDClass GetClass() const = 0;
	virtual void GetDeviceInfo(HMDDeviceInfo& info) const = 0;

	virtual void GetCameraSetupInfo(float& fov, float& aspectRatioFactor) const = 0;
	virtual void GetAsymmetricCameraSetupInfo(int nEye, float& fov, float& aspectRatio, float& asymH, float& asymV, float& eyeDist) const = 0;

	virtual void UpdateInternal(EInternalUpdate) = 0;
	virtual void RecenterPose() = 0;
	virtual void UpdateTrackingState() = 0;
	virtual const HMDTrackingState& GetTrackingState() const = 0;

protected:
	virtual ~IHMDDevice() {}
};

//////////////////////////////////////////////////////////////////////////
// vendor specifics

#ifndef EXCLUDE_OCULUS_SDK

namespace CryOVR
{

struct TextureDesc
{
	uint32 width;
	uint32 height;
	uint32 format;
};

struct TextureSet
{
	void* deviceSwapTextureSet;
	uint32 numTextures;
	void** textures;
};

struct Texture
{
	void* deviceTexture;
	void* texture;
};

struct EyeTarget
{
	void* deviceSwapTextureSet;
	Vec2i viewportPosition;
	Vec2i viewportSize;
};

struct IOculusDevice : public IHMDDevice
{
public:
	virtual bool CreateSwapTextureSetD3D11(void* d3d11Device, TextureDesc desc, TextureSet* set) = 0;
	virtual bool CreateMirrorTextureD3D11(void* d3d11Device, TextureDesc desc, Texture* texture) = 0;
	virtual void DestroySwapTextureSet(TextureSet* set) = 0;
	virtual void DestroyMirrorTexture(Texture* texture) = 0;
	virtual void PrepareTexture(TextureSet* set, uint32 frameIndex) = 0;
	virtual void SubmitFrame(EyeTarget leftEye, EyeTarget rightEye) = 0;
protected:
	virtual ~IOculusDevice() {}
};

}

#endif //ifndef EXCLUDE_OCULUS_SDK

#endif //#ifndef _I_HMD_DEVICE_H_
