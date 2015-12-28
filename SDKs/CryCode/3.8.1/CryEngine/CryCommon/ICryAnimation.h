////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002 - 2014.
// -------------------------------------------------------------------------
//  File name:   ICryAnimation.h
//  Created:     1/8/2004 by Ivo Herzeg
//  Description: CryAnimation interface
//
////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef CRYANIMATION_API
	#ifdef CRYANIMATION_EXPORTS
		#define CRYANIMATION_API DLL_EXPORT
	#else
		#define CRYANIMATION_API DLL_IMPORT
	#endif
#endif

#define ENABLE_RUNTIME_POSE_MODIFIERS

// Define ENABLE_BBOX_EXCLUDE_LIST if you want to add support for the
// BBoxExcludeList feature in the chrparams file. This feature is deprecated.
// Instead, we recommend you use a combination of BBoxIncludeList (listing a
// minimal amount of joints to get a reasonable bounding box) and
// BBoxExtension (to extend the bbox a little bit).
//
// #define ENABLE_BBOX_EXCLUDE_LIST

//DOC-IGNORE-BEGIN
#include <Cry_Math.h>
#include <Cry_Geo.h>

#include <IRenderer.h>
#include <IPhysics.h>
#include <I3DEngine.h>
#include <IEntityRenderState.h>
#include <IRenderAuxGeom.h>
#include <IEntitySystem.h>
#include <CryExtension/ICryUnknown.h>

#include "CryCharAnimationParams.h"


// Flags used by ICharacterInstance::SetFlags and GetFlags
enum ECharRenderFlags
{
	CS_FLAG_DRAW_MODEL           = 1 << 0,
	CS_FLAG_DRAW_NEAR            = 1 << 1,
	CS_FLAG_UPDATE               = 1 << 2,
	CS_FLAG_UPDATE_ALWAYS        = 1 << 3,
	CS_FLAG_COMPOUND_BASE        = 1 << 4,

	CS_FLAG_DRAW_WIREFRAME       = 1 << 5, //just for debug
	CS_FLAG_DRAW_TANGENTS        = 1 << 6, //just for debug
	CS_FLAG_DRAW_BINORMALS       = 1 << 7, //just for debug
	CS_FLAG_DRAW_NORMALS         = 1 << 8, //just for debug

	CS_FLAG_DRAW_LOCATOR         = 1 << 9, //just for debug
	CS_FLAG_DRAW_SKELETON        = 1 << 10,//just for debug

	CS_FLAG_BIAS_SKIN_SORT_DIST  = 1 << 11,

	CS_FLAG_STREAM_HIGH_PRIORITY = 1 << 12,
};


enum CHRLOADINGFLAGS
{
	CA_IGNORE_LOD                = 0x01,
	CA_CharEditModel             = 0x02,
	CA_PreviewMode               = 0x04,
	CA_DoNotStreamStaticObjects  = 0x08,
	CA_SkipSkelRecreation        = 0x10,
	CA_DisableLogWarnings        = 0x20,
};


enum EReloadCAFResult
{
	CR_RELOAD_FAILED,
	CR_RELOAD_SUCCEED,
	CR_RELOAD_GAH_NOT_IN_ARRAY
};

enum CharacterToolFlags
{
	CA_CharacterTool       = 0x01,
	CA_DrawSocketLocation  = 0x02,
	CA_BindPose            = 0x04,
	CA_AllowRedirection    = 0x08, //allow redirection in bindpose
};

#define CHR (0x11223344)
#define CGA (0x55aa55aa)

#define NULL_ANIM "null"
#define NULL_ANIM_FILE "null"


// Forward declarations
struct  IShader;
struct  SRendParams;
struct CryEngineDecalInfo;
struct ParticleParams;
struct CryCharMorphParams;
struct IMaterial;
struct IStatObj;
struct IRenderMesh;
class CDLight;

class CDefaultSkeleton;

struct ICharacterManager;
struct ICharacterInstance;
struct ISkin;
struct IAttachmentSkin;
struct IDefaultSkeleton;
struct IAnimationSet;

struct ISkeletonAnim;
struct ISkeletonPose;
struct IAttachmentManager;

struct IAttachment;
struct IAttachmentObject; // Entity, static object or character
struct IAnimEvents;
struct TFace;
struct IFacialInstance;
struct IFacialAnimation;

class ICrySizer;
struct CryCharAnimationParams;

struct IAnimationPoseModifier;
typedef int (*CallBackFuncType)(ICharacterInstance*, void*);

struct IAnimationStreamingListener;
struct IAnimationSetListener;

struct IVertexFrames;
class CLodValue;

namespace Serialization { class IArchive; }

struct IAnimationSerializable :
	public ICryUnknown
{
	CRYINTERFACE_DECLARE(IAnimationSerializable, 0x69b4f3ae61974bee, 0xba70d361b7975e69)

	virtual void Serialize(Serialization::IArchive& ar) = 0;
};


DECLARE_BOOST_POINTERS(IAnimationSerializable);



// Description:
//     This class is the main access point for any character animation
//     required for a program which uses CryEngine.
// See Also:
//     CreateCharManager
struct ICharacterManager
{
	// Description:
	//     Priority when requested to load a DBA
	// See also:
	//     DBA_PreLoad
	//     DBA_LockStatus
	enum EStreamingDBAPriority
	{
		eStreamingDBAPriority_Normal = 0,
		eStreamingDBAPriority_Urgent = 1,
	};

	// Description:
	//     Contains statistics about CryCharManager.
	struct Statistics
	{
		// Number of character instances
		unsigned numCharacters;
		// Number of character models (CGF)
		unsigned numCharModels;
		// Number of animobjects
		unsigned numAnimObjects;
		// Number of animobject models
		unsigned numAnimObjectModels;
	};

	// <interfuscator:shuffle>
	virtual ~ICharacterManager() {}

	// Description:
	//     Will fill the Statistics parameters with statistic on the instance
	//     of the Animation System.
	//     It isn't recommended to call this function often.
	// Arguments:
	//     rStats - Structure which holds the statistics
	// Summary:
	//     Get statistics on the Animation System
	virtual void GetStatistics(Statistics& rStats) const = 0;

	// Description:
	//     Gather the memory currently used by the animation. The information
	//     returned is classified according to the flags set in the sizer
	//     argument.
	// Arguments:
	//     pSizer - Sizer class which will store the memory usage
	// Summary:
	//     Track memory usage
	virtual void GetMemoryUsage(ICrySizer* pSizer) const = 0;

