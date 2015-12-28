#include "StdAfx.h"
#include "FlowBaseNode.h"

#include <IEntitySystem.h>
#include <IAISystem.h>
#include <IAgent.h>
#include "IAIObject.h"

class CFlowNode_Dynamics : public CFlowBaseNode<eNCT_Singleton>
{
public:
	CFlowNode_Dynamics( SActivationInfo * pActInfo ) 
	{
	}

	virtual void GetConfiguration( SFlowNodeConfig& config )
	{
		static const SInputPortConfig in_config[] = {
			InputPortConfig<bool>("active", true, _HELP("Update data on/off")),
			{0}
		};

		static const SOutputPortConfig out_config[] = {
			OutputPortConfig<Vec3>("v", _HELP("Velocity of entity")),
			OutputPortConfig<Vec3>("a", _HELP("Acceleration of entity")),
			OutputPortConfig<Vec3>("w", _HELP("Angular velocity of entity")),
			OutputPortConfig<Vec3>("wa", _HELP("Angular acceleration of entity")),
			OutputPortConfig<float>("m", _HELP("Mass of entity")),
			{0}
		};
		config.nFlags |= EFLN_TARGET_ENTITY;
		config.pInputPorts = in_config;
		config.pOutputPorts = out_config;
		config.sDescription = _HELP("Dynamic physical state of an entity");
		config.SetCategory(EFLN_APPROVED); // POLICY CHANGE: Maybe an Enable/Disable Port
	}


	virtual void GetMemoryUsage(ICrySizer * s) const
	{
		s->Add(*this);
	}
	virtual void ProcessEvent( EFlowEvent event, SActivationInfo *pActInfo )
	{
		switch (event)
		{
		case eFE_Initialize:
			pActInfo->pGraph->SetRegularlyUpdated( pActInfo->myID, true );
			break;
		case eFE_Update:
			{
				if(!GetPortBool(pActInfo, 0))
					return;

				IEntity * pEntity = pActInfo->pEntity;
				if (pEntity)
				{
					IPhysicalEntity * pPhysEntity = pEntity->GetPhysics();
					if (pPhysEntity)
					{
						pe_status_dynamics dyn;
						pPhysEntity->GetStatus( &dyn );
						ActivateOutput(pActInfo, 0, dyn.v);
						ActivateOutput(pActInfo, 1, dyn.a);
						ActivateOutput(pActInfo, 2, dyn.w);
						ActivateOutput(pActInfo, 3, dyn.wa);
						ActivateOutput(pActInfo, 4, dyn.mass);
					}
				}
			}
		}
	}
};

class CFlowNode_PhysicsSleepQuery : public CFlowBaseNode<eNCT_Instanced>
{
	enum EInPorts
	{
		IN_CONDITION = 0,
		IN_RESET
	};
	enum EOutPorts
	{
		OUT_SLEEPING = 0,
		OUT_ONSLEEP,
		OUT_ONAWAKE
	};

	bool m_Activated;

public:
	CFlowNode_PhysicsSleepQuery( SActivationInfo * pActInfo ) {}

	virtual void GetMemoryUsage(ICrySizer * s) const
	{
		s->Add(*this);
	}

	virtual void GetConfiguration( SFlowNodeConfig& config )
	{
		static const SInputPortConfig in_config[] = {
			InputPortConfig<bool>( "condition", _HELP("Setting this input to TRUE sends the sleeping condition of the entity to the [sleeping] port, and triggers [sleep] and [awake]") ),
			InputPortConfig<bool>( "reset", _HELP("Triggering this input to TRUE resets the node") ),
			{0}
		};
		static const SOutputPortConfig out_config[] = {
			OutputPortConfig<bool>( "sleeping", _HELP("Set to TRUE when the physics of the entity are sleeping (passes the value through)") ),
			OutputPortConfig<SFlowSystemVoid>( "sleep", _HELP("Triggers to TRUE when the physics of the entity switch to sleep") ),
			OutputPortConfig<SFlowSystemVoid>( "awake", _HELP("Triggers to TRUE when the physics of the entity switch to awake") ),
			{0}
		};
		config.nFlags |= EFLN_TARGET_ENTITY;
		config.pInputPorts = in_config;
		config.pOutputPorts = out_config;
		config.sDescription = _HELP("Node that returns the sleeping state of the physics of a given entity.");
		config.SetCategory(EFLN_APPROVED);
	}

