////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002-2012
// -------------------------------------------------------------------------
//  File name:   IMovieSystem.h
//  Created:     26/4/2002 by Timur.
//
////////////////////////////////////////////////////////////////////////////

#pragma once

#include <Range.h>
#include <AnimKey.h>
#include <ICryAnimation.h>
#include <CryName.h>

// forward declaration.
struct IAnimTrack;
struct IAnimNode;
struct IAnimSequence;
struct IMovieSystem;
struct IKey;
class XmlNodeRef;
struct ISplineInterpolator;
struct ILightAnimWrapper;

typedef IMovieSystem* (*PFNCREATEMOVIESYSTEM)(struct ISystem*);

#define MAX_ANIM_NAME_LENGTH 64
//! Very high priority for cut scene sounds.
#define MOVIE_SOUND_PRIORITY 230

#if !defined(CONSOLE)
#define MOVIESYSTEM_SUPPORT_EDITING
#endif

typedef std::vector<IAnimSequence*> AnimSequences;
typedef std::vector<string> TrackEvents;

//////////////////////////////////////////////////////////////////////////
//! Node-Types
// 
// You need to register new types in Movie.cpp/RegisterNodeTypes for serialization.
// Note: Enums are serialized by string now, there is no need for specific IDs
// anymore for new parameters. Values are for backward compatibility.
enum EAnimNodeType
{
	eAnimNodeType_Invalid					= 0x00,
	eAnimNodeType_Entity					= 0x01,
	eAnimNodeType_Director				= 0x02,
	eAnimNodeType_Camera					= 0x03,
	eAnimNodeType_CVar						= 0x04,
	eAnimNodeType_ScriptVar				= 0x05,
	eAnimNodeType_Material				= 0x06,
	eAnimNodeType_Event						= 0x07,
	eAnimNodeType_Group						= 0x08,
	eAnimNodeType_Layer						= 0x09,
	eAnimNodeType_Comment					= 0x10,
	eAnimNodeType_RadialBlur			= 0x11,
	eAnimNodeType_ColorCorrection = 0x12,
	eAnimNodeType_DepthOfField		= 0x13,
	eAnimNodeType_ScreenFader			= 0x14,
	eAnimNodeType_Light 					= 0x15,
	eAnimNodeType_HDRSetup				= 0x16,
	eAnimNodeType_ShadowSetup			= 0x17,
	eAnimNodeType_Alembic					= 0x18, // Used in cinebox, added so nobody uses that number
	eAnimNodeType_GeomCache				= 0x19,
	eAnimNodeType_Environment,
	eAnimNodeType_ScreenDropsSetup,
	eAnimNodeType_Num
};

//! Flags that can be set on animation node.
enum EAnimNodeFlags
{
	eAnimNodeFlags_Expanded				= BIT(0), //!< Deprecated, handled by sandbox now
	eAnimNodeFlags_EntitySelected	= BIT(1), //!< Set if the referenced entity is selected in the editor
	eAnimNodeFlags_CanChangeName	= BIT(2), //!< Set if this node allow changing of its name.
	eAnimNodeFlags_Disabled				= BIT(3), //!< Disable this node.
};

enum ENodeExportType
{
	eNodeExportType_Global = 0,
	eNodeExportType_Local = 1,
};

// Static common parameters IDs of animation node.
//
// You need to register new params in Movie.cpp/RegisterParamTypes for serialization.
// Note: Enums are serialized by string now, there is no need for specific IDs
// anymore for new parameters. Values are for backward compatibility.
//
// If you want to expand CryMovie to control new stuff this is probably  the enum you want to change. 
// For named params see eAnimParamType_ByString & CAnimParamType
enum EAnimParamType
{
	// Parameter is specified by string. See CAnimParamType
	eAnimParamType_ByString							= 8,

	eAnimParamType_FOV									= 0,
	eAnimParamType_Position							= 1,
	eAnimParamType_Rotation							= 2,
	eAnimParamType_Scale								= 3,
	eAnimParamType_Event								= 4,
	eAnimParamType_Visibility						= 5,
	eAnimParamType_Camera								= 6,
	eAnimParamType_Animation						= 7,
	eAnimParamType_Sound								= 10,
	eAnimParamType_Sequence							= 13,
	eAnimParamType_Expression						= 14,
	eAnimParamType_Console							= 17,
	eAnimParamType_Music								= 18,
	eAnimParamType_Float								= 19,
	eAnimParamType_FaceSequence					= 20,
	eAnimParamType_LookAt							  = 21,
	eAnimParamType_TrackEvent					  = 22,

	eAnimParamType_ShakeAmplitudeA			= 23,
	eAnimParamType_ShakeAmplitudeB			= 24,
	eAnimParamType_ShakeFrequencyA			= 25,
	eAnimParamType_ShakeFrequencyB			= 26,
	eAnimParamType_ShakeMultiplier			= 27,
	eAnimParamType_ShakeNoise						= 28,
	eAnimParamType_ShakeWorking					= 29,
	eAnimParamType_ShakeAmpAMult				= 61,
	eAnimParamType_ShakeAmpBMult				= 62,
	eAnimParamType_ShakeFreqAMult				= 63,
	eAnimParamType_ShakeFreqBMult				= 64,
	
	eAnimParamType_DepthOfField					= 30,
	eAnimParamType_FocusDistance				= 31,
	eAnimParamType_FocusRange						= 32,
	eAnimParamType_BlurAmount						= 33,

	eAnimParamType_Capture							= 34,
	eAnimParamType_TransformNoise				= 35,	
	eAnimParamType_TimeWarp							= 36,
	eAnimParamType_FixedTimeStep				= 37,
	eAnimParamType_NearZ								= 38,
	eAnimParamType_Goto									= 39,

	eAnimParamType_PositionX						= 51,
	eAnimParamType_PositionY						= 52,
	eAnimParamType_PositionZ						= 53,

	eAnimParamType_RotationX						= 54,
	eAnimParamType_RotationY						= 55,
	eAnimParamType_RotationZ					  = 56,

	eAnimParamType_ScaleX								= 57,
	eAnimParamType_ScaleY								= 58,
	eAnimParamType_ScaleZ								= 59,

	eAnimParamType_ColorR								= 82,
	eAnimParamType_ColorG								= 83,
	eAnimParamType_ColorB								= 84,

	eAnimParamType_CommentText					= 70,
	eAnimParamType_ScreenFader					= 71,

	eAnimParamType_LightDiffuse					= 81,
	eAnimParamType_LightRadius					= 85,
	eAnimParamType_LightDiffuseMult			= 86,
	eAnimParamType_LightHDRDynamic			= 87,
	eAnimParamType_LightSpecularMult		= 88,
	eAnimParamType_LightSpecPercentage	= 89,

