/*************************************************************************
"BeatGames" Source File.
Copyright (C), BeatGames, 2014-2015
-------------------------------------------------------------------------
-------------------------------------------------------------------------
History:

- 25.03.2015   13:49 : Created by AfroStalin(chernecoff)
- 07.04.2015   01:26 : Edited by AfroStalin(chernecoff)
-------------------------------------------------------------------------

*************************************************************************/

#include "StdAfx.h"
#include "Nodes/G2FlowBaseNode.h"

enum EFoundationSides
{
	EFoundationSideA = 0,
	EFoundationSideB,
	EFoundationSideC,
	EFoundationSideD,
};

float CalculateDistance(Vec2 a, Vec2 b)
{
	float distance, x, y;

	x = a.x - b.x; 
	y = a.y - b.y;

	if(x<0 && y<0) distance = -(x) - y;
	if(x>0 && y>0) distance = x + y;
	if(x<0 && y>0) distance = -(x) + y;
	if(x>0 && y<0) distance = x - y;

	return distance;
}

class CFlowNode_AlignToTerrain: public CFlowBaseNode<eNCT_Instanced>
{
	enum INPUTS
	{
		EIP_Calculate = 0,
		EIP_WorldPos,
	};

	enum OUTPUTS
	{
		EOP_TerrainPos = 0,
	};

public:
	CFlowNode_AlignToTerrain( SActivationInfo * pActInfo )
	{
	}

	~CFlowNode_AlignToTerrain()
	{
	}

	IFlowNodePtr Clone(SActivationInfo* pActInfo)
	{
		return new CFlowNode_AlignToTerrain(pActInfo);
	}

	virtual void GetMemoryUsage(ICrySizer * s) const
	{
		s->Add(*this);
	}

	void GetConfiguration( SFlowNodeConfig& config )
	{
		static const SInputPortConfig in_ports[] = 
		{
			InputPortConfig_Void( "Calculate", _HELP("Calculate terrain position")),
			InputPortConfig<Vec3>( "StartPosition", _HELP("Spawn position (hitpoint on raycast camera)")),
			{0}
		};
		static const SOutputPortConfig out_ports[] = 
		{
			OutputPortConfig<Vec3>("AlignPosition", _HELP("Compleate terrain position")),
			{0}
		};
		config.pInputPorts = in_ports;
		config.pOutputPorts = out_ports;
		config.sDescription = _HELP("Align object to terrain");
		config.SetCategory(EFLN_APPROVED);
	}

	void ProcessEvent( EFlowEvent event, SActivationInfo *pActInfo )
	{
		switch (event)
		{
		case eFE_Initialize:
			{
				m_actInfo = *pActInfo;
			}
			break;
		case eFE_Activate:
			{
				if(IsPortActive(pActInfo, EIP_Calculate))
				{
					ray_hit hit;
					Vec3 origin = GetPortVec3(pActInfo,EIP_WorldPos);
					Vec3 dir = Vec3(0, 0, -1);
					int numHits = gEnv->pPhysicalWorld->RayWorldIntersection(origin,dir,ent_terrain,rwi_stop_at_pierceable|rwi_colltype_any,&hit, 1);

					if(numHits > 0)
					{
						Vec3 terrainPoint = hit.pt;
						ActivateOutput(pActInfo,EOP_TerrainPos,terrainPoint);
					}
				}
			}
			break;
		}
	}

protected:
	SActivationInfo m_actInfo;
};

class CFlowNode_RemoveEntity: public CFlowBaseNode<eNCT_Instanced>
{
	enum INPUTS
	{
		EIP_Remove = 0,
		EIP_EntityId,
	};

	enum OUTPUTS
	{
	};

public:
	CFlowNode_RemoveEntity( SActivationInfo * pActInfo )
	{
	}

	~CFlowNode_RemoveEntity()
	{
	}

	IFlowNodePtr Clone(SActivationInfo* pActInfo)
	{
		return new CFlowNode_RemoveEntity(pActInfo);
	}

	virtual void GetMemoryUsage(ICrySizer * s) const
	{
		s->Add(*this);
	}

	void GetConfiguration( SFlowNodeConfig& config )
	{
		static const SInputPortConfig in_ports[] = 
		{
			InputPortConfig_Void( "Remove", _HELP("Remove entity")),
			InputPortConfig<EntityId>( "EntityId", _HELP("Entity id")),
			{0}
		};
		static const SOutputPortConfig out_ports[] = 
		{
			{0}
		};
		config.pInputPorts = in_ports;
		config.pOutputPorts = out_ports;
		config.sDescription = _HELP("Remove enity");
		config.SetCategory(EFLN_APPROVED);
	}

	void ProcessEvent( EFlowEvent event, SActivationInfo *pActInfo )
	{
		switch (event)
		{
		case eFE_Initialize:
			{
				m_actInfo = *pActInfo;
			}
			break;
		case eFE_Activate:
			{
				if(IsPortActive(pActInfo, EIP_Remove))
				{
					EntityId id = GetPortEntityId(pActInfo,EIP_EntityId);

					if(id)
					{
						gEnv->pEntitySystem->RemoveEntity(id,true);
					}		
				}
			}
			break;
		}
	}

protected:
	SActivationInfo m_actInfo;
};

class CFlowNode_CheckCollision: public CFlowBaseNode<eNCT_Instanced>
{
	enum INPUTS
	{
		EIP_Check = 0,
		EIP_Entity,
	};