	// Description:
	//     Create a new instance for a model. Load the model file along with any animation file that might be
	//     available.
	// See Also:
	//     RemoveCharacter
	// Arguments:
	//     szFilename - Filename of the model to be loaded
	//     nFlags     - Set how the model will be kept in memory after being used. Uses flags defined with EModelPersistence.
	// Return Value:
	//     A pointer to a ICharacterInstance class if the model could be loaded
	//     properly.
	//     NULL if the model couldn't be loaded.
	// Summary:
	//     Create a new instance of a model
	virtual ICharacterInstance* CreateInstance(const char* szFilename, uint32 nLoadingFlags = 0) = 0;
	virtual IDefaultSkeleton* LoadModelSKEL(const char* szFilePath, uint32 nLoadingFlags) = 0;
	virtual ISkin* LoadModelSKIN(const char* szFilePath, uint32 nLoadingFlags) = 0;

	// Description:
	//     Find and prefetch all resources for the required file character
	// Arguments:
	//     szFilename - Filename of the character to be prefetched
	//     nFlags     - Set how the model will be kept in memory after being used. Uses flags defined with EModelPersistence.
	// Return Value:
	//     Smart pointer to the placeholder representation of the character
	virtual bool LoadAndLockResources(const char* szFilePath, uint32 nLoadingFlags) = 0;

	// Description:
	//     Ensure that render meshes are resident, and if not, begin streaming them.
	//     Each call with bKeep == true must be paired with a call with bKeep == false when no longer needed
	virtual void StreamKeepCharacterResourcesResident(const char* szFilePath, int nLod, bool bKeep, bool bUrgent = false) = 0;
	virtual bool StreamHasCharacterResources(const char* szFilePath, int nLod) = 0;

	// Description:
	//     Cleans up all resources. Currently deletes all bodies and characters even if there are references on them.
	// Arguments:
	//     bForceCleanup - When set to true will force all models to be deleted, even if references to them still left.
	// Summary:
	//     Cleans up all resources
	virtual void ClearResources(bool bForceCleanup) = 0;

	// Description:
	//     Update the Animation System. It's important to call this function at every frame. This should perform very fast.
	// Summary:
	//     Update the Animation System
	virtual void Update(bool bPaused) = 0;

	// Description:
	//     Update the character streaming.
	virtual void UpdateStreaming(int nFullUpdateRoundId, int nFastUpdateRoundId) = 0;

	// Description:
	//     Increment the frame counter.
	// Summary:
	//     Useful to prevent log spam: "several updates per frame..."
	virtual void DummyUpdate() = 0;

	// Description:
	//     Releases any resource allocated by the Animation System and shut it down properly.
	// Summary:
	//     Release the Animation System
	virtual void Release() = 0;

	// Description:
	//     Retrieve all loaded models.
	virtual void GetLoadedModels(IDefaultSkeleton** pIDefaultSkeletons, uint32& nCount) const = 0;

	// Description
	//     Reloads loaded model
	virtual void ReloadAllModels() = 0;
	virtual void ReloadAllCHRPARAMS() = 0;

	virtual void PreloadLevelModels() = 0;

	//////////////////////////////////////////////////////////////////////////
	// Retrieve facial animation interface.
	virtual IFacialAnimation* GetIFacialAnimation() = 0;
	virtual const IFacialAnimation* GetIFacialAnimation() const = 0;

	virtual IAnimEvents* GetIAnimEvents() = 0;
	virtual const IAnimEvents* GetIAnimEvents() const = 0;


	// Description:
	//    Use to synchronize all animation computations like forward kinematics, and calls
	//    all function which must occur after these like SkeletonPostProcess
	//    Should be called only once per frame and as late as possible to prevent waiting
	//    for functions which run asynchronously
	virtual void SyncAllAnimations() = 0;

	// Description:
	//    Adds an animation to the synchronize queue from SyncAllAnimations
	virtual void AddAnimationToSyncQueue(ICharacterInstance* pCharInstance) = 0;

	// Description:
	//    Removes an animation to the synchronize queue from SyncAllAnimations
	virtual void RemoveAnimationToSyncQueue(ICharacterInstance* pCharInstance) = 0;

	// Functions to load, lock and unload dba files
	virtual bool DBA_PreLoad(const char* filepath, ICharacterManager::EStreamingDBAPriority priority) = 0;
	virtual bool DBA_LockStatus(const char* filepath, uint32 status, ICharacterManager::EStreamingDBAPriority priority) = 0;
	virtual bool DBA_Unload(const char* filepath) = 0;
	virtual bool DBA_Unload_All() = 0;

	// Adds a runtime reference to a CAF animation; if not loaded it starts streaming it
	virtual bool CAF_AddRef(uint32 filePathCRC) = 0;
	virtual bool CAF_IsLoaded(uint32 filePathCRC) const = 0;
	virtual bool CAF_Release(uint32 filePathCRC) = 0;
	virtual bool CAF_LoadSynchronously(uint32 filePathCRC) = 0;
	virtual bool LMG_LoadSynchronously(uint32 filePathCRC, const IAnimationSet* pAnimationSet) = 0;

	virtual EReloadCAFResult ReloadCAF(const char* szFilePathCAF) = 0;
	virtual int ReloadLMG(const char* szFilePathCAF) = 0;

	// Return the DBA filename of an animation that is stored in a DBA
	virtual const char* GetDBAFilePathByGlobalID(int32 globalID) const = 0;

	// Set the listener which listens to events that happen during animation streaming
	virtual void SetStreamingListener(IAnimationStreamingListener* pListener) = 0;

	// Add nTicks to the number of Ticks spent this frame in animation functions
	virtual void AddFrameTicks(uint64 nTicks) = 0;

	// Add nTicks to the number of Ticks spent this frame in syncing animation jobs
	virtual void AddFrameSyncTicks(uint64 nTicks) = 0;

	// Reset Ticks Counter
	virtual void ResetFrameTicks() = 0;

	// Get number of Ticks accumulated over this frame
	virtual uint64 NumFrameTicks() const = 0;

	// Get number of Ticks accumulated over this frame in sync functions
	virtual uint64 NumFrameSyncTicks() const = 0;

	// Get the number of character instances that were processed asynchronously
	virtual uint32 NumCharacters() const = 0;

	virtual uint32 GetNumInstancesPerModel(const IDefaultSkeleton& rIDefaultSkeleton) const = 0;
	virtual ICharacterInstance* GetICharInstanceFromModel(const IDefaultSkeleton& rIDefaultSkeleton, uint32 num) const = 0;;

	virtual void SetAnimMemoryTracker(const SAnimMemoryTracker& amt) = 0;
	virtual SAnimMemoryTracker GetAnimMemoryTracker() const = 0;

