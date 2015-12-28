/*************************************************************************
  Crytek Source File.
  Copyright (C), Crytek Studios, 2001-2004.
 -------------------------------------------------------------------------
  $Id$
  $DateTime$
  
 -------------------------------------------------------------------------
  History:
  - 20:9:2004     : Created by Filippo De Luca
  - September 2010: Jens Sch�bel created a smooth extended camera shake

*************************************************************************/
#include "StdAfx.h"

#include <Cry_Camera.h>
#include "View.h"
#include "GameObjects/GameObject.h"
#include "IGameSessionHandler.h"

//FIXME:not very pretty
static ICVar *pCamShakeMult = 0;

//------------------------------------------------------------------------
CView::CView(ISystem *pSystem)
:	m_pSystem(pSystem)
,	m_linkedTo(0)
,	m_frameAdditiveAngles(0.0f, 0.0f, 0.0f)
,	m_scale(1.0f)
,	m_zoomedScale(1.0f)
, m_pAudioListener(NULL)
{
	if (!pCamShakeMult)
	{
		pCamShakeMult = gEnv->pConsole->GetCVar("c_shakeMult");
	}

	CreateAudioListener();
}

//------------------------------------------------------------------------
CView::~CView()
{
	if (m_pAudioListener != NULL)
	{
		gEnv->pEntitySystem->RemoveEntityEventListener(m_pAudioListener->GetId(), ENTITY_EVENT_DONE, this);
		m_pAudioListener = NULL;
	}
}

//-----------------------------------------------------------------------
void CView::Release()
{
	delete this;
}