	virtual void ProcessEvent( EFlowEvent event, SActivationInfo *pActInfo )
	{
		switch (event)
		{
		case eFE_Initialize:
			pActInfo->pGraph->SetRegularlyUpdated( pActInfo->myID, true );
			m_Activated = false;
			break;

		case eFE_Update:
			{
				bool bReset = GetPortBool(pActInfo, IN_RESET);
				bool bCondition = GetPortBool(pActInfo, IN_CONDITION);

				if(bReset)
				{
					ActivateOutput(pActInfo, OUT_SLEEPING, !bCondition);
				} else
				{
					if(bCondition != m_Activated)
					{
						IEntity * pEntity = pActInfo->pEntity;

						if (pEntity)
						{
							IPhysicalEntity * pPhysEntity = pEntity->GetPhysics();

							if (pPhysEntity)
							{
								pe_status_awake psa;

								bool isSleeping = pPhysEntity->GetStatus( &psa ) ? false : true;

								ActivateOutput(pActInfo, OUT_SLEEPING, isSleeping);

								if(isSleeping)
									ActivateOutput(pActInfo, OUT_ONSLEEP, true);
								else
									ActivateOutput(pActInfo, OUT_ONAWAKE, true);
							}
						}

						m_Activated = bCondition;
					}
				}
			}
		}
	}
};

class CFlowNode_ActionImpulse : public CFlowBaseNode<eNCT_Singleton>
{
	enum ECoordSys
	{
		CS_PARENT = 0,
		CS_WORLD,
		CS_LOCAL
	};

	enum EInPorts
	{
		IN_ACTIVATE = 0,
		IN_IMPULSE,
		IN_ANGIMPULSE,
		IN_POINT,
		IN_PARTINDEX,
		IN_COORDSYS,
	};



public:
	CFlowNode_ActionImpulse( SActivationInfo * pActInfo ) {}

	virtual void GetMemoryUsage(ICrySizer * s) const
	{
		s->Add(*this);
	}

	virtual void GetConfiguration( SFlowNodeConfig& config )
	{
		static const SInputPortConfig in_config[] = {
			InputPortConfig_Void( "activate", _HELP("Trigger the impulse") ),
			InputPortConfig<Vec3>( "impulse", Vec3(0,0,0), _HELP("Impulse vector") ),
			InputPortConfig<Vec3>( "angImpulse", Vec3(0,0,0), _HELP("The angular impulse") ),
			InputPortConfig<Vec3>( "Point", Vec3(0,0,0), _HELP("Point of application (optional)") ),
			InputPortConfig<int>( "partIndex", 0, _HELP("Index of the part that will receive the impulse (optional, 1-based, 0=unspecified)") ),
			InputPortConfig<int> ( "CoordSys", 1, _HELP("Defines which coordinate system is used for the inputs values"), _HELP("CoordSys"), _UICONFIG("enum_int:Parent=0,World=1,Local=2") ),
			{0}
		};
		config.nFlags |= EFLN_TARGET_ENTITY|EFLN_AISEQUENCE_SUPPORTED;
		config.pInputPorts = in_config;
		config.pOutputPorts = 0;
		config.sDescription = _HELP("Applies an impulse on an entity");
		config.SetCategory(EFLN_APPROVED);
	}

	virtual void ProcessEvent( EFlowEvent event, SActivationInfo *pActInfo )
	{
		if (!pActInfo->pEntity)
			return;

		if (event == eFE_Activate && IsPortActive(pActInfo, IN_ACTIVATE))
		{
			pe_action_impulse action;

			int ipart = GetPortInt( pActInfo, IN_PARTINDEX );
			if (ipart>0)
				action.ipart = ipart-1;

			IEntity* pEntity = pActInfo->pEntity;
			ECoordSys coordSys = (ECoordSys)GetPortInt( pActInfo, IN_COORDSYS );

			if (coordSys==CS_PARENT && !pEntity->GetParent())
				coordSys = CS_WORLD;

			// When a "zero point" is set in the node, the value is left undefined and physics assume it is the CM of the object.
			// but when the entity has a parent (is linked), then we have to use a real world coordinate for the point, because we have to apply the impulse to the highest entity 
			// on the hierarchy and physics will use the position of that entity instead of the position of the entity assigned to the node
			bool bHaveToUseTransformedZeroPoint = false;
			Vec3 transformedZeroPoint;
			Matrix34 transMat;

		  switch (coordSys)
			{
				case CS_WORLD:
				default:
				{
					transMat.SetIdentity();
					bHaveToUseTransformedZeroPoint = pEntity->GetParent()!=NULL;
					transformedZeroPoint = pEntity->GetWorldPos();
					break;
				}

				case CS_PARENT:
				{
					transMat = pEntity->GetParent()->GetWorldTM();
					bHaveToUseTransformedZeroPoint = pEntity->GetParent()->GetParent()!=NULL;
					transformedZeroPoint = pEntity->GetParent()->GetWorldPos();
					break;
				}

				case CS_LOCAL:
				{
					transMat = pEntity->GetWorldTM();
					bHaveToUseTransformedZeroPoint = pEntity->GetParent()!=NULL;
					transformedZeroPoint = pEntity->GetWorldPos();
					break;
				}
			}

			action.impulse = GetPortVec3( pActInfo, IN_IMPULSE );
			action.impulse = transMat.TransformVector( action.impulse );

			Vec3 angImpulse = GetPortVec3( pActInfo, IN_ANGIMPULSE );
			if (!angImpulse.IsZero())
				action.angImpulse = transMat.TransformVector( angImpulse );

			Vec3 pointApplication = GetPortVec3( pActInfo, IN_POINT );
			if (!pointApplication.IsZero())
				action.point = transMat.TransformPoint( pointApplication );
			else
			{
				if (bHaveToUseTransformedZeroPoint)
					action.point = transformedZeroPoint;
			}


			// the impulse has to be applied to the highest entity in the hierarchy. This comes from how physics manage linked entities.
			IEntity* pEntityImpulse = pEntity;
			while (pEntityImpulse->GetParent())
			{
				pEntityImpulse = pEntityImpulse->GetParent();
			}

			IPhysicalEntity * pPhysEntity = pEntityImpulse->GetPhysics();
			if (pPhysEntity)
				pPhysEntity->Action( &action );
		}
	}
};

