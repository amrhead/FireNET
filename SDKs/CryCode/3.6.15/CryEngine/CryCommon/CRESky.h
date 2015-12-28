 
#ifndef __CRESKY_H__
#define __CRESKY_H__

//=============================================================

#include "VertexFormats.h"

class CRESky : public CRendElementBase
{
  friend class CRender3D;

public:

  float m_fTerrainWaterLevel;
	float m_fSkyBoxStretching;
  float m_fAlpha;
  int m_nSphereListId;

public:
  CRESky()
  {
    mfSetType( eDATA_Sky );
    mfUpdateFlags( FCEF_TRANSFORM );
    m_fTerrainWaterLevel = 0;
    m_fAlpha = 1;
    m_nSphereListId = 0;
		m_fSkyBoxStretching=1.f;
  }

  virtual ~CRESky();
  virtual void mfPrepare(bool bCheckOverflow);
  virtual bool mfDraw(CShader *ef, SShaderPass *sfm);

	virtual void GetMemoryUsage(ICrySizer *pSizer) const 
	{
		pSizer->AddObject(this, sizeof(*this));
	}
};

class CREHDRSky : public CRendElementBase
{
public:
	CREHDRSky()
	: m_pRenderParams(0)
	, m_skyDomeTextureLastTimeStamp(-1)
	, m_frameReset(0)
	, m_pStars(0)
	, m_pSkyDomeTextureMie(0)
	, m_pSkyDomeTextureRayleigh(0)
	{
		mfSetType(eDATA_HDRSky);
		mfUpdateFlags(FCEF_TRANSFORM);
		Init();
	}

	virtual ~CREHDRSky();
	virtual void mfPrepare(bool bCheckOverflow);
	virtual bool mfDraw(CShader *ef, SShaderPass *sfm);

	virtual void GetMemoryUsage(ICrySizer *pSizer) const 
	{
		pSizer->AddObject(this, sizeof(*this));
	}

	void GenerateSkyDomeTextures(int32 width, int32 height);

public:
	const SSkyLightRenderParams* m_pRenderParams;
	int m_moonTexId;
	class CTexture* m_pSkyDomeTextureMie;
	class CTexture* m_pSkyDomeTextureRayleigh;

	static void SetCommonMoonParams(CShader *ef, bool bUseMoon = false);

private:
	void Init();

private:
	int m_skyDomeTextureLastTimeStamp;
	int m_frameReset;
	class CStars* m_pStars;
};


#endif  // __CRESKY_H__