	virtual void UpdateRendererFrame() = 0;
	virtual void PostInit() = 0;
	// </interfuscator:shuffle>

#if BLENDSPACE_VISUALIZATION
	virtual void CreateDebugInstances(const char* szFilename) = 0;
	virtual void DeleteDebugInstances() = 0;
	virtual void RenderDebugInstances(const SRenderingPassInfo& passInfo) = 0;
	virtual void RenderBlendSpace(const SRenderingPassInfo& passInfo, ICharacterInstance* character, float fCharacterScale, unsigned int debugFlags) = 0;
	virtual bool HasDebugInstancesCreated(const char* szFilename) const = 0;
#endif
#ifdef EDITOR_PCDEBUGCODE
	virtual void GetMotionParameterDetails(SMotionParameterDetails& outDetails, EMotionParamID paramId) const = 0;
	virtual bool InjectCDF(const char* pathname, const char* content, size_t contentLength) = 0;
	virtual void ClearCDFCache() = 0;
	virtual void ClearAllKeepInMemFlags() = 0;
	virtual void InjectCHRPARAMS(const char* pathname, const char* content, size_t contentLength) = 0;
	virtual void ClearCHRPARAMSCache() = 0;
	virtual void InjectBSPACE(const char* pathname, const char* content, size_t contentLength) = 0;
	virtual void ClearBSPACECache() = 0;
#endif
};

// Description:
//    This struct defines the interface for a class that listens to
//    AnimLoaded, AnimUnloaded and AnimReloaded events

struct IAnimationStreamingListener
{
	// <interfuscator:shuffle>
	virtual ~IAnimationStreamingListener() {}

	// Called when an animation finished loading
	virtual void NotifyAnimLoaded(const int32 globalID) = 0;

	// Called when an animation gets unloaded
	virtual void NotifyAnimUnloaded(const int32 globalID) = 0;

	// Called when an animation is reloaded from file
	virtual void NotifyAnimReloaded(const int32 globalID) = 0;
	// </interfuscator:shuffle>
};


struct SJointProperty
{
	SJointProperty(const char* pname, float val) { name = pname; type = 0; fval = val; }
	SJointProperty(const char* pname, bool val) { name = pname; type = 1; bval = val; }
	SJointProperty(const char* pname, const char* val) { name = pname; type = 2; strval = val; }
	const char* name;
	int type;
	union
	{
		float fval;
		bool bval;
		const char* strval;
	};
};



//////////////////////////////////////////////////////////////////////////
typedef unsigned int LimbIKDefinitionHandle;

struct IDefaultSkeleton
{
	// <interfuscator:shuffle>
	virtual ~IDefaultSkeleton() {}
	virtual uint32 GetJointCount() const = 0;

	virtual int32 GetJointParentIDByID(int32 id) const = 0;
	virtual int32 GetControllerIDByID(int32 id) const = 0;

	virtual int32 GetJointIDByCRC32(uint32 crc32) const = 0;
	virtual uint32 GetJointCRC32ByID(int32 id) const = 0;

	virtual const char* GetJointNameByID(int32 id) const = 0;
	virtual int32 GetJointIDByName(const char* name) const = 0;

	virtual const QuatT& GetDefaultAbsJointByID(uint32 nJointIdx) const = 0;
	virtual const QuatT& GetDefaultRelJointByID(uint32 nJointIdx) const = 0;

	virtual const char* GetModelFilePath() const = 0; // Returns the file-path for this model

	// NOTE: Will become deprecated.
	//All render-meshes will be removed from the CDefaultSkeleton-class
	virtual const phys_geometry* GetJointPhysGeom(uint32 jointIndex) const = 0;  //just for statistics of physics proxies
	virtual int32 GetLimbDefinitionIdx(LimbIKDefinitionHandle handle) const = 0;
	virtual void PrecacheMesh(bool bFullUpdate, int nRoundId, int nLod) = 0;
	virtual IRenderMesh* GetIRenderMesh() const = 0;
	virtual Vec3 GetRenderMeshOffset() const = 0;
	virtual uint32 GetTextureMemoryUsage2(ICrySizer* pSizer = 0) const = 0;
	virtual uint32 GetMeshMemoryUsage(ICrySizer* pSizer = 0) const = 0;
	// END: Will become deprecated.
	// </interfuscator:shuffle>
};

//////////////////////////////////////////////////////////////////////////
struct ISkin
{
	// <interfuscator:shuffle>
	virtual ~ISkin() {}

	// Precache (streaming support)
	virtual void PrecacheMesh(bool bFullUpdate, int nRoundId, int nLod) = 0;

	// Retrieve render mesh for specified lod.
	virtual IRenderMesh* GetIRenderMesh(uint32 nLOD) const = 0;
	virtual const char* GetModelFilePath() const = 0;
	virtual IMaterial* GetIMaterial(uint32 nLOD) const = 0;
	// </interfuscator:shuffle>

	virtual uint32 GetNumLODs() const = 0;

#ifdef EDITOR_PCDEBUGCODE
	virtual Vec3 GetRenderMeshOffset(uint32 nLOD) const = 0;
	virtual uint32 GetTextureMemoryUsage2(ICrySizer* pSizer = 0) const = 0;
	virtual uint32 GetMeshMemoryUsage(ICrySizer* pSizer = 0) const = 0;
	virtual const IVertexFrames* GetVertexFrames() const = 0;
#endif
};



//DOC-IGNORE-BEGIN
//! TODO:
//! Split this interface up into a few logical interfaces, starting with the ICryCharModel
//DOC-IGNORE-END

struct SCharUpdateFeedback
{
	SCharUpdateFeedback() { flags = 0; pPhysHost = 0; mtxDelta.SetIdentity(); }
	int flags;                  // |1 if pPhysHost is valid, |2 is mtxDelta is valid
	IPhysicalEntity* pPhysHost; // tells the caller to restore this host as the main phys entity
	Matrix34 mtxDelta;          // tells the caller to instantly post-multiply its matrix with this one
};

struct SAnimationProcessParams
{
	QuatTS locationAnimation;
	bool bOnRender;
	float zoomAdjustedDistanceFromCamera;
	float overrideDeltaTime;

	SAnimationProcessParams() :
		locationAnimation(IDENTITY),
		bOnRender(false),
		zoomAdjustedDistanceFromCamera(0.0f),
		overrideDeltaTime(-1.0f)
	{
	}
};

// Description:
//     This interface contains methods for manipulating and querying an animated character
//     instance. The methods allow to modify the animated instance, animate it, render,
//     retrieve BBox/etc, control physics, particles and skinning, transform.
// Summary:
//     Interface to character animation
struct ICharacterInstance
{
	// <interfuscator:shuffle>
	virtual ~ICharacterInstance() {}