//------------------------------------------------------------------------
void CView::Update(float frameTime,bool isActive)
{
	//FIXME:some cameras may need to be updated always
	if (!isActive)
		return;

	CGameObject * pLinkedTo = GetLinkedGameObject();
	if (pLinkedTo && !pLinkedTo->CanUpdateView())
	  pLinkedTo = NULL;
	IEntity* pEntity = pLinkedTo ? 0 : GetLinkedEntity();

	if (pLinkedTo || pEntity)
	{
		m_viewParams.SaveLast();

		CCamera *pSysCam = &m_pSystem->GetViewCamera();

		//process screen shaking
		ProcessShaking(frameTime);
		
		//FIXME:to let the updateView implementation use the correct shakeVector
		m_viewParams.currentShakeShift = m_viewParams.rotation * m_viewParams.currentShakeShift;
		
    m_viewParams.frameTime=frameTime;
		//update view position/rotation
		if (pLinkedTo)
		{
			pLinkedTo->UpdateView(m_viewParams);
			if (!m_viewParams.position.IsValid())
			{
				m_viewParams.position = m_viewParams.GetPositionLast();
				CRY_ASSERT_MESSAGE(0, "Camera position is invalid, reverting to old position");
			}
			if (!m_viewParams.rotation.IsValid())
			{
				m_viewParams.rotation = m_viewParams.GetRotationLast();
				CRY_ASSERT_MESSAGE(0, "Camera rotation is invalid, reverting to old rotation");
			}
		}
		else
		{
			Matrix34 mat = pEntity->GetWorldTM();
			mat.OrthonormalizeFast();
			m_viewParams.position = mat.GetTranslation();
			m_viewParams.rotation = Quat(mat);
		}

		ApplyFrameAdditiveAngles(m_viewParams.rotation);

    if (pLinkedTo)
    {
      pLinkedTo->PostUpdateView(m_viewParams);
    }

		const float fNearZ  = gEnv->pGame->GetIGameFramework()->GetIViewSystem()->GetDefaultZNear();
		
		//see if the view have to use a custom near clipping plane
		const float nearPlane = (m_viewParams.nearplane >= 0.01f) ? (m_viewParams.nearplane) : fNearZ;
		const float farPlane = gEnv->p3DEngine->GetMaxViewDistance();
		const float fov = (m_viewParams.fov < 0.001) ? DEFAULT_FOV : m_viewParams.fov;
		
		m_camera.SetFrustum(pSysCam->GetViewSurfaceX(),pSysCam->GetViewSurfaceZ(),fov,nearPlane,farPlane, pSysCam->GetPixelAspectRatio());

    //TODO: (14, 06, 2010, "the player view should always get updated, this due to the hud being visable, without shocking, in cutscenes - todo is to see if we can optimise this code");
    IActor * pActor = gEnv->pGame->GetIGameFramework()->GetClientActor();
    if (pActor)
    {
      CGameObject * linkToObj = (CGameObject*)pActor->GetEntity()->GetProxy( ENTITY_PROXY_USER );
      if (linkToObj && linkToObj != pLinkedTo)
      {
        linkToObj->PostUpdateView(m_viewParams);
      }
    }

		//apply shake & set the view matrix
		m_viewParams.rotation *= m_viewParams.currentShakeQuat;
		m_viewParams.rotation.NormalizeSafe();
		m_viewParams.position += m_viewParams.currentShakeShift;

		// Camera space Rendering calculations on Entity
		if(pLinkedTo)
		{
			IEntity* pLinkedToEntity = pLinkedTo->GetEntity();
			if(pLinkedToEntity)
			{
				const int slotIndex = 0;
				uint32 entityFlags = pLinkedToEntity->GetSlotFlags(slotIndex);
				if(entityFlags & ENTITY_SLOT_RENDER_NEAREST)
				{
					// Get camera pos relative to entity
					const Vec3 cameraLocalPos = m_viewParams.position;

					// Set entity's camera space position
					const Vec3 cameraSpacePos(-cameraLocalPos * m_viewParams.rotation);
					pLinkedToEntity->SetSlotCameraSpacePos(slotIndex,cameraSpacePos);

					// Add world pos onto camera local pos
					m_viewParams.position = pLinkedToEntity->GetWorldPos() + cameraLocalPos;
				}
			}
		}

		// Blending between cameras needs to happen after Camera space rendering calculations have been applied
		// so that the m_viewParams.position is in World Space again
		m_viewParams.UpdateBlending(frameTime);

		Matrix34 viewMtx(m_viewParams.rotation);
		viewMtx.SetTranslation(m_viewParams.position);
		m_camera.SetMatrix(viewMtx);
	}
	else
	{
		m_linkedTo = 0;

		CCryAction *pCryAction = CCryAction::GetCryAction();
		if (!pCryAction->IsGameSessionMigrating())		// If we're host migrating, leave the camera where it was
		{
			// Check if we're leaving a game mid way through - if we are then stop the camera from being reset for a frame or 2 before the unload happens
			if (!pCryAction->GetIGameSessionHandler()->IsMidGameLeaving())
			{
				m_camera.SetPosition(Vec3(1,1,1));
			}
		}
	}
}

//-----------------------------------------------------------------------
void CView::ApplyFrameAdditiveAngles(Quat& cameraOrientation)
{
	if ((m_frameAdditiveAngles.x != 0.f) || (m_frameAdditiveAngles.y != 0.f) || (m_frameAdditiveAngles.z != 0.f))
	{
		Ang3 cameraAngles(cameraOrientation);
		cameraAngles += m_frameAdditiveAngles;

		cameraOrientation.SetRotationXYZ(cameraAngles);

		m_frameAdditiveAngles.Set(0.0f, 0.0f, 0.0f);
	}
}

//------------------------------------------------------------------------
void CView::SetViewShake(Ang3 shakeAngle,Vec3 shakeShift,float duration,float frequency,float randomness,int shakeID, bool bFlipVec, bool bUpdateOnly, bool bGroundOnly)
{
	SShakeParams params;
	params.shakeAngle = shakeAngle;
	params.shakeShift = shakeShift;
	params.frequency = frequency;
	params.randomness = randomness;
	params.shakeID = shakeID;
	params.bFlipVec = bFlipVec;
	params.bUpdateOnly = bUpdateOnly;
	params.bGroundOnly = bGroundOnly;
	params.fadeInDuration = 0;					//
	params.fadeOutDuration = duration;  // originally it was faded out from start. that is why the values are set this way here, to preserve compatibility.
	params.sustainDuration = 0;		  		//
	
	SetViewShakeEx( params );
}


