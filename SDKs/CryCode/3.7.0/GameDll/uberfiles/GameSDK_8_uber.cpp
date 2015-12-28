#ifdef _DEVIRTUALIZE_
	#include <GameSDK_devirt_defines.h>
#endif

#include "../UI/UIManager.cpp"
#include "../UI/UIMenuEvents.cpp"
#include "../UI/UIMultiPlayer.cpp"
#include "../UI/UIObjectives.cpp"
#include "../UI/UISettings.cpp"
#include "../UI/Utils/ILoadingMessageProvider.cpp"
#include "../UI/Utils/LocalizedStringManager.cpp"
#include "../UI/Utils/ScreenLayoutManager.cpp"
#include "../UI/Utils/ScreenLayoutUtils.cpp"
#include "../UI/WarningsManager.cpp"
#include "../VehicleActionAutoTarget.cpp"
#include "../VehicleDamageBehaviorAudioFeedback.cpp"
#include "../VehicleMountedWeapon.cpp"
#include "../VehicleMovementHelicopter.cpp"
#include "../QuatTBlender.cpp"
#include "../ScriptBind_Actor.cpp"
#include "../BodyDamage.cpp"
#include "../BodyDestruction.cpp"
#include "../BodyManager.cpp"
#include "../BodyManagerCVars.cpp"
#include "../GodMode.cpp"
#include "../NetPlayerInput.cpp"
#include "../Player.cpp"
#include "../PlayerInput.cpp"
#include "../PlayerMovementController.cpp"

#ifdef _DEVIRTUALIZE_
	#include <GameSDK_wrapper_includes.h>
#endif
