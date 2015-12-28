#ifdef _DEVIRTUALIZE_
	#include <GameSDK_devirt_defines.h>
#endif

#include "../PersistantStats.cpp"
#include "../PlayerModifiableValues.cpp"
#include "../PlayerPlugin.cpp"
#include "../PlayerPlugin_CurrentlyTargetting.cpp"
#include "../PlayerPlugin_Interaction.cpp"
#include "../PlayerProgression.cpp"
#include "../ProgressionUnlocks.cpp"
#include "../SkillKill.cpp"
#include "../SkillRanking.cpp"
#include "../DualCharacterProxy.cpp"
#include "../IKTorsoAim_Helper.cpp"
#include "../LookAim_Helper.cpp"
#include "../TransformationPinning.cpp"
#include "../WeaponFPAiming.cpp"
#include "../Battlechatter.cpp"
#include "../EquipmentLoadout.cpp"
#include "../GameRulesModules/GameRulesAssistScoring.cpp"
#include "../GameRulesModules/GameRulesCommonDamageHandling.cpp"
#include "../GameRulesModules/GameRulesExtractionVictoryConditions.cpp"
#include "../GameRulesModules/GameRulesModulesManager.cpp"
#include "../GameRulesModules/GameRulesMPActorAction.cpp"
#include "../GameRulesModules/GameRulesMPDamageHandling.cpp"
#include "../GameRulesModules/GameRulesMPSimpleSpawning.cpp"
#include "../GameRulesModules/GameRulesMPSpawning.cpp"
#include "../GameRulesModules/GameRulesMPSpawningWithLives.cpp"

#ifdef _DEVIRTUALIZE_
	#include <GameSDK_wrapper_includes.h>
#endif