	eAnimParamType_MaterialDiffuse			= 90,
	eAnimParamType_MaterialSpecular		= 91,
	eAnimParamType_MaterialEmissive			= 92,
	eAnimParamType_MaterialOpacity			= 93,
	eAnimParamType_MaterialSmoothness		= 94,

	eAnimParamType_TimeRanges						= 95, //!< Generic track with keys that have time ranges

	eAnimParamType_Physics              = 96,	//! Not used anymore, see Physicalize & PhysicsDriven

	eAnimParamType_ProceduralEyes       = 97,

	eAnimParamType_Mannequin						= 98, //!< Mannequin parameter.

	// Add new param types without explicit ID here:
	// NOTE: don't forget to register in Movie.cpp
	eAnimParamType_GameCameraInfluence,

	eAnimParamType_GSMCache,

	eAnimParamType_ShutterSpeed,

	eAnimParamType_Physicalize,
	eAnimParamType_PhysicsDriven,

	eAnimParamType_SunLongitude,
	eAnimParamType_SunLatitude,
	eAnimParamType_MoonLongitude,
	eAnimParamType_MoonLatitude,

	eAnimParamType_User									= 100000, //!< User node params.	
	
	eAnimParamType_Invalid							= 0xFFFFFFFF
};

// Common parameters of animation node.
class CAnimParamType
{
	friend class CMovieSystem;

public:
	static const uint kParamTypeVersion = 8;

	CAnimParamType() : m_type( eAnimParamType_Invalid ) {}

	CAnimParamType( const string &name ) 
	{
		*this = name;
	}

	CAnimParamType( const EAnimParamType type ) 
	{
		*this = type;
	}

	// Convert from old enum or int
	void operator =( const int type )
	{
		m_type = (EAnimParamType)type;
	}

	void operator =( const string &name ) 
	{
		m_type = eAnimParamType_ByString;
		m_name = name;
	}

	// Convert to enum. This needs to be explicit, 
	// otherwise operator== will be ambiguous
	EAnimParamType GetType() const { return m_type; }

	// Get name
	const char *GetName() const { return m_name.c_str(); }

	bool operator ==( const CAnimParamType &animParamType ) const 
	{
		if( m_type == eAnimParamType_ByString && animParamType.m_type == eAnimParamType_ByString )
			return m_name == animParamType.m_name;

		return m_type == animParamType.m_type;
	}

	bool operator !=( const CAnimParamType &animParamType ) const 
	{
		return !( *this == animParamType );
	}

	bool operator <( const CAnimParamType &animParamType ) const 
	{
		if( m_type == eAnimParamType_ByString && animParamType.m_type == eAnimParamType_ByString )
			return m_name < animParamType.m_name;
		else if( m_type == eAnimParamType_ByString )
			return false; // Always sort named params last
		else if( animParamType.m_type == eAnimParamType_ByString )
			return true; // Always sort named params last

		if( m_type < animParamType.m_type )		
			return true;		

		return false;
	}

	// Serialization. Defined in Movie.cpp
	inline void Serialize( XmlNodeRef &xmlNode, bool bLoading, const uint version = kParamTypeVersion );

private:
	EAnimParamType m_type;
	CCryName m_name;	
};

//! Types of animation track.
// Do not change values! they are serialized
//
// Attention: This should only be expanded if you add a completely new way how tracks store data!
// If you just want to control a new parameter of an entity etc. extend EParamType
//
// Note: TCB splines are only for backward compatibility, Bezier is the default
//
enum EAnimCurveType
{
	eAnimCurveType_TCBFloat			= 1,
	eAnimCurveType_TCBVector		= 2,
	eAnimCurveType_TCBQuat			= 3,
	eAnimCurveType_BezierFloat	= 4,

	eAnimCurveType_Unknown			= 0xFFFFFFFF
};

//! Values that animation track can hold.
// Do not change values! they are serialized
//
// Attention: This should only be expanded if you add a completely new value type that tracks can control!
// If you just want to control a new parameter of an entity etc. extend EParamType
//
// Note: If the param type of a track is known and valid these can be derived from the node.
//       These are serialized in case the parameter got invalid (for example for material nodes)
//
enum EAnimValue
{	
	eAnimValue_Float = 0,
	eAnimValue_Vector = 1,
	eAnimValue_Quat = 2,
	eAnimValue_Bool = 3,
	eAnimValue_Select = 5,
	eAnimValue_Vector4 = 15,
	eAnimValue_DiscreteFloat = 16,
	eAnimValue_RGB = 20,
	eAnimValue_CharacterAnim = 21,
	
	eAnimValue_Unknown = 0xFFFFFFFF
};

enum ETrackMask
{
	eTrackMask_MaskSound = 1 << 11, // Old: 1 << ATRACK_SOUND
	eTrackMask_MaskMusic = 1 << 14, // Old: 1 << ATRACK_MUSIC	
};

//! Structure passed to Animate function.
struct SAnimContext
{
	SAnimContext()
	{
		bSingleFrame = false;
		bForcePlay = false;
		bResetting = false;
		time = 0;
		dt = 0;
		fps = 0;
		pSequence = NULL;
		trackMask = 0;
		startTime = 0;
		m_activeCameraEntity = INVALID_ENTITYID;
	}

	float time;					//!< Current time in seconds.
	float	dt;						//!< Delta of time from previous animation frame in seconds.
	float	fps;					//!< Last calculated frames per second value.
	bool bSingleFrame;	//!< This is not a playing animation, more a single-frame update
	bool bForcePlay;		//!< Set when force playing animation
	bool bResetting;		//!< Set when animation sequence is resetted.
	
	IAnimSequence *pSequence; //!< Sequence in which animation performed.
	EntityId m_activeCameraEntity; //!< Used for editor to pass viewport camera to CryMovie
	
	// TODO: Replace trackMask with something more type safe
	// TODO: Mask should be stored with dynamic length
	uint32 trackMask;		//!< To update certain types of tracks only
	float startTime;		//!< The start time of this playing sequence 

	void Serialize(XmlNodeRef& xmlNode, bool bLoading);
};

/** Parameters for cut-scene cameras
*/
struct SCameraParams
{
	SCameraParams()
	{
		cameraEntityId = INVALID_ENTITYID;
		fFOV = 0.0f;
		fGameCameraInfluence = 0.0f;
		fNearZ = DEFAULT_NEAR;
		justActivated = false;
	}
	EntityId cameraEntityId; // Camera entity id.
	float fFOV;
	float fNearZ;
	float fGameCameraInfluence;
	bool justActivated;
};