	// Increase reference count of the interface
	virtual void AddRef() = 0;

	// Decrease reference count of the interface
	virtual void Release() = 0;

	// Query current reference count
	virtual int GetRefCount() const = 0;

	//////////////////////////////////////////////////////////////////////
	// Description:
	//     Return a pointer of the instance of an ISkeletonAnim derived class applicable for the model.
	// Return Value:
	//     A pointer to an ISkeletonAnim derived class
	// Summary:
	//     Get the skeleton for this instance
	virtual ISkeletonAnim* GetISkeletonAnim() = 0;
	virtual const ISkeletonAnim* GetISkeletonAnim() const = 0;

	//////////////////////////////////////////////////////////////////////
	// Description:
	//     Return a pointer of the instance of an ISkeletonPose derived class applicable for the model.
	// Return Value:
	//     A pointer to an ISkeletonPose derived class
	// Summary:
	//     Get the skeleton for this instance
	virtual ISkeletonPose* GetISkeletonPose() = 0;
	virtual const ISkeletonPose* GetISkeletonPose() const = 0;

	//////////////////////////////////////////////////////////////////////
	// Description:
	//     Return a pointer of the instance of an IAttachmentManager derived class applicable for the model.
	// Return Value:
	//     A pointer to an IAttachmentManager derived class
	// Summary:
	//     Get the attachment manager for this instance
	virtual IAttachmentManager* GetIAttachmentManager() = 0;
	virtual const IAttachmentManager* GetIAttachmentManager() const = 0;

	//////////////////////////////////////////////////////////////////////
	// Description:
	//     Return the shared character model used by this instance.
	virtual IDefaultSkeleton& GetIDefaultSkeleton() = 0;
	virtual const IDefaultSkeleton& GetIDefaultSkeleton() const = 0;

	// Description:
	//     Return a pointer of the instance of an ICryAnimationSet derived class applicable for the model.
	// Return Value:
	//     A pointer to a ICryAnimationSet derived class
	// Summary:
	//     Get the Animation Set defined for the model
	virtual IAnimationSet* GetIAnimationSet() = 0;
	virtual const IAnimationSet* GetIAnimationSet() const = 0;

	// Description:
	//    Get the name of the file that stores the animation event definitions for this model. This
	//    is usually stored in the CHRPARAMS file
	// Return Value:
	//    A pointer to a null terminated char string which contains the filename of the database
	// Summary:
	//    Get the animation event file
	virtual const char* GetModelAnimEventDatabase() const = 0;

	//enables/disables StartAnimation* calls; puts warning into the log if StartAnimation* is called while disabled
	virtual void EnableStartAnimation(bool bEnable) = 0;
	virtual void StartAnimationProcessing(const SAnimationProcessParams& params) = 0;

	//! Return dynamic bbox of object
	// Description:
	// Arguments:
	// Summary:
	//     Get the bounding box
	virtual const AABB& GetAABB() const = 0;

	// Description:
	//     Return the radius of the object, squared
	// Arguments:
	// Summary:
	//     Get the radius of the object, squared
	virtual float GetRadiusSqr() const = 0;

	// Summary:
	//     Return the extent (length, volume, or area) of the object
	// Arguments:
	//     eForm - See RandomPos
	virtual float GetExtent(EGeomForm eForm) = 0;

	// Summary:
	//     Generate a random point in object.
	// Arguments:
	//     eForm - Object aspect to generate on (surface, volume, etc)
	virtual void GetRandomPos(PosNorm& ran, EGeomForm eForm) const = 0;

	// Description:
	//     Computes the render LOD from the wanted LOD value
	virtual CLodValue ComputeLod(int wantedLod, const SRenderingPassInfo& passInfo) = 0;

	// Description:
	//     Draw the character using specified rendering parameters.
	// Arguments:
	//     RendParams - Rendering parameters
	// Summary:
	//     Draw the character
	virtual void Render(const SRendParams& RendParams, const QuatTS& Offset,  const SRenderingPassInfo& passInfo) = 0;

	// Description:
	//     Set rendering flags defined in ECharRenderFlags for this character instance
	// Arguments:
	//     Pass the rendering flags
	// Summary:
	//     Set rendering flags
	virtual void SetFlags(int nFlags) = 0;

	// Description:
	//     Get the rendering flags enabled. The valid flags are the ones declared in ECharRenderFlags.
	// Return Value:
	//     Return an integer value which holds the different rendering flags
	// Summary:
	//     Set rendering flags
	virtual int GetFlags() const = 0;

	// Description:
	//     Get the object type contained inside the character instance. It will return either the CHR or CGA constant.
	// Return Value:
	//     An object type constant
	// Summary:
	//     Get the object type
	virtual int GetObjectType() const = 0;

	// Description:
	//     Get the filename of the character.
	//     This is either the model path or the CDF path.
	// Return Value:
	//     A pointer to a null terminated char string which contain the filename of the character.
	// Summary:
	//     Get the filename of the character
	virtual const char* GetFilePath() const = 0;

	// Description:
	//     Enable and disable decals for this instance. By default its always disabled
	virtual void EnableDecals(uint32 d) = 0;

	// Description:
	//     Spawn a decal on animated characters
	//     The decal hit direction and source point are in the local coordinates of the character.
	virtual void CreateDecal(CryEngineDecalInfo& DecalLCS) = 0;

	virtual void ComputeGeometricMean(SMeshLodInfo& lodInfo) const = 0;

	virtual bool HasVertexAnimation() const = 0;

	// Description:
	//     Returns material used to render this character, can be either a custom or model material.
	// Return Value:
	//     A pointer to an IMaterial class.
	virtual IMaterial* GetIMaterial() const = 0;

	// Description:
	//     Set custom instance material for this character.
	// Arguments:
	//     pMaterial - A valid pointer to the material.
	virtual void SetIMaterial_Instance(IMaterial* pMaterial) = 0;

	// Description:
	//     Returns the instance specific material - if this is 0, then the default model material is in use.
	// Return Value:
	//     A pointer to the material, or 0.
	virtual IMaterial* GetIMaterial_Instance() const = 0;

	//////////////////////////////////////////////////////////////////////////
	// Facial interface.
	//////////////////////////////////////////////////////////////////////////
	virtual IFacialInstance* GetFacialInstance() = 0;
	virtual const IFacialInstance* GetFacialInstance() const = 0;
	virtual void EnableFacialAnimation(bool bEnable) = 0;
	virtual void EnableProceduralFacialAnimation(bool bEnable) = 0;