class CFlowNode_Raycast : public CFlowBaseNode<eNCT_Singleton>
{
public:
	CFlowNode_Raycast( SActivationInfo * pActInfo ) {}

	enum EInPorts
	{
		GO = 0,
		DIR,
		MAXLENGTH,
		POS,
		TRANSFORM_DIRECTION,
	};
	enum EOutPorts
	{
		NOHIT = 0,
		HIT,
		DIROUT,
		DISTANCE,
		HITPOINT,
		NORMAL,
		SURFTYPE,
		HIT_ENTITY,
	};

	virtual void GetMemoryUsage(ICrySizer * s) const
	{
		s->Add(*this);
	}

	virtual void GetConfiguration( SFlowNodeConfig& config )
	{
		static const SInputPortConfig in_config[] = {
			InputPortConfig<SFlowSystemVoid>( "go", SFlowSystemVoid(), _HELP("Perform Raycast") ),
			InputPortConfig<Vec3>( "direction", Vec3(0,1,0), _HELP("Direction of Raycast") ),
			InputPortConfig<float>( "maxLength", 10.0f, _HELP("Maximum length of Raycast") ),
			InputPortConfig<Vec3>( "position", Vec3(0,0,0), _HELP("Ray start position, relative to entity") ),
			InputPortConfig<bool>( "transformDir", true, _HELP("Direction is transformed by entity orientation.") ),
			{0}
		};
		static const SOutputPortConfig out_config[] = {
			OutputPortConfig<SFlowSystemVoid>( "nohit", _HELP("Triggered if NO object was hit by raycast") ),
			OutputPortConfig<SFlowSystemVoid>( "hit", _HELP("Triggered if an object was hit by raycast") ),
			OutputPortConfig<Vec3>( "direction", _HELP("Actual direction of the cast ray (transformed by entity rotation")),
			OutputPortConfig<float>( "distance", _HELP("Distance to object hit") ),
			OutputPortConfig<Vec3>( "hitpoint", _HELP("Position the ray hit") ),
			OutputPortConfig<Vec3>( "normal", _HELP("Normal of the surface at the hitpoint") ),
			OutputPortConfig<int>( "surfacetype", _HELP("Surface type index at the hit point") ),
			OutputPortConfig<EntityId> ( "entity", _HELP("Entity which was hit")), 
			{0}
		};
		config.nFlags |= EFLN_TARGET_ENTITY;
		config.pInputPorts = in_config;
		config.pOutputPorts = out_config;
		config.SetCategory(EFLN_APPROVED);
	}

	virtual void ProcessEvent( EFlowEvent event, SActivationInfo *pActInfo )
	{
		if (event == eFE_Activate && IsPortActive(pActInfo, GO))
		{
			IEntity * pEntity = pActInfo->pEntity;
			if (pEntity)
			{
				ray_hit hit;
				IPhysicalEntity *pSkip = pEntity->GetPhysics();
				Vec3 direction = GetPortVec3(pActInfo, DIR).GetNormalized();
				if(GetPortBool(pActInfo, TRANSFORM_DIRECTION))
					direction = pEntity->GetWorldTM().TransformVector( GetPortVec3(pActInfo, DIR).GetNormalized() );
				IPhysicalWorld * pWorld = gEnv->pPhysicalWorld;
				int numHits = pWorld->RayWorldIntersection( 
					pEntity->GetPos() + GetPortVec3(pActInfo, POS),
					direction * GetPortFloat(pActInfo, MAXLENGTH),
					ent_all,
					rwi_stop_at_pierceable|rwi_colltype_any,
					&hit, 1, 
					&pSkip, 1 );
					
				if (numHits)
				{
					pEntity = (IEntity*)hit.pCollider->GetForeignData(PHYS_FOREIGN_ID_ENTITY);    
					ActivateOutput( pActInfo, HIT,(bool)true );
					ActivateOutput( pActInfo, DIROUT, direction );
					ActivateOutput( pActInfo, DISTANCE, hit.dist );
					ActivateOutput( pActInfo, HITPOINT, hit.pt );
					ActivateOutput( pActInfo, NORMAL, hit.n );
					ActivateOutput( pActInfo, SURFTYPE, (int)hit.surface_idx );
					ActivateOutput( pActInfo, HIT_ENTITY, pEntity ? pEntity->GetId() : 0);
				}
				else
					ActivateOutput( pActInfo, NOHIT, false);
			}
		}
	}
};

