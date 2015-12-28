// ============================================================================
// ============================================================================
// ============================================================================
// ====
// ==== Mastermind Module Shared
// ====
// ==== Various definitions that are shared between the mastermind module and
// ==== depended systems.
// ====
//


#include "StdAfx.h"
#include "MastermindModule.h"


using namespace Mastermind;


// ============================================================================
// ============================================================================
// ============================================================================
//
// -- MastermindModuleInstanceConfig -- MastermindModuleInstanceConfig --
//
// ============================================================================
// ============================================================================
// ============================================================================


MastermindModuleInstanceConfig::MastermindModuleInstanceConfig() :
	m_SearchRadius(0.0f)
	, m_ResurrectionRadius(0.0f)
	, m_ResurrectablesAvailableSignal()
	, m_ResurrectablesUnavailableSignal()
	, m_ShieldForwardOffset(0.0f)
	, m_ShieldUpwardOffset(0.0f)
	, m_HusksSpawnedSignal()
	, m_HusksAllDestroyedSignal()
	, m_EnemySpottedByHuskNotifyDelay(0.0f)
{
}