	//////////////////////////////////////////////////////////////////////////
	// Lip sync character with the played sound.
	//////////////////////////////////////////////////////////////////////////
	virtual void LipSyncWithSound(uint32 nSoundId, bool bStop = false) = 0;

	// Description:
	//     Set animation speed scale
	//     This is the scale factor that affects the animation speed of the character.
	//     All the animations are played with the constant real-time speed multiplied by this factor.
	//     So, 0 means still animations (stuck at some frame), 1 - normal, 2 - twice as fast, 0.5 - twice slower than normal.
	virtual void SetPlaybackScale(f32 fSpeed) = 0;
	virtual f32 GetPlaybackScale() const = 0;
	virtual uint32 IsCharacterVisible() const = 0;

	// Skeleton effects interface.
	virtual void SpawnSkeletonEffect(int animID, const char* animName, const char* effectName, const char* boneName, const Vec3& offset, const Vec3& dir, const QuatTS& entityLoc) = 0;
	virtual void KillAllSkeletonEffects() = 0;

	virtual void SetViewdir(const Vec3& rViewdir) = 0;
	virtual float GetUniformScale() const = 0;

	virtual void CopyPoseFrom(const ICharacterInstance& instance) = 0;

	// Functions for asynchronous execution of ProcessAnimationUpdate and ForwardKinematics

	// Makes sure all functions which must be called after forward kinematics are executed and also synchronizes the execution
	// Is called during GetISkeletonAnim and GetISkeletonPose if the bNeedSync flag is set to true(the default) to ensure all computations have finished when
	// needed
	virtual void FinishAnimationComputations() = 0;

	// This is a hack to keep entity attachments in synch.
	virtual void SetAttachmentLocation_DEPRECATED(const QuatTS& newCharacterLocation) = 0;
	virtual void ProcessAttachment(IAttachment* pIAttachment) = 0;
	virtual void OnDetach() = 0; // is called when the character is detached (if it was an attachment)
	virtual void HideMaster(uint32 h) = 0; // disable rendering of this render this instance

	//! Pushes the underlying tree of objects into the given Sizer object for statistics gathering
	virtual void GetMemoryUsage(class ICrySizer* pSizer) const = 0;
	virtual void Serialize(TSerialize ser) = 0;
	// </interfuscator:shuffle>

#ifdef EDITOR_PCDEBUGCODE
	virtual uint32 GetResetMode() const = 0;  //obsolere when we remove CharEdit
	virtual void SetResetMode(uint32 rm) = 0; //obsolere when we remove CharEdit
	virtual f32 GetAverageFrameTime() const = 0;
	virtual void SetCharEditMode(uint32 m) = 0;
	virtual uint32 GetCharEditMode() const = 0;
	virtual void DrawWireframeStatic(const Matrix34& m34, int nLOD, uint32 color) = 0;

	// Reload the animation set at any time
	virtual void ReloadCHRPARAMS() = 0;
#endif
};





//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------

#include <IAnimationPoseModifier.h> // <> required for Interfuscator

struct ISkeletonAnim
{
	// <interfuscator:shuffle>
	enum
	{
		LayerCount = 16
	};

	virtual ~ISkeletonAnim() {}
	// Description:
	//     Enable special debug text for this skeleton
	virtual void SetDebugging(uint32 flags) = 0;

	// Motion initialization
	virtual void SetMirrorAnimation(uint32 ts)  = 0;
	virtual void SetAnimationDrivenMotion(uint32 ts)  = 0;
	virtual uint32 GetAnimationDrivenMotion() const = 0;
	virtual void SetTrackViewExclusive(uint32 i) = 0;
	virtual void SetTrackViewMixingWeight(uint32 layer, f32 weight) = 0;
	virtual uint32 GetTrackViewStatus() const = 0;

	// Motion playback and blending
	virtual bool StartAnimation(const char* szAnimName0, const CryCharAnimationParams& Params) = 0;
	virtual bool StartAnimationById(int32 id, const CryCharAnimationParams& Params) = 0;
	virtual bool StopAnimationInLayer(int32 nLayer, f32 BlendOutTime) = 0;
	virtual bool StopAnimationsAllLayers() = 0;

	// Description
	//   Find an animation with a given user token.
	// Arguments:
	//   nUserToken - User token that identifies an animation to search for.
	//   nLayer - Layer of FIFO where to search for animation, if -1 all layers are searched.
	virtual CAnimation* FindAnimInFIFO(uint32 nUserToken, int nLayer = 1) = 0;
	virtual const CAnimation* FindAnimInFIFO(uint32 nUserToken, int nLayer = 1) const = 0;

	// Description
	//   Remove an animation with a given index and given layer.
	// Arguments:
	//   nLayer - Animation layer to remove from.
	//   num - Transition queue index to remove at.
	//   forceRemove - Ignore special conditions and force a removal from the layer.
	virtual bool RemoveAnimFromFIFO(uint32 nLayer, uint32 num, bool forceRemove = false) = 0;
	virtual int  GetNumAnimsInFIFO(uint32 nLayer) const = 0;
	virtual void ClearFIFOLayer(uint32 nLayer) = 0;
	virtual CAnimation& GetAnimFromFIFO(uint32 nLayer, uint32 num) = 0;
	virtual const CAnimation& GetAnimFromFIFO(uint32 nLayer, uint32 num) const = 0;
	// If manual update is set for anim, then set anim time and handle anim events.
	virtual void ManualSeekAnimationInFIFO(uint32 nLayer, uint32 num, float time, bool triggerAnimEvents) = 0;

	// Makes sure there's no anim in this layer's queue that could cause a delay (useful when you want to play an
	// animation that you want to be 100% sure is going to be transitioned to immediately)
	virtual void RemoveTransitionDelayConditions(uint32 nLayer) = 0;

	virtual void SetLayerBlendWeight(int32 nLayer, f32 fMult) = 0;
	virtual void SetLayerPlaybackScale(int32 nLayer, f32 fSpeed) = 0;
	virtual f32 GetLayerPlaybackScale(uint32 nLayer) const = 0; //! NOTE: This does NOT override the overall animation speed, but it multiplies it

	virtual void SetDesiredMotionParam(EMotionParamID id, f32 value, f32 frametime) = 0; // Updates the given parameter (will perform clamping and clearing as needed)
	virtual bool GetDesiredMotionParam(EMotionParamID id, float& value) const = 0;

	// Set the time for the specified running animation to a value in the range [0..1]
	// When entireClip is true, set the animation normalized time.
	// When entireClip is false, set the current segment normalized time.
	virtual void SetAnimationNormalizedTime(CAnimation* pAnimation, f32 normalizedTime, bool entireClip = true) = 0;