class CFlowNode_RaycastCamera : public CFlowBaseNode<eNCT_Singleton>
{
public:
	CFlowNode_RaycastCamera( SActivationInfo * pActInfo ) {}

	enum EInPorts
	{
		GO = 0,
		POS,
		MAXLENGTH,
	};
	enum EOutPorts
	{
		NOHIT = 0,
		HIT,
		DIROUT,
		DISTANCE,
		HITPOINT,
		NORMAL,
		SURFTYPE,
		PARTID,
		HIT_ENTITY,
		HIT_ENTITY_PHID,
	};

	virtual void GetMemoryUsage(ICrySizer * s) const
	{
		s->Add(*this);
	}

	virtual void GetConfiguration( SFlowNodeConfig& config )
	{
		static const SInputPortConfig in_config[] = {
			InputPortConfig<SFlowSystemVoid>( "go", SFlowSystemVoid(), _HELP("Perform Raycast") ),
			InputPortConfig<Vec3>( "offset", Vec3(0,0,0), _HELP("Ray start position, relative to camera") ),
			InputPortConfig<float>( "maxLength", 10.0f, _HELP("Maximum length of Raycast") ),
			{0}
		};
		static const SOutputPortConfig out_config[] = {
			OutputPortConfig<SFlowSystemVoid>( "nohit", _HELP("Triggered if NO object was hit by raycast") ),
			OutputPortConfig<SFlowSystemVoid>( "hit", _HELP("Triggered if an object was hit by raycast") ),
			OutputPortConfig<Vec3>( "direction", _HELP("Actual direction of the cast ray (transformed by entity rotation")),
			OutputPortConfig<float>( "distance", _HELP("Distance to object hit") ),
			OutputPortConfig<Vec3>( "hitpoint", _HELP("Position the ray hit") ),
			OutputPortConfig<Vec3>( "normal", _HELP("Normal of the surface at the hitpoint") ),
			OutputPortConfig<int>( "surfacetype", _HELP("Surface type index at the hit point") ),
			OutputPortConfig<int>( "partid", _HELP("Hit part id") ),
			OutputPortConfig<EntityId> ( "entity", _HELP("Entity which was hit")), 
			OutputPortConfig<EntityId> ( "entityPhysId", _HELP("Id of the physical entity that was hit")),
			{0}
		};
		config.sDescription = _HELP("Perform a raycast relative to the camera");
		config.pInputPorts = in_config;
		config.pOutputPorts = out_config;
		config.SetCategory(EFLN_APPROVED);
	}

	virtual void ProcessEvent( EFlowEvent event, SActivationInfo *pActInfo )
	{
		if (event == eFE_Activate && IsPortActive(pActInfo, GO))
		{
			IEntity * pEntity = pActInfo->pEntity;
			// if (pEntity)
			{
				ray_hit hit;
				CCamera& cam = GetISystem()->GetViewCamera();
				Vec3 pos = cam.GetPosition()+cam.GetViewdir();
				Vec3 direction = cam.GetViewdir();
				IPhysicalWorld * pWorld = gEnv->pPhysicalWorld;
//				IPhysicalEntity *pSkip = 0; // pEntity->GetPhysics();
				int numHits = pWorld->RayWorldIntersection( 
				pos + GetPortVec3(pActInfo, POS),
				direction * GetPortFloat(pActInfo, MAXLENGTH),
				ent_all,
				rwi_stop_at_pierceable|rwi_colltype_any,
				&hit, 1
				/* ,&pSkip, 1 */  );
				if (numHits)
				{
					pEntity = (IEntity*)hit.pCollider->GetForeignData(PHYS_FOREIGN_ID_ENTITY);    
					ActivateOutput( pActInfo, HIT,(bool)true );
					ActivateOutput( pActInfo, DIROUT, direction );
					ActivateOutput( pActInfo, DISTANCE, hit.dist );
					ActivateOutput( pActInfo, HITPOINT, hit.pt );
					ActivateOutput( pActInfo, NORMAL, hit.n );
					ActivateOutput( pActInfo, SURFTYPE, (int)hit.surface_idx );
					ActivateOutput( pActInfo, PARTID, hit.partid );
					ActivateOutput( pActInfo, HIT_ENTITY, pEntity ? pEntity->GetId() : 0);
					ActivateOutput( pActInfo, HIT_ENTITY_PHID, gEnv->pPhysicalWorld->GetPhysicalEntityId(hit.pCollider));
				}
				else
					ActivateOutput( pActInfo, NOHIT, false );
			}
		}
	}
};

//////////////////////////////////////////////////////////////////////////
// Enable/Disable AI for an entity/
//////////////////////////////////////////////////////////////////////////
class CFlowNode_PhysicsEnable : public CFlowBaseNode<eNCT_Singleton>
{
public:
	enum EInputs {
		IN_ENABLE,
		IN_DISABLE,
		IN_ENABLE_AI,
		IN_DISABLE_AI,
	};
	CFlowNode_PhysicsEnable( SActivationInfo * pActInfo ) {};