//! Interface for movie-system implemented by user for advanced function-support
struct IMovieUser
{
	// <interfuscator:shuffle>
	virtual ~IMovieUser(){}
	//! Called when movie system requests a camera-change.
	virtual void SetActiveCamera( const SCameraParams &Params ) = 0;
	//! Called when movie system enters into cut-scene mode.
	virtual void BeginCutScene( IAnimSequence* pSeq, unsigned long dwFlags,bool bResetFX) = 0;
	//! Called when movie system exits from cut-scene mode.
	virtual void EndCutScene( IAnimSequence* pSeq, unsigned long dwFlags) = 0;
	//! Called when movie system wants to send a global event.
	virtual void SendGlobalEvent( const char *pszEvent ) = 0;
	//! Called to play subtitles for specified sound.
	//virtual void PlaySubtitles( IAnimSequence* pSeq, ISound *pSound ) = 0;
	// </interfuscator:shuffle>
};

//! Callback-class
struct IMovieCallback
{
	//! Callback-reasons
	enum ECallbackReason
	{
		CBR_CHANGENODE, // Node is changing.
		CBR_CHANGETRACK // Track of the node is changing.
	};

	// <interfuscator:shuffle>
	virtual ~IMovieCallback(){}
	// Called by movie system.
	virtual void OnMovieCallback( ECallbackReason reason,IAnimNode *pNode ) = 0;
	virtual void OnSetCamera( const SCameraParams &Params ) = 0;
	virtual bool IsSequenceCamUsed() const = 0;
	// </interfuscator:shuffle>
};

/**	Interface of Animation Track.
*/
struct IAnimTrack : public _i_reference_target_t
{
	//! Flags that can be set on animation track.
	enum EAnimTrackFlags
	{
		eAnimTrackFlags_Linear   = BIT(1), //!< Use only linear interpolation between keys.
		eAnimTrackFlags_Loop     = BIT(2), //!< Play this track in a loop.
		eAnimTrackFlags_Cycle    = BIT(3), //!< Cycle track.
		eAnimTrackFlags_Disabled = BIT(4), //!< Disable this track.

		//////////////////////////////////////////////////////////////////////////
		// Used by editor.
		//////////////////////////////////////////////////////////////////////////
		eAnimTrackFlags_Hidden   = BIT(5), //!< Set when track is hidden in track view.
		eAnimTrackFlags_Muted    = BIT(8), //!< Mute this sound track. This only affects the playback in editor.
	};
	
	// <interfuscator:shuffle>
	//////////////////////////////////////////////////////////////////////////
	virtual EAnimCurveType GetCurveType() = 0;
	virtual EAnimValue     GetValueType() = 0;
	
#ifdef MOVIESYSTEM_SUPPORT_EDITING
	// This color is used for the editor.
	virtual ColorB GetCustomColor() const = 0;
	virtual void SetCustomColor(ColorB color) = 0;
	virtual bool HasCustomColor() const = 0;
	virtual void ClearCustomColor() = 0;
#endif

	// Return what parameter of the node, this track is attached to.
	virtual CAnimParamType  GetParameterType() const = 0;
	// Assign node parameter ID for this track.
	virtual void SetParameterType( CAnimParamType type ) = 0;

	//////////////////////////////////////////////////////////////////////////
	// Animation track can contain sub-tracks (Position XYZ anim track have sub-tracks for x,y,z)
	// Get count of sub tracks.
	virtual int GetSubTrackCount() const = 0;
	// Retrieve pointer the specfied sub track.
	virtual IAnimTrack* GetSubTrack( int nIndex ) const = 0;
	virtual const char* GetSubTrackName(int nIndex) const = 0;
	virtual void SetSubTrackName(int nIndex, const char *name) = 0;
	//////////////////////////////////////////////////////////////////////////

	virtual void GetKeyValueRange(float& fMin, float& fMax ) const = 0;
	virtual void SetKeyValueRange(float fMin, float fMax ) = 0;

	//! Return number of keys in track.
	virtual int GetNumKeys() const = 0;

	//! Return true if keys exists in this track
	virtual bool HasKeys() const = 0;

	//! Set number of keys in track.
	//! If needed adds empty keys at end or remove keys from end.
	virtual void SetNumKeys( int numKeys ) = 0;

	//! Remove specified key.
	virtual void RemoveKey( int num ) = 0;

	//! Get key at specified location.
	//! @param key Must be valid pointer to compatable key structure, to be filled with specified key location.
	virtual void GetKey( int index,IKey *key ) const = 0;

	//! Get time of specified key.
	//! @return key time.
	virtual float GetKeyTime( int index ) const = 0;

	//! Find key at given time.
	//! @return Index of found key, or -1 if key with this time not found.
	virtual int FindKey( float time ) = 0;

	//! Get flags of specified key.
	//! @return key time.
	virtual int GetKeyFlags( int index ) = 0;

	//! Set key at specified location.
	//! @param key Must be valid pointer to compatable key structure.
	virtual void SetKey( int index,IKey *key ) = 0;

	//! Set time of specified key.
	virtual void SetKeyTime( int index,float time ) = 0;

	//! Set flags of specified key.
	virtual void SetKeyFlags( int index,int flags ) = 0;

	//! Sort keys in track (after time of keys was modified).
	virtual void SortKeys() = 0;

	//! Get track flags.
	virtual int GetFlags() = 0;

	//! Check if track is masked by mask
	// TODO: Mask should be stored with dynamic length
	virtual bool IsMasked( const uint32 mask ) const = 0;
	
	//! Set track flags.
	virtual void SetFlags( int flags ) = 0;

	//! Create key at given time, and return its index.
	//! @return Index of new key.
	virtual int CreateKey( float time ) = 0;

	//! Clone key at specified index.
	//! @retun Index of new key.
	virtual int CloneKey( int key ) = 0;

	//! Clone key at specified index from another track of SAME TYPE.
	//! @retun Index of new key.
	virtual int CopyKey( IAnimTrack *pFromTrack, int nFromKey ) = 0;

	//! Get info about specified key.
	//! @param Short human readable text description of this key.
	//! @param duration of this key in seconds.
	virtual void GetKeyInfo( int key,const char* &description,float &duration ) = 0;

	//////////////////////////////////////////////////////////////////////////
	// Get track value at specified time.
	// Interpolates keys if needed.
	//////////////////////////////////////////////////////////////////////////
	virtual void GetValue( float time,float &value ) = 0;
	virtual void GetValue( float time,Vec3 &value ) = 0;
	virtual void GetValue( float time,Vec4 &value ) = 0;
	virtual void GetValue( float time,Quat &value ) = 0;
	virtual void GetValue( float time,bool &value ) = 0;