	enum OUTPUTS
	{
		EOP_True = 0,
		EOP_False,
		EOP_IntersectionEntity,
	};

public:
	CFlowNode_CheckCollision( SActivationInfo * pActInfo )
	{
	}

	~CFlowNode_CheckCollision()
	{
	}

	IFlowNodePtr Clone(SActivationInfo* pActInfo)
	{
		return new CFlowNode_CheckCollision(pActInfo);
	}

	virtual void GetMemoryUsage(ICrySizer * s) const
	{
		s->Add(*this);
	}

	void GetConfiguration( SFlowNodeConfig& config )
	{
		static const SInputPortConfig in_ports[] = 
		{
			InputPortConfig_Void( "Check", _HELP("Check collision")),
			InputPortConfig<EntityId>( "Entity", _HELP("Enitity")),
			{0}
		};
		static const SOutputPortConfig out_ports[] = 
		{
			OutputPortConfig<bool>("True", _HELP("Entity intersection with some objects")),
			OutputPortConfig<bool>("False", _HELP("Entity not intersection with some objects")),
			OutputPortConfig<EntityId>( "IntersectionEntity", _HELP("Intersection enitity")),
			{0}
		};
		config.pInputPorts = in_ports;
		config.pOutputPorts = out_ports;
		config.sDescription = _HELP("Check collision entitys");
		config.SetCategory(EFLN_APPROVED);
	}

	void ProcessEvent( EFlowEvent event, SActivationInfo *pActInfo )
	{
		switch (event)
		{
		case eFE_Initialize:
			{
				m_actInfo = *pActInfo;
			}
			break;
		case eFE_Activate:
			{
				if(IsPortActive(pActInfo, EIP_Check))
				{
					EntityId id = GetPortEntityId(pActInfo,EIP_Entity);		
					IEntity* pEntity = gEnv->pEntitySystem->GetEntity(id);
					IPhysicalEntity *pIgnore = pEntity->GetPhysics();
					geom_contact *contacts;

					AABB bb;
					pEntity->GetWorldBounds(bb);


					primitives::cylinder cylinder;
					cylinder.center = pEntity->GetPos();
					cylinder.hh = bb.GetSize().z;
					cylinder.r = bb.GetRadius();

					intersection_params params;
					params.bStopAtFirstTri = true;
					params.bNoBorder = true;
					params.bNoAreaContacts = true;

					int rayFlags = rwi_stop_at_pierceable;
					int objTypes = ent_independent | ent_living | ent_sleeping_rigid | ent_rigid | ent_static;

					gEnv->pPhysicalWorld->PrimitiveWorldIntersection(primitives::cylinder::type, &cylinder, Vec3(ZERO), objTypes, &contacts, 0,	3 , &params, 0, 0, &pIgnore, pIgnore?1:0);
	
					if(contacts)
					{
						IPhysicalEntity *pCollider = gEnv->pPhysicalWorld->GetPhysicalEntityById(contacts->iPrim[0]);
						IEntity *pNewEntity = gEnv->pEntitySystem->GetEntityFromPhysics(pCollider);

						if(contacts->iPrim[0] > 0)
						{
							ActivateOutput(pActInfo,EOP_True, true);
							ActivateOutput(pActInfo,EOP_IntersectionEntity, pNewEntity ? pNewEntity->GetId() : 0);
						}
					}
					else
						ActivateOutput(pActInfo,EOP_False, false);
				}
			}
			break;
		}
	}

protected:
	SActivationInfo m_actInfo;
};

class CFlowNode_PlaceEntity: public CFlowBaseNode<eNCT_Instanced>
{
	enum INPUTS
	{
		EIP_Place = 0,
		EIP_EntityClass,
		EIP_Intersection,
		EIP_IntersectionEntity,
		EIP_Pos,
		EIP_Rot,
	};

	enum OUTPUTS
	{
		EOP_Success = 0,
		EOP_Fail,
	};

public:
	CFlowNode_PlaceEntity( SActivationInfo * pActInfo )
	{
	}

	~CFlowNode_PlaceEntity()
	{
	}

	IFlowNodePtr Clone(SActivationInfo* pActInfo)
	{
		return new CFlowNode_PlaceEntity(pActInfo);
	}

	virtual void GetMemoryUsage(ICrySizer * s) const
	{
		s->Add(*this);
	}

	void GetConfiguration( SFlowNodeConfig& config )
	{
		static const SInputPortConfig in_ports[] = 
		{
			InputPortConfig_Void( "Place", _HELP("Try place entity")),
			InputPortConfig<string>( "EntityClass", _HELP("Enitity class")),
			InputPortConfig<bool>( "Intersection", _HELP("Entity intersection with some entitys?")),
			InputPortConfig<EntityId>( "IntersectionEntity", _HELP("Intersection enitity id")),
			InputPortConfig<Vec3>( "Pos", _HELP("Position")),
			InputPortConfig<Vec3>( "Rot", _HELP("Rotation")),
			{0}
		};
		static const SOutputPortConfig out_ports[] = 
		{
			OutputPortConfig_Void("Success", _HELP("If entity sucess placed")),
			OutputPortConfig_Void("Fail", _HELP("If entity failed placed")),
			{0}
		};
		config.pInputPorts = in_ports;
		config.pOutputPorts = out_ports;
		config.sDescription = _HELP("Try placing entity");
		config.SetCategory(EFLN_APPROVED);
	}

