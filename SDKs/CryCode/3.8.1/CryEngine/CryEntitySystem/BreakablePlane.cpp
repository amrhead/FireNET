/*************************************************************************
  Crytek Source File.
  Copyright (C), Crytek Studios, 2001-2005.
 -------------------------------------------------------------------------
  $Id$
  $DateTime$
  
 -------------------------------------------------------------------------
  History:
  - 20:1:2005: Created by Anton Knyazyev

*************************************************************************/

#include "stdafx.h"
#include "BreakablePlane.h"
#include "ParticleParams.h"
#include "IParticles.h"
#include "ITimer.h"
#include "BreakableManager.h"


template<class T> struct triplet {
	triplet(T *ptr) { pdata = ptr; }
	void set(const T& v0, const T& v1, const T& v2) { pdata[0]=v0; pdata[1]=v1; pdata[2]=v2; }
	T *pdata;
};

int CBreakablePlane::g_nPieces = 0;
float CBreakablePlane::g_maxPieceLifetime = 0;


int UpdateBrokenMat(IMaterial *pRenderMat, int idx, const char *substName)
{
	IMaterial *pSubMtl,*pSubMtl1;
	int i,len; const char *name,*name1;
	if (pRenderMat)
		/*if ((pSubMtl=pRenderMat->GetSubMtl(idx+1)) && strstr(pSubMtl->GetName(), "broken"))
			idx++;
		else*/ if ((pSubMtl=pRenderMat->GetSubMtl(idx)) && (len=strlen(name=pSubMtl->GetName())) && (len<7 || strcmp(name+len-7,"_broken")))
		{
			for(i=pRenderMat->GetSubMtlCount()-1; i>=0; i--) 
				if ((pSubMtl1=pRenderMat->GetSubMtl(i)) && !strnicmp(name1=pSubMtl1->GetName(),name,len) && !strcmp(name1+len,"_broken"))
				{
					idx=i; break;
				}
			if (i<0 && *substName)
			{
				for(i=pRenderMat->GetSubMtlCount()-1; i>=0; i--) 
					if ((pSubMtl1=pRenderMat->GetSubMtl(i)) && !stricmp(pSubMtl1->GetName(),substName))
						break;
				if (i<0)
				{
					if (IMaterial *pMatSubst = gEnv->p3DEngine->GetMaterialManager()->LoadMaterial(substName,false))
					{
						idx = pRenderMat->GetSubMtlCount();
						pRenderMat->SetSubMtlCount(idx+1);
						pRenderMat->SetSubMtl(idx, pMatSubst);
					}
				}	else
					idx = i;
			}
		}
	return idx;
}


bool CBreakablePlane::SetGeometry(IStatObj *pStatObj, IMaterial *pRenderMat, int bStatic, int seed)
{
	int i,j,nVtx=0,idir,i0,j0,i1,iup,iter0,iter1,ibest[2]={-1,-1};
	Vec3 n,pt,c,sz,axis;
	float mincos,maxcos,cosa,maxarea[2]={1E-15f,1E-15f};
	primitives::box bbox;
	phys_geometry *pPhysGeom;
	mesh_data *pPhysMesh;
	CMesh *pMesh;
	vector2df *pVtx=0;
	m_bStatic = bStatic;

	if ((pPhysGeom=pStatObj->GetPhysGeom()))
	{
		pPhysGeom->pGeom->GetBBox(&bbox);
		iup = idxmin3((float*)&bbox.size);
		if (min(bbox.size[inc_mod3[iup]],bbox.size[dec_mod3[iup]]) < bbox.size[iup]*4)
		{
			CryLog("[Breakable2d] : geometry is not thin enough, breaking denied ( %s %s)", pStatObj->GetFilePath(), pStatObj->GetGeoName());
			return false;
		}
		bbox.size[iup] = max(bbox.size[iup], m_cellSize*0.01f);
		axis = bbox.Basis.GetRow(iup); 
		if (iup!=2) 
		{
			n=bbox.Basis.GetRow(inc_mod3[iup]); c=bbox.Basis.GetRow(dec_mod3[iup]);
			bbox.Basis.SetRow(0,n); bbox.Basis.SetRow(1,c); bbox.Basis.SetRow(2,axis);
		}
		m_R = bbox.Basis.T();
		Vec3 axisAbs(axis.abs());
		m_bAxisAligned = axis[idxmax3((float*)&axisAbs)]>0.995f;
		m_center = bbox.center;
		m_nudge = m_cellSize*0.075f;
		m_nCells.set((int)(bbox.size[inc_mod3[iup]]*2/m_cellSize+0.5f), (int)(bbox.size[dec_mod3[iup]]*2/m_cellSize+0.5f));
		m_nCells.x = max(1,m_nCells.x); m_nCells.y = max(1,m_nCells.y); 
		if (m_nCells.x*m_nCells.y>4000)
		{
			CryLogAlways("[Breakable2d] : breakable tessellation is too fine, breaking denied ( %s %s)", pStatObj->GetFilePath(), pStatObj->GetGeoName());
			return false;
		}
		m_pGeom = 0; m_pSampleRay = 0;

		if (pPhysGeom->pGeom->GetType()==GEOM_TRIMESH)
		{
			pPhysMesh = (mesh_data*)pPhysGeom->pGeom->GetData();
			if (pPhysMesh->pMats) for(i=1; i<pPhysMesh->nTris; i++)
				if (pPhysMesh->pMats[i] != pPhysMesh->pMats[0])
				{
					CryLogAlways("[Breakable2d] : geometry has several submaterials, breaking denied ( %s %s)", pStatObj->GetFilePath(), pStatObj->GetGeoName());
					return false;
				}
			for(i=0; i<pPhysMesh->nTris; i++) if (fabs_tpl(pPhysMesh->pNormals[i]*axis)>0.95f)
			{
				for(j=0;j<3 && pPhysMesh->pTopology[i].ibuddy[j]>=0 && 
						fabs_tpl(pPhysMesh->pNormals[pPhysMesh->pTopology[i].ibuddy[j]]*pPhysMesh->pNormals[i])>0.97f; j++);
				if (j<3) break;
			}
			i0=i;	
			if (i<pPhysMesh->nTris)	
			{
				mincos = (float)sgnnz(pPhysMesh->pNormals[i]*axis);
				m_R.SetColumn(2, m_R.GetColumn(2)*mincos);
				m_R.SetColumn(0, m_R.GetColumn(0)*mincos);
				axis *= mincos;
				mincos=maxcos = pPhysMesh->pNormals[i]*axis;
				j0=j; iter1=0; do {
					// cppcheck-suppress memleakOnRealloc
					if (!(nVtx & 31)) pVtx = (vector2df*)realloc(pVtx, (nVtx+32)*sizeof(pVtx[0]));
					pVtx[nVtx++] = Vec2((pPhysMesh->pVertices[pPhysMesh->pIndices[i*3+j]]-m_center)*m_R);
					for(iter0=0; iter0<pPhysMesh->nTris; iter0++) {
						j = inc_mod3[j];
						if ((i1=pPhysMesh->pTopology[i].ibuddy[j])<0 || pPhysMesh->pNormals[i1]*pPhysMesh->pNormals[i]<0.97f)
							break;
						for(j=0; j<2 && pPhysMesh->pTopology[i1].ibuddy[j]!=i; j++);
						i = i1;
						mincos=min(mincos, cosa=pPhysMesh->pNormals[i]*axis); maxcos=max(maxcos,cosa);
					}
				}	while(++iter1<=pPhysMesh->nTris*3 && (i!=i0 || j!=j0));
				if (iter1>pPhysMesh->nTris*3) {
					free(pVtx); return false;
				}
			}	else
				return false;

			if (mincos<0.5f || (cosa = sqrt_tpl(1.0f-min(0.999f,sqr(mincos)))*maxcos - sqrt_tpl(1.0f-min(0.999f,sqr(maxcos))) * mincos)>0.7f) {
				free(pVtx); return false;

			}
			if (cosa > 0.03f)
			{
				(m_pGeom = pPhysGeom->pGeom)->PrepareForRayTest(bbox.size[iup]*2.3f);
				m_pGeom->AddRef();
				primitives::ray aray; aray.origin.zero(); aray.dir = axis;
				m_pSampleRay = gEnv->pPhysicalWorld->GetGeomManager()->CreatePrimitive(primitives::ray::type, &aray);
			}
		} 
		else
		{
			pVtx = (vector2df*)malloc(4*sizeof(pVtx[0])); nVtx = 4;
			for(i=0;i<4;i++) pVtx[i].set(bbox.size[inc_mod3[iup]]*(((i^i*2)&2)-1), bbox.size[dec_mod3[iup]]*((i&2)-1));
		}
		if (!pStatObj->GetIndexedMesh(true)) {
			free(pVtx); return false;
		}
		
		pMesh = pStatObj->GetIndexedMesh(true)->GetMesh();
		assert(pMesh->m_pPositionsF16 == 0);	

		m_thicknessOrg = bbox.size[iup];
		bbox.size[iup] = max(bbox.size[iup], 0.005f);
		m_z[0] = -bbox.size[iup]; m_z[1] = bbox.size[iup];
		{
			const int indexCount = pMesh->GetIndexCount();
			for(i=0; i<indexCount; i+=3)
			{
				n = pMesh->m_pPositions[pMesh->m_pIndices[i+1]]-pMesh->m_pPositions[pMesh->m_pIndices[i]] ^
						pMesh->m_pPositions[pMesh->m_pIndices[i+2]]-pMesh->m_pPositions[pMesh->m_pIndices[i]];
				for(idir=0;idir<2;idir++)
					if (sqr_signed(n*axis)*(idir*2-1)>n.len2()*sqr(0.95f) && n.len2()>maxarea[idir])
						maxarea[idir]=n.len2(), ibest[idir]=i;
			}
		}
		if (ibest[0]<0 && ibest[1]<0)
			ibest[0] = ibest[1] = 0;
		m_bOneSided = 0;

		for(idir=0;idir<2;idir++)	
		{
			if (ibest[idir]<0)
				ibest[idir] = ibest[idir^1], m_bOneSided = 1;
			for(i=0;i<3;i++) 	
			{
				m_ptRef[idir][i] = Vec2((pMesh->m_pPositions[pMesh->m_pIndices[ibest[idir]+i]]-m_center)*m_R);
				m_texRef[idir][i] = pMesh->m_pTexCoord[pMesh->m_pIndices[ibest[idir]+i]];
				m_TangentRef[idir][i] = pMesh->m_pTangents[pMesh->m_pIndices[ibest[idir]+i]];
			}
			m_refArea[idir] = (idir*2-1)/sqrt_tpl(maxarea[idir]);
		}
		if (m_pGeom) for(idir=0;idir<2;idir++)	
		{
			n = pMesh->m_pPositions[pMesh->m_pIndices[ibest[idir]+1]]-pMesh->m_pPositions[pMesh->m_pIndices[ibest[idir]]] ^
			    pMesh->m_pPositions[pMesh->m_pIndices[ibest[idir]+2]]-pMesh->m_pPositions[pMesh->m_pIndices[ibest[idir]]];

			Matrix33 R = Matrix33::CreateRotationV0V1(n.GetNormalized(), axis * (float)(idir * 2 - 1));
			for (i = 0; i < 3; i++) 	
			{
				m_TangentRef[idir][i].RotateBy(R);
			}
		}
		m_pMat = pStatObj->GetMaterial();
		if (pMesh->m_subsets.size()>0) 
		{
			for(i=0; i<pMesh->m_subsets.size()-1 && pMesh->m_subsets[i].nNumIndices==0; i++);
			m_matSubindex = UpdateBrokenMat(pRenderMat, pMesh->m_subsets[i].nMatID, m_mtlSubstName);
			m_matFlags = pMesh->m_subsets[i].nMatFlags;
		}
		IGeomManager *pGeoman = gEnv->pPhysicalWorld->GetGeomManager();
		m_pGrid = pGeoman->GenerateBreakableGrid(pVtx,nVtx, m_nCells, m_bStatic, seed);
		free(pVtx);
		pStatObj->FreeIndexedMesh();
		//pPhysGeom->pGeom->SetForeignData(this,1);
	}

	return m_pGrid!=0;
}