	//////////////////////////////////////////////////////////////////////////
	// Set track value at specified time.
	// Adds new keys if required.
	//////////////////////////////////////////////////////////////////////////
	virtual void SetValue( float time,const float &value,bool bDefault=false ) = 0;
	virtual void SetValue( float time,const Vec3 &value,bool bDefault=false ) = 0;
	virtual void SetValue( float time,const Vec4 &value,bool bDefault=false ) = 0;
	virtual void SetValue( float time,const Quat &value,bool bDefault=false ) = 0;
	virtual void SetValue( float time,const bool &value,bool bDefault=false ) = 0;

	// Only for position tracks, offset all track keys by this amount.
	virtual void OffsetKeyPosition( const Vec3 &value ) = 0;

	// Assign active time range for this track.
	virtual void SetTimeRange( const Range &timeRange ) = 0;

	// Serialize this animation track to XML.
	virtual bool Serialize( XmlNodeRef &xmlNode,bool bLoading, bool bLoadEmptyTracks=true ) = 0;

	virtual bool SerializeSelection( XmlNodeRef &xmlNode,bool bLoading, bool bCopySelected=false,float fTimeOffset=0) = 0;

	//! For custom track animate parameters.
	virtual void Animate( SAnimContext &ec ) {};

	// Get access to the internal spline of the track.
	virtual ISplineInterpolator* GetSpline() const { return 0; };

	virtual bool IsKeySelected(int key) const { return false; }

	virtual void SelectKey(int key, bool select) {}
	
	//! Return the index of the key which lies right after the given key in time.
	//! @param key Index of of key.
	//! @return Index of the next key in time. If the last key given, this returns -1.
	// In case of keys sorted, it's just 'key+1', but if not sorted, it can be another value.
	virtual int NextKeyByTime(int key) const { if (key+1 < GetNumKeys()) { return key+1; } else { return -1; } }

	//! Get the animation layer index assigned. (only for character/look-at tracks ATM)
	virtual int GetAnimationLayerIndex() const { return -1; }
	//! Set the animation layer index. (only for character/look-at tracks ATM)
	virtual void SetAnimationLayerIndex(int index) { }

	// collect memory informations
	virtual void GetMemoryUsage(ICrySizer *pSizer ) const =0;

	// </interfuscator:shuffle>
protected:
	virtual ~IAnimTrack() {};
};


/** Callback called by animation node when its animated.
*/
struct IAnimNodeOwner
{
	// <interfuscator:shuffle>
	virtual ~IAnimNodeOwner(){}
	virtual void OnNodeAnimated( IAnimNode *pNode ) = 0;
	virtual void OnNodeVisibilityChanged( IAnimNode *pNode, const bool bHidden ) = 0;
	virtual void OnNodeReset( IAnimNode *pNode ) {}
	// </interfuscator:shuffle>
};

/**	Base class for all Animation nodes,
		can host multiple animation tracks, and execute them other time.
		Animation node is reference counted.
*/
struct IAnimNode : virtual public _i_reference_target_t
{
public:
	//////////////////////////////////////////////////////////////////////////
	// Supported params.
	//////////////////////////////////////////////////////////////////////////
	enum ESupportedParamFlags
	{
		eSupportedParamFlags_MultipleTracks = 0x01, // Set if parameter can be assigned multiple tracks.
	};

public:

	// <interfuscator:shuffle>
	//! Set node name.
	virtual void SetName( const char *name ) = 0;
	
	//! Get node name.
	virtual const char* GetName() = 0;

	// Get Type of this node.
	virtual EAnimNodeType GetType() const = 0;

	// Return Animation Sequence that owns this node.
	virtual IAnimSequence* GetSequence() = 0;

	// Called when sequence is activated / deactivated
	virtual void Activate( bool bActivate ) = 0;

	// Set AnimNode flags.
	// @param flags One or more flags from EAnimNodeFlags.
	// @see EAnimNodeFlags
	virtual void SetFlags( int flags ) = 0;

	// Get AnimNode flags.
	// @return flags set on node.
	// @see EAnimNodeFlags
	virtual int GetFlags() const = 0;

	// Assign an entity guid to the node, (Only for Entity targeting nodes)
	virtual void SetEntityGuid( const EntityGUID &guid ) = 0;
	// Retrieve a pointer to the entity Guid, return NULL if it is not an entity targeting node.
	virtual EntityGUID* GetEntityGuid() = 0;

	// Override the entity ID of a node (only called from game)
	virtual void SetEntityId(const int entityId) {}

	// Query entity of this node, if assigned.
	virtual IEntity* GetEntity() = 0;

	// Assign an entities guides to the lookAt nodes
	virtual void SetEntityGuidTarget( const EntityGUID &guid ) = 0;
	virtual void SetEntityGuidSource( const EntityGUID &guid ) = 0;

	// Return movie system that created this node.
	virtual IMovieSystem*	GetMovieSystem() = 0;

	//////////////////////////////////////////////////////////////////////////
	// Space position/orientation scale.
	//////////////////////////////////////////////////////////////////////////
	//! Translate entity node.
	virtual void SetPos( float time,const Vec3 &pos ) = 0;
	//! Rotate entity node.
	virtual void SetRotate( float time,const Quat &quat ) = 0;
	//! Scale entity node.
	virtual void SetScale( float time,const Vec3 &scale ) = 0;

	//! Get current entity position.
	virtual Vec3 GetPos() = 0;
	//! Get current entity rotation.
	virtual Quat GetRotate() = 0;
	//! Get current entity scale.
	virtual Vec3 GetScale() = 0;

	// General Set param.
	// Set float/vec3/vec4 parameter at given time.
	// @return true if parameter set, false if this parameter not exist in node.
	virtual bool SetParamValue( float time,CAnimParamType param,float value ) = 0;
	virtual bool SetParamValue( float time,CAnimParamType param,const Vec3 &value ) = 0;
	virtual bool SetParamValue( float time,CAnimParamType param,const Vec4 &value ) = 0;
	// Get float/vec3/vec4 parameter at given time.
	// @return true if parameter exist, false if this parameter not exist in node.
	virtual bool GetParamValue( float time,CAnimParamType param,float &value ) = 0;
	virtual bool GetParamValue( float time,CAnimParamType param,Vec3 &value ) = 0;
	virtual bool GetParamValue( float time,CAnimParamType param,Vec4 &value ) = 0;
	
	//! Evaluate animation node while not playing animation.
	virtual void StillUpdate() = 0;

	//! Evaluate animation to the given time.
	virtual void Animate( SAnimContext &ec ) = 0;

	// Description:
	//		Returns number of supported parameters by this animation node (position,rotation,scale,etc..).
	// Returns:
	//		Number of supported parameters.
	virtual unsigned int GetParamCount() const = 0;
	