	void ProcessEvent( EFlowEvent event, SActivationInfo *pActInfo )
	{
		switch (event)
		{
		case eFE_Initialize:
			{
				m_actInfo = *pActInfo;
			}
			break;
		case eFE_Activate:
			{
				if(IsPortActive(pActInfo, EIP_Place))
				{
					if(!GetPortBool(pActInfo,EIP_Intersection))
					{
						string className(GetPortString(pActInfo,EIP_EntityClass));

						EntityId id = GetPortEntityId(pActInfo,EIP_IntersectionEntity);	
						IEntity* pIntersectionEntity = gEnv->pEntitySystem->GetEntity(id);

						Vec3 pos = GetPortVec3(pActInfo,EIP_Pos);
						Vec3 rot = GetPortVec3(pActInfo,EIP_Rot);
						Vec3 scale = Vec3(1,1,1);



						SEntitySpawnParams params;
						params.pClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass(className.c_str());
						params.vPosition = pos;
						params.vScale = scale;

						Matrix33 mat;
						Ang3 ang(DEG2RAD(rot.x), DEG2RAD(rot.y), DEG2RAD(rot.z));
						mat.SetRotationXYZ(ang);
						params.qRotation = Quat(mat);


						IEntity *pEntity = gEnv->pEntitySystem->SpawnEntity(params);

						if (NULL == pEntity)
							ActivateOutput(pActInfo, EOP_Fail, true);
						else
							ActivateOutput(pActInfo, EOP_Success, true);

					}
					else
					{
						CryLog("%s can't spawn, because entity intersection with entity = %d",GetPortString(pActInfo,EIP_EntityClass),GetPortEntityId(pActInfo,EIP_IntersectionEntity));
						ActivateOutput(pActInfo, EOP_Fail, true);
					}
				}
			}
			break;
		}
	}

protected:
	SActivationInfo m_actInfo;
};

class CFlowNode_AlignToFoundation: public CFlowBaseNode<eNCT_Instanced>
{
	enum INPUTS
	{
		EIP_Align = 0,
		EIP_AlignEntity,
		EIP_Foundation,
	};

	enum OUTPUTS
	{
		EOP_Pos = 0,
		EOP_Rot,
	};

public:
	CFlowNode_AlignToFoundation( SActivationInfo * pActInfo )
	{
	}

	~CFlowNode_AlignToFoundation()
	{
	}

	IFlowNodePtr Clone(SActivationInfo* pActInfo)
	{
		return new CFlowNode_AlignToFoundation(pActInfo);
	}

	virtual void GetMemoryUsage(ICrySizer * s) const
	{
		s->Add(*this);
	}

	void GetConfiguration( SFlowNodeConfig& config )
	{
		static const SInputPortConfig in_ports[] = 
		{
			InputPortConfig_Void( "AlignToFoundation", _HELP("Try align to foundation")),
			InputPortConfig<EntityId>( "AlignEntityId", _HELP("Entity to align")),
			InputPortConfig<EntityId>( "FoundationId", _HELP("Foundation id")),
			{0}
		};
		static const SOutputPortConfig out_ports[] = 
		{
			OutputPortConfig<Vec3>("Position", _HELP("Compleate align position")),
			OutputPortConfig<Vec3>("Rotation", _HELP("Compleate align rotation")),
			{0}
		};
		config.pInputPorts = in_ports;
		config.pOutputPorts = out_ports;
		config.sDescription = _HELP("Align object to foundation");
		config.SetCategory(EFLN_APPROVED);
	}

