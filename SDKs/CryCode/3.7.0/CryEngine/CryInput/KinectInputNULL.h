// CryEngine Header File.
// Copyright (C), Crytek, 1999-2014.


#pragma once

class CKinectInputNULL: public IKinectInput
{
public:
	CKinectInputNULL(){}
	~CKinectInputNULL(){}

	virtual bool Init() OVERRIDE {return false;}
	virtual void Update() OVERRIDE {}
	virtual bool IsEnabled() { return false; }
	virtual const char* GetUserStatusMessage() OVERRIDE {return ""; }

	virtual void RegisterInputListener(IKinectInputListener* pInputListener, const char* name) OVERRIDE {}
	virtual void UnregisterInputListener(IKinectInputListener* pInputListener) OVERRIDE {}
	virtual bool RegisterArcRail(int gripId, int railId, const Vec2& vScreenPos, const Vec3& vDir, float fLenght, float fDeadzoneLength, float fToleranceConeAngle) OVERRIDE {return false;}
	virtual void UnregisterArcRail(int gripId) OVERRIDE {}
	virtual bool RegisterHoverTimeRail(int gripId, int railId, const Vec2& vScreenPos, float fHoverTime, float fTimeTillCommit, SKinGripShape* pGripShape = NULL) OVERRIDE {return false;}
	virtual void UnregisterHoverTimeRail(int gripId) OVERRIDE {}
	virtual void UnregisterAllRails() OVERRIDE {}
	
	virtual bool GetBodySpaceHandles(SKinBodyShapeHandles& bodyShapeHandles) OVERRIDE { return false; }
	virtual bool GetSkeletonRawData(uint32 iUser, SKinSkeletonRawData &skeletonRawData) const  OVERRIDE { return false; };
	virtual bool GetSkeletonDefaultData(uint32 iUser, SKinSkeletonDefaultData &skeletonDefaultData) const  OVERRIDE { return false; };
	virtual void DebugDraw() OVERRIDE {}
	
	//Skeleton
	virtual void EnableSeatedSkeletonTracking(bool bValue) OVERRIDE {}
	virtual uint32 GetClosestTrackedSkeleton() const OVERRIDE {return KIN_SKELETON_INVALID_TRACKING_ID;}

	//	Wave
	virtual void EnableWaveGestureTracking(bool bEnable) OVERRIDE {};
	virtual float GetWaveGestureProgress(DWORD* pTrackingId) OVERRIDE { return 0.f; }

	// Identity
	virtual bool IdentityDetectedIntentToPlay(DWORD dwTrackingId) OVERRIDE {return false;}
	virtual bool IdentityIdentify(DWORD dwTrackingId, KinIdentifyCallback callbackFunc, void* pData) OVERRIDE {return false;}

	// Speech
	virtual bool SpeechEnable() OVERRIDE {return false;}
	virtual void SpeechDisable() OVERRIDE {}
	virtual void SetEnableGrammar(const string & grammarName, bool enable) OVERRIDE {}
	virtual bool KinSpeechSetEventInterest(unsigned long ulEvents) OVERRIDE { return false; }
	virtual bool KinSpeechLoadDefaultGrammar() OVERRIDE { return false;}
	virtual bool KinSpeechStartRecognition() OVERRIDE {return false;}
	virtual void KinSpeechStopRecognition() OVERRIDE {};
};