	// Description:
	//		Returns the type of a param by index
	// Arguments:
	//		nIndex - parameter index in range 0 <= nIndex < GetSupportedParamCount()
	virtual CAnimParamType GetParamType(unsigned int nIndex) const = 0;

	// Description:
	//		Check if parameter is supported by this node.
	virtual bool IsParamValid(const CAnimParamType &paramType) const = 0;

	// Description:
	//		Returns name of supported parameter of this animation node or NULL if not available
	// Arguments:
	//			paramType - parameter id
	virtual const char *GetParamName(const CAnimParamType &paramType) const = 0;

	// Description:
	//		Returns the params value type
	virtual EAnimValue GetParamValueType(const CAnimParamType &paramType) const = 0;

	// Description:
	//		Returns the params value type
	virtual ESupportedParamFlags GetParamFlags(const CAnimParamType &paramType) const = 0;

	//////////////////////////////////////////////////////////////////////////
	// Working with Tracks.
	//////////////////////////////////////////////////////////////////////////
	virtual int GetTrackCount() const = 0;

	// Return track assigned to the specified parameter.
	virtual IAnimTrack* GetTrackByIndex( int nIndex ) const = 0;

	// Return first track assigned to the specified parameter.
	virtual IAnimTrack* GetTrackForParameter( const CAnimParamType &paramType ) const = 0;

	// Return the i-th track assigned to the specified parameter in case of multiple tracks.
	virtual IAnimTrack* GetTrackForParameter( const CAnimParamType &paramType, uint32 index ) const = 0;

	// Get the index of a given track among tracks with the same parameter type in this node.
	virtual uint32 GetTrackParamIndex( const IAnimTrack *pTrack ) const = 0;

	// Creates a new track for given parameter.
	virtual IAnimTrack* CreateTrack( const CAnimParamType &paramType ) = 0;

	// Initializes track default values after de-serialization / user creation. Only called in editor.
	virtual void InitializeTrackDefaultValue( IAnimTrack *pTrack, const CAnimParamType &paramType ) = 0;

	// Assign animation track to parameter.
	// if track parameter is NULL track with parameter id param will be removed.
	virtual void SetTrack( const CAnimParamType &paramType,IAnimTrack *track ) = 0;

	// Set time range for all tracks in this sequence.
	virtual void SetTimeRange( Range timeRange ) = 0;

	// Remove track from anim node.
	virtual void AddTrack( IAnimTrack *pTrack ) = 0;

	// Remove track from anim node.
	virtual bool RemoveTrack( IAnimTrack *pTrack ) = 0;

	// Description:
	//		Creates default set of tracks supported by this node.
	virtual void CreateDefaultTracks() = 0;
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// Callback for animation node used by editor.
	// Register notification callback with animation node.
	virtual void SetNodeOwner( IAnimNodeOwner *pOwner ) = 0;
	virtual IAnimNodeOwner* GetNodeOwner() = 0;

	// Serialize this animation node to XML.
	virtual void Serialize( XmlNodeRef &xmlNode,bool bLoading,bool bLoadEmptyTracks ) = 0;
	
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	// Groups interface
	//////////////////////////////////////////////////////////////////////////
	virtual void SetParent( IAnimNode* pParent ) = 0;
	virtual IAnimNode* GetParent() const = 0;
	virtual IAnimNode * HasDirectorAsParent() const = 0;
	//////////////////////////////////////////////////////////////////////////

	// collect allocated memory
	virtual void GetMemoryUsage( ICrySizer *pSizer ) const=0;

	virtual void Render() = 0;

	virtual bool NeedToRender() const = 0;

	// Called from editor if dynamic params need updating
	virtual void UpdateDynamicParams() = 0;
	// </interfuscator:shuffle>

	// Used by AnimCameraNode
	virtual bool GetShakeRotation(const float &time, Quat &rot){return false;}

	virtual void SetCameraShakeSeed(const uint shakeSeed){};

protected:
	virtual ~IAnimNode() {};
};

//! Track event listener
struct ITrackEventListener
{
	//! Reasons
	enum ETrackEventReason
	{
		eTrackEventReason_Added,
		eTrackEventReason_Removed,
		eTrackEventReason_Renamed,
		eTrackEventReason_Triggered,
		eTrackEventReason_MovedUp,
		eTrackEventReason_MovedDown,
	};

	// <interfuscator:shuffle>
	virtual ~ITrackEventListener(){}
	// Description:
	//		Called when track event is updated
	// Arguments:
	//		pSeq - Animation sequence
	//		reason - Reason for update (see EReason)
	//		event - Track event added
	//		pUserData - Data to accompany reason
	virtual void OnTrackEvent(IAnimSequence *pSequence, int reason, const char* event, void* pUserData) = 0;
	virtual void GetMemoryUsage(ICrySizer *pSizer) const{};
	// </interfuscator:shuffle>
};

struct IAnimSequenceOwner
{
	// <interfuscator:shuffle>
	virtual ~IAnimSequenceOwner() {}
	virtual void OnModified() = 0;
	// </interfuscator:shuffle>
};

struct IAnimStringTable : public _i_reference_target_t
{
	virtual const char* Add(const char* p) = 0;
};

/**	Animation sequence, operates on animation nodes contained in it.
 */
struct IAnimSequence : public _i_reference_target_t
{
	static const int kSequenceVersion = 3;

	//! Flags used for SetFlags(),GetFlags(),SetParentFlags(),GetParentFlags() methods.
	enum EAnimSequenceFlags
	{
		eSeqFlags_PlayOnReset					= BIT(0),	//!< Start playing this sequence immediately after reset of movie system(Level load).
		eSeqFlags_OutOfRangeConstant	= BIT(1),	//!< Constant Out-Of-Range,time continues normally past sequence time range.
		eSeqFlags_OutOfRangeLoop			= BIT(2),	//!< Loop Out-Of-Range,time wraps back to the start of range when reaching end of range.
		eSeqFlags_CutScene						= BIT(3),	//!< Cut scene sequence.
		eSeqFlags_NoHUD								= BIT(4),	//!< Don`t display HUD
		eSeqFlags_NoPlayer						= BIT(5),	//!< Disable input and drawing of player
		eSeqFlags_16To9								= BIT(8),	//!< 16:9 bars in sequence
		eSeqFlags_NoGameSounds				= BIT(9),	//!< Suppress all game sounds.
		eSeqFlags_NoSeek							= BIT(10), //!< Cannot seek in sequence. 
		eSeqFlags_NoAbort							= BIT(11), //!< Cutscene can not be aborted
		eSeqFlags_NoSpeed							= BIT(13), //!< Cannot modify sequence speed - TODO: add interface control if required
		eSeqFlags_CanWarpInFixedTime	= BIT(14), //!< Timewarping will work with a fixed time step.
		eSeqFlags_EarlyMovieUpdate		= BIT(15), //!< Turn the 'sys_earlyMovieUpdate' on during the sequence.
		eSeqFlags_LightAnimationSet   = BIT(16), //!< A special unique sequence for light animations 		
		eSeqFlags_NoMPSyncingNeeded   = BIT(17), //!< this sequence doesn't require MP net syncing
	};