	void ProcessEvent( EFlowEvent event, SActivationInfo *pActInfo )
	{
		switch (event)
		{
		case eFE_Initialize:
			{
				m_actInfo = *pActInfo;
			}
			break;
		case eFE_Activate:
			{
				if(IsPortActive(pActInfo, EIP_Align))
				{
					EntityId entId = GetPortEntityId(pActInfo, EIP_AlignEntity);
					EntityId foundId = GetPortEntityId(pActInfo, EIP_Foundation);

					IEntity *pEntity = gEnv->pEntitySystem->GetEntity(entId);
					IEntity *pFoundation = gEnv->pEntitySystem->GetEntity(foundId);

					if(pEntity != NULL && pFoundation != NULL)
					{				
						IEntityClass *entityClass = pEntity->GetClass();
						const char* eClass = entityClass->GetName();

						IEntityClass *foundationClass = pFoundation->GetClass();
						const char* fClass = foundationClass->GetName();

						if(!strcmp(eClass,fClass))
						{
							// Получаем координаты пивот поинта фундамента и сущности
							Vec3 foundationCoords = pFoundation->GetPos();
							Vec3 entityCoords = pEntity->GetPos(); 
							// Получаем ротацию фундамента
							Quat foundationRot = pFoundation->GetRotation();
							// Высота фундамента. Неизменна.
							const float Z = foundationCoords.z;

							// Получаем координаты хелперов фундамента
							Vec3 rootHelper, sideA, sideB, sideC, sideD;
							Vec3 pillarA, pillarB, pillarC, pillarD;

							IStatObj *pStatObj = pFoundation->GetStatObj(0);

							if(pStatObj)
							{
								rootHelper = pStatObj->GetHelperPos("root");
								rootHelper = pFoundation->GetSlotWorldTM(0).TransformPoint(rootHelper);

								sideA = pStatObj->GetHelperPos("side_a");
								sideA = pFoundation->GetSlotWorldTM(0).TransformPoint(sideA);

								sideB = pStatObj->GetHelperPos("side_b");
								sideB = pFoundation->GetSlotWorldTM(0).TransformPoint(sideB);

								sideC = pStatObj->GetHelperPos("side_c");
								sideC = pFoundation->GetSlotWorldTM(0).TransformPoint(sideC);

								sideD = pStatObj->GetHelperPos("side_d");
								sideD = pFoundation->GetSlotWorldTM(0).TransformPoint(sideD);

								pillarA = pStatObj->GetHelperPos("pillar_point_a");
								pillarA = pFoundation->GetSlotWorldTM(0).TransformPoint(pillarA);

								pillarB = pStatObj->GetHelperPos("pillar_point_b");
								pillarB = pFoundation->GetSlotWorldTM(0).TransformPoint(pillarB);

								pillarC = pStatObj->GetHelperPos("pillar_point_c");
								pillarC = pFoundation->GetSlotWorldTM(0).TransformPoint(pillarC);

								pillarD = pStatObj->GetHelperPos("pillar_point_d");
								pillarD = pFoundation->GetSlotWorldTM(0).TransformPoint(pillarD);
							}

							// Получаем координаты хелперов сущности, которую пытаемся пристыковать (фундамент №2)
							Vec3 ent_rootHelper, ent_sideA, ent_sideB, ent_sideC, ent_sideD;
							Vec3 ent_pillarA, ent_pillarB, ent_pillarC, ent_pillarD;

							IStatObj *ent_pStatObj = pEntity->GetStatObj(0);

							if(ent_pStatObj)
							{
								ent_rootHelper = ent_pStatObj->GetHelperPos("root");
								ent_rootHelper = pEntity->GetSlotWorldTM(0).TransformPoint(ent_rootHelper);

								ent_sideA = ent_pStatObj->GetHelperPos("side_a");
								ent_sideA = pEntity->GetSlotWorldTM(0).TransformPoint(ent_sideA);

								ent_sideB = ent_pStatObj->GetHelperPos("side_b");
								ent_sideB = pEntity->GetSlotWorldTM(0).TransformPoint(ent_sideB);

								ent_sideC = ent_pStatObj->GetHelperPos("side_c");
								ent_sideC = pEntity->GetSlotWorldTM(0).TransformPoint(ent_sideC);

								ent_sideD = ent_pStatObj->GetHelperPos("side_d");
								ent_sideD = pEntity->GetSlotWorldTM(0).TransformPoint(ent_sideD);

								ent_pillarA = ent_pStatObj->GetHelperPos("pillar_point_a");
								ent_pillarA = pEntity->GetSlotWorldTM(0).TransformPoint(ent_pillarA);

								ent_pillarB = ent_pStatObj->GetHelperPos("pillar_point_b");
								ent_pillarB = pEntity->GetSlotWorldTM(0).TransformPoint(ent_pillarB);

								ent_pillarC = ent_pStatObj->GetHelperPos("pillar_point_c");
								ent_pillarC = pEntity->GetSlotWorldTM(0).TransformPoint(ent_pillarC);

								ent_pillarD = ent_pStatObj->GetHelperPos("pillar_point_d");
								ent_pillarD = pEntity->GetSlotWorldTM(0).TransformPoint(ent_pillarD);
							}


							// Находим дистанцию от всех точек фундамента №2 до всех точек стоящего фундамента
							Vec2 fSideA = Vec2(sideA.x, sideA.y); Vec2 eSideA = Vec2(ent_sideA.x, ent_sideA.y); 
							Vec2 fSideB = Vec2(sideB.x, sideB.y); Vec2 eSideB = Vec2(ent_sideB.x, ent_sideB.y); 
							Vec2 fSideC = Vec2(sideC.x, sideC.y); Vec2 eSideC = Vec2(ent_sideC.x, ent_sideC.y); 
							Vec2 fSideD = Vec2(sideD.x, sideD.y); Vec2 eSideD = Vec2(ent_sideD.x, ent_sideD.y); 

							float distToA, distToB, distToC, distToD;
							float distToA1, distToB1, distToC1, distToD1;
							float distToA2, distToB2, distToC2, distToD2;
							float distToA3, distToB3, distToC3, distToD3;

							distToA = CalculateDistance(eSideA, fSideA);
							distToB = CalculateDistance(eSideA, fSideB);
							distToC = CalculateDistance(eSideA, fSideC);
							distToD = CalculateDistance(eSideA, fSideD);

							distToA1 = CalculateDistance(eSideB, fSideA);
							distToB1 = CalculateDistance(eSideB, fSideB);
							distToC1 = CalculateDistance(eSideB, fSideC);
							distToD1 = CalculateDistance(eSideB, fSideD);
	
							distToA2 = CalculateDistance(eSideC, fSideA);
							distToB2 = CalculateDistance(eSideC, fSideB);
							distToC2 = CalculateDistance(eSideC, fSideC);
							distToD2 = CalculateDistance(eSideC, fSideD);
						
							distToA3 = CalculateDistance(eSideD, fSideA);
							distToB3 = CalculateDistance(eSideD, fSideB);
							distToC3 = CalculateDistance(eSideD, fSideC);
							distToD3 = CalculateDistance(eSideD, fSideD);

							// Находим минимальную дистанцию
							float minFromA, minFromA1, minFromA2;
							float minFromB, minFromB1, minFromB2;
							float minFromC, minFromC1, minFromC2;
							float minFromD, minFromD1, minFromD2;

							minFromA1 = min(distToA, distToB);
							minFromA2 = min(distToC, distToD);
							minFromA = min(minFromA1, minFromA2);

							minFromB1 = min(distToA1, distToB1);
							minFromB2 = min(distToC1, distToD1);
							minFromB = min(minFromB1, minFromB2);

							minFromC1 = min(distToA2, distToB2);
							minFromC2 = min(distToC2, distToD2);
							minFromC = min(minFromC1, minFromC2);

							minFromD1 = min(distToA3, distToB3);
							minFromD2 = min(distToC3, distToD3);
							minFromD = min(minFromD1, minFromD2);

							float minDistance, minDistance1, minDistance2;
							minDistance1 = min(minFromA, minFromB);
							minDistance2 = min(minFromC, minFromD);
							minDistance = min(minDistance1, minDistance2);

							// Вычисляем какие стороны фундаментов находится ближе всего друг к другу
							EFoundationSides entitySidePoint;
							EFoundationSides foundationSidePoint;
							// A
							if(distToA == minDistance || distToB == minDistance || distToC == minDistance || distToD == minDistance)
							{
								entitySidePoint = EFoundationSideA;

								if(distToA == minDistance) foundationSidePoint = EFoundationSideA;
								if(distToB == minDistance) foundationSidePoint = EFoundationSideB;
								if(distToC == minDistance) foundationSidePoint = EFoundationSideC;
								if(distToD == minDistance) foundationSidePoint = EFoundationSideD;
							}
							// B
							if(distToA1 == minDistance || distToB1 == minDistance || distToC1 == minDistance || distToD1 == minDistance)
							{
								entitySidePoint = EFoundationSideB;

								if(distToA1 == minDistance) foundationSidePoint = EFoundationSideA;
								if(distToB1 == minDistance) foundationSidePoint = EFoundationSideB;
								if(distToC1 == minDistance) foundationSidePoint = EFoundationSideC;
								if(distToD1 == minDistance) foundationSidePoint = EFoundationSideD;
							}
							// C
							if(distToA2 == minDistance || distToB2 == minDistance || distToC2 == minDistance || distToD2 == minDistance)
							{
								entitySidePoint = EFoundationSideC;

								if(distToA2 == minDistance) foundationSidePoint = EFoundationSideA;
								if(distToB2 == minDistance) foundationSidePoint = EFoundationSideB;
								if(distToC2 == minDistance) foundationSidePoint = EFoundationSideC;
								if(distToD2 == minDistance) foundationSidePoint = EFoundationSideD;
							}
							// D
							if(distToA3 == minDistance || distToB3 == minDistance || distToC3 == minDistance || distToD3 == minDistance)
							{
								entitySidePoint = EFoundationSideD;

								if(distToA3 == minDistance) foundationSidePoint = EFoundationSideA;
								if(distToB3 == minDistance) foundationSidePoint = EFoundationSideB;
								if(distToC3 == minDistance) foundationSidePoint = EFoundationSideC;
								if(distToD3 == minDistance) foundationSidePoint = EFoundationSideD;
							}

							Vec3 newEntityPos;

							// Вычисляем новые координаты для фундамента №2
							
							// Прилипаем к точке фундамент А
							if(entitySidePoint == EFoundationSideA && foundationSidePoint == EFoundationSideA)
							{
								Vec2 distanceToPointXFromCenter = fSideA - Vec2(foundationCoords.x, foundationCoords.y);
								newEntityPos = Vec3(fSideA.x + distanceToPointXFromCenter.x, fSideA.y + distanceToPointXFromCenter.y, Z);
								ActivateOutput(pActInfo,EOP_Pos, newEntityPos);
							}
							if(entitySidePoint == EFoundationSideB && foundationSidePoint == EFoundationSideA)
							{
								Vec2 distanceToPointXFromCenter = fSideB - Vec2(foundationCoords.x, foundationCoords.y);
								newEntityPos = Vec3(fSideB.x + distanceToPointXFromCenter.x , fSideB.y + distanceToPointXFromCenter.y, Z);
								ActivateOutput(pActInfo,EOP_Pos, newEntityPos);
							}
							if(entitySidePoint == EFoundationSideC && foundationSidePoint == EFoundationSideA)
							{
								Vec2 distanceToPointXFromCenter = fSideC - Vec2(foundationCoords.x, foundationCoords.y);
								newEntityPos = Vec3(fSideC.x + distanceToPointXFromCenter.x , fSideC.y + distanceToPointXFromCenter.y, Z);
								ActivateOutput(pActInfo,EOP_Pos, newEntityPos);
							}
							if(entitySidePoint == EFoundationSideD && foundationSidePoint == EFoundationSideA)
							{
								Vec2 distanceToPointXFromCenter = fSideD - Vec2(foundationCoords.x, foundationCoords.y);
								newEntityPos = Vec3(fSideD.x + distanceToPointXFromCenter.x , fSideD.y + distanceToPointXFromCenter.y, Z);
								ActivateOutput(pActInfo,EOP_Pos, newEntityPos);
							}
							//









							// Отрисовываем хелперы. ТОЛЬКО ДЛЯ ОТЛАДКИ
							gEnv->pRenderer->DrawLabel(rootHelper,1.5,"root");
							gEnv->pRenderer->DrawLabel(sideA,1.5,"sideA");
							gEnv->pRenderer->DrawLabel(sideB,1.5,"sideB");
							gEnv->pRenderer->DrawLabel(sideC,1.5,"sideC");
							gEnv->pRenderer->DrawLabel(sideD,1.5,"sideD");

							gEnv->pRenderer->DrawLabel(ent_sideA,1.5,"sideA");
							gEnv->pRenderer->DrawLabel(ent_sideB,1.5,"sideB");
							gEnv->pRenderer->DrawLabel(ent_sideC,1.5,"sideC");
							gEnv->pRenderer->DrawLabel(ent_sideD,1.5,"sideD");

						}
					}

				}
			}
			break;
		}
	}

protected:
	SActivationInfo m_actInfo;
};