void CBreakablePlane::FillVertexData(CMesh *pMesh,int ivtx, const vector2df &pos, int iside)
{
	int i,ncont;
	float k;
	Quat qnRot; qnRot.SetIdentity();
	Vec2 Coord(ZERO), uv;
	Vec3 Tangent(ZERO), Bitangent(ZERO), t, b;
	int16 r;

	assert(pMesh->m_pPositionsF16 == 0);	

	pMesh->m_pPositions[ivtx] = m_R * Vec3(pos.x,pos.y,m_z[iside]) + m_center;
	pMesh->m_pNorms    [ivtx] = SMeshNormal(m_R * Vec3(0,0,iside*2-1.f));
	if (m_pGeom)
	{
		geom_world_data gwd[2];
		geom_contact *pcont;
		gwd[1].offset = m_R*Vec3(pos.x*0.99f,pos.y*0.99f,m_z[iside]*1.01f)+m_center;
		i=1-iside*2; gwd[1].R(0,0)*=i; gwd[1].R(1,1)*=i; gwd[1].R(2,2)*=i;
		{ WriteLockCond lock;
		if (ncont = m_pGeom->IntersectLocked(m_pSampleRay,gwd,gwd+1,0,pcont,lock)) 
		{
			pMesh->m_pPositions[ivtx] += m_R.GetColumn(2)*(m_R.GetColumn(2)*(pcont[ncont-1].pt-pMesh->m_pPositions[ivtx]));

			Vec3 n = pMesh->m_pNorms[ivtx].GetN();
			qnRot = Quat::CreateRotationV0V1(n, pcont[ncont-1].n);
			pMesh->m_pNorms[ivtx] = SMeshNormal(pcont[ncont-1].n);
		}
		} // lock
	}

	m_TangentRef[iside][0].GetR(r);
	for (i = 0; i < 3; i++) 
	{
		k = (m_ptRef[iside][inc_mod3[i]] - pos ^ m_ptRef[iside][dec_mod3[i]] - pos) * m_refArea[iside];

		m_texRef    [iside][i].GetUV(uv);
		m_TangentRef[iside][i].GetTB(t, b);

		Coord     += uv * k;
		Tangent   += t  * k;
		Bitangent += b  * k;
	}

	Tangent.NormalizeSafe(); (Bitangent -= Tangent * (Tangent * Bitangent)).NormalizeSafe();

	pMesh->m_pTexCoord[ivtx] = SMeshTexCoord(Coord);
	pMesh->m_pTangents[ivtx] = SMeshTangents(qnRot * Tangent, qnRot * Bitangent, r);
}


