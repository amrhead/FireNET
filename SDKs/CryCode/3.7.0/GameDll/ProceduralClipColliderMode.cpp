// (c) 2001 - 2012 Crytek GmbH.
#include "StdAfx.h"

#include "ICryMannequin.h"
#include <ICryAnimation.h>
#include <IAnimationPoseModifier.h>
#include <CryExtension/Impl/ClassWeaver.h>
#include <Mannequin/Serialization.h>

#include "ProceduralContextColliderMode.h"



namespace ColliderModeRemapping
{
	uint32 g_colliderModeCrc[ eColliderMode_COUNT ] = { 0 };

	EColliderMode GetFromCrcParam( const uint32 paramCrc )
	{
		const bool initialized = ( g_colliderModeCrc[ eColliderMode_Undefined ] != 0 );
		if ( ! initialized )
		{
			Crc32Gen* pCrc32Gen = gEnv->pSystem->GetCrc32Gen();
			g_colliderModeCrc[ eColliderMode_Undefined ]         = pCrc32Gen->GetCRC32Lowercase( "Undefined" );
			g_colliderModeCrc[ eColliderMode_Disabled ]          = pCrc32Gen->GetCRC32Lowercase( "Disabled" );
			g_colliderModeCrc[ eColliderMode_GroundedOnly ]      = pCrc32Gen->GetCRC32Lowercase( "GroundedOnly" );
			g_colliderModeCrc[ eColliderMode_Pushable ]          = pCrc32Gen->GetCRC32Lowercase( "Pushable" );
			g_colliderModeCrc[ eColliderMode_NonPushable ]       = pCrc32Gen->GetCRC32Lowercase( "NonPushable" );
			g_colliderModeCrc[ eColliderMode_PushesPlayersOnly ] = pCrc32Gen->GetCRC32Lowercase( "PushesPlayersOnly" );
			g_colliderModeCrc[ eColliderMode_Spectator ]         = pCrc32Gen->GetCRC32Lowercase( "Spectator" );
		}

		for ( size_t i = 0; i < eColliderMode_COUNT; ++i )
		{
			const uint32 colliderModeCrc = g_colliderModeCrc[ i ];
			if ( colliderModeCrc == paramCrc )
			{
				return static_cast< EColliderMode >( i );
			}
		}
		
		return eColliderMode_Undefined;
	}
}

struct SColliderModeParams : IProceduralParams
{
	virtual void Serialize(Serialization::IArchive& ar)
	{
		ar(mode, "Mode", "Mode");
	}

	SProcDataCRC mode;
};

class CProceduralClipColliderMode
	: public TProceduralContextualClip< CProceduralContextColliderMode, SColliderModeParams>
{
public:
	CProceduralClipColliderMode()
		: m_requestId( 0 )
	{
	}

	virtual void OnEnter( float blendTime, float duration, const SColliderModeParams& params )
	{
		const EColliderMode mode = ColliderModeRemapping::GetFromCrcParam( params.mode.crc );

		m_requestId = m_context->RequestColliderMode( mode );
	}

	virtual void OnExit( float blendTime )
	{
		m_context->CancelRequest( m_requestId );
	}

	virtual void Update( float timePassed )
	{
	}

private:
	uint32 m_requestId;
};


REGISTER_PROCEDURAL_CLIP(CProceduralClipColliderMode, "ColliderMode");