	//! Set the name of this sequence. (ex. "Intro" in the same case as above)
	virtual void SetName( const char *name) = 0;
	//! Get the name of this sequence. (ex. "Intro" in the same case as above)
	virtual const char* GetName() const = 0;
	//! Get the ID (unique in a level and consistent across renaming) of this sequence.
	virtual uint32 GetId () const = 0;

	virtual void SetOwner( IAnimSequenceOwner *pOwner ) = 0;
	virtual IAnimSequenceOwner* GetOwner() const = 0;

	//! Set the currently active director node.
	virtual void SetActiveDirector(IAnimNode *pDirectorNode) = 0;
	//! Get the currently active director node, if any.
	virtual IAnimNode *GetActiveDirector() const = 0;

	//! Get the fixed time step for timewarping.
	virtual float GetFixedTimeStep() const = 0;
	//! Set the fixed time step for timewarping.
	virtual void SetFixedTimeStep(float dt) = 0;

	//! Set animation sequence flags
	virtual void SetFlags( int flags ) = 0;
	//! Get animation sequence flags
	virtual int GetFlags() const = 0;
	//! Get cutscene related animation sequence flags
	virtual int GetCutSceneFlags(const bool localFlags = false) const = 0;
	//! Set parent animation sequence
	virtual void SetParentSequence(IAnimSequence *pParentSequence) = 0;
	//! Get parent animation sequence
	virtual const IAnimSequence* GetParentSequence() const = 0;
	//! Check whether this sequence has the given sequence as a descendant through one of its sequence tracks.
	virtual bool IsAncestorOf(const IAnimSequence *pSequence) const = 0;

	//! Return number of animation nodes in sequence.
	virtual int GetNodeCount() const = 0;
	//! Get animation node at specified index.
	virtual IAnimNode* GetNode( int index ) const = 0;
	
	//! Add animation node to sequence.
	//! @return True if node added, same node will not be added 2 times.
	virtual bool AddNode( IAnimNode *node ) = 0;

	// Reorders the array of nodes, so the specified node is placed after or before the given pivot node depending on the parameter 'next'.
	virtual void ReorderNode( IAnimNode *node,IAnimNode *pPivotNode, bool next ) = 0;
	
	// Description:
	//		Creates a new animation node with specified type.
	// Arguments:
	//		nodeType - Type of node, one of EAnimNodeType enums.
	virtual IAnimNode* CreateNode( EAnimNodeType nodeType ) = 0;

	// Description:
	//		Creates a new animation node from serialized node XML.
	// Arguments:
	//		node - Serialized form of node
	virtual IAnimNode* CreateNode( XmlNodeRef node ) = 0;

	//! Remove animation node from sequence.
	virtual void RemoveNode( IAnimNode *node ) = 0;
	
	// Finds node by name, can be slow.
	// If the node belongs to a director, the director node also should be given 
	// since there can be multiple instances of the same node(i.e. the same name)
  // across multiple director nodes. 
	virtual IAnimNode* FindNodeByName( const char *sNodeName, const IAnimNode *pParentDirector ) = 0;

	//! Remove all nodes from sequence.
	virtual void RemoveAll() = 0;

	// Activate sequence by binding sequence animations to nodes.
	// must be called prior animating sequence.
	virtual void Activate() = 0;	

	/** Check if sequence is activated
	*/
	virtual bool IsActivated() const = 0;
	
	// Deactivates sequence by unbinding sequence animations from nodes.
	virtual void Deactivate() = 0;

	// Pre-caches data associated with this anim sequence.
	virtual void PrecacheData(float startTime = 0.0f) = 0;

	// Update sequence while not playing animation.
	virtual void StillUpdate() = 0;

	// Render function call for some special node.
	virtual void Render() = 0;

	// Evaluate animations of all nodes in sequence.
	// Sequence must be activated before animating.
	virtual void Animate(const SAnimContext &ec) = 0;

	//! Set time range of this sequence.
	virtual void SetTimeRange( Range timeRange ) = 0;

	//! Get time range of this sequence.
	virtual Range GetTimeRange() = 0;

	//! Resets the sequence
	virtual void Reset( bool bSeekToStart ) = 0;

	//! This can have more time-consuming tasks performed additional to tasks of the usual 'Reset()' method.
	virtual void ResetHard() = 0;

	// Called to pause sequence.
	virtual void Pause() = 0;
	
	// Called to resume sequence.
	virtual void Resume() = 0;

	/** Called to check if sequence is paused.
	*/
	virtual bool IsPaused() const = 0;

	/** Move/Scale all keys in tracks from previous time range to new time range.
	*/
	virtual void AdjustKeysToTimeRange( const Range &timeRange ) = 0;

	// Serialize this sequence to XML.
	virtual void Serialize( XmlNodeRef &xmlNode,bool bLoading, bool bLoadEmptyTracks=true, uint32 overrideId=0, bool bUndo=false ) = 0;

	// Copy some nodes of this sequence to XML.
	virtual void CopyNodes(XmlNodeRef& xmlNode, IAnimNode** pSelectedNodes, uint32 count) = 0;

	// Paste nodes given by the XML to this sequence.
	virtual void PasteNodes(const XmlNodeRef& xmlNode, IAnimNode *pParent) = 0;

	// Summary:
	//  Adds/removes track events in sequence.
	virtual bool AddTrackEvent(const char* szEvent) = 0;
	virtual bool RemoveTrackEvent(const char* szEvent) = 0;
	virtual bool RenameTrackEvent(const char* szEvent, const char* szNewEvent) = 0;
	virtual bool MoveUpTrackEvent(const char* szEvent) = 0;
	virtual bool MoveDownTrackEvent(const char* szEvent) = 0;
	virtual void ClearTrackEvents() = 0;

	// Summary:
	//  Gets the track events in the sequence.
	virtual int GetTrackEventsCount() const = 0;
	// Summary:
	//  Gets the specified track event in the sequence.
	virtual char const* GetTrackEvent(int iIndex) const = 0;

	virtual IAnimStringTable* GetTrackEventStringTable() = 0;

	// Summary:
	//	 Called to trigger a track event.
	virtual void TriggerTrackEvent(const char* event, const char* param = NULL) = 0;

	//! Track event listener
	virtual void AddTrackEventListener(ITrackEventListener *pListener) = 0;
	virtual void RemoveTrackEventListener(ITrackEventListener *pListener) = 0;