#define RANDOM() ((((float)cry_rand()/(float)CRY_RAND_MAX)*2.0f)-1.0f)

//------------------------------------------------------------------------
void CView::SetViewShakeEx( const SShakeParams& params )
{
	float shakeMult = GetScale();
	if (shakeMult<0.001f)
		return;

	int shakes(m_shakes.size());
	SShake *pSetShake(NULL);

	for (int i=0;i<shakes;++i)
	{
		SShake *pShake = &m_shakes[i];
		if (pShake->ID == params.shakeID)
		{
			pSetShake = pShake;
			break;
		}
	}

	if (!pSetShake)
	{
		m_shakes.push_back(SShake(params.shakeID));
		pSetShake = &m_shakes.back();
	}

	if (pSetShake)
	{
		// this can be set dynamically
		pSetShake->frequency = max(0.00001f, params.frequency);

		// the following are set on a 'new' shake as well
		if (params.bUpdateOnly == false)
		{
			pSetShake->amount = params.shakeAngle * shakeMult;
			pSetShake->amountVector = params.shakeShift * shakeMult;
			pSetShake->randomness = params.randomness;
			pSetShake->doFlip = params.bFlipVec;
			pSetShake->groundOnly = params.bGroundOnly;
      pSetShake->isSmooth = params.isSmooth;
			pSetShake->permanent = params.bPermanent;
      pSetShake->fadeInDuration = params.fadeInDuration;
      pSetShake->sustainDuration = params.sustainDuration;
			pSetShake->fadeOutDuration = params.fadeOutDuration;
			pSetShake->timeDone = 0;
			pSetShake->updating = true;
			pSetShake->interrupted = false;
      pSetShake->goalShake = Quat(ZERO);
      pSetShake->goalShakeSpeed = Quat(ZERO);
      pSetShake->goalShakeVector = Vec3(ZERO);
      pSetShake->goalShakeVectorSpeed = Vec3(ZERO);
			pSetShake->nextShake = 0.0f;
		}
	}
}

//------------------------------------------------------------------------
void CView::SetScale(const float scale)
{
	CRY_ASSERT_MESSAGE(scale == 1.0f || m_scale == 1.0f, "Attempting to CView::SetScale but has already been set!");
	m_scale = scale;
}

void CView::SetZoomedScale(const float scale)
{
	CRY_ASSERT_MESSAGE(scale == 1.0f || m_zoomedScale == 1.0f, "Attempting to CView::SetZoomedScale but has already been set!");
	m_zoomedScale = scale;
}

//------------------------------------------------------------------------
const float CView::GetScale()
{
	float shakeMult(pCamShakeMult->GetFVal());
	return m_scale * shakeMult * m_zoomedScale;
}

//------------------------------------------------------------------------
void CView::ProcessShaking(float frameTime)
{
	m_viewParams.currentShakeQuat.SetIdentity();
	m_viewParams.currentShakeShift.zero();
	m_viewParams.shakingRatio = 0;
	m_viewParams.groundOnly = false;

	int shakes(m_shakes.size());
	for (int i=0;i<shakes;++i)
		ProcessShake(&m_shakes[i],frameTime);
}

//------------------------------------------------------------------------
void CView::ProcessShake(SShake *pShake,float frameTime)
{
	if (!pShake->updating)
		return;

  pShake->timeDone += frameTime;

  if ( pShake->isSmooth ) {
    ProcessShakeSmooth(pShake, frameTime);
  } else {
    ProcessShakeNormal(pShake, frameTime);
  }
}

