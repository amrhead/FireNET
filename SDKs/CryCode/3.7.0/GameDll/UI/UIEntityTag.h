////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001-2011.
// -------------------------------------------------------------------------
//  File name:   UIEntityTag.h
//  Version:     v1.00
//  Created:     30/4/2011 by Paul Reindell.
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////
#ifndef __UIEntityTag_H__
#define __UIEntityTag_H__

#include "IUIGameEventSystem.h"
#include <IFlashUI.h>
#include <IEntitySystem.h>

class CUIEntityTag 
	: public IUIGameEventSystem
	, public IUIModule
	, public IUIElementEventListener
	, public IEntityEventListener
{
public:
	// IUIGameEventSystem
	UIEVENTSYSTEM( "CUIEntityTag" );
	virtual void InitEventSystem();
	virtual void UnloadEventSystem();
	virtual void OnUpdate( float fDelta );

	// IUIModule
	virtual void Reset();
	virtual void Reload();
	// ~IUIModule

	// IUIElementEventListener
	virtual void OnInstanceDestroyed( IUIElement* pSender, IUIElement* pDeletedInstance );
	// ~IUIElementEventListener

	// IEntityEventListener
	virtual void OnEntityEvent( IEntity *pEntity,SEntityEvent &event );
	// ~IEntityEventListener

	struct STagInfo
	{
		STagInfo(EntityId ownerId, const string& idx, const Vec3& offset, const SUIMovieClipDesc* pDesc, IUIElement* pElement, IFlashVariableObject* pObj, bool scale, const Vec3& currPos, bool bCamAttached)
			: OwnerId(ownerId)
			, Idx(idx)
			, vOffset(offset)
			, pMCDesc(pDesc)
			, pUIElement(pElement)
			, pVarObj(pObj)
			, bScale(scale)
			, vCurrPos(currPos)
			, fLerp(1)
			, fLerpSpeed(0)
			, bAttachedToCam(bCamAttached)
		{
		}
		EntityId OwnerId;
		string Idx;
		Vec3 vOffset;
		Vec3 vCurrPos;
		bool bAttachedToCam;
		float fLerpSpeed;
		float fLerp;
		const SUIMovieClipDesc* pMCDesc;
		IFlashVariableObject* pVarObj;
		IUIElement* pUIElement;
		bool bScale;
	};

		const STagInfo* GetTagInfo(EntityId entityId, const string& tagIdx) const;

private:
	SUIArgumentsRet OnAddTaggedEntity( EntityId entityId, const char* uiTemplate, int instanceId, const Vec3& offset, const char* idx, bool scale, bool bCamAttached);
	void OnUpdateTaggedEntity( EntityId entityId, const string& idx, const Vec3& offset, bool bCamAttached, float speed );
	void OnRemoveTaggedEntity( EntityId entityId, const string& idx );
	void OnRemoveAllTaggedEntity( EntityId entityId );

	void RemoveAllEntityTags( EntityId entityId, bool bUnregisterListener = true );
	void ClearAllTags();
	inline bool HasEntityTag( EntityId entityId ) const;
	Vec3 AddOffset( const Matrix34& camMat, const Vec3& vPos, const Vec3 &offset, bool bRelToCam );

private:
	SUIEventReceiverDispatcher<CUIEntityTag> s_EventDispatcher;
	IUIEventSystem* m_pUIOFct;

	typedef std::vector< STagInfo > TTags;
	TTags m_Tags;
};

// --------------------------------------------------------------
#endif // __UIEntityTag_H__

