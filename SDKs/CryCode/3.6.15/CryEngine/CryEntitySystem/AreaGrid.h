////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2007.
// -------------------------------------------------------------------------
//  File name:   AreaGrid.h
//  Version:     v1.00
//  Created:     08/03/2007 by Michael Smith.
//  Compilers:   Visual Studio.NET 2005
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////
#ifndef __AREAGRID_H__
#define __AREAGRID_H__

class CArea;
class CAreaGrid;

typedef std::vector<CArea*>		TAreaPointers;
typedef std::vector<EntityId>	TEntityIDs;

class CAreaGrid
{
public:

	CAreaGrid();
	~CAreaGrid();

	bool ResetArea(CArea* pArea);
	void Compile(CEntitySystem* pEntitySystem, TAreaPointers const& rAreas);
	void Reset();
	TAreaPointers const& GetAreas(Vec3 const& position);
	void Draw();
	
	size_t GetNumAreas() const { return (m_papAreas != NULL) ? m_papAreas->size() : 0; }

protected:
	
	TAreaPointers const& GetAreas(int x, int y);
	bool GetAreaIndex(CArea const* const pArea, size_t& nIndexOut);
	void AddAreaBit(const Vec2i& start, const Vec2i& end, uint32 areaIndex);
	void RemoveAreaBit(uint32 areaIndex);
	void AddArea(CArea* pArea, uint32 areaIndex);
	void ClearAllBits();
	void ClearTmpAreas() { m_apAreasTmp.clear(); }
	
	float GetGreatestFadeDistance(CArea const* const pArea);

#ifndef _RELEASE
	void Debug_CheckBB(Vec2 const& vBBCentre, Vec2 const& vBBExtent, CArea const* const pArea);
#endif

	static const uint32 GRID_CELL_SIZE;
	static const float GRID_CELL_SIZE_R;

	CEntitySystem* m_pEntitySystem;

	uint32 m_nCells;
	
	uint32* m_pbitFieldX;         // start of the X bit field
	uint32* m_pbitFieldY;         // start of the Y bit field
	uint32 m_bitFieldSizeU32;     // number of u32s per cell
	uint32 m_maxNumAreas;         // maximum number of areas compiled into the grid
	Vec2i (*m_pAreaBounds)[2];    // Points to area bounds in the bit field array
	
	// Points to area pointers array in AreaManager.
	TAreaPointers const* m_papAreas;

	// Holds temporary pointers to areas, populated when GetAreas() is called.
	TAreaPointers m_apAreasTmp;
};



#endif //__AREAGRID_H__