IStatObj *CBreakablePlane::CreateFlatStatObj(int *&pIdx, vector2df *pt, vector2df *bounds, const Matrix34 &mtxWorld, 
						IParticleEffect *pEffect, bool bNoPhys, bool bUseEdgeAlpha)
{
	int i,j,nVtx,nCntVtx,nTris,itri,itriCnt,nCntVtxTri,bValid,clrmask;
	int *pVtxMap,*pCntVtxMap,*pCntVtxList;
	Vec3 n,Tangent,(*pVtxEdge)[2],direff,axis=m_R*Vec3(0,0,1);
	vector2df pos;
	float nudge,k;
	SMeshTexCoord tex;
	IStatObj *pStatObj;
	IIndexedMesh *pIdxMesh;
	CMesh *pMesh;
	SMeshColor *pColor,dummyClr;

	if (*pIdx<0)
		return 0;
	pStatObj = gEnv->p3DEngine->CreateStatObj();
	//pStatObj->AddRef();
	pMesh = (pIdxMesh=pStatObj->GetIndexedMesh())->GetMesh();
	assert(pMesh->m_pPositionsF16 == 0);	
	pStatObj->SetMaterial(m_pMat);

	nVtx = (m_nCells.x+2)*(m_nCells.y+2);
	memset(pVtxMap = new int[nVtx], -1,sizeof(pVtxMap[0])*nVtx);
	memset(pCntVtxMap = new int[nVtx], -1,sizeof(pVtxMap[0])*nVtx);
	pVtxEdge = (Vec3(*)[2])(new Vec3[nVtx*2]);

	for(i=nVtx=nTris=nCntVtx=nCntVtxTri=0; pIdx[i]>=0; i+=4,nTris++) for(j=0;j<3;j++)
	{
		nVtx += pVtxMap[pIdx[i+j]]>>1 & 1; pVtxMap[pIdx[i+j]] &= ~2;
		nCntVtx += pVtxMap[pIdx[i+j]] & (pIdx[i+3]>>j^1) & 1;
		pVtxMap[pIdx[i+j]] &= pIdx[i+3]>>j & 1 | ~1;
		nCntVtxTri += (pIdx[i+3]>>j^1) & 1;
	}
	nudge = pIdx[i]==-1 ? m_nudge:0;
	int b2Sided = pIdx[i]==-1 || !m_bOneSided;

	int nOutVtx = nVtx+(nVtx+nCntVtx*4)*b2Sided;
	pIdxMesh->SetVertexCount(nOutVtx);
	pIdxMesh->SetTexCoordCount(nOutVtx);
	pIdxMesh->SetTangentCount(nOutVtx);
	pIdxMesh->SetIndexCount(nTris*3+(nTris*3+nCntVtxTri*6)*b2Sided);
	if (bUseEdgeAlpha) {
		pIdxMesh->SetColorCount(nOutVtx);
		memset(pMesh->m_pColor0, 255, nOutVtx*sizeof(pMesh->m_pColor0[0]));
		pColor = pMesh->m_pColor0; clrmask = -1;
	} 
	else
	{
		pColor = &dummyClr; clrmask = 0;
	}
	pCntVtxList = new int[nCntVtx];
	nVtx = nCntVtx = nTris = 0;
	direff = mtxWorld.TransformVector(m_R*Vec3(0,0,1));
	bounds[0].set( 1E10, 1E10);
	bounds[1].set(-1E10,-1E10);

	for(i=0;pIdx[i]>=0;i+=4)
	{
		for(j=0;j<3;j++) 
		{
			if (pVtxMap[pIdx[i+j]]<0)
			{
				FillVertexData(pMesh,nVtx, vector2df(pt[pIdx[i+j]].x,pt[pIdx[i+j]].y), 1);
				bounds[0].x = min(bounds[0].x,pt[pIdx[i+j]].x);	bounds[0].y = min(bounds[0].y,pt[pIdx[i+j]].y);
				bounds[1].x = max(bounds[1].x,pt[pIdx[i+j]].x);	bounds[1].y = max(bounds[1].y,pt[pIdx[i+j]].y);
				pVtxMap[pIdx[i+j]] = nVtx++;
			}
			pMesh->m_pIndices[nTris*3+j] = pVtxMap[pIdx[i+j]];
			pColor[pVtxMap[pIdx[i+j]] & clrmask].MaskA(-(pIdx[i+3]>>(j+6) & 1));
		}
		for(j=0;j<3;j++) if (!(pIdx[i+3] & 1<<j))
		{
			// offset vtx inward along the edge normal
			n = Vec3(pt[pIdx[i+inc_mod3[j]]].x-pt[pIdx[i+j]].x, pt[pIdx[i+inc_mod3[j]]].y-pt[pIdx[i+j]].y, 0).normalized();
			pVtxEdge[pIdx[i+j]][1] = pVtxEdge[pIdx[i+inc_mod3[j]]][0] = m_R*n;

			if (!(pIdx[i+3] & 8<<j))
			{
				n = m_R*Vec3(-n.y,n.x,0);
				pMesh->m_pPositions[pVtxMap[pIdx[i+j]]] += n*nudge;
				pMesh->m_pPositions[pVtxMap[pIdx[i+inc_mod3[j]]]] += n*nudge;
			}

			if (pCntVtxMap[pIdx[i+j]]<0)
			{
				pCntVtxMap[pIdx[i+j]] = nCntVtx;
				pCntVtxList[nCntVtx++] = pIdx[i+j];
			}
		}
		nTris++;
	}

	if (b2Sided)
	{
		for(i=0;i<nVtx;i++)	
		{ // add the bottom vertices
			FillVertexData(pMesh,nVtx+i, vector2df((pMesh->m_pPositions[i]-m_center)*m_R), 0);
			pColor[nVtx+i & clrmask] = pColor[i & clrmask];
		}

		for(i=0,n=-axis; i<nCntVtx*2; i++)
		{	// add the side vertices (split copies of top and bottom contours)
			Tangent = pVtxEdge[pCntVtxList[i>>1]][i&1];

			pMesh->m_pPositions[nVtx*2+i] = pMesh->m_pPositions[j = pVtxMap[pCntVtxList[i>>1]]];
			pMesh->m_pTexCoord[nVtx*2+i] = pMesh->m_pTexCoord[j];
			pMesh->m_pTangents[(nVtx+nCntVtx)*2+i] = pMesh->m_pTangents[nVtx*2+i] = SMeshTangents(Tangent, n, -1);
			pMesh->m_pNorms[(nVtx+nCntVtx)*2+i] = pMesh->m_pNorms[nVtx*2+i] = SMeshNormal(Tangent.Cross(n));
			pColor[(nVtx+nCntVtx)*2+i & clrmask] = pColor[nVtx*2+i & clrmask] = pColor[j & clrmask];
		}
		for(i=0; i<nCntVtx*2; i++) 
		{
			pMesh->m_pPositions[(nVtx+nCntVtx)*2+i] = pMesh->m_pPositions[(j = pVtxMap[pCntVtxList[i>>1]])+nVtx];
			pos = vector2df((pMesh->m_pPositions[j]-m_center)*m_R) + vector2df(pVtxEdge[pCntVtxList[i>>1]][i&1]*m_R).rot90cw()*(m_z[1]-m_z[0]);

			Vec2 Coord(ZERO);
			for (j = 0; j < 3; j++) 
			{
				k = (m_ptRef[1][inc_mod3[j]]-pos ^ m_ptRef[1][dec_mod3[j]]-pos)*m_refArea[1];
				
				Coord += m_texRef[1][j].GetUV() * k;
			}

			pMesh->m_pTexCoord[(nVtx+nCntVtx)*2+i] = SMeshTexCoord(Coord);//pVtxMap[pCntVtxList[i>>1]]];
		}

		for(i=0,bValid=1,itri=nTris,itriCnt=nTris*2; pIdx[i]>=0; i+=4,itri++) 
		{
			for(j=0;j<3;j++)
				pMesh->m_pIndices[itri*3+2-j] = pVtxMap[pIdx[i+j]]+nVtx;
			for(j=0;j<3;j++) if (!(pIdx[i+3] & 1<<j))
			{
				triplet<vtx_idx>(pMesh->m_pIndices+itriCnt++*3).set(pVtxMap[pIdx[i+inc_mod3[j]]], pVtxMap[pIdx[i+j]], pVtxMap[pIdx[i+j]]+nVtx);
				triplet<vtx_idx>(pMesh->m_pIndices+itriCnt++*3).set(pVtxMap[pIdx[i+j]]+nVtx, pVtxMap[pIdx[i+inc_mod3[j]]]+nVtx, pVtxMap[pIdx[i+inc_mod3[j]]]);
			}
		}
	}

	for(i=0,bValid=1; pIdx[i]>=0; i+=4) 
	{
		n = pMesh->m_pPositions[pVtxMap[pIdx[i+1]]]-pMesh->m_pPositions[pVtxMap[pIdx[i]]] ^ 
				pMesh->m_pPositions[pVtxMap[pIdx[i+2]]]-pMesh->m_pPositions[pVtxMap[pIdx[i]]];
		bValid &= isnonneg(n*axis);
	}
	if (pEffect && pIdx[nTris*4]!=-2)
		for(i=0;pIdx[i]>=0;i+=4) for(j=0;j<3;j++) if (!(pIdx[i+3] & 9<<j))
			pEffect->Spawn( false, IParticleEffect::ParticleLoc( 
				mtxWorld*(pMesh->m_pPositions[pVtxMap[pIdx[i+j]]]*(2.0f/3)+pMesh->m_pPositions[pVtxMap[pIdx[i+inc_mod3[j]]]]*(1.0f/3)),
				direff) );

	pIdx += nTris*4;

	if (*pIdx!=-2 && !bValid)
	{
		pStatObj->Release();
		delete[] pCntVtxList; delete[] pVtxEdge;
		delete[] pCntVtxMap;  delete[] (Vec3*)pVtxMap;
		return 0;
	}

	pIdxMesh->SetSubSetCount(1);
	pIdxMesh->SetSubsetMaterialId(0, m_matSubindex);
	pIdxMesh->SetSubsetMaterialProperties(0, m_matFlags, PHYS_GEOM_TYPE_DEFAULT);
	pIdxMesh->SetSubsetIndexVertexRanges(0, 0, pMesh->GetIndexCount(), 0, pMesh->GetVertexCount());
	pIdxMesh->SetSubsetBounds(
		0,
		m_R*Vec3(bounds[0].x+bounds[1].x,bounds[0].y+bounds[1].y,0)*0.5f + m_center,
		sqrt_tpl(sqr(bounds[1].x-bounds[0].x)+sqr(bounds[1].y-bounds[0].y))*0.5f);

	if (bNoPhys && *pIdx!=-2)
	{
		Vec3 center = m_R*Vec3(bounds[0].x+bounds[1].x,bounds[0].y+bounds[1].y,0)*0.5f + m_center;
		const int vertexCount = pMesh->GetVertexCount();
		for(i=0;i<vertexCount;i++)
			pMesh->m_pPositions[i] -= center;
	}

	IGeomManager *pGeoman = gEnv->pPhysicalWorld->GetGeomManager();
	IGeometry *pGeom = 0;
	bool fail = false;
	if (*pIdx==-2 && m_bAxisAligned) // && m_bStatic)
	{
		SVoxGridParams params;
		primitives::grid *pgrid = m_pGrid->GetGridData();
		params.origin = m_R*Vec3(pgrid->origin.x,pgrid->origin.y,m_z[0]-(m_z[1]-m_z[0])*0.01f)+m_center;
		n = (m_R*Vec3((float)pgrid->size.x,(float)pgrid->size.y,2)).abs();
		params.size.Set(max(1,(int)(n.x+0.5f)), max(1,(int)(n.y+0.5f)), max(1,(int)(n.z+0.5f)));
		params.step = m_R*Vec3(pgrid->step.x,pgrid->step.y,(m_z[1]-m_z[0])*0.505f);
		params.origin.x += params.size.x*min(0.0f,params.step.x);
		params.origin.y += params.size.y*min(0.0f,params.step.y);
		params.origin.z += params.size.z*min(0.0f,params.step.z);
		params.step = params.step.abs();
		fail = !(pGeom = pGeoman->CreateMesh(pMesh->m_pPositions,pMesh->m_pIndices,0,0,pMesh->GetIndexCount()/3, mesh_VoxelGrid|mesh_no_vtx_merge|mesh_transient, 0,&params));
	} 
	else if (*pIdx==-2 || !bNoPhys)
		fail = !(pGeom = pGeoman->CreateMesh(pMesh->m_pPositions,pMesh->m_pIndices,0,0,pMesh->GetIndexCount()/3, mesh_AABB_plane_optimise|mesh_AABB_rotated|mesh_no_vtx_merge|mesh_multicontact0|mesh_transient, 0,4,8));

	if (!fail && (*pIdx==-2 || bNoPhys || pGeom->GetErrorCount()==0))
	{
		if (b2Sided)
		{
			if (!m_pGeom && fabs_tpl(m_z[1]-m_z[0]-m_thicknessOrg*2)>m_thicknessOrg*0.01f)
			{
				Vec3 zax = m_R.GetColumn(2);
				const int vertexCount = pMesh->GetVertexCount();
				for(i=0; i<vertexCount; i++)
				{
					float zorg = (pMesh->m_pPositions[i]-m_center)*zax;
					float znew = m_thicknessOrg*sgnnz(zorg);
					pMesh->m_pPositions[i] += zax*(znew-zorg);
				}
			}
			pIdx -= nTris*4;
			for(i=0,itri=nTris,itriCnt=nTris*2; pIdx[i]>=0; i+=4,itri++) for(j=0;j<3;j++) if (!(pIdx[i+3] & 1<<j))
			{
				triplet<vtx_idx>(pMesh->m_pIndices+itriCnt++*3).set((pCntVtxMap[pIdx[i+inc_mod3[j]]]+nVtx)*2, (pCntVtxMap[pIdx[i+j]]+nVtx)*2+1, 
					(pCntVtxMap[pIdx[i+j]]+nVtx+nCntVtx)*2+1);
				triplet<vtx_idx>(pMesh->m_pIndices+itriCnt++*3).set((pCntVtxMap[pIdx[i+j]]+nVtx+nCntVtx)*2+1, (pCntVtxMap[pIdx[i+inc_mod3[j]]]+nVtx+nCntVtx)*2, 
					(pCntVtxMap[pIdx[i+inc_mod3[j]]]+nVtx)*2);
			}
			pIdx += nTris*4;
		}
		pStatObj->SetFlags(STATIC_OBJECT_GENERATED);
		if (pGeom)
		{
			if (*pIdx==-2)
				pGeom->SetForeignData(this,1);
			phys_geometry *pPhysGeom;
			if (m_bStatic)
				(pPhysGeom = pGeoman->RegisterGeometry(0, m_matId))->pGeom = pGeom;	// this way phys mass properties are not computed
			else {
				pPhysGeom = pGeoman->RegisterGeometry(pGeom, m_matId);
				pGeom->Release();
			}
			pStatObj->SetPhysGeom(pPhysGeom);
		}
		pStatObj->Invalidate();
	}
	else
	{
		pStatObj->Release();
		pStatObj = 0;
	}
	delete[] pCntVtxList; delete[] pVtxEdge;
	delete[] pCntVtxMap;  delete[] (Vec3*)pVtxMap;
	
	return pStatObj;
}