	/*
	IFlowNodePtr Clone( SActivationInfo * pActInfo )
	{
		return this;
	}
	*/

	virtual void GetConfiguration( SFlowNodeConfig &config )
	{
		static const SInputPortConfig in_config[] = {
			InputPortConfig_Void( "Enable",_HELP("Enable Physics for target entity") ),
			InputPortConfig_Void( "Disable",_HELP("Disable Physics for target entity") ),
			InputPortConfig_Void( "AI_Enable",_HELP("Enable AI for target entity") ),
			InputPortConfig_Void( "AI_Disable",_HELP("Disable AI for target entity") ),
			{0}
		};
		config.sDescription = _HELP( "Enables/Disables Physics" );
		config.nFlags |= EFLN_TARGET_ENTITY;
		config.pInputPorts = in_config;
		config.pOutputPorts = 0;
		config.SetCategory(EFLN_ADVANCED);
	}
	virtual void ProcessEvent( EFlowEvent event,SActivationInfo *pActInfo )
	{
		switch (event)
		{
			case eFE_Activate:
			{
				if (!pActInfo->pEntity)
					return;

				if(IsPortActive(pActInfo, IN_ENABLE))
				{
					IEntityPhysicalProxy *pPhysicalProxy = (IEntityPhysicalProxy*)pActInfo->pEntity->GetProxy(ENTITY_PROXY_PHYSICS);
					if (pPhysicalProxy)
						pPhysicalProxy->EnablePhysics(true);
				}
				if(IsPortActive(pActInfo, IN_DISABLE))
				{
					IEntityPhysicalProxy *pPhysicalProxy = (IEntityPhysicalProxy*)pActInfo->pEntity->GetProxy(ENTITY_PROXY_PHYSICS);
					if (pPhysicalProxy)
						pPhysicalProxy->EnablePhysics(false);
				}
	
				if(IsPortActive(pActInfo, IN_ENABLE_AI))
				{
					if (IAIObject* aiObject = pActInfo->pEntity->GetAI())
						aiObject->Event(AIEVENT_ENABLE,0);
				}
				if(IsPortActive(pActInfo, IN_DISABLE_AI))
				{
					if (IAIObject* aiObject = pActInfo->pEntity->GetAI())
						aiObject->Event(AIEVENT_DISABLE,0);
				}
				break;
			}

		case eFE_Initialize:
			break;
		}
	}

	virtual void GetMemoryUsage(ICrySizer * s) const
	{
		s->Add(*this);
	}
};


class CFlowNode_CameraProxy : public CFlowBaseNode<eNCT_Singleton>
{
public:
	enum EInputs {
		IN_CREATE,
		IN_ID
	};
	enum EOutputs {
		OUT_ID
	};
	CFlowNode_CameraProxy( SActivationInfo * pActInfo ) {};

	virtual void GetConfiguration( SFlowNodeConfig &config )
	{
		static const SInputPortConfig in_config[] = {
			InputPortConfig<SFlowSystemVoid>( "Create", SFlowSystemVoid(), _HELP("Create the proxy if it doesnt exist yet") ),
			InputPortConfig<EntityId>( "EntityHost", 0, _HELP("Activate to sync proxy rotation with the current view camera") ),
			{0}
		};
		static const SOutputPortConfig out_config[] = {
			OutputPortConfig<int>("EntityCamera", _HELP("")),
			{0}
		};
		config.sDescription = _HELP( "Retrieves or creates a physicalized camera proxy attached to EntityHost" );
		config.pInputPorts = in_config;
		config.pOutputPorts = out_config;
		config.SetCategory(EFLN_ADVANCED);
	}