	// Get the animation normalized time for the specified running animation. The return value is in the range [0..1]
	virtual f32 GetAnimationNormalizedTime(const CAnimation* pAnimation) const = 0;

	virtual void SetLayerNormalizedTime(uint32 layer, f32 normalizedTime) = 0;
	virtual f32 GetLayerNormalizedTime(uint32 layer) const = 0;

	// Calculates duration of blend space up to the point where it starts to
	// repeat, that is a Least Common Multiple of a number of segments of
	// different examples. For instance, if segments of caf1: A B C, caf2: 1 2.
	// The whole duration will be calculated for sequence A1 B2 C1 A2 B1 C2.
	virtual f32 CalculateCompleteBlendSpaceDuration(const CAnimation& rAnimation) const = 0;

	virtual Vec3 GetCurrentVelocity() const = 0;

	virtual void SetEventCallback(CallBackFuncType func, void* pdata) = 0;
	virtual AnimEventInstance GetLastAnimEvent() = 0;

	virtual const QuatT& GetRelMovement() const = 0;

	virtual f32  GetUserData(int i) const = 0;

	virtual bool PushPoseModifier(uint32 layer, IAnimationPoseModifierPtr poseModifier, const char* name = NULL) = 0;

#ifdef ENABLE_RUNTIME_POSE_MODIFIERS
	virtual IAnimationSerializablePtr GetPoseModifierSetup() = 0;
	virtual IAnimationSerializableConstPtr GetPoseModifierSetup() const = 0;
#endif

	// This function will move outside of this interface. Use at your own risk.
	virtual QuatT CalculateRelativeMovement(const float deltaTime, const bool CurrNext = 0) const = 0;

#ifdef EDITOR_PCDEBUGCODE
	virtual bool ExportHTRAndICAF(const char* szAnimationName, const char* saveDirectory) const = 0;
	virtual bool ExportVGrid(const char* szAnimationName) const = 0;
#endif
	// </interfuscator:shuffle>
};

struct IAnimationPoseBlenderDir;


struct ISkeletonPhysics
{
	// <interfuscator:shuffle>
	virtual ~ISkeletonPhysics() { }

	virtual void BuildPhysicalEntity(IPhysicalEntity* pent, f32 mass, int surface_idx, f32 stiffness_scale = 1.0f, int nLod = 0, int partid0 = 0, const Matrix34& mtxloc = Matrix34(IDENTITY)) = 0;
	virtual IPhysicalEntity* CreateCharacterPhysics(IPhysicalEntity* pHost, f32 mass, int surface_idx, f32 stiffness_scale, int nLod = 0, const Matrix34& mtxloc = Matrix34(IDENTITY)) = 0;
	virtual int CreateAuxilaryPhysics(IPhysicalEntity* pHost, const Matrix34& mtx, int nLod = 0) = 0;
	virtual IPhysicalEntity* GetCharacterPhysics() const = 0;
	virtual IPhysicalEntity* GetCharacterPhysics(const char* pRootBoneName) const = 0;
	virtual IPhysicalEntity* GetCharacterPhysics(int iAuxPhys) const = 0;
	virtual void SetCharacterPhysics(IPhysicalEntity* pent) = 0;
	virtual void SynchronizeWithPhysicalEntity(IPhysicalEntity* pent, const Vec3& posMaster = Vec3(ZERO), const Quat& qMaster = Quat(1, 0, 0, 0)) = 0;
	virtual IPhysicalEntity* RelinquishCharacterPhysics(const Matrix34& mtx, f32 stiffness = 0.0f, bool bCopyJointVelocities = false, const Vec3& velHost = Vec3(ZERO)) = 0;
	virtual void DestroyCharacterPhysics(int iMode = 0) = 0;
	virtual bool AddImpact(int partid, Vec3 point, Vec3 impact) = 0;
	virtual int TranslatePartIdToDeadBody(int partid) = 0;
	virtual int GetAuxPhysicsBoneId(int iAuxPhys, int iBone = 0) const = 0;

	virtual bool BlendFromRagdoll(QuatTS& location, IPhysicalEntity*& pPhysicalEntity, bool b3dof) = 0;

	virtual int GetFallingDir() const = 0;

	virtual int getBonePhysParentOrSelfIndex(int nBoneIndex, int nLod = 0) const = 0;

	virtual int GetBoneSurfaceTypeId(int nBoneIndex, int nLod = 0) const = 0;

	virtual IPhysicalEntity* GetPhysEntOnJoint(int32 nId) = 0;
	virtual const IPhysicalEntity* GetPhysEntOnJoint(int32 nId) const = 0;

	virtual void SetPhysEntOnJoint(int32 nId, IPhysicalEntity* pPhysEnt) = 0;
	virtual int GetPhysIdOnJoint(int32 nId) const = 0;
	virtual DynArray<SJointProperty> GetJointPhysProperties_ROPE(uint32 jointIndex, int nLod) const = 0;
	virtual bool SetJointPhysProperties_ROPE(uint32 jointIndex, int nLod, const DynArray<SJointProperty>& props) = 0;
	// </interfuscator:shuffle>
};


struct ISkeletonPose : public ISkeletonPhysics
{
	// <interfuscator:shuffle>
	virtual ~ISkeletonPose() { }

	virtual const QuatT& GetAbsJointByID(int32 nJointID) const = 0; //runtime skeleton pose
	virtual const QuatT& GetRelJointByID(int32 nJointID) const = 0; //runtime skeleton pose

	virtual void SetPostProcessCallback(int (*func)(ICharacterInstance*, void*), void* pdata) = 0;
	virtual void SetForceSkeletonUpdate(int32 i) = 0;
	virtual void SetDefaultPose() = 0;
	virtual void SetStatObjOnJoint(int32 nId, IStatObj* pStatObj) = 0;
	virtual IStatObj* GetStatObjOnJoint(int32 nId) = 0;
	virtual const IStatObj* GetStatObjOnJoint(int32 nId) const = 0;
	virtual void SetMaterialOnJoint(int32 nId, IMaterial* pMaterial) = 0;
	virtual IMaterial* GetMaterialOnJoint(int32 nId) = 0;
	virtual const IMaterial* GetMaterialOnJoint(int32 nId) const = 0;
	virtual void DrawSkeleton(const Matrix34& rRenderMat34, uint32 shift = 0) = 0;

	// -------------------------------------------------------------------------
	// Pose Modifiers (soon obsolete)
	// -------------------------------------------------------------------------
	virtual IAnimationPoseBlenderDir* GetIPoseBlenderAim() = 0;
	virtual const IAnimationPoseBlenderDir* GetIPoseBlenderAim() const = 0;
	virtual IAnimationPoseBlenderDir* GetIPoseBlenderLook() = 0;
	virtual const IAnimationPoseBlenderDir* GetIPoseBlenderLook() const = 0;
	virtual void ApplyRecoilAnimation(f32 fDuration, f32 fKinematicImpact, f32 fKickIn, uint32 arms = 3) = 0;
	virtual uint32 SetHumanLimbIK(const Vec3& wgoal, const char* limb) = 0;