int *CBreakablePlane::Break(const Vec3 &pthit, float r, vector2df *&ptout, int seed, float filterAng,float ry)
{
	return m_pGrid->BreakIntoChunks(vector2df((pthit-m_center)*m_R),r,ptout, m_maxPatchTris,m_jointhresh, seed, filterAng,ry);
}

float circ_segment_area(float r,float h) 
{
	float a=0.0f, b=1.0f;
	if (h<0.0f)
		h=-h, a=r*r*gf_PI, b=-1.0f;
	h = min(r*0.9999f, h);
	return a+b* (r*r*acos_tpl(h/r) - h*sqrt_tpl(r*r-h*h));
}
float _boxCircleIntersArea(const Vec2 &sz, const Vec2 &c, float r)
{
	if (r<=0.0f)
		return 0.0f;
	int i,sg;
	float h,a,b,area=gf_PI*r*r;
	Vec2 pt;
	for(i=0;i<2;i++) for(sg=-1;sg<=1;sg+=2)	if ((h=c[i]*sg+r-sz[i])>0)
		area -= circ_segment_area(r,r-h);
	for(sg=-1;sg<=1;sg+=2) for(i=-1;i<=1;i+=2) if ((pt=Vec2(sz.x*sg,sz.y*i)-c).GetLength2() < r*r) {
		a = sqrt_tpl(r*r-pt.y*pt.y)-pt.x*sg;
		b = sqrt_tpl(r*r-pt.x*pt.x)-pt.y*i;
		area += a*b*0.5f+circ_segment_area(r,sqrt_tpl(r*r-(a*a+b*b)*0.25f));
	}
	return area;
}