	IEntity *GetCameraEnt(IEntity *pHost, bool bCreate)
	{
		if (!pHost->GetPhysics())
			return bCreate ? pHost:0;

		pe_params_articulated_body pab;
		IEntityLink *plink = pHost->GetEntityLinks();
		for(; plink && strcmp(plink->name,"CameraProxy"); plink=plink->next);
		if (plink)
		{
			IEntity *pCam = gEnv->pEntitySystem->GetEntity(plink->entityId);
			if (!pCam)
			{
				pHost->RemoveEntityLink(plink);
				return bCreate ? pHost:0;
			}
			pab.qHostPivot = !pHost->GetRotation()*Quat(Matrix33(GetISystem()->GetViewCamera().GetMatrix()));
			pCam->GetPhysics()->SetParams(&pab);
			return pCam;
		} else if (bCreate) 
		{
			CCamera& cam = GetISystem()->GetViewCamera();
			Quat qcam = Quat(Matrix33(cam.GetMatrix()));
			SEntitySpawnParams esp; 
			esp.sName = "CameraProxy"; esp.nFlags = 0;
			esp.pClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass("Default");
			IEntity *pCam = gEnv->pEntitySystem->SpawnEntity(esp);
			pCam->SetPos(cam.GetPosition()); pCam->SetRotation(qcam);
			pHost->AddEntityLink("CameraProxy", pCam->GetId());
			SEntityPhysicalizeParams epp;	epp.type = PE_ARTICULATED;
			pCam->Physicalize(epp);
			pe_geomparams gp;
			gp.flags=0; gp.flagsCollider=0;
			primitives::sphere sph; sph.r=0.1f; sph.center.zero();
			IGeometry *psph = gEnv->pPhysicalWorld->GetGeomManager()->CreatePrimitive(primitives::sphere::type, &sph);
			phys_geometry *pGeom = gEnv->pPhysicalWorld->GetGeomManager()->RegisterGeometry(psph); psph->Release();
			pCam->GetPhysics()->AddGeometry(pGeom,&gp);	pGeom->nRefCount--;
			pe_params_pos pp; pp.iSimClass=2;
			pCam->GetPhysics()->SetParams(&pp);
			pab.pHost = pHost->GetPhysics();
			pab.posHostPivot = (cam.GetPosition()-pHost->GetPos())*pHost->GetRotation();
			pab.qHostPivot = !pHost->GetRotation()*qcam;
			pCam->GetPhysics()->SetParams(&pab);
			return pCam;
		}
		return 0;
	}

	virtual void ProcessEvent( EFlowEvent event,SActivationInfo *pActInfo )
	{
		if (event==eFE_Activate)
		{
			bool bCreate = IsPortActive(pActInfo,IN_CREATE);
			if (IEntity *pHost = gEnv->pEntitySystem->GetEntity(GetPortEntityId(pActInfo,IN_ID)))
				if (IEntity *pCam = GetCameraEnt(pHost,bCreate))
					if (bCreate)
						ActivateOutput(pActInfo, OUT_ID, pCam->GetId());
		}
	}

	virtual void GetMemoryUsage(ICrySizer * s) const
	{
		s->Add(*this);
	}
};


class CFlowNode_Constraint : public CFlowBaseNode<eNCT_Instanced>
{
public:
	enum EInputs {
		IN_CREATE,
		IN_BREAK,
		IN_ID,
		IN_ENTITY0,
		IN_PARTID0,
		IN_ENTITY1,
		IN_PARTID1,
		IN_POINT,
		IN_IGNORE_COLLISIONS,
		IN_BREAKABLE,
		IN_FORCE_AWAKE,
		IN_MAX_FORCE,
		IN_MAX_TORQUE,
		IN_MAX_FORCE_REL,
		IN_AXIS,
		IN_MIN_ROT,
		IN_MAX_ROT,
		IN_MAX_BEND,
	};
	enum EOutputs {
		OUT_ID,
		OUT_BROKEN,
	};
	CFlowNode_Constraint( SActivationInfo * pActInfo ) {};
	~CFlowNode_Constraint() {}

	struct SConstraintRec {
		SConstraintRec() : next(0),prev(0),pent(0),pNode(0),idConstraint(-1),bBroken(0),minEnergy(0) {}
		~SConstraintRec() { Free(); }
		void Free() {
			if (idConstraint>=0) 
			{
				if (minEnergy>0)
				{
					pe_simulation_params sp; sp.minEnergy=minEnergy;
					pe_params_articulated_body pab; pab.minEnergyLyingMode=minEnergyRagdoll;
					pent->SetParams(&sp); pent->SetParams(&pab);
				}
				if (pent)
					pent->Release();
				idConstraint=-1; pent=0;
				(prev ? prev->next : g_pConstrRec0) = next;
				if (next) next->prev = prev;	
			}
		}
		SConstraintRec *next,*prev;
		IPhysicalEntity *pent;
		CFlowNode_Constraint *pNode;
		int idConstraint;
		int bBroken;
		float minEnergy,minEnergyRagdoll;
	};
	static SConstraintRec *g_pConstrRec0;

	static int OnConstraintBroken(const EventPhysJointBroken *ejb)
	{
		for(SConstraintRec *pRec=g_pConstrRec0; pRec; pRec=pRec->next) if (pRec->pent==ejb->pEntity[0] && pRec->idConstraint==ejb->idJoint)
			pRec->bBroken = 1;
		return 1;
	}

