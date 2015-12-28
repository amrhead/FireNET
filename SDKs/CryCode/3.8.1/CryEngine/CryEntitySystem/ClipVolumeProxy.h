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
	virtual void Initialize( const SComponentInitializer& init ) override {};
	virtual	void ProcessEvent( SEntityEvent& event ) override;
	//////////////////////////////////////////////////////////////////////////

	// IEntityProxy
	//////////////////////////////////////////////////////////////////////////
	virtual EEntityProxy GetType() { return ENTITY_PROXY_CLIPVOLUME; }
	virtual void Release() override;
	virtual void Done()  {};
	virtual void Update(SEntityUpdateContext& context) override {};
	virtual bool Init(IEntity* pEntity, SEntitySpawnParams& params) override;
	virtual void Reload(IEntity* pEntity, SEntitySpawnParams& params) override;
	virtual void SerializeXML(XmlNodeRef &entityNodeXML, bool loading) override;
	virtual void Serialize(TSerialize serialize) override {};
	virtual bool NeedSerialize() { return false; }
	virtual bool GetSignature(TSerialize signature) override;
	virtual void GetMemoryUsage(ICrySizer* pSizer) const override;
	//////////////////////////////////////////////////////////////////////////

	virtual void UpdateRenderMesh(IRenderMesh* pRenderMesh, const DynArray<Vec3>& meshFaces) override;
	virtual IClipVolume* GetClipVolume() const override { return m_pClipVolume; }
	virtual IBSPTree3D* GetBspTree() const override { return m_pBspTree; }

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