inline int qmax(int op1,int op2) { return op1 - (op1-op2 & (op1-op2)>>31); }
inline int qmin(int op1,int op2) { return op2 + (op1-op2 & (op1-op2)>>31); }
inline float qmin(float op1,float op2) { return (op1+op2-fabsf(op1-op2))*0.5f; }
inline float qmax(float op1,float op2) { return (op1+op2+fabsf(op1-op2))*0.5f; }

void CBreakablePlane::ExtractMeshIsland(const SExtractMeshIslandIn& in, SExtractMeshIslandOut& out)
{
	IStatObj*& pStatObj = out.pStatObj;
	pStatObj = in.pStatObj;
	IRenderMesh *pRndMesh = pStatObj->GetRenderMesh();
	phys_geometry *pPhysGeom = pStatObj->GetPhysGeom();
	mesh_data *pmd = (mesh_data*)pPhysGeom->pGeom->GetData();;
	IGeomManager *pGeoman = gEnv->pPhysicalWorld->GetGeomManager();
	int i,j,idxMin,ichunk,ivtxMin,ivtxMax,queue[64],ihead,itail,nNewTris,imat;

	static volatile int g_lockExtractMeshIsland = 0;
	WriteLock lock(g_lockExtractMeshIsland);
	out.islandCenter.zero(); 
	out.islandSize.zero();
	out.pIsleStatObj = 0;

	if ((unsigned)in.itriSeed<(unsigned)pmd->nTris && pmd->pMats[in.itriSeed]>=0 )
	{
		if (!(pStatObj->GetFlags() & STATIC_OBJECT_CLONE)) 
		{
			int surfaceTypesIds[MAX_SUB_MATERIALS];
			phys_geometry *pCloneGeom = pGeoman->RegisterGeometry(0, pPhysGeom->surface_idx, surfaceTypesIds,
																														in.pRenderMat->FillSurfaceTypeIds(surfaceTypesIds)); 
			pCloneGeom->origin=pPhysGeom->origin;
			pCloneGeom->q=pPhysGeom->q; 
			pCloneGeom->Ibody=pPhysGeom->Ibody;
			pCloneGeom->V=pPhysGeom->V;
			pCloneGeom->pGeom = pGeoman->CloneGeometry(pPhysGeom->pGeom);
			pStatObj = pStatObj->Clone(true,true,false);
			pStatObj->SetFlags(pStatObj->GetFlags() | STATIC_OBJECT_CLONE);
			pStatObj->SetPhysGeom(pCloneGeom);
			pStatObj->SetFilePath(in.pStatObj->GetFilePath());
			pmd = (mesh_data*)(pPhysGeom=pCloneGeom)->pGeom->GetData();
			pRndMesh = pStatObj->GetRenderMesh();
		}
		if (!pRndMesh)
			return;

		IRenderMesh::ThreadAccessLock lockrm(pRndMesh);

    strided_pointer<Vec3> pVtx;
		pVtx.data = (Vec3*)pRndMesh->GetPosPtr(pVtx.iStride, FSL_READ);
		vtx_idx *pIdx = pRndMesh->GetIndexPtr(FSL_SYSTEM_UPDATE);
		if(!pIdx || !pVtx.data)
			return;

		// Determine which polys are the island
		Vec3 center(ZERO);
		float r=0;
		queue[0] = in.itriSeed;
		imat = pmd->pMats[queue[0]];
		pmd->pMats[queue[0]] = -2;
		for(nNewTris=itail=ivtxMax=0,ihead=1,ivtxMin=pmd->nVertices; ihead!=itail && nNewTris<pmd->nTris; nNewTris++) {
			i = queue[itail]; itail = itail+1 & sizeof(queue)/sizeof(queue[0])-1;	
			for(j=0;j<3;j++) {
				if (pmd->pTopology[i].ibuddy[j]>=0 && pmd->pMats[pmd->pTopology[i].ibuddy[j]]==imat) {
					pmd->pMats[queue[ihead] = pmd->pTopology[i].ibuddy[j]] = -2;
					ihead = ihead+1 & sizeof(queue)/sizeof(queue[0])-1;
				}	
				if (in.bCreateIsle)
				{
					int ivtx = pIdx[pmd->pForeignIdx[i]*3+j];
					ivtxMin = qmin(ivtxMin, ivtx); ivtxMax = qmax(ivtxMax, ivtx);
					ivtx = pmd->pIndices[i*3+j];
					ivtxMin = qmin(ivtxMin, ivtx); ivtxMax = qmax(ivtxMax, ivtx);
					center += pmd->pVertices[ivtx];
				}
			}
		}

		if (in.bCreateIsle)
		{
			IIndexedMesh *pIdxMesh;
			IStatObj *pIsleStatObj = gEnv->p3DEngine->CreateStatObj();
			pIsleStatObj->SetFlags(pIsleStatObj->GetFlags() | STATIC_OBJECT_GENERATED);
			CMesh *pMesh = (pIdxMesh=pIsleStatObj->GetIndexedMesh())->GetMesh();
			assert(pMesh->m_pPositionsF16 == 0);
			pIsleStatObj->SetMaterial(pStatObj->GetMaterial());
			pIdxMesh->SetIndexCount(nNewTris*3);
			unsigned short *pNewIdx = new unsigned short[nNewTris*3];


			for(i=j=0;i<pmd->nTris;i++) if (pmd->pMats[i]==-2) {
				int ivtx,idx; 
				for(ivtx=0;ivtx<3;ivtx++)
					pNewIdx[j++]=pmd->pIndices[i*3+ivtx]-ivtxMin;
				for(ivtx=0;ivtx<3;ivtx++)
					pMesh->m_pIndices[j-3+ivtx]=pIdx[idx = pmd->pForeignIdx[i]*3+ivtx]-ivtxMin, pIdx[idx]=0;
				pmd->pMats[i] = -1;
				if (j>=nNewTris*3)
					break;
			}

			IGeometry *pIsleGeom = pGeoman->CreateMesh(pmd->pVertices+ivtxMin,pNewIdx,0,0,nNewTris, mesh_AABB_plane_optimise|mesh_AABB_rotated|mesh_multicontact1,0);
			delete[] pNewIdx;

			TRenderChunkArray &meshChunks = pRndMesh->GetChunks();
			idxMin = pmd->pForeignIdx[in.itriSeed]*3;
			for(ichunk=meshChunks.size()-1; ichunk>0 && (unsigned int)idxMin-meshChunks[ichunk].nFirstIndexId>=meshChunks[ichunk].nNumIndices; ichunk--);
			pIdxMesh->SetVertexCount(ivtxMax-ivtxMin+1);
			pIdxMesh->SetTexCoordCount(ivtxMax-ivtxMin+1);
			pIdxMesh->SetTangentCount(ivtxMax-ivtxMin+1);
			strided_pointer<Vec2> pTex;
			strided_pointer<SPipTangents> pTangs; 
			pTex.data    = (Vec2*)pRndMesh->GetUVPtr(pTex.iStride, FSL_READ);
			pTangs.data  = (SPipTangents*)pRndMesh->GetTangentPtr(pTangs.iStride, FSL_READ);

			for (i = 0, center *= (1.0f / (nNewTris * 3)); i <= ivtxMax - ivtxMin; i++) {
				r = qmax(r, (pVtx[i+ivtxMin]-center).len2());

				pMesh->m_pPositions[i] = pVtx[i + ivtxMin];
				pMesh->m_pTexCoord [i] = SMeshTexCoord(pTex[i + ivtxMin]);
				pMesh->m_pTangents [i] = SMeshTangents(pTangs[i + ivtxMin]);
				pMesh->m_pNorms    [i] = SMeshNormal(pMesh->m_pTangents[i].GetN());
			}
			
			pIdxMesh->SetSubSetCount(1);
			pIdxMesh->SetSubSetCount(1);
			pIdxMesh->SetSubsetIndexVertexRanges(0, 0, j, 0, ivtxMax-ivtxMin+1);
			pIdxMesh->SetSubsetMaterialId(0, meshChunks[ichunk].m_nMatID);
			pIdxMesh->SetSubsetMaterialProperties(0, meshChunks[ichunk].m_nMatFlags, PHYS_GEOM_TYPE_DEFAULT);
			pIdxMesh->SetSubsetBounds(0, center, r);
			pRndMesh->UnlockIndexStream();
			pIsleStatObj->SetPhysGeom(pPhysGeom = pGeoman->RegisterGeometry(pIsleGeom,in.idMat));
			pIsleGeom->Release();
			pIsleStatObj->Invalidate(false);
			out.pIsleStatObj = pIsleStatObj;
			return;
		}
		else
		{
			// Simply remove the polys from existing physics and stat obj
			// and compute a simple bounding box
			Vec3 ptmin(+1e6f),ptmax(-1e6);
			for(i=j=0;i<pmd->nTris;i++) if (pmd->pMats[i]==-2) {
				for(int ivtx=0;ivtx<3;ivtx++)
				{
					int idx = pmd->pForeignIdx[i]*3+ivtx;
					const Vec3& vert = pVtx[pIdx[idx]];
					ptmin.x=min(ptmin.x,vert.x); ptmin.y=min(ptmin.y,vert.y); ptmin.z=min(ptmin.z,vert.z);
					ptmax.x=max(ptmax.x,vert.x); ptmax.y=max(ptmax.y,vert.y); ptmax.z=max(ptmax.z,vert.z);
					pIdx[idx]=0;
				}
				pmd->pMats[i] = -1;
				if (j>=nNewTris*3)
					break;
			}
			out.islandCenter = (ptmax+ptmin)*0.5f;
			out.islandSize = (ptmax-ptmin)*0.5f;
			out.pIsleStatObj = 0;
			pRndMesh->UnlockIndexStream();
			return;
		}
	}
	out.pIsleStatObj = 0;
}


