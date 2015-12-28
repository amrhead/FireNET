#ifdef _DEVIRTUALIZE_
	#include <GameSDK_devirt_defines.h>
#endif

#include "../Effects/RenderNodes/GameRenderNodeSoftCodeLibrary.cpp"
#include "../Environment/DangerousRigidBody.cpp"
#include "../Environment/DeflectorShield.cpp"
#include "../GameDll.cpp"
#include "../GameLocalizationManager.cpp"
#include "../GameRulesModules/GameRulesObjective_Predator.cpp"
#include "../GameStartup.cpp"
#include "../Actor.cpp"
#include "../ActorDamageEffectController.cpp"
#include "../ActorImpulseHandler.cpp"
#include "../ActorLuaCache.cpp"
#include "../ActorManager.cpp"
#include "../ActorTelemetry.cpp"
#include "../AIDemoInput.cpp"
#include "../HeavyMountedWeapon.cpp"
#include "../ItemAnimation.cpp"
#include "../MovementAction.cpp"
#include "../MPPathFollowingManager.cpp"
#include "../Network/Lobby/PlaylistActivityTracker.cpp"
#include "../Nodes/AIAssignmentNodes.cpp"
#include "../PlayerEntityInteraction.cpp"
#include "../PlayerPositionChecker.cpp"
#include "../PlayerStateLadder.cpp"
#include "../PlayerStateLinked.cpp"
#include "../PlayerStateEntry.cpp"

#ifdef _DEVIRTUALIZE_
	#include <GameSDK_wrapper_includes.h>
#endif
