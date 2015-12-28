// ============================================================================
// ============================================================================
// ============================================================================
// ====
// ==== Mastermind Module - Death Listener
// ====
// ==== An AI system listener that will reports deaths that are relevant to 
// ==== the mastermind.
// ====
//


#pragma once

#ifndef MastermindModuleDeathListener_h
#define MastermindModuleDeathListener_h

#include <AISystemListener.h>


// Forward declarations:
class CActor;


namespace Mastermind
{

// Forward declarations:
class MastermindModule;


// The mastermind modules provides various services for the Mastermind agents.
class MastermindModuleDeathListener : public IAISystemListener
{
public:
	typedef IAISystemListener BaseClass;


public:
	MastermindModuleDeathListener();
	
	virtual ~MastermindModuleDeathListener();
		
	virtual void                        OnAgentDeath(EntityId deadEntityID);
	void                                Init(MastermindModule *mastermindModule);

private:
	// A pointer to the owner mastermind module (NULL if not initialized).
	MastermindModule *					m_MastermindModule;

private:

	// Resurrection:
	bool								IsEntityResurrectable(const IEntity* deadEntity) const;	
};


}; // namespace Mastermind


#endif // MastermindModuleDeathListener_h