	virtual void GetConfiguration( SFlowNodeConfig &config )
	{
		static const SInputPortConfig in_config[] = {
			InputPortConfig<SFlowSystemVoid>( "Create", SFlowSystemVoid(), _HELP("Creates the constraint") ),
			InputPortConfig<SFlowSystemVoid>( "Break", SFlowSystemVoid(), _HELP("Breaks a constraint Id from EntityA if EntityA is activated, or a previously created one") ),
			InputPortConfig<int>( "Id", 1000, _HELP("Requested constraint Id; -1 to auto-generate") ),
			InputPortConfig<EntityId>( "EntityA", 0, _HELP("Constraint owner entity") ),
			InputPortConfig<int>( "PartIdA", -1, _HELP("Part id to attach to; -1 to use default") ),
			InputPortConfig<EntityId>( "EntityB", 0, _HELP("Constraint 'buddy' entity") ),
			InputPortConfig<int>( "PartIdB", -1, _HELP("Part id to attach to; -1 to use default") ),
			InputPortConfig<Vec3>( "Point", Vec3(ZERO), _HELP("Connection point in world space") ),
			InputPortConfig<bool>( "IgnoreCollisions", true, _HELP("Disables collisions between constrained entities") ),
			InputPortConfig<bool>( "Breakable", false, _HELP("Break if force limit is reached") ),
			InputPortConfig<bool>( "ForceAwake", false, _HELP("Make EntityB always awake; restore previous sleep params on Break") ),
			InputPortConfig<float>( "MaxForce", 0.0f, _HELP("Force limit") ),
			InputPortConfig<float>( "MaxTorque", 0.0f, _HELP("Rotational force (torque) limit") ),
			InputPortConfig<bool>( "MaxForceRelative", true, _HELP("Make limits relative to EntityB's mass") ),
			InputPortConfig<Vec3>( "TwistAxis", Vec3(0,0,1), _HELP("Main rotation axis in world space") ),
			InputPortConfig<float>( "MinTwist", 0.0f, _HELP("Lower rotation limit around TwistAxis") ),
			InputPortConfig<float>( "MaxTwist", 0.0f, _HELP("Upper rotation limit around TwistAxis") ),
			InputPortConfig<float>( "MaxBend", 0.0f, _HELP("Maximum bend of the TwistAxis") ),

			{0}
		};
		static const SOutputPortConfig out_config[] = {
			OutputPortConfig<int>("Id", _HELP("Constraint Id")),
			OutputPortConfig<bool>("Broken", _HELP("Activated when the constraint breaks")),
			{0}
		};
		config.sDescription = _HELP( "Creates a physical constraint between EntityA and EntityB" );
		config.pInputPorts = in_config;
		config.pOutputPorts = out_config;
		config.SetCategory(EFLN_ADVANCED);
	}