//------------------------------------------------------------------------
void CView::ProcessShakeNormal( SShake *pShake, float frameTime )
{
  float endSustain = pShake->fadeInDuration + pShake->sustainDuration;
  float totalDuration = endSustain + pShake->fadeOutDuration;

  bool finalDamping = (!pShake->permanent && pShake->timeDone>totalDuration) || (pShake->interrupted && pShake->ratio<0.05f);

  if (finalDamping)
    ProcessShakeNormal_FinalDamping( pShake, frameTime );
  else
  {
    ProcessShakeNormal_CalcRatio( pShake, frameTime, endSustain );
    ProcessShakeNormal_DoShaking( pShake, frameTime );

    //for the global shaking ratio keep the biggest
    if (pShake->groundOnly)
      m_viewParams.groundOnly = true;
    m_viewParams.shakingRatio = max(m_viewParams.shakingRatio,pShake->ratio);
    m_viewParams.currentShakeQuat *= pShake->shakeQuat;
    m_viewParams.currentShakeShift += pShake->shakeVector;
  }
}

//////////////////////////////////////////////////////////////////////////
void CView::ProcessShakeSmooth( SShake *pShake, float frameTime )
{
  assert(pShake->timeDone >= 0);

  float endTimeFadeIn  = pShake->fadeInDuration;
  float endTimeSustain = pShake->sustainDuration + endTimeFadeIn;
  float totalTime = endTimeSustain + pShake->fadeOutDuration;

  if ( pShake->interrupted && endTimeFadeIn <= pShake->timeDone && pShake->timeDone < endTimeSustain)
  {
    pShake->timeDone = endTimeSustain;
  }

  float damping = 1.f;
  if ( pShake->timeDone < endTimeFadeIn )
  {
    damping = pShake->timeDone / endTimeFadeIn;
  }
  else if ( endTimeSustain < pShake->timeDone && pShake->timeDone < totalTime )
  {
    damping = (totalTime - pShake->timeDone) / (totalTime - endTimeSustain);
  }
  else if ( totalTime <= pShake->timeDone ) {
    pShake->shakeQuat.SetIdentity();
    pShake->shakeVector.zero();
    pShake->ratio = 0.0f;
    pShake->nextShake = 0.0f;
    pShake->flip = false;
    pShake->updating = false;
    return;
  }

  ProcessShakeSmooth_DoShaking(pShake, frameTime);

  if (pShake->groundOnly)
    m_viewParams.groundOnly = true;
  pShake->ratio = (3.f - 2.f * damping) * damping * damping; // smooth ration change
  m_viewParams.shakingRatio = max(m_viewParams.shakingRatio,pShake->ratio);
  m_viewParams.currentShakeQuat  *= Quat::CreateSlerp(IDENTITY,pShake->shakeQuat,pShake->ratio);
  m_viewParams.currentShakeShift += Vec3::CreateLerp(ZERO,pShake->shakeVector,pShake->ratio);
}

//////////////////////////////////////////////////////////////////////////
void CView::GetRandomQuat( Quat &quat, SShake *pShake )
{
  quat.SetRotationXYZ(pShake->amount);
  float randomAmt(pShake->randomness);
  float len(fabs(pShake->amount.x) + fabs(pShake->amount.y) + fabs(pShake->amount.z));
  len /= 3.f;
  quat *= Quat::CreateRotationXYZ(Ang3(RANDOM()*len*randomAmt,RANDOM()*len*randomAmt,RANDOM()*len*randomAmt));
}

//////////////////////////////////////////////////////////////////////////
void CView::GetRandomVector( Vec3 &vec, SShake *pShake )
{
  vec = pShake->amountVector;
  float randomAmt(pShake->randomness);
  float len = fabs(pShake->amountVector.x) + fabs(pShake->amountVector.y) + fabs(pShake->amountVector.z);
  len /= 3.f;
  vec += Vec3(RANDOM()*len*randomAmt,RANDOM()*len*randomAmt,RANDOM()*len*randomAmt);
}

