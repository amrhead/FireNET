#ifndef __IMESHBAKING_H__
#define __IMESHBAKING_H__

struct SMeshBakingMaterialParams
{
	float rayLength;
	float rayIndent;
	bool bAlphaCutout;
	bool bIgnore;
};

struct SMeshBakingInputParams
{
	IStatObj *pCageMesh;
	ICharacterInstance *pCageCharacter;
	IStatObj *pInputMesh;
	ICharacterInstance *pInputCharacter;
	const SMeshBakingMaterialParams *pMaterialParams;
	ColorF defaultBackgroundColour;
	ColorF dilateMagicColour;
	int outputTextureWidth;
	int outputTextureHeight;
	int numMaterialParams;
	int nLodId;
	bool bDoDilationPass;
	bool bSmoothNormals;
	bool bSaveSpecular;
	IMaterial *pMaterial;
};

struct SMeshBakingOutput
{
	ITexture* ppOuputTexture[3];
	ITexture* ppIntermediateTexture[3];
};

#endif // __IMESHBAKING_H__