	virtual void ProcessEvent( EFlowEvent event,SActivationInfo *pActInfo )
	{
		SConstraintRec *pRec = 0, *pRecNext;
		if (event == eFE_Update)
		{
			pe_status_pos sp;
			for(pRec=g_pConstrRec0; pRec; pRec=pRecNext)
			{
				pRecNext = pRec->next;
				if (pRec->pNode==this && (pRec->bBroken || pRec->pent->GetStatus(&sp) && sp.iSimClass==7))
				{
					ActivateOutput(pActInfo, OUT_BROKEN, true);
					ActivateOutput(pActInfo, OUT_ID, pRec->idConstraint);
					delete pRec;
				}
			}
			if (!g_pConstrRec0)
				pActInfo->pGraph->SetRegularlyUpdated(pActInfo->myID, false);
		}
		else if (event == eFE_Activate)
		{
			if (IsPortActive(pActInfo, IN_CREATE))
			{
				IEntity *pent[2] = { gEnv->pEntitySystem->GetEntity(GetPortEntityId(pActInfo,IN_ENTITY0)), gEnv->pEntitySystem->GetEntity(GetPortEntityId(pActInfo,IN_ENTITY1)) };
				if (!pent[0] || !pent[0]->GetPhysics())
					return;
				IPhysicalEntity *pentPhys = pent[0]->GetPhysics();
				pe_action_add_constraint aac;
				int i; float f;
				aac.pBuddy = pent[1] ? pent[1]->GetPhysics() : WORLD_ENTITY;
				aac.id = GetPortInt(pActInfo, IN_ID);
				for(int iop=0;iop<2;iop++) if ((i=GetPortInt(pActInfo, IN_PARTID0+iop*2))>=0)
					aac.partid[iop] = i;
				aac.pt[0] = GetPortVec3(pActInfo, IN_POINT);
				aac.flags = world_frames | (GetPortBool(pActInfo,IN_IGNORE_COLLISIONS) ? constraint_ignore_buddy:0) | (GetPortBool(pActInfo,IN_BREAKABLE) ? 0:constraint_no_tears);
				pe_status_dynamics sd; sd.mass = 1.0f;
				if (GetPortBool(pActInfo,IN_MAX_FORCE_REL))
					pentPhys->GetStatus(&sd);
				if ((f = GetPortFloat(pActInfo, IN_MAX_FORCE))>0)
					aac.maxPullForce = f*sd.mass;
				if ((f = GetPortFloat(pActInfo, IN_MAX_TORQUE))>0)
					aac.maxBendTorque = f*sd.mass;
				for(int iop=0;iop<2;iop++)
					aac.xlimits[iop] = DEG2RAD(GetPortFloat(pActInfo, IN_MIN_ROT+iop));
				aac.yzlimits[0] = 0;
				aac.yzlimits[1] = DEG2RAD(GetPortFloat(pActInfo, IN_MAX_BEND));
				if (aac.xlimits[1]<=aac.xlimits[0] && aac.yzlimits[1]<=aac.yzlimits[0])
					aac.flags |= constraint_no_rotation;
				else if (aac.xlimits[0]<gf_PI*-1.01f && aac.xlimits[1]>gf_PI*1.01f && aac.yzlimits[1]>gf_PI*0.51f)
					MARK_UNUSED aac.xlimits[0],aac.xlimits[1],aac.yzlimits[0],aac.yzlimits[1];
				aac.qframe[0] = aac.qframe[1] = Quat::CreateRotationV0V1(Vec3(1,0,0),GetPortVec3(pActInfo, IN_AXIS));
				if (GetPortBool(pActInfo, IN_FORCE_AWAKE))
				{
					pRec = new SConstraintRec;
					pe_simulation_params sp; sp.minEnergy=0;
					pentPhys->GetParams(&sp); pRec->minEnergy=pRec->minEnergyRagdoll = sp.minEnergy;
					new(&sp) pe_simulation_params; sp.minEnergy = 0;
					pentPhys->SetParams(&sp);
					pe_params_articulated_body pab; 
					if (pentPhys->GetParams(&pab))
					{
						pRec->minEnergyRagdoll = pab.minEnergyLyingMode;
						new(&pab) pe_params_articulated_body; pab.minEnergyLyingMode=0;
						pentPhys->SetParams(&pab);
					}
					pe_action_awake aa;	aa.minAwakeTime=0.1f;
					pentPhys->Action(&aa);
					if (aac.pBuddy!=WORLD_ENTITY)
						aac.pBuddy->Action(&aa);
				}
				pe_params_flags pf;
				int queued = aac.pBuddy==WORLD_ENTITY ? 0 : aac.pBuddy->SetParams(&pf)-1, id = pentPhys->Action(&aac,-queued>>31);
				ActivateOutput(pActInfo, OUT_ID, id);
				if (!is_unused(aac.maxPullForce || !is_unused(aac.maxBendTorque)) && !(aac.flags & constraint_no_tears))
				{
					if (!pRec)
						pRec = new SConstraintRec;
					gEnv->pPhysicalWorld->AddEventClient(EventPhysJointBroken::id, (int(*)(const EventPhys*))OnConstraintBroken, 1);
				}
				if (pRec)
				{
					(pRec->pent = pentPhys)->AddRef();
					pRec->idConstraint = id;
					pRec->pNode = this;
					if (g_pConstrRec0)
						g_pConstrRec0->prev = pRec;
					pRec->next = g_pConstrRec0; g_pConstrRec0 = pRec;
					pActInfo->pGraph->SetRegularlyUpdated(pActInfo->myID, true);
				}
				return;
			}

			if (IsPortActive(pActInfo, IN_BREAK) || IsPortActive(pActInfo, IN_POINT))
				if (IEntity *pent = gEnv->pEntitySystem->GetEntity(GetPortEntityId(pActInfo,IN_ENTITY0)))
					if (IPhysicalEntity *pentPhys = pent->GetPhysics())
					{
						pe_action_update_constraint auc; 
						auc.idConstraint = GetPortInt(pActInfo, IN_ID);
						if (IsPortActive(pActInfo, IN_BREAK))
							auc.bRemove = 1;
						if (IsPortActive(pActInfo, IN_POINT))
							auc.pt[1] = GetPortVec3(pActInfo, IN_POINT);
						pentPhys->Action(&auc);
						if (auc.bRemove) 
						{
							for(pRec=g_pConstrRec0; pRec; pRec=pRecNext) 
							{
								pRecNext = pRec->next;
								if (pRec->pent==pentPhys && pRec->idConstraint==auc.idConstraint)
									delete pRec;
							}
							ActivateOutput(pActInfo, OUT_BROKEN, true);
							ActivateOutput(pActInfo, OUT_ID, auc.idConstraint);
							if (!g_pConstrRec0)
								pActInfo->pGraph->SetRegularlyUpdated(pActInfo->myID, false);
						}
					}
		}
	}

	virtual void GetMemoryUsage(ICrySizer * s) const
	{
		s->Add(*this);
	}
};

CFlowNode_Constraint::SConstraintRec *CFlowNode_Constraint::g_pConstrRec0 = 0;


REGISTER_FLOW_NODE( "Physics:Dynamics", CFlowNode_Dynamics );
REGISTER_FLOW_NODE( "Physics:ActionImpulse", CFlowNode_ActionImpulse );
REGISTER_FLOW_NODE( "Physics:RayCast", CFlowNode_Raycast );
REGISTER_FLOW_NODE( "Physics:RayCastCamera", CFlowNode_RaycastCamera );
REGISTER_FLOW_NODE( "Physics:PhysicsEnable", CFlowNode_PhysicsEnable );
REGISTER_FLOW_NODE( "Physics:PhysicsSleepQuery", CFlowNode_PhysicsSleepQuery );
REGISTER_FLOW_NODE( "Physics:Constraint", CFlowNode_Constraint );
REGISTER_FLOW_NODE( "Physics:CameraProxy", CFlowNode_CameraProxy );