	// collect memory informations
	virtual void GetMemoryUsage(ICrySizer *pSizer ) const =0;
	// </interfuscator:shuffle>
};

/**	Movie Listener interface.
    Register at movie system to get notified about movie events
*/
struct IMovieListener
{
	enum EMovieEvent
	{
		eMovieEvent_Started = 0,   // fired when sequence is started
		eMovieEvent_Stopped,       // fired when sequence ended normally		
		eMovieEvent_Aborted,       // fired when sequence was aborted before normal end (STOP and ABORTED event are mutually exclusive!)
		eMovieEvent_Updated,			 // fired after sequence time or playback speed was updated
	};

	// <interfuscator:shuffle>
	virtual ~IMovieListener(){}
	//! callback on movie events
	virtual void OnMovieEvent(EMovieEvent movieEvent, IAnimSequence* pAnimSequence) = 0;
	// </interfuscator:shuffle>

	void GetMemoryUsage(ICrySizer *pSizer ) const{}
};

/**	Movie System interface.
		Main entrance point to engine movie capability.
		Enumerate available movies, update all movies, create animation nodes and tracks.
 */
struct IMovieSystem
{
	enum ESequenceStopBehavior
	{
		eSSB_LeaveTime = 0,			// When sequence is stopped it remains in last played time.
		eSSB_GotoEndTime = 1,		// Default behavior in game, sequence is animated at end time before stop.
		eSSB_GotoStartTime = 2,	// Default behavior in editor, sequence is animated at start time before stop.
	};

	// <interfuscator:shuffle>
	virtual ~IMovieSystem(){}

	virtual void GetMemoryUsage( ICrySizer *pSizer ) const=0;

	//! Release movie system.
	virtual void Release() = 0;
	//! Set the user.
	virtual void SetUser(IMovieUser *pUser) = 0;
	//! Get the user.
	virtual IMovieUser* GetUser() = 0;
	//! Loads all nodes and sequences from a specific file (should be called when the level is loaded).
	virtual bool Load(const char *pszFile, const char *pszMission) = 0;

	// Description:
	//		Creates a new animation track with specified type.
	// Arguments:
	//		type - Type of track, one of EAnimTrackType enums.
	virtual IAnimTrack* CreateTrack( EAnimCurveType type ) = 0;

	virtual IAnimSequence* CreateSequence( const char *sequence, bool bLoad=false, uint32 id=0 ) = 0;
	virtual IAnimSequence* LoadSequence( XmlNodeRef &xmlNode, bool bLoadEmpty=true ) = 0;
	virtual void AddSequence(IAnimSequence *pSequence) = 0;
	virtual void RemoveSequence(IAnimSequence *pSequence) = 0;
	virtual IAnimSequence* FindSequence( const char *sequence ) const = 0;
	virtual IAnimSequence* FindSequenceById( uint32 id ) const = 0;
	virtual IAnimSequence* GetSequence(int i) const = 0;
	virtual int GetNumSequences() const = 0;
	virtual IAnimSequence* GetPlayingSequence(int i) const = 0;
	virtual int GetNumPlayingSequences() const = 0;
	virtual bool IsCutScenePlaying() const = 0;

	virtual uint32 GrabNextSequenceId() = 0;

	//////////////////////////////////////////////////////////////////////////
	//
	// If the name of a sequence changes, the keys that refer it in the 
	// sequence track of the director node should be properly updated also.
	//
	// @param   before The old name of the sequence.
	// @param   after The new name of the sequence.
	// @return  Number of modified sequence keys.
	//
	//////////////////////////////////////////////////////////////////////////
	virtual int OnSequenceRenamed( const char *before, const char *after ) = 0;
	//////////////////////////////////////////////////////////////////////////
	//
	// If the name of a camera changes, the keys that refer it in the 
	// camera track of the director node should be properly updated also.
	// This updates the name of the corresponding camera node also, if any.
	//
	// @param   before The old name of the camera.
	// @param   after The new name of the camera.
	// @return  Number of modified camera keys.
	//
	//////////////////////////////////////////////////////////////////////////
	virtual int OnCameraRenamed( const char *before, const char *after ) = 0;
	
	// Adds a listener to a sequence
	// @param pSequence Pointer to sequence
	// @param pListener Pointer to an IMovieListener
	// @return true on successful add, false otherwise
	virtual bool AddMovieListener(IAnimSequence* pSequence, IMovieListener* pListener) = 0;

	// Removes a listener from a sequence
	// @param pSequence Pointer to sequence
	// @param pListener Pointer to an IMovieListener
	// @return true on successful removal, false otherwise
	virtual bool RemoveMovieListener(IAnimSequence* pSequence, IMovieListener* pListener) = 0;

	virtual ISystem* GetSystem() = 0;

	// Remove all sequences from movie system.
	virtual void RemoveAllSequences() = 0;

	//////////////////////////////////////////////////////////////////////////
	// Sequence playback.
	//////////////////////////////////////////////////////////////////////////
	// Start playing sequence.
	// Call ignored if sequence is already playing.
	// @param sequence Name of sequence to play.
	virtual void PlaySequence(const char *pSequenceName, IAnimSequence *pParentSeq, bool bResetFX, bool bTrackedSequence, float startTime = -FLT_MAX, float endTime = -FLT_MAX) = 0;

	// Start playing sequence.
	// Call ignored if sequence is already playing.
	// @param sequence Pointer to Valid sequence to play.
	virtual void PlaySequence(IAnimSequence *pSequence, IAnimSequence *pParentSeq, bool bResetFX, bool bTrackedSequence, float startTime = -FLT_MAX, float endTime = -FLT_MAX) = 0;

	// Stops currently playing sequence.
	// Ignored if sequence is not playing.
	// Returns true if sequence has been stopped. false otherwise
	// @param sequence Name of playing sequence to stop.
	virtual bool StopSequence(const char *pSequenceName) = 0;

	// Stop's currently playing sequence.
	// Ignored if sequence is not playing.
	// Returns true if sequence has been stopped. false otherwise
	// @param sequence Pointer to Valid sequence to stop.
	virtual bool StopSequence(IAnimSequence *pSequence) = 0;

	/** Aborts a currently playing sequence.
			Ignored if sequence is not playing.
			Calls IMovieListener with MOVIE_EVENT_ABORTED event (MOVIE_EVENT_DONE is NOT called)
			Returns true if sequence has been aborted. false otherwise
			@param sequence Pointer to Valid sequence to stop.
			@param bLeaveTime If false, uses default stop behavior, otherwise leaves the sequence at time
	*/
	virtual bool AbortSequence(IAnimSequence *pSequence, bool bLeaveTime = false) = 0;

	// Stops all currently playing sequences.
	virtual void StopAllSequences() = 0;