//////////////////////////////////////////////////////////////////////////
void CView::CubeInterpolateQuat(float t, SShake *pShake )
{
  Quat p0 = pShake->startShake;
  Quat p1 = pShake->goalShake;
  Quat v0 = pShake->startShakeSpeed * 0.5f;
  Quat v1 = pShake->goalShakeSpeed * 0.5f;

  pShake->shakeQuat = (((p0 *  2.f + p1 * -2.f + v0        + v1) *t
                    +   (p0 * -3.f + p1 *  3.f + v0 * -2.f - v1))*t
                    +   (                        v0            ))*t
                    +    p0;

  pShake->shakeQuat.Normalize();
}

//////////////////////////////////////////////////////////////////////////
void CView::CubeInterpolateVector(float t, SShake *pShake )
{
  Vec3 p0 = pShake->startShakeVector;
  Vec3 p1 = pShake->goalShakeVector;
  Vec3 v0 = pShake->startShakeVectorSpeed * 0.8f;
  Vec3 v1 = pShake->goalShakeVectorSpeed * 0.8f;

  pShake->shakeVector = (((p0 *  2.f + p1 * -2.f + v0        + v1) *t
                      +   (p0 * -3.f + p1 *  3.f + v0 * -2.f - v1))*t
                      +   (                        v0            ))*t
                      +    p0;
  
}

//////////////////////////////////////////////////////////////////////////
void CView::ProcessShakeSmooth_DoShaking( SShake *pShake, float frameTime)
{
  if ( pShake->nextShake <= 0.0f )
  {
    pShake->nextShake = pShake->frequency;

    pShake->startShake            = pShake->goalShake;
    pShake->startShakeSpeed       = pShake->goalShakeSpeed;
    pShake->startShakeVector      = pShake->goalShakeVector;
    pShake->startShakeVectorSpeed = pShake->goalShakeVectorSpeed;

    GetRandomQuat(pShake->goalShake, pShake);
    GetRandomQuat(pShake->goalShakeSpeed, pShake);
    GetRandomVector(pShake->goalShakeVector, pShake);
    GetRandomVector(pShake->goalShakeVectorSpeed, pShake);

		if (pShake->flip)
		{
			pShake->goalShake.Invert();
			pShake->goalShakeSpeed.Invert();
			pShake->goalShakeVector = -pShake->goalShakeVector;
			pShake->goalShakeVectorSpeed = -pShake->goalShakeVectorSpeed;
		}

		if (pShake->doFlip)
			pShake->flip = !pShake->flip;

  }

  pShake->nextShake -= frameTime;

  float t = (pShake->frequency - pShake->nextShake) / pShake->frequency;
  CubeInterpolateQuat(t, pShake);
  CubeInterpolateVector(t, pShake);
}

//////////////////////////////////////////////////////////////////////////
void CView::ProcessShakeNormal_FinalDamping( SShake *pShake, float frameTime)
{
	pShake->shakeQuat = Quat::CreateSlerp(pShake->shakeQuat,IDENTITY,frameTime * 5.0f);
	m_viewParams.currentShakeQuat *= pShake->shakeQuat;

	pShake->shakeVector = Vec3::CreateLerp(pShake->shakeVector,ZERO,frameTime * 5.0f);
	m_viewParams.currentShakeShift += pShake->shakeVector;

	float svlen2(pShake->shakeVector.len2());
	bool quatIsIdentity(Quat::IsEquivalent(IDENTITY,pShake->shakeQuat,0.0001f));

	if (quatIsIdentity && svlen2<0.01f)
	{
		pShake->shakeQuat.SetIdentity();
		pShake->shakeVector.zero();

		pShake->ratio = 0.0f;
		pShake->nextShake = 0.0f;
		pShake->flip = false;

		pShake->updating = false;
	}
}


