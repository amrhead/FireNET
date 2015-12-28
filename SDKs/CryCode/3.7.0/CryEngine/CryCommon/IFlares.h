#pragma once
////////////////////////////////////////////////////////////////////////////
//  Crytek Engine Source File.
//  (c) 2001 - 2012 Crytek GmbH
// -------------------------------------------------------------------------
//  File name:   IFlares.h
//  Created:     8/12/2011
////////////////////////////////////////////////////////////////////////////
#include <IFuncVariable.h> // <> required for Interfuscator
#include <IXml.h> // <> required for Interfuscator
#include "smartptr.h"

struct IShader;
class CCamera;

class __MFPA{};
class __MFPB{};
#define MFP_SIZE_ENFORCE : public __MFPA, public __MFPB

enum EFlareType
{
	eFT__Base__,
	eFT_Root,
	eFT_Group,
	eFT_Ghost,
	eFT_MultiGhosts,
	eFT_Glow,
	eFT_ChromaticRing,
	eFT_IrisShafts,
	eFT_CameraOrbs, 
	eFT_ImageSpaceShafts,
	eFT_Streaks,
	eFT_Reference,
	eFT_Proxy,
	eFT_Max
};

#define FLARE_LIBS_PATH "Libs/Flares/"
#define FLARE_EXPORT_FILE "LensFlareList.xml"
#define FLARE_EXPORT_FILE_VERSION "1"

struct FlareInfo
{
	EFlareType type;
	const char* name;
#if defined(FLARES_SUPPORT_EDITING)
	const char* imagename;
#endif
};

#if defined(FLARES_SUPPORT_EDITING)
#	define ADD_FLARE_INFO(type, name, imagename) {type, name, imagename}
#else
#	define ADD_FLARE_INFO(type, name, imagename) {type, name}
#endif

class FlareInfoArray
{
public:
	struct Props
	{
		const FlareInfo* p;
		size_t size;
	};

	static const Props Get()
	{
		static const FlareInfo flareInfoArray[] = 
		{
			ADD_FLARE_INFO(eFT__Base__, "__Base__", NULL),
			ADD_FLARE_INFO(eFT_Root, "Root", NULL),
			ADD_FLARE_INFO(eFT_Group, "Group", NULL),
			ADD_FLARE_INFO(eFT_Ghost, "Ghost", "EngineAssets/Textures/flares/icons/ghost.dds"),
			ADD_FLARE_INFO(eFT_MultiGhosts, "Multi Ghost", "EngineAssets/Textures/flares/icons/multi_ghost.dds"),
			ADD_FLARE_INFO(eFT_Glow, "Glow", "EngineAssets/Textures/flares/icons/glow.dds"),
			ADD_FLARE_INFO(eFT_ChromaticRing, "ChromaticRing", "EngineAssets/Textures/flares/icons/ring.dds"),
			ADD_FLARE_INFO(eFT_IrisShafts, "IrisShafts", "EngineAssets/Textures/flares/icons/iris_shafts.dds"),
			ADD_FLARE_INFO(eFT_CameraOrbs, "CameraOrbs", "EngineAssets/Textures/flares/icons/orbs.dds"),
			ADD_FLARE_INFO(eFT_ImageSpaceShafts, "Vol Shafts", "EngineAssets/Textures/flares/icons/vol_shafts.dds"),
			ADD_FLARE_INFO(eFT_Streaks, "Streaks", "EngineAssets/Textures/flares/icons/iris_shafts.dds")
		};

		Props ret;
		ret.p = flareInfoArray;
		ret.size = sizeof(flareInfoArray) / sizeof(flareInfoArray[0]);
		return ret;
	}

private:
	FlareInfoArray();
	~FlareInfoArray();
};

struct SLensFlareRenderParam
{
	SLensFlareRenderParam() :
		pCamera(NULL),
		pShader(NULL)
	{
	}
	~SLensFlareRenderParam(){}
	bool IsValid() const
	{
		return pCamera && pShader;
	}
	CCamera* pCamera;
	IShader* pShader;
};

UNIQUE_IFACE class ISoftOcclusionQuery
{
public:

	virtual ~ISoftOcclusionQuery() {}

	virtual void AddRef() = 0;
	virtual void Release() = 0;

};

UNIQUE_IFACE class IOpticsElementBase MFP_SIZE_ENFORCE
{
public:

	IOpticsElementBase(): m_nRefCount(0)
	{
	}
	void AddRef()
	{
		CryInterlockedIncrement(&m_nRefCount);
	}
	void Release()
	{
		if (CryInterlockedDecrement(&m_nRefCount) <= 0)
			delete this;
	}

	virtual EFlareType GetType() = 0;
	virtual bool IsGroup() const = 0;
	virtual string GetName() const = 0;
	virtual void SetName(const char* ch_name) = 0;
	virtual void Load( IXmlNode* pNode ) = 0;

	virtual IOpticsElementBase* GetParent() const = 0;
	virtual ~IOpticsElementBase(){}

	virtual bool IsEnabled() const = 0;
	virtual void SetEnabled(bool b) = 0;

	virtual void AddElement(IOpticsElementBase* pElement) = 0;
	virtual void InsertElement( int nPos, IOpticsElementBase* pElement) = 0;
	virtual void Remove( int i ) = 0;
	virtual void RemoveAll() = 0;
	virtual int GetElementCount() const = 0;
	virtual IOpticsElementBase* GetElementAt( int  i ) const = 0;

	virtual void GetMemoryUsage( ICrySizer *pSizer ) const = 0;
	virtual void Invalidate() = 0;

	virtual void Render( SLensFlareRenderParam* pParam, const Vec3& vPos ) = 0;

	virtual void SetOpticsReference( IOpticsElementBase* pReference ) {}
	virtual IOpticsElementBase* GetOpticsReference() const	{	return NULL; }


#if defined(FLARES_SUPPORT_EDITING)
	virtual DynArray<FuncVariableGroup> GetEditorParamGroups()=0;
#endif

private:

	volatile int m_nRefCount;
};

UNIQUE_IFACE class IOpticsManager
{	
public:

	virtual ~IOpticsManager(){}
	virtual void Reset() = 0;
	virtual IOpticsElementBase* Create( EFlareType type ) const = 0;
	virtual bool Load( const char* fullFlareName, int& nOutIndex ) = 0;
	virtual bool Load( XmlNodeRef& rootNode, int& nOutIndex ) = 0;
	virtual IOpticsElementBase* GetOptics( int nIndex ) = 0;
	virtual bool AddOptics( IOpticsElementBase* pOptics, const char* name, int& nOutNewIndex ) = 0;
	virtual bool Rename( const char* fullFlareName, const char* newFullFlareName ) = 0;
	virtual void GetMemoryUsage( ICrySizer* pSizer ) const = 0;
	virtual void Invalidate() = 0;

};

typedef _smart_ptr<IOpticsElementBase> IOpticsElementBasePtr;