	// Stops all playing cut-scene sequences.
	// This will not stop all sequences, but only those with CUT_SCENE flag set.
	virtual void StopAllCutScenes() = 0;

	// Checks if specified sequence is playing.
	virtual bool IsPlaying( IAnimSequence *seq ) const = 0;

	/** Resets playback state of movie system,
			usually called after loading of level,
	*/
	virtual void Reset( bool bPlayOnReset, bool bSeekToStart ) = 0;

	// Sequences with PLAY_ONRESET flag will start playing after this call.
	virtual void PlayOnLoadSequences() = 0;

	// Update movie system while not playing animation.
	virtual void StillUpdate() = 0;

	// Updates movie system every frame before the entity system to animate all playing sequences.
	virtual void PreUpdate( const float dt ) = 0;

	// Updates movie system every frame after the entity system to animate all playing sequences.
	virtual void PostUpdate( const float dt ) = 0;

	// Render function call of some special node.
	virtual void Render() = 0;

	// Signal the capturing start.
	virtual void StartCapture(const ICaptureKey& key) = 0;

	// Signal the capturing end.
	virtual void EndCapture() = 0;

	// Actually turn on/off the capturing.
	// This is needed for the timing issue of turning on/off the capturing.
	virtual void ControlCapture() = 0;

	// Set movie system into recording mode,
	// While in recording mode any changes made to node will be added as keys to tracks.
	virtual void SetRecording( bool recording ) = 0;
	virtual bool IsRecording() const = 0;

	virtual void EnableCameraShake(bool bEnabled)=0;
	virtual bool IsCameraShakeEnabled() const =0;

	// Pause any playing sequences.
	virtual void Pause() = 0;

	// Resume playing sequences.
	virtual void Resume() = 0;

	// Pause cut scenes in editor.
	virtual void PauseCutScenes() = 0;

	// Resume cut scenes in editor.
	virtual void ResumeCutScenes() = 0;

	// Callback when animation-data changes
	virtual void SetCallback( IMovieCallback *pCallback ) = 0;

	virtual IMovieCallback* GetCallback() = 0;

	// Serialize to XML.
	virtual void Serialize( XmlNodeRef &xmlNode,bool bLoading, bool bRemoveOldNodes=false, bool bLoadEmpty=true ) = 0;

	virtual const SCameraParams& GetCameraParams() const = 0;
	virtual void SetCameraParams( const SCameraParams &Params ) = 0;
	virtual void SendGlobalEvent( const char *pszEvent ) = 0;

	// Gets the float time value for a sequence that is already playing
	virtual float GetPlayingTime(IAnimSequence * pSeq) = 0;
	virtual float GetPlayingSpeed(IAnimSequence * pSeq) = 0;
	// Sets the time progression of an already playing cutscene.
	// If IAnimSequence:NO_SEEK flag is set on pSeq, this call is ignored.
	virtual bool SetPlayingTime(IAnimSequence * pSeq, float fTime)=0;
	virtual bool SetPlayingSpeed(IAnimSequence * pSeq, float fSpeed)=0;
	// Set behavior pattern for stopping sequences.
	virtual void SetSequenceStopBehavior( ESequenceStopBehavior behavior )=0;

	// Set the start and end time of an already playing cutscene.
	virtual bool GetStartEndTime(IAnimSequence *pSeq,float &fStartTime,float &fEndTime) = 0;
	virtual bool SetStartEndTime(IAnimSequence *pSeq,const float fStartTime,const float fEndTime) = 0;

	// Make the specified sequence go to a given frame time.
	// @param seqName A sequence name.
	// @param targetFrame A target frame to go to in time.
	virtual void GoToFrame( const char * seqName, float targetFrame ) = 0;

	// Get the name of camera used for sequences instead of cameras specified in the director node.
	virtual const char * GetOverrideCamName() const = 0;

  // Get behavior pattern for stopping sequences.
	virtual IMovieSystem::ESequenceStopBehavior GetSequenceStopBehavior()=0;

	// These are used to disable 'Ragdollize' events in the editor when the 'AI/Physics' is off.
	virtual bool IsPhysicsEventsEnabled() const = 0;
	virtual void EnablePhysicsEvents(bool enable) = 0;

	virtual void EnableBatchRenderMode(bool bOn) = 0;
	virtual bool IsInBatchRenderMode() const = 0;

	// Accessor for entity node params for displaying in sandbox
	virtual int GetEntityNodeParamCount() const = 0;
	virtual CAnimParamType GetEntityNodeParamType(int index) const = 0;
	virtual const char *GetEntityNodeParamName(int index) const = 0;
	virtual IAnimNode::ESupportedParamFlags GetEntityNodeParamFlags(int index) const = 0;

	virtual ILightAnimWrapper* CreateLightAnimWrapper(const char* name) const = 0;

	// Should only be called from CAnimParamType
	virtual void SerializeParamType(CAnimParamType &animParamType, XmlNodeRef &xmlNode, bool bLoading, const uint version) = 0;

#ifdef MOVIESYSTEM_SUPPORT_EDITING
	virtual EAnimNodeType GetNodeTypeFromString(const char *pString) const = 0;
	virtual CAnimParamType GetParamTypeFromString(const char *pString) const = 0;
#endif

	// </interfuscator:shuffle>
};

inline void SAnimContext::Serialize(XmlNodeRef& xmlNode, bool bLoading)
{
	if(bLoading)
	{
		XmlString name;
		if(xmlNode->getAttr("sequence", name))
			pSequence = gEnv->pMovieSystem->FindSequence(name.c_str());
		xmlNode->getAttr("dt", dt);
		xmlNode->getAttr("fps", fps);
		xmlNode->getAttr("time", time);
		xmlNode->getAttr("bSingleFrame", bSingleFrame);
		xmlNode->getAttr("bResetting", bResetting);
		xmlNode->getAttr("trackMask", trackMask);
		xmlNode->getAttr("startTime", startTime);
	}
	else
	{
		if(pSequence)
		{
			string fullname = pSequence->GetName();
			xmlNode->setAttr("sequence", fullname.c_str());
		}
		xmlNode->setAttr("dt", dt);
		xmlNode->setAttr("fps", fps);
		xmlNode->setAttr("time", time);
		xmlNode->setAttr("bSingleFrame", bSingleFrame);
		xmlNode->setAttr("bResetting", bResetting);
		xmlNode->setAttr("trackMask", trackMask);
		xmlNode->setAttr("startTime", startTime);
	}
}

void CAnimParamType::Serialize( XmlNodeRef &xmlNode, bool bLoading, const uint version )
{
	gEnv->pMovieSystem->SerializeParamType(*this, xmlNode, bLoading, version);
}
