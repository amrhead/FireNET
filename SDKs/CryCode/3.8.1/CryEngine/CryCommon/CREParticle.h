#ifndef __CREPARTICLE_H__
#define __CREPARTICLE_H__

#include "CryThread.h"
#include <IJobManager.h>
#include <MemoryAccess.h>

// forward declarations
class CREParticle;
typedef SVF_P3F_C4B_T4B_N3F2 SVF_Particle;	

struct SRenderVertices
{
	FixedDynArray<SVF_Particle>			aVertices;
	FixedDynArray<uint16>						aIndices;
	float														fPixels;
	
	ILINE SRenderVertices()
		: fPixels(0.f) {}
};

struct SCameraInfo
{
	const CCamera*	pCamera;
	IVisArea*				pCameraVisArea;
	bool						bCameraUnderwater;
	bool						bRecursivePass;

	SCameraInfo (const SRenderingPassInfo& passInfo) :
		pCamera(&passInfo.GetCamera()),
		pCameraVisArea(gEnv->p3DEngine->GetVisAreaFromPos(passInfo.GetCamera().GetOccPos())),
		bCameraUnderwater(passInfo.IsCameraUnderWater()),
		bRecursivePass(passInfo.IsRecursivePass())
	{}
};

struct IParticleVertexCreator
{
	// Create the vertices for the particle emitter.
	virtual void ComputeVertices( const SCameraInfo& camInfo, CREParticle* pRE, uint32 uRenderFlags, float fMaxPixels ) = 0;

	virtual ~IParticleVertexCreator() {}
};

class CREParticle : public CRendElementBase
{
public:
	enum EParticleObjFlags
	{
		ePOF_HALF_RES = 0x01,
		ePOF_VOLUME_FOG = 0x02,
	};

	CREParticle();
	void Reset( IParticleVertexCreator* pVC, int nThreadId );
	
	// Custom copy constructor required to avoid m_Lock copy.
	CREParticle( const CREParticle& in )
	: m_pVertexCreator(in.m_pVertexCreator)
	, m_nThreadId(in.m_nThreadId)
	{
	}
		
	virtual void GetMemoryUsage(ICrySizer *pSizer) const 
	{
	}

	// CRendElement implementation.
	virtual CRendElementBase* mfCopyConstruct()
	{
		return new CREParticle(*this);
	}
	virtual int Size()
	{
		return sizeof(*this);
	}
	
	virtual void mfPrepare(bool bCheckOverflow);

	virtual bool mfPreDraw( SShaderPass *sl );
	virtual bool mfDraw( CShader *ef, SShaderPass *sl );

	// Additional methods.

	// Interface to alloc render verts and indices from 3DEngine code.	
	virtual SRenderVertices* AllocVertices( int nAllocVerts, int nAllocInds );

	void ComputeVertices( SCameraInfo camInfo, uint32 uRenderFlags );

	float GetPixels() const
	{
		return m_RenderVerts.fPixels;
	}

private:
	IParticleVertexCreator*							m_pVertexCreator;		// Particle object which computes vertices.
	SRenderVertices											m_RenderVerts;
	uint16															m_nThreadId;
	uint16															m_nFirstVertex;
	uint32															m_nFirstIndex;
};

#endif  // __CREPARTICLE_H__