// "ratio" is the amplitude of the shaking
void CView::ProcessShakeNormal_CalcRatio( SShake *pShake, float frameTime, float endSustain )
{
	const float FADEOUT_TIME_WHEN_INTERRUPTED = 0.5f;
	
	if (pShake->interrupted)
		pShake->ratio = max(0.f, pShake->ratio - ( frameTime / FADEOUT_TIME_WHEN_INTERRUPTED ));  // fadeout after interrupted
	else
	if (pShake->timeDone>=endSustain && pShake->fadeOutDuration>0)  
	{
		float timeFading = pShake->timeDone - endSustain;
		pShake->ratio = clamp_tpl( 1.f - timeFading / pShake->fadeOutDuration, 0.f, 1.f ); // fadeOut
	}
	else
	if (pShake->timeDone>=pShake->fadeInDuration) 
	{
		pShake->ratio = 1.f; // sustain
	}
	else  
	{
		pShake->ratio = min( 1.f, pShake->timeDone / pShake->fadeInDuration ); // fadeIn
	}

	if (pShake->permanent && pShake->timeDone>=pShake->fadeInDuration && !pShake->interrupted)
	pShake->ratio = 1.f;  // permanent standing
}

//////////////////////////////////////////////////////////////////////////
void CView::ProcessShakeNormal_DoShaking( SShake *pShake, float frameTime)
{
	float t;
	if (pShake->nextShake <= 0.0f)
	{
		//angular
		pShake->goalShake.SetRotationXYZ(pShake->amount);
		if (pShake->flip)
			pShake->goalShake.Invert();

		//translational
		pShake->goalShakeVector = pShake->amountVector;
		if (pShake->flip)
			pShake->goalShakeVector = -pShake->goalShakeVector;

		if (pShake->doFlip)
			pShake->flip = !pShake->flip;

		//randomize it a little
		float randomAmt(pShake->randomness);
		float len(fabs(pShake->amount.x) + fabs(pShake->amount.y) + fabs(pShake->amount.z));
		len /= 3.0f;
		pShake->goalShake *= Quat::CreateRotationXYZ(Ang3(RANDOM()*len*randomAmt,RANDOM()*len*randomAmt,RANDOM()*len*randomAmt));

		//translational randomization
		len = fabs(pShake->amountVector.x) + fabs(pShake->amountVector.y) + fabs(pShake->amountVector.z);
		len /= 3.0f;
		pShake->goalShakeVector += Vec3(RANDOM()*len*randomAmt,RANDOM()*len*randomAmt,RANDOM()*len*randomAmt);

		//damp & bounce it in a non linear fashion
		t = 1.0f-(pShake->ratio*pShake->ratio);
		pShake->goalShake = Quat::CreateSlerp(pShake->goalShake,IDENTITY,t);
		pShake->goalShakeVector = Vec3::CreateLerp(pShake->goalShakeVector,ZERO,t);

		pShake->nextShake = pShake->frequency;
	}

	pShake->nextShake = max(0.0f,pShake->nextShake - frameTime);

	t = min(1.0f,frameTime * (1.0f/pShake->frequency));
	pShake->shakeQuat = Quat::CreateSlerp(pShake->shakeQuat,pShake->goalShake,t);
	pShake->shakeQuat.Normalize();
	pShake->shakeVector = Vec3::CreateLerp(pShake->shakeVector,pShake->goalShakeVector,t);
}


//------------------------------------------------------------------------
void CView::StopShake( int shakeID )
{
	uint32 num = m_shakes.size();
	for (uint32 i=0; i<num; ++i)
	{
		if (m_shakes[i].ID == shakeID && m_shakes[i].updating)
		{
			m_shakes[i].interrupted = true;
		}
	}
}


//------------------------------------------------------------------------
void CView::ResetShaking()
{
	// disable shakes
	std::vector<SShake>::iterator iter = m_shakes.begin();
	std::vector<SShake>::iterator iterEnd = m_shakes.end();
	while (iter != iterEnd)
	{
		SShake& shake = *iter;
		shake.updating = false;
		shake.timeDone = 0;
		++iter;
	}
}

//------------------------------------------------------------------------
void CView::LinkTo(IGameObject *follow)
{
	CRY_ASSERT(follow);
	m_linkedTo = follow->GetEntityId();
	m_viewParams.targetPos = follow->GetEntity()->GetWorldPos();
}