	// </interfuscator:shuffle>
};

// Description:
//     This interface holds a set of animations in which each animation is described as properties.
// Summary:
//     Hold description of a set of animations
struct IAnimationSet
{
	// <interfuscator:shuffle>
	virtual ~IAnimationSet() {}

	// Summary:
	//   Retrieves the amount of animations.
	// Return Value:
	//     An integer holding the amount of animations
	virtual uint32 GetAnimationCount() const = 0;

	// Description:
	//   Returns the index of the animation in the set, -1 if there's no such animation
	// Summary:
	//   Searches for the index of an animation using its name.
	// Arguments:
	//   szAnimationName - Null terminated string holding the name of the animation.
	// Return Value:
	//   An integer representing the index of the animation. In case the animation
	//   couldn't be found, -1 will be returned.
	virtual int GetAnimIDByName(const char* szAnimationName) const = 0;

	// Description:
	//   Returns the given animation name
	// Summary:
	//   Gets the name of the specified animation.
	// Arguments:
	//   nAnimationId - Id of an animation.
	// Return Value:
	//   A null terminated string holding the name of the animation. In case the
	//   animation wasn't found, the string "!NEGATIVE ANIMATION ID!" will be
	//   returned.
	virtual const char* GetNameByAnimID(int nAnimationId) const = 0;

	virtual int GetAnimIDByCRC(uint32 animationCRC) const = 0;
	virtual uint32 GetCRCByAnimID(int nAnimationId) const = 0;
	virtual uint32 GetFilePathCRCByAnimID(int nAnimationId) const = 0;
	virtual const char* GetFilePathByName(const char* szAnimationName) const = 0;
	virtual const char* GetFilePathByID(int nAnimationId) const = 0;

	// Returns the duration of the animation.
	//
	// Returns 0.0f when the id or referenced animation is invalid.
	// Never returns a negative value.
	virtual f32 GetDuration_sec(int nAnimationId) const = 0;

	virtual uint32 GetAnimationFlags(int nAnimationId) const  = 0;
	virtual uint32 GetAnimationSize(const uint32 nAnimationId) const = 0;
	virtual bool IsAnimLoaded(int nAnimationId) const = 0;
	virtual bool IsAimPose(int nAnimationId, const IDefaultSkeleton& defaultSkeleton) const = 0;
	virtual bool IsLookPose(int nAnimationId, const IDefaultSkeleton& defaultSkeleton) const = 0;

	virtual void AddRef(const int32 nAnimationId) const = 0;
	virtual void Release(const int32 nAnimationId) const = 0;

	// Retrieve the 'DCC world space' location of the first frame.
	//
	// Returns true on success.
	// Returns false and IDENTITY when animation is invalid.
	virtual bool GetAnimationDCCWorldSpaceLocation(const char* szAnimationName, QuatT& startLocation) const = 0;

	// Retrieve the 'DCC world space' location of the first frame.
	//
	// Returns true on success.
	// Returns false and IDENTITY when animation is invalid.
	virtual bool GetAnimationDCCWorldSpaceLocation(int32 AnimID, QuatT& startLocation) const = 0;

	// Retrieve the 'DCC world space' location of the current frame for a specific controller.
	//
	// Returns true on success.
	// Returns false and IDENTITY when animation is invalid.
	virtual bool GetAnimationDCCWorldSpaceLocation(const CAnimation* pAnim, QuatT& startLocation, uint32 nControllerID) const = 0;


	enum ESampleResult
	{
		eSR_Success,

		eSR_InvalidAnimationId,
		eSR_UnsupportedAssetType,
		eSR_NotInMemory,
		eSR_ControllerNotFound,
	};

	// Sample the location of a controller at a specific time in a non-parametric animation.
	//
	// On success, returns eSR_Success and fills in the relativeLocationOutput parameter. The location uses relative (aka parent-local) coordinates.
	// On failure, returns an error code and sets relativeLocationOutput to IDENTITY.
	virtual ESampleResult SampleAnimation(int32 animationId, float animationNormalizedTime, uint32 controllerId, QuatT& relativeLocationOutput) const = 0;

#ifdef EDITOR_PCDEBUGCODE
	virtual void GetSubAnimations(DynArray<int>& animIdsOut, int animId) const = 0;
	virtual int GetNumFacialAnimations() const = 0;
	virtual const char* GetFacialAnimationPathByName(const char* szName) const = 0; // Returns 0 if name not found.
	virtual const char* GetFacialAnimationName(int index) const = 0; // Returns 0 on invalid index.
	virtual int32 GetGlobalIDByName(const char* szAnimationName) const = 0;
	virtual int32 GetGlobalIDByAnimID(int nAnimationId) const = 0;
	virtual const char* GetAnimationStatus(int nAnimationId) const = 0;
	virtual uint32 GetTotalPosKeys(const uint32 nAnimationId) const = 0;
	virtual uint32 GetTotalRotKeys(const uint32 nAnimationId) const = 0;
	virtual const char* GetDBAFilePath(const uint32 nAnimationId) const = 0;
	virtual int  AddAnimationByPath(const char* animationName, const char* animationPath, const IDefaultSkeleton* pIDefaultSkeleton) = 0;
	virtual void RebuildAimHeader(const char* szAnimationName, const IDefaultSkeleton* pIDefaultSkeleton) = 0;
	virtual void   RegisterListener(IAnimationSetListener* pListener) = 0;
	virtual void   UnregisterListener(IAnimationSetListener* pListener) = 0;
	virtual bool GetMotionParameters(uint32 nAnimationId, int32 parameterIndex, IDefaultSkeleton* pDefaultSkeleton, Vec4& outParameters) const = 0;
	virtual bool GetMotionParameterRange(uint32 nAnimationId, EMotionParamID paramId, float& outMin, float& outMax) const = 0;
	virtual bool IsCombinedBlendSpace(int nAnimationId) const = 0;
#endif

	// </interfuscator:shuffle>
};

struct IAnimationSetListener
{
	IAnimationSet* m_pIAnimationSet;
	IDefaultSkeleton* m_pIDefaultSkeleton;

	IAnimationSetListener() : m_pIAnimationSet(0), m_pIDefaultSkeleton(0) {}
	virtual ~IAnimationSetListener()
	{
#ifdef EDITOR_PCDEBUGCODE
		if (m_pIAnimationSet)
			m_pIAnimationSet->UnregisterListener(this);
#endif
	}

