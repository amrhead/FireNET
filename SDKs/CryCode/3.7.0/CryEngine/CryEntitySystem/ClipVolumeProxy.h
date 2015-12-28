//  Copyright (C), Crytek Studios, 2001-2014

#ifndef __CLIPVOLUMEPROXY_H__
#define __CLIPVOLUMEPROXY_H__

#pragma once

#include "EntitySystem.h"
#include "IEntityClass.h"
#include "IEntityProxy.h"
#include "ISerialize.h"
#include <IRenderMesh.h>


//////////////////////////////////////////////////////////////////////////
// Description:
//    Proxy for storage of entity attributes.
//////////////////////////////////////////////////////////////////////////
class CClipVolumeProxy : public IClipVolumeProxy
{
public:
	CClipVolumeProxy();

	// IComponent interface implementation.
	//////////////////////////////////////////////////////////////////////////
	virtual void Initialize( const SComponentInitializer& init ) OVERRIDE {};
	virtual	void ProcessEvent( SEntityEvent& event ) OVERRIDE;
	//////////////////////////////////////////////////////////////////////////

	// IEntityProxy
	//////////////////////////////////////////////////////////////////////////
	VIRTUAL EEntityProxy GetType() { return ENTITY_PROXY_CLIPVOLUME; }
	VIRTUAL void Release() OVERRIDE;
	VIRTUAL void Done()  {};
	VIRTUAL	void Update(SEntityUpdateContext& context) OVERRIDE {};
	VIRTUAL bool Init(IEntity* pEntity, SEntitySpawnParams& params) OVERRIDE;
	VIRTUAL void Reload(IEntity* pEntity, SEntitySpawnParams& params) OVERRIDE;
	VIRTUAL void SerializeXML(XmlNodeRef &entityNodeXML, bool loading) OVERRIDE;
	VIRTUAL void Serialize(TSerialize serialize) OVERRIDE {};
	VIRTUAL bool NeedSerialize() { return false; }
	VIRTUAL bool GetSignature(TSerialize signature) OVERRIDE;
	VIRTUAL void GetMemoryUsage(ICrySizer* pSizer) const OVERRIDE;
	//////////////////////////////////////////////////////////////////////////

	VIRTUAL void UpdateRenderMesh(IRenderMesh* pRenderMesh, const DynArray<Vec3>& meshFaces) OVERRIDE;
	VIRTUAL IClipVolume* GetClipVolume() const OVERRIDE { return m_pClipVolume; }
	VIRTUAL IBSPTree3D* GetBspTree() const OVERRIDE { return m_pBspTree; }

private:
	bool LoadFromFile(const char* szFilePath);

private:
	// Host entity.
	IEntity *m_pEntity;

	// Engine clip volume
	IClipVolume* m_pClipVolume;

	// Render Mesh
	_smart_ptr<IRenderMesh> m_pRenderMesh;

	// BSP tree
	IBSPTree3D* m_pBspTree;

	// In-game stat obj
	string m_GeometryFileName;
};

DECLARE_BOOST_POINTERS(CClipVolumeProxy)

#endif //__CLIPVOLUMEPROXY_H__