#ifdef _DEVIRTUALIZE_
	#include <GameSDK_devirt_defines.h>
#endif

#include "../PickAndThrowUtilities.cpp"
#include "../PlayerAI.cpp"
#include "../PlayerControl/PlayerEnslavement.cpp"
#include "../PlayerPlugin_InteractiveEntityMonitor.cpp"
#include "../PlayerStateAIMovement.cpp"
#include "../ActionCoopAnim.cpp"
#include "../AI/AIAwarenessToPlayerHelper.cpp"
#include "../AI/AICounters.cpp"
#include "../AI/HazardModule/Hazard.cpp"
#include "../AI/HazardModule/HazardModule.cpp"
#include "../AI/HazardModule/HazardProjectile.cpp"
#include "../AI/HazardModule/HazardRayCast.cpp"
#include "../AI/HazardModule/HazardShared.cpp"
#include "../AI/HazardModule/HazardSphere.cpp"
#include "../AntiCheat/ClientCheatMonitor.cpp"
#include "../AntiCheat/ServerCheatMonitor.cpp"
#include "../Audio/GameAudioUtils.cpp"
#include "../AnimActionAIAimPose.cpp"
#include "../AnimActionAIMovement.cpp"
#include "../AntiCheat/ShotCounter.cpp"
#include "../BasicEventListener.cpp"
#include "../Effects/GameEffects/GameEffectSoftCodeLibrary.cpp"
#include "../Effects/GameEffects/SceneBlurGameEffect.cpp"
#include "../Effects/RenderElements/GameRenderElement.cpp"
#include "../Effects/RenderElements/GameRenderElementSoftCodeLibrary.cpp"

#ifdef _DEVIRTUALIZE_
	#include <GameSDK_wrapper_includes.h>
#endif