	virtual void OnAnimationSetAddAnimation(const char* animationPath, const char* animationName) {}
	virtual void OnAnimationSetReload() {}
};

#include <IAttachment.h>

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

struct SAnimationStatistics
{
	const char* name;
	long count;
};


struct IAnimEventList
{
	// <interfuscator:shuffle>
	virtual ~IAnimEventList() {}

	virtual uint32 GetCount() const = 0;
	virtual const CAnimEventData& GetByIndex(uint32 animEventIndex) const = 0;
	virtual CAnimEventData& GetByIndex(uint32 animEventIndex) = 0;
	virtual void Append(const CAnimEventData& animEvent) = 0;
	virtual void Remove(uint32 animEventIndex) = 0;
	virtual void Clear() = 0;
	// </interfuscator:shuffle>
};


struct IAnimEvents
{
	// <interfuscator:shuffle>
	virtual ~IAnimEvents() {}

	virtual IAnimEventList* GetAnimEventList(const char* animationFilePath) = 0;
	virtual const IAnimEventList* GetAnimEventList(const char* animationFilePath) const = 0;

	virtual bool SaveAnimEventToXml(const CAnimEventData& dataIn, XmlNodeRef& dataOut) = 0;
	virtual bool LoadAnimEventFromXml(const XmlNodeRef& dataIn, CAnimEventData& dataOut) = 0;

	virtual void InitializeSegmentationDataFromAnimEvents(const char* animationFilePath) = 0;

	virtual size_t GetGlobalAnimCount() = 0;
	// </interfuscator:shuffle>
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
extern "C" {
#endif

	// Description:
	//    Experimental way to track the interface version.
	//    This value will be compared with value passed from system module
	const char gAnimInterfaceVersion[64] = __TIMESTAMP__;

	// Description:
	//    CreateCryAnimation function type definition.
	typedef ICharacterManager* (*PFNCREATECRYANIMATION)(ISystem* pSystem, const char* szInterfaceVersion);

	// Description:
	//     Create an instance of the Animation System. It should usually be called
	//     by ISystem::InitAnimationSystem().
	// See Also:
	//     ICharacterManager, ICharacterManager::Release
	// Arguments:
	//     ISystem            - Pointer to the current ISystem instance
	//     szInterfaceVersion - String version of the build date
	// Summary:
	//     Create an instance of the Animation System
	CRYANIMATION_API ICharacterManager* CreateCharManager(ISystem* pSystem, const char* szInterfaceVersion = gAnimInterfaceVersion);

#ifdef __cplusplus
}
#endif
///////////////////////////////////////////////////////////////////////////////////////////////////////////////


#if defined(ENABLE_LW_PROFILERS)
class CAnimationLightProfileSection
{
public:
	CAnimationLightProfileSection()
		: m_nTicks(CryGetTicks())
	{
	}

	~CAnimationLightProfileSection()
	{
		ICharacterManager* pCharacterManager = gEnv->pCharacterManager;
		IF(pCharacterManager != NULL, 1)
		{
			pCharacterManager->AddFrameTicks(CryGetTicks() - m_nTicks);
		}
	}
private:
	uint64 m_nTicks;
};

class CAnimationLightSyncProfileSection
{
public:
	CAnimationLightSyncProfileSection()
		: m_nTicks(CryGetTicks())
	{}
	~CAnimationLightSyncProfileSection()
	{
		ICharacterManager* pCharacterManager = gEnv->pCharacterManager;
		IF(pCharacterManager != NULL, 1)
		{
			pCharacterManager->AddFrameSyncTicks(CryGetTicks() - m_nTicks);
		}
	}
private:
	uint64 m_nTicks;
};

#define ANIMATION_LIGHT_PROFILER() CAnimationLightProfileSection _animationLightProfileSection;
#define ANIMATION_LIGHT_SYNC_PROFILER() CAnimationLightSyncProfileSection _animationLightSyncProfileSection;
#else
#define ANIMATION_LIGHT_PROFILER()
#define ANIMATION_LIGHT_SYNC_PROFILER()
#endif


// Description:
//    Utility class to automatically start loading & lock a CAF file.
//
//    Either it is 'empty' or it holds a reference to a CAF file.
//    It asserts gEnv->pCharacterManager exists when it isn't empty.
//
// See Also:
//     ICharacterManager
struct CAutoResourceCache_CAF
{
	// Arguments:
	//   filePathCRC - Either 0 (in which case this class refers to 'nothing') or a valid CAF filePathCRC
	explicit CAutoResourceCache_CAF(const uint32 filePathCRC = 0)
		: m_filePathCRC(filePathCRC)
	{
		SafeAddRef();
	}

	CAutoResourceCache_CAF(const CAutoResourceCache_CAF& rhs)
		: m_filePathCRC(rhs.m_filePathCRC)
	{
		SafeAddRef();
	}

	~CAutoResourceCache_CAF()
	{
		SafeRelease();
	}

	CAutoResourceCache_CAF& operator=(const CAutoResourceCache_CAF& anim)
	{
		if (m_filePathCRC != anim.m_filePathCRC)
		{
			SafeRelease();
			m_filePathCRC = anim.m_filePathCRC;
			SafeAddRef();
		}
		return *this;
	}

	bool IsLoaded() const
	{
		if (m_filePathCRC)
		{
			assert(gEnv->pCharacterManager);
			PREFAST_ASSUME(gEnv->pCharacterManager);
			return gEnv->pCharacterManager->CAF_IsLoaded(m_filePathCRC);
		}
		else
		{
			return false;
		}
	}

	uint32 GetFilePathCRC() const
	{
		return m_filePathCRC;
	}

	void Serialize(TSerialize ser)
	{
		ser.BeginGroup("AutoResourceCache_CAF");
		ser.Value("m_filePathCRC", m_filePathCRC);
		ser.EndGroup();
		if (ser.IsReading())
		{
			SafeAddRef();
		}
	}

private:
	void SafeAddRef() const
	{
		if (m_filePathCRC)
		{
			assert(gEnv->pCharacterManager);
			PREFAST_ASSUME(gEnv->pCharacterManager);
			gEnv->pCharacterManager->CAF_AddRef(m_filePathCRC);
		}
	}

	void SafeRelease() const
	{
		if (m_filePathCRC)
		{
			assert(gEnv->pCharacterManager);
			PREFAST_ASSUME(gEnv->pCharacterManager);
			gEnv->pCharacterManager->CAF_Release(m_filePathCRC);
		}
	}

	uint32 m_filePathCRC;
};