int CBreakablePlane::ProcessImpact(const SProcessImpactIn& in, SProcessImpactOut& out)
{
	FUNCTION_PROFILER(GetISystem(), PROFILE_ENTITY);

	out.pStatObjNew = 0;
	out.pStatObjAux = in.pStatObjAux;

	IStatObj *pNewStatObj=0;
	IStatObj *pStatObj = in.pStatObj;
 	IStatObj *pIsleStatObj = in.pStatObj;
	IStatObj *pStatObjBase = 0;
	phys_geometry *pPhysGeom;
	float lifetime=5,filterAng=0.0f,ry=0.0f,curFracture;
	vector2df *ptout,bounds[2];
	IEntitySystem *pEntitySystem = gEnv->pEntitySystem;
	IEntityClass *pClass = 0;
	SEntitySpawnParams params;
	IEntity *pEntity;
	IParticleEffect *pEffect=0;
	IParticleEmitter* pEmitter=0;
	ISurfaceType::SBreakable2DParams *pBreak2DParams = in.pMat->GetBreakable2DParams();
	if (!pBreak2DParams || !pStatObj->GetRenderMesh())
	{
		out.pStatObjNew = pStatObj;
		return eProcessImpact_Done;
	}
	const char *strEffectName;
	SEntityPhysicalizeParams epp;
	pe_params_particle ppp;
	pe_action_set_velocity asv;
	pe_action_impulse ai;
	Matrix33 mtxrot = Matrix33(in.mtx);
	Vec3 size,pos0,rhit;
	bool bRigidBody=true, bUseImpulse, bUseEdgeAlpha=false;
	params.vPosition = pos0 = in.mtx.GetTranslation();
	params.vScale.x = params.vScale.y = params.vScale.z = mtxrot.GetRow(0).len();
	mtxrot.OrthonormalizeFast();
	params.qRotation = Quat(mtxrot);
	epp.type = PE_RIGID;
	ai.point = in.pthit;
	ppp.areaCheckPeriod = 0;
	int bStatic = 1^iszero(int32(in.processFlags & ePlaneBreak_Static));
	int bAutoSmash = 1^iszero(int32(in.processFlags & ePlaneBreak_AutoSmash));
	int bFractureEffect = iszero(int32(in.processFlags & ePlaneBreak_NoFractureEffect));
	IParticleEffect *pFractureFx = 0;
	const float scale = in.mtx.GetColumn0().len();
	if (!in.bLoading)
	{
		union {const float* pF; const int* pI;} u; u.pF = &in.pthit.x;
		out.eventSeed = u.pI[0]+u.pI[1]+u.pI[2] & 0x7FFFFFFF;
	}
	else
	{
		out.eventSeed = in.eventSeed;
	}

	cry_random_seed(gEnv->bNoRandomSeed?0:out.eventSeed);

	static ICVar *particle_limit=gEnv->pConsole->GetCVar("g_breakage_particles_limit");
	int nMaxParticles, nCurParticles=gEnv->pPhysicalWorld->GetEntityCount(PE_PARTICLE), icount=0,mask;
	float curTime = gEnv->pTimer->GetCurrTime();
	nMaxParticles = particle_limit ? particle_limit->GetIVal():200;
	if (curTime > g_maxPieceLifetime)
		g_nPieces = 0;
	nCurParticles = min(g_nPieces, nCurParticles);

	EProcessImpactResult result = eProcessImpact_Done;

	if (pPhysGeom=pStatObj->GetPhysGeom()) 
	{
		mesh_data *pmd;
		IRenderMesh *pRndMesh;
		bool bMeshPrepOnly = false;
		if (bStatic && pPhysGeom->pGeom->GetType()==GEOM_TRIMESH && 
				(pmd = (mesh_data*)pPhysGeom->pGeom->GetData()) && pmd->pMats && pmd->nIslands>1 && pmd->pForeignIdx &&
				(pRndMesh=pStatObj->GetRenderMesh()) && 
				pRndMesh->GetVerticesCount()==pmd->nVertices && pRndMesh->GetIndicesCount()==pmd->nTris*3
				)
		{
			Matrix34 mtxInv = in.mtx.GetInverted();
			Vec3 ptHit=mtxInv * in.pthit;
			Vec3 dirHit=mtxInv.TransformVector(in.hitvel.normalized());
			SExtractMeshIslandOut islandOut;

			SExtractMeshIslandIn islandIn;
			islandIn.pStatObj = pStatObj;
			islandIn.bCreateIsle = bAutoSmash^1;
			islandIn.idMat = in.pMat->GetId();
			islandIn.itriSeed = in.itriHit;
			islandIn.pRenderMat = in.pRenderMat;
			islandIn.processFlags = in.processFlags;

			ExtractMeshIsland(islandIn, islandOut);

			pIsleStatObj = islandOut.pIsleStatObj;
			pStatObj = islandOut.pStatObj;

			if (bAutoSmash && pIsleStatObj==NULL)
			{
				if (bFractureEffect && pBreak2DParams->full_fracture_fx[0] && 
						(pFractureFx = gEnv->pParticleManager->FindEffect(pBreak2DParams->full_fracture_fx)))
				{
					// NB: the islandSize is only approximate because it might not be orientated to the mesh basis
					float effectSize = max(islandOut.islandSize.x, islandOut.islandSize.y);
					if (effectSize>0.f)
					{
						Vec3 n = in.hitnorm*(float)-sgnnz(in.hitnorm*in.hitvel);
						Vec3 p = in.mtx*islandOut.islandCenter;
						pFractureFx->Spawn(false, IParticleEffect::ParticleLoc(p, n, scale*effectSize));
					}
				}	
				out.pStatObjNew = pStatObj;
				return eProcessImpact_Done;
			}

			if (!pIsleStatObj)
			{
				out.pStatObjNew = in.pStatObj;
				return eProcessImpact_Done;
			}
			pStatObjBase = pStatObj;
			pStatObj = pIsleStatObj;
			pPhysGeom = pIsleStatObj->GetPhysGeom();
		}

		primitives::box bbox;
		pPhysGeom->pGeom->GetBBox(&bbox);
		int iup = idxmin3((float*)&bbox.size);

		const float approxArea = bbox.size[dec_mod3[iup]] * bbox.size[inc_mod3[iup]] * scale * scale;
		if (approxArea < in.glassAutoShatterMinArea)
			bAutoSmash = 1;

		CBreakablePlane *pPlane = (CBreakablePlane*)pPhysGeom->pGeom->GetForeignData(1);
		pPhysGeom->pGeom->SetForeignData(0,1);
		if (!pPlane && min(bbox.size[inc_mod3[iup]], bbox.size[dec_mod3[iup]]) < bbox.size[iup]*4)
		{
			if (!in.bVerify)
				CryLog("[Breakable2d] : geometry is not suitable for breaking, breaking denied ( %s %s)", in.pStatObj->GetFilePath(), in.pStatObj->GetGeoName());
			if (pStatObj!=in.pStatObj)
				pStatObj->Release();
			if (pStatObjBase && pStatObjBase != in.pStatObj)
				pStatObjBase->Release();
			out.pStatObjNew = in.pStatObj;
			return eProcessImpact_BadGeometry;
		}
		curFracture = pPlane ? pPlane->m_pGrid->GetFracture() : 0.0f;
		bRigidBody = pBreak2DParams->rigid_body != 0;
		if (!bRigidBody && nCurParticles>=nMaxParticles && pBreak2DParams->no_procedural_full_fracture)
			curFracture = 10.0f;

		float r = pPlane || pStatObj->GetFlags() & STATIC_OBJECT_GENERATED ? pBreak2DParams->blast_radius : pBreak2DParams->blast_radius_first;
		if (r<=0)
			bAutoSmash = 0;
		lifetime = pBreak2DParams->life_time;
		strEffectName = pBreak2DParams->particle_effect;
		if (bFractureEffect && strEffectName[0])
			pEffect = gEnv->pParticleManager->FindEffect(strEffectName);
		bUseEdgeAlpha = pBreak2DParams->use_edge_alpha != 0;
		ry = r*(1.0f+cry_random(0.0f, pBreak2DParams->vert_size_spread));
		ry = max(ry,in.hitradius);
		r = max(r,in.hitradius);
		out.hitradius = r;
		filterAng = pBreak2DParams->filter_angle*gf_PI*(1.0f/180);
		Vec2 c=Vec2((bbox.Basis*(in.mtx.GetInverted()*in.pthit-bbox.center)).GetPermutated(iup)), sz=Vec2(bbox.size.GetPermutated(iup));
		if (bAutoSmash || _boxCircleIntersArea(sz,c,r/scale)/(sz.x*sz.y*4) + curFracture >= pBreak2DParams->max_fracture)
		{
			r=ry = 1e5f;
			if (bFractureEffect && pBreak2DParams->full_fracture_fx[0] && 
					(pFractureFx = gEnv->pParticleManager->FindEffect(pBreak2DParams->full_fracture_fx)))
			{
				Vec3 n = in.mtx.TransformVector(bbox.Basis.GetRow(iup));
				pFractureFx->Spawn(false, IParticleEffect::ParticleLoc(in.mtx*bbox.center, n*(float)-sgnnz(n*in.hitvel), 
					scale*max(sz.x,sz.y)));
			}	
			if (bAutoSmash || pBreak2DParams->no_procedural_full_fracture || curFracture>9.0f)
			{
				out.hitradius = 0.0f;
				if (pPlane)
					delete pPlane;
				if (pStatObjBase && pIsleStatObj!=pStatObjBase)
					pIsleStatObj->Release();
				out.pStatObjNew = pStatObjBase;
				return eProcessImpact_Done;
			}
		}
		if (bFractureEffect && !pFractureFx && pBreak2DParams->fracture_fx[0] && 
				(pFractureFx = gEnv->pParticleManager->FindEffect(pBreak2DParams->fracture_fx)))
		{
			Vec3 n = in.mtx.TransformVector(bbox.Basis.GetRow(iup));
			pFractureFx->Spawn(false, IParticleEffect::ParticleLoc(in.pthit, n*(float)-sgnnz(n*in.hitvel), scale));
		}

		if (r<=0)
		{
			if (!(pStatObj->GetFlags() & (STATIC_OBJECT_CLONE|STATIC_OBJECT_GENERATED)))
			{
				pStatObj = pStatObj->Clone(true,true,false);
				pStatObj->SetFlags(pStatObj->GetFlags() | STATIC_OBJECT_GENERATED);
			}
			pNewStatObj = pStatObj;
			int i,nMatID=-1;
			if (IIndexedMesh *pIdxMesh = pStatObj->GetIndexedMesh())
			{
				for(i=pIdxMesh->GetSubSetCount()-1; i>0 && !pIdxMesh->GetSubSet(i).nNumIndices; i--);
				nMatID = UpdateBrokenMat(in.pRenderMat, pIdxMesh->GetSubSet(i).nMatID, pBreak2DParams->broken_mtl);
				pIdxMesh->SetSubsetMaterialId(i, nMatID);
			}
			if (pRndMesh = pStatObj->GetRenderMesh())
			{
				TRenderChunkArray &chunks = pRndMesh->GetChunks();
				for(i=chunks.size()-1; i>0 && !chunks[i].nNumIndices; i--);
				chunks[i].m_nMatID = nMatID>=0 ? nMatID : UpdateBrokenMat(in.pRenderMat, chunks[i].m_nMatID, pBreak2DParams->broken_mtl);
				pRndMesh->SetChunk(i, chunks[i]);
			}
		}	else
		if (!pPlane)
		{
			pPlane = new CBreakablePlane;
			pPlane->m_cellSize = pBreak2DParams->cell_size;
			pPlane->m_maxPatchTris = pBreak2DParams->max_patch_tris;
			pPlane->m_density = pBreak2DParams->shard_density;
			strcpy(pPlane->m_mtlSubstName, pBreak2DParams->broken_mtl);
			if (pPlane->SetGeometry(pStatObj,in.pRenderMat,bStatic,out.eventSeed))
			{
				if (in.bVerify)
				{
					delete pPlane;
					pPlane = 0;
				}
				else
				{
					pPlane->m_matId = in.pMat->GetId();
					if (in.pRenderMat)
						pPlane->m_pMat = in.pRenderMat;
				}
			}
			else
			{
				delete pPlane;
				pPlane = 0;
				result = eProcessImpact_BadGeometry;
			}
		}
		else
			pPlane->m_bStatic = bStatic;

		if (in.bVerify)
			return result;

		int *pIdx,*pIdx0;
		if (pPlane && (pIdx0=pIdx = pPlane->Break(in.mtx.GetInverted()*in.pthit,r,ptout,out.eventSeed,filterAng,ry)))
		{
			epp.density = pPlane->m_density;
			if (pPlane->m_density<=0)
				ppp.iPierceability = -1;
			do 
			{
				if (pNewStatObj = pPlane->CreateFlatStatObj(pIdx,ptout,bounds,in.mtx,pEffect,!bRigidBody,bUseEdgeAlpha))
				{
					pNewStatObj->SetFilePath(pStatObj->GetFilePath());
					if (*pIdx==-1)
					{ // create a new entity for this stat obj
						if (!pClass)
							pClass = pEntitySystem->GetClassRegistry()->FindClass("Default");
						params.sName = "breakable_plane_piece";
						params.pClass = pClass;
						params.nFlags = ENTITY_FLAG_CLIENT_ONLY|ENTITY_FLAG_NO_PROXIMITY;
						params.id = 0;
						Vec3 center = pPlane->m_R*Vec3(bounds[0].x+bounds[1].x,bounds[0].y+bounds[1].y,0)*0.5f + pPlane->m_center;
						rhit = in.pthit-in.mtx*center;
						asv.v = in.hitvel*0.5f;
						ai.impulse = in.hitvel*in.hitmass;
						if (!bRigidBody)
						{
							epp.type = PE_PARTICLE;
							epp.pParticle = &ppp;
							ppp.size = max(bounds[1].x-bounds[0].x,bounds[1].y-bounds[0].y);
							ppp.thickness = pPlane->m_z[1]-pPlane->m_z[0];
							ppp.surface_idx = pPlane->m_matId;
							ppp.mass = ppp.size*ppp.thickness*pPlane->m_density;
							if ((bUseImpulse = in.hitmass<ppp.mass) && ai.impulse.len2() > sqr(ppp.mass*10.0f))
								ai.impulse = ai.impulse.normalized()*ppp.mass*10.0f;
							ppp.flags = particle_no_path_alignment|particle_no_roll|pef_traceable|particle_no_self_collisions;
							ppp.normal = pPlane->m_R*Vec3(0,0,1);
							ppp.q0 = params.qRotation;
							params.vPosition = pos0+params.qRotation*(center*params.vScale.x);
						}	else
							bUseImpulse = in.hitmass<0.2f;
						asv.w = (rhit/rhit.len2())^in.hitvel;

						if (!bRigidBody && lifetime>0)
						{
							if (nCurParticles*7>nMaxParticles)
								mask = 7;
							else if (nCurParticles*6>nMaxParticles)
								mask = 3;
							if (nCurParticles*4>nMaxParticles)
								mask = 1;
							else
								mask = 0;
							if (in.bLoading || ++icount & mask) {
								pNewStatObj->Release();
								continue;
							}
							ParticleParams pp;
							pp.fCount = 0;
							pp.fParticleLifeTime.Set( lifetime, 0.25f );
							pp.bRemainWhileVisible = false;
							pp.fSize = params.vScale.x;
							pp.ePhysicsType = pp.ePhysicsType.SimplePhysics;
							pe_params_pos ppos;
							ppos.pos = params.vPosition;
							ppos.q = params.qRotation;
							IPhysicalEntity* pent = gEnv->pPhysicalWorld->CreatePhysicalEntity(PE_PARTICLE,&ppos);
							pent->SetParams(&ppp);
							pent->Action(bUseImpulse ? (pe_action*)&ai:&asv);
							pNewStatObj->AddRef();
							if (!pEmitter)
								pEmitter = gEnv->pParticleManager->CreateEmitter(Matrix34(Vec3(1.0f),params.qRotation,params.vPosition),pp,ePEF_Independent);
							if (pEmitter)
								pEmitter->EmitParticle(pNewStatObj, pent);
							if (pNewStatObj->Release()<=0)
								gEnv->pPhysicalWorld->DestroyPhysicalEntity(pent);
							nCurParticles++;
							g_nPieces++; g_maxPieceLifetime = max(g_maxPieceLifetime, curTime+lifetime);
						} 
						else
						{
							pEntity = pEntitySystem->SpawnEntity(params);
							if (!in.bLoading)
								in.addChunkFunc(pEntity->GetId());
							pEntity->CreateProxy(ENTITY_PROXY_RENDER);
							pEntity->SetStatObj(pNewStatObj, 0,false);
							//if (!bRigidBody)
							//	pEntity->SetSlotLocalTM(0, Matrix34::CreateTranslationMat(-center));
							pEntity->Physicalize(epp);
							if (pEntity->GetPhysics())
								pEntity->GetPhysics()->Action(bUseImpulse ? (pe_action*)&ai:&asv);
						}
						pNewStatObj=0; 
					}
				}
			}	while(*pIdx++!=-2);
			gEnv->pPhysicalWorld->GetPhysUtils()->DeletePointer(pIdx0);
		}
		if (pPlane && !pNewStatObj)
			delete pPlane;
	}

	if (pStatObjBase)
	{
		// An Island Extraction took place, pStatObjBase = Remaining unbroken piece
		// pStatObjAux = new broken glass pieces
		if (pStatObj && pStatObj!=pNewStatObj)
			pStatObj->Release();
		out.pStatObjNew = pStatObjBase;
		out.pStatObjAux = pNewStatObj;
	}
	else
	{
		// Otherwise pNewStatObj is the new broken glass
		out.pStatObjNew = pNewStatObj;
		out.pStatObjAux = in.pStatObjAux;
	}
	return result;
}