class CFlowNode_CalculateFoundationPos: public CFlowBaseNode<eNCT_Instanced>
{
	enum INPUTS
	{
		EIP_Entity = 0,
		EIP_Calculate,
		EIP_WorldPos,
	};

	enum OUTPUTS
	{
		EOP_Pos = 0,
	};

public:
	CFlowNode_CalculateFoundationPos( SActivationInfo * pActInfo )
	{
	}

	~CFlowNode_CalculateFoundationPos()
	{
	}

	IFlowNodePtr Clone(SActivationInfo* pActInfo)
	{
		return new CFlowNode_CalculateFoundationPos(pActInfo);
	}

	virtual void GetMemoryUsage(ICrySizer * s) const
	{
		s->Add(*this);
	}

	void GetConfiguration( SFlowNodeConfig& config )
	{
		static const SInputPortConfig in_ports[] = 
		{
			InputPortConfig<EntityId>( "Entity", _HELP("Entity id")),
			InputPortConfig_Void( "Calculate", _HELP("Calculate terrain position")),
			InputPortConfig<Vec3>( "StartPosition", _HELP("Spawn position (hitpoint on raycast camera)")),
			{0}
		};
		static const SOutputPortConfig out_ports[] = 
		{
			OutputPortConfig<Vec3>("Position", _HELP("Compleate foundation position")),
			{0}
		};
		config.pInputPorts = in_ports;
		config.pOutputPorts = out_ports;
		config.sDescription = _HELP("Align foundation to terrain or other foundation");
		config.SetCategory(EFLN_APPROVED);
	}