//------------------------------------------------------------------------
void CView::LinkTo(IEntity *follow)
{
	CRY_ASSERT(follow);
	m_linkedTo = follow->GetId();
	m_viewParams.targetPos = follow->GetWorldPos();
}

//------------------------------------------------------------------------
void CView::SetFrameAdditiveCameraAngles( const Ang3& addFrameAngles )
{
	m_frameAdditiveAngles = addFrameAngles;
}

//------------------------------------------------------------------------
CGameObject * CView::GetLinkedGameObject()
{
	IEntity * pEntity = gEnv->pEntitySystem->GetEntity( m_linkedTo );

	if (!pEntity)
		return NULL;

	return (CGameObject*) pEntity->GetProxy( ENTITY_PROXY_USER );
}

//------------------------------------------------------------------------
IEntity * CView::GetLinkedEntity()
{
	IEntity * pEntity = gEnv->pEntitySystem->GetEntity( m_linkedTo );
	return pEntity;
}

void CView::GetMemoryUsage(ICrySizer * s) const
{
	s->AddObject(this, sizeof(*this));
	s->AddObject(m_shakes);
}

void CView::Serialize(TSerialize ser)
{
	if (ser.IsReading())
	{
		ResetShaking();
	}
}

//////////////////////////////////////////////////////////////////////////
void CView::PostSerialize()
{
	CreateAudioListener();
}

//////////////////////////////////////////////////////////////////////////
void CView::OnEntityEvent(IEntity* pEntity, SEntityEvent& event)
{
	switch (event.event)
	{
	case ENTITY_EVENT_DONE:
		{
			// In case something destroys our listener entity before we had the chance to remove it.
			if (pEntity->GetId() == m_pAudioListener->GetId())
			{
				gEnv->pEntitySystem->RemoveEntityEventListener(m_pAudioListener->GetId(), ENTITY_EVENT_DONE, this);
				m_pAudioListener = NULL;
			}

			break;
		}
	default:
		{
			break;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CView::CreateAudioListener()
{
	if (m_pAudioListener == NULL)
	{
		SEntitySpawnParams oEntitySpawnParams;
		oEntitySpawnParams.sName  = "SoundListener";
		oEntitySpawnParams.pClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass("SoundListener");
		m_pAudioListener = gEnv->pEntitySystem->SpawnEntity(oEntitySpawnParams, true);

		if (m_pAudioListener != NULL)
		{
			m_pAudioListener->SetFlagsExtended(m_pAudioListener->GetFlagsExtended() | ENTITY_FLAG_EXTENDED_AUDIO_LISTENER);
			gEnv->pEntitySystem->AddEntityEventListener(m_pAudioListener->GetId(), ENTITY_EVENT_DONE, this);
			CryFixedStringT<64> sTemp;
			sTemp.Format("SoundListener(%d)", static_cast<int>(m_pAudioListener->GetId()));
			m_pAudioListener->SetName(sTemp.c_str());

			IEntityAudioProxyPtr pIEntityAudioProxy = crycomponent_cast<IEntityAudioProxyPtr>(m_pAudioListener->CreateProxy(ENTITY_PROXY_AUDIO));
			CRY_ASSERT(pIEntityAudioProxy.get());
		}
		else
		{
			CryFatalError("<Sound>: audio listener creation failed in CView ctor!");
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CView::UpdateAudioListener(Matrix34 const& rMatrix, bool bInvalidateListener)
{
	if (m_pAudioListener != NULL)
	{
		m_pAudioListener->SetWorldTM(rMatrix);
		if (bInvalidateListener)
			m_pAudioListener->InvalidateTM(ENTITY_XFORM_POS);
	}
}

//////////////////////////////////////////////////////////////////////////
void CView::SetActive(bool const bActive)
{
	if (bActive)
	{
		// Make sure we have a valid audio listener entity on an active view!
		CreateAudioListener();
	}
}

#include UNIQUE_VIRTUAL_WRAPPER(IView)
