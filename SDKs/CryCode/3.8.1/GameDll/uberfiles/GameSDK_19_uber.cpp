#ifdef _DEVIRTUALIZE_
	#include <GameSDK_devirt_defines.h>
#endif

#include "../Utility/CryWatch.cpp"
#include "../Utility/DesignerWarning.cpp"
#include "../Utility/SingleAllocTextBlock.cpp"
#include "../SShootHelper.cpp"
#include "../Utility/StringUtils.cpp"
#include "../Effects/GameEffectsSystem.cpp"
#include "../Effects/GameEffects/ExplosionGameEffect.cpp"
#include "../Effects/GameEffects/GameEffect.cpp"
#include "../Effects/GameEffects/HitRecoilGameEffect.cpp"
#include "../Effects/GameEffects/HudInterferenceGameEffect.cpp"
#include "../Effects/GameEffects/KillCamGameEffect.cpp"
#include "../Effects/GameEffects/PlayerHealthEffect.cpp"
#include "../Effects/HUDEventListeners/LetterBoxHudEventListener.cpp"
#include "../Effects/Tools/CVarActivationSystem.cpp"
#include "../Effects/Tools/PostEffectActivationSystem.cpp"
#include "../GameCodeCoverage/GameCodeCoverageGUI.cpp"
#include "../GameCodeCoverage/GameCodeCoverageManager.cpp"
#include "../GameCodeCoverage/GameCodeCoverageTracker.cpp"
#include "../GameCodeCoverage/IGameCodeCoverageListener.cpp"
#include "../Audio/Announcer.cpp"
#include "../Audio/AreaAnnouncer.cpp"
#include "../Audio/MiscAnnouncer.cpp"
#include "../Graphics/2DRenderUtils.cpp"
#include "../Network/GameNetworkUtils.cpp"
#include "../Network/Lobby/GameAchievements.cpp"

#ifdef _DEVIRTUALIZE_
	#include <GameSDK_wrapper_includes.h>
#endif
