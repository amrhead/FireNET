/*************************************************************************
  Crytek Source File.
  Copyright (C), Crytek Studios, 2001-2004.
 -------------------------------------------------------------------------
  $Id$
  $DateTime$
  Description: View System interfaces.
  
 -------------------------------------------------------------------------
  History:
  - 24:9:2004 : Created by Filippo De Luca

*************************************************************************/
#ifndef __VIEW_H__
#define __VIEW_H__

#if _MSC_VER > 1000
# pragma once
#endif

#include "IViewSystem.h"

class CGameObject;

class CView : public IView, public IEntityEventListener
{
public:

	CView(ISystem *pSystem);
	virtual ~CView();

	//shaking
	struct SShake
	{
		bool updating;
		bool flip;
		bool doFlip;
		bool groundOnly;
		bool permanent;
		bool interrupted; // when forcefully stopped
    bool isSmooth;
		
		int ID;

		float nextShake;
		float timeDone;
		float sustainDuration;
		float fadeInDuration;
		float fadeOutDuration;

		float frequency;
		float ratio;

		float randomness;
		
    Quat startShake;
    Quat startShakeSpeed;
    Vec3 startShakeVector;
    Vec3 startShakeVectorSpeed;

    Quat goalShake;
    Quat goalShakeSpeed;
    Vec3 goalShakeVector;
    Vec3 goalShakeVectorSpeed;

		Ang3 amount;
		Vec3 amountVector;

		Quat shakeQuat;
		Vec3 shakeVector;

		SShake(int shakeID)
		{
			memset(this,0,sizeof(SShake));

      startShake.SetIdentity();
      startShakeSpeed.SetIdentity();
			goalShake.SetIdentity();
			shakeQuat.SetIdentity();

			randomness = 0.5f;

			ID = shakeID;
		}

		void GetMemoryUsage(ICrySizer *pSizer) const {/*nothing*/}
	};


	// IView
	virtual void Release();
	virtual void Update(float frameTime, bool isActive);
	virtual void ProcessShaking(float frameTime);
	virtual void ProcessShake(SShake *pShake,float frameTime);
	virtual void ResetShaking();
	virtual void ResetBlending() { m_viewParams.ResetBlending(); }
	//FIXME: keep CGameObject *  or use IGameObject *?
	virtual void LinkTo(IGameObject *follow);
	virtual void LinkTo(IEntity* follow);
	virtual EntityId GetLinkedId() {return m_linkedTo;};
	virtual void SetCurrentParams( SViewParams &params ) { m_viewParams = params; };
	virtual const SViewParams * GetCurrentParams() {return &m_viewParams;}
	virtual void SetViewShake(Ang3 shakeAngle,Vec3 shakeShift,float duration,float frequency,float randomness,int shakeID, bool bFlipVec = true, bool bUpdateOnly=false, bool bGroundOnly=false);
	virtual void SetViewShakeEx(  const SShakeParams& params );
	virtual void StopShake( int shakeID );
	virtual void SetFrameAdditiveCameraAngles(const Ang3& addFrameAngles);
	virtual void SetScale(const float scale);
	virtual void SetZoomedScale(const float scale);
	virtual void SetActive(bool const bActive);
	// ~IView

	// IEntityEventListener
	virtual void OnEntityEvent(IEntity* pEntity, SEntityEvent& event);
	// ~IEntityEventListener

	void			Serialize(TSerialize ser);
	void			PostSerialize();
	CCamera&	GetCamera() { return m_camera; };
	void			UpdateAudioListener(Matrix34 const& rMatrix);

	void GetMemoryUsage(ICrySizer * s) const;
	
protected:

	CGameObject * GetLinkedGameObject();
	IEntity* GetLinkedEntity();
  void ProcessShakeNormal( SShake *pShake, float frameTime );
  void ProcessShakeNormal_FinalDamping( SShake *pShake, float frameTime);
	void ProcessShakeNormal_CalcRatio( SShake *pShake, float frameTime, float endSustain );
	void ProcessShakeNormal_DoShaking( SShake *pShake, float frameTime);

  void ProcessShakeSmooth( SShake *pShake, float frameTime );
  void ProcessShakeSmooth_DoShaking( SShake *pShake, float frameTime);

	void ApplyFrameAdditiveAngles(Quat& cameraOrientation);

	const float GetScale();

private:

  void	GetRandomQuat( Quat &quat, SShake *pShake );
  void	GetRandomVector( Vec3 &vec3, SShake *pShake );
  void	CubeInterpolateQuat(float t, SShake *pShake );
  void	CubeInterpolateVector(float t, SShake *pShake );
	void	CreateAudioListener();

protected:

	EntityId m_linkedTo;
	
	SViewParams m_viewParams;
	CCamera m_camera;

	ISystem *m_pSystem;
	
	std::vector<SShake> m_shakes;

	IEntity*	m_pAudioListener;
	Ang3			m_frameAdditiveAngles; // Used mainly for cinematics, where the game can slightly override camera orientation

	float m_scale;
	float m_zoomedScale;
};

#endif //__VIEW_H__