	void ProcessEvent( EFlowEvent event, SActivationInfo *pActInfo )
	{
		switch (event)
		{
		case eFE_Initialize:
			{
				m_actInfo = *pActInfo;
			}
			break;
		case eFE_Activate:
			{
				if(IsPortActive(pActInfo, EIP_Calculate))
				{
					// Привязка фундамента к террейну
					ray_hit hit;
					Vec3 entityPos = GetPortVec3(pActInfo,EIP_WorldPos);
					Vec3 dir = Vec3(0, 0, -1);
					Vec3 terrainPoint;

					int numHits = gEnv->pPhysicalWorld->RayWorldIntersection(entityPos,dir,ent_terrain,rwi_stop_at_pierceable|rwi_colltype_any,&hit, 1);
					if(numHits > 0)
					{
						terrainPoint = hit.pt;
					}
					// Определение столкновения с каким либо объектом
					IEntity* pEntity = gEnv->pEntitySystem->GetEntity(GetPortEntityId(pActInfo,EIP_Entity));
					IPhysicalEntity *pIgnore = pEntity->GetPhysics();
					geom_contact *contacts;

					AABB bb;
					pEntity->GetWorldBounds(bb);


					primitives::cylinder cylinder;
					cylinder.center = pEntity->GetPos();
					cylinder.hh = bb.GetSize().z;
					cylinder.r = bb.GetRadius() - 0.5f;

					intersection_params params;
					params.bStopAtFirstTri = false;
					params.bNoBorder = false;
					params.bNoAreaContacts = true;

					int rayFlags = rwi_stop_at_pierceable;
					int objTypes = ent_independent | ent_living | ent_sleeping_rigid | ent_rigid | ent_static;

					gEnv->pPhysicalWorld->PrimitiveWorldIntersection(primitives::cylinder::type, &cylinder, Vec3(ZERO), objTypes, &contacts, 0,	3 , &params, 0, 0, &pIgnore, pIgnore?1:0);

					if(contacts)
					{
						IPhysicalEntity *pCollider = gEnv->pPhysicalWorld->GetPhysicalEntityById(contacts->iPrim[0]);
						IEntity *pFoundation = gEnv->pEntitySystem->GetEntityFromPhysics(pCollider);

						if(pEntity != NULL && pFoundation != NULL)
						{				
							IEntityClass *entityClass = pEntity->GetClass();
							const char* eClass = entityClass->GetName();

							IEntityClass *foundationClass = pFoundation->GetClass();
							const char* fClass = foundationClass->GetName();

							Vec3 newEntityPos;

							if(!strcmp(eClass,fClass))
							{
								// Получаем координаты пивот поинта фундамента 
								Vec3 foundationCoords = pFoundation->GetPos();		
								float Z = foundationCoords.z;
								// Получаем координаты хелперов фундамента
								Vec3 rootHelper, sideA, sideB, sideC, sideD;

								IStatObj *pStatObj = pFoundation->GetStatObj(0);

								if(pStatObj)
								{
									rootHelper = pStatObj->GetHelperPos("root");
									rootHelper = pFoundation->GetSlotWorldTM(0).TransformPoint(rootHelper);

									sideA = pStatObj->GetHelperPos("side_a");
									sideA = pFoundation->GetSlotWorldTM(0).TransformPoint(sideA);

									sideB = pStatObj->GetHelperPos("side_b");
									sideB = pFoundation->GetSlotWorldTM(0).TransformPoint(sideB);

									sideC = pStatObj->GetHelperPos("side_c");
									sideC = pFoundation->GetSlotWorldTM(0).TransformPoint(sideC);

									sideD = pStatObj->GetHelperPos("side_d");
									sideD = pFoundation->GetSlotWorldTM(0).TransformPoint(sideD);
								}

								// Получаем координаты хелперов сущности, которую пытаемся пристыковать (фундамент №2)
								Vec3 ent_rootHelper, ent_sideA, ent_sideB, ent_sideC, ent_sideD;

								IStatObj *ent_pStatObj = pEntity->GetStatObj(0);

								if(ent_pStatObj)
								{
									ent_rootHelper = ent_pStatObj->GetHelperPos("root");
									ent_rootHelper = pEntity->GetSlotWorldTM(0).TransformPoint(ent_rootHelper);

									ent_sideA = ent_pStatObj->GetHelperPos("side_a");
									ent_sideA = pEntity->GetSlotWorldTM(0).TransformPoint(ent_sideA);

									ent_sideB = ent_pStatObj->GetHelperPos("side_b");
									ent_sideB = pEntity->GetSlotWorldTM(0).TransformPoint(ent_sideB);

									ent_sideC = ent_pStatObj->GetHelperPos("side_c");
									ent_sideC = pEntity->GetSlotWorldTM(0).TransformPoint(ent_sideC);

									ent_sideD = ent_pStatObj->GetHelperPos("side_d");
									ent_sideD = pEntity->GetSlotWorldTM(0).TransformPoint(ent_sideD);

								}

								// Находим дистанцию от всех точек фундамента №2 до всех точек стоящего фундамента
								Vec2 fSideA = Vec2(sideA.x, sideA.y); Vec2 eSideA = Vec2(ent_sideA.x, ent_sideA.y); 
								Vec2 fSideB = Vec2(sideB.x, sideB.y); Vec2 eSideB = Vec2(ent_sideB.x, ent_sideB.y); 
								Vec2 fSideC = Vec2(sideC.x, sideC.y); Vec2 eSideC = Vec2(ent_sideC.x, ent_sideC.y); 
								Vec2 fSideD = Vec2(sideD.x, sideD.y); Vec2 eSideD = Vec2(ent_sideD.x, ent_sideD.y); 

								float distToA, distToB, distToC, distToD;
								float distToA1, distToB1, distToC1, distToD1;
								float distToA2, distToB2, distToC2, distToD2;
								float distToA3, distToB3, distToC3, distToD3;

								distToA = CalculateDistance(eSideA, fSideA);
								distToB = CalculateDistance(eSideA, fSideB);
								distToC = CalculateDistance(eSideA, fSideC);
								distToD = CalculateDistance(eSideA, fSideD);

								distToA1 = CalculateDistance(eSideB, fSideA);
								distToB1 = CalculateDistance(eSideB, fSideB);
								distToC1 = CalculateDistance(eSideB, fSideC);
								distToD1 = CalculateDistance(eSideB, fSideD);

								distToA2 = CalculateDistance(eSideC, fSideA);
								distToB2 = CalculateDistance(eSideC, fSideB);
								distToC2 = CalculateDistance(eSideC, fSideC);
								distToD2 = CalculateDistance(eSideC, fSideD);

								distToA3 = CalculateDistance(eSideD, fSideA);
								distToB3 = CalculateDistance(eSideD, fSideB);
								distToC3 = CalculateDistance(eSideD, fSideC);
								distToD3 = CalculateDistance(eSideD, fSideD);

								// Находим минимальную дистанцию
								float minFromA, minFromA1, minFromA2;
								float minFromB, minFromB1, minFromB2;
								float minFromC, minFromC1, minFromC2;
								float minFromD, minFromD1, minFromD2;

								minFromA1 = min(distToA, distToB);
								minFromA2 = min(distToC, distToD);
								minFromA = min(minFromA1, minFromA2);

								minFromB1 = min(distToA1, distToB1);
								minFromB2 = min(distToC1, distToD1);
								minFromB = min(minFromB1, minFromB2);

								minFromC1 = min(distToA2, distToB2);
								minFromC2 = min(distToC2, distToD2);
								minFromC = min(minFromC1, minFromC2);

								minFromD1 = min(distToA3, distToB3);
								minFromD2 = min(distToC3, distToD3);
								minFromD = min(minFromD1, minFromD2);

								float minDistance, minDistance1, minDistance2;
								minDistance1 = min(minFromA, minFromB);
								minDistance2 = min(minFromC, minFromD);
								minDistance = min(minDistance1, minDistance2);

								// Вычисляем какие стороны фундаментов находится ближе всего друг к другу
								EFoundationSides entitySidePoint;
								EFoundationSides foundationSidePoint;
								// A
								if(distToA == minDistance || distToB == minDistance || distToC == minDistance || distToD == minDistance)
								{
									entitySidePoint = EFoundationSideA;

									if(distToA == minDistance) foundationSidePoint = EFoundationSideA;
									if(distToB == minDistance) foundationSidePoint = EFoundationSideB;
									if(distToC == minDistance) foundationSidePoint = EFoundationSideC;
									if(distToD == minDistance) foundationSidePoint = EFoundationSideD;
								}
								// B
								if(distToA1 == minDistance || distToB1 == minDistance || distToC1 == minDistance || distToD1 == minDistance)
								{
									entitySidePoint = EFoundationSideB;

									if(distToA1 == minDistance) foundationSidePoint = EFoundationSideA;
									if(distToB1 == minDistance) foundationSidePoint = EFoundationSideB;
									if(distToC1 == minDistance) foundationSidePoint = EFoundationSideC;
									if(distToD1 == minDistance) foundationSidePoint = EFoundationSideD;
								}
								// C
								if(distToA2 == minDistance || distToB2 == minDistance || distToC2 == minDistance || distToD2 == minDistance)
								{
									entitySidePoint = EFoundationSideC;

									if(distToA2 == minDistance) foundationSidePoint = EFoundationSideA;
									if(distToB2 == minDistance) foundationSidePoint = EFoundationSideB;
									if(distToC2 == minDistance) foundationSidePoint = EFoundationSideC;
									if(distToD2 == minDistance) foundationSidePoint = EFoundationSideD;
								}
								// D
								if(distToA3 == minDistance || distToB3 == minDistance || distToC3 == minDistance || distToD3 == minDistance)
								{
									entitySidePoint = EFoundationSideD;

									if(distToA3 == minDistance) foundationSidePoint = EFoundationSideA;
									if(distToB3 == minDistance) foundationSidePoint = EFoundationSideB;
									if(distToC3 == minDistance) foundationSidePoint = EFoundationSideC;
									if(distToD3 == minDistance) foundationSidePoint = EFoundationSideD;
								}

								// Вычисляем новые координаты для фундамента №2
								//CryLog("From ent %d , to foundation %d", entitySidePoint, foundationSidePoint);

								
								if(foundationSidePoint == EFoundationSideA)
								{
									Vec2 distanceToPointXFromCenter = fSideA - Vec2(foundationCoords.x, foundationCoords.y);
									terrainPoint = Vec3(fSideA.x + distanceToPointXFromCenter.x , fSideA.y + distanceToPointXFromCenter.y, Z);
								}
								if(foundationSidePoint == EFoundationSideB)
								{
									Vec2 distanceToPointXFromCenter = fSideB - Vec2(foundationCoords.x, foundationCoords.y);
									terrainPoint = Vec3(fSideB.x + distanceToPointXFromCenter.x , fSideB.y + distanceToPointXFromCenter.y, Z);
								}
								if(foundationSidePoint == EFoundationSideC)
								{
									Vec2 distanceToPointXFromCenter = fSideC - Vec2(foundationCoords.x, foundationCoords.y);
									terrainPoint = Vec3(fSideC.x + distanceToPointXFromCenter.x , fSideC.y + distanceToPointXFromCenter.y, Z);
								}
								if(foundationSidePoint == EFoundationSideD)
								{
									Vec2 distanceToPointXFromCenter = fSideD - Vec2(foundationCoords.x, foundationCoords.y);
									terrainPoint = Vec3(fSideD.x + distanceToPointXFromCenter.x , fSideD.y + distanceToPointXFromCenter.y, Z);
								}


								// Отрисовываем хелперы. ТОЛЬКО ДЛЯ ОТЛАДКИ
								/*gEnv->pRenderer->DrawLabel(sideA,1.5,"sideA");
								gEnv->pRenderer->DrawLabel(sideB,1.5,"sideB");
								gEnv->pRenderer->DrawLabel(sideC,1.5,"sideC");
								gEnv->pRenderer->DrawLabel(sideD,1.5,"sideD");

								gEnv->pRenderer->DrawLabel(ent_sideA,1.5,"sideA");
								gEnv->pRenderer->DrawLabel(ent_sideB,1.5,"sideB");
								gEnv->pRenderer->DrawLabel(ent_sideC,1.5,"sideC");
								gEnv->pRenderer->DrawLabel(ent_sideD,1.5,"sideD");*/

								//if(newEntityPos == Vec3(0,0,0))
									//ActivateOutput(pActInfo,EOP_Pos, terrainPoint);
							}
							//else
								//ActivateOutput(pActInfo,EOP_Pos, terrainPoint);
						}
						//else
							//ActivateOutput(pActInfo,EOP_Pos, terrainPoint);
					}
					//else
						//ActivateOutput(pActInfo,EOP_Pos, terrainPoint);

					ActivateOutput(pActInfo,EOP_Pos, terrainPoint);
				}
			}
			break;
		}
	}

protected:
	SActivationInfo m_actInfo;
};

//

REGISTER_FLOW_NODE("InteractiveBuildings:AlignToTerrain", CFlowNode_AlignToTerrain);
REGISTER_FLOW_NODE("InteractiveBuildings:AlignToFoundation", CFlowNode_AlignToFoundation);
REGISTER_FLOW_NODE("InteractiveBuildings:RemoveEntity",	CFlowNode_RemoveEntity);
REGISTER_FLOW_NODE("InteractiveBuildings:CheckCollision", CFlowNode_CheckCollision);
REGISTER_FLOW_NODE("InteractiveBuildings:PlaceEntity", CFlowNode_PlaceEntity);

REGISTER_FLOW_NODE("InteractiveBuildings:CalculateFoundationPos", CFlowNode_CalculateFoundationPos);