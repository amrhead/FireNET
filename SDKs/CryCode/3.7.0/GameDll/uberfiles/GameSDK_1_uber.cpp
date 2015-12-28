#ifdef _DEVIRTUALIZE_
	#include <GameSDK_devirt_defines.h>
#endif

#include "../DoubleMagazine.cpp"
#include "../Effects/GameEffects/LightningGameEffect.cpp"
#include "../Effects/GameEffects/ParameterGameEffect.cpp"
#include "../Effects/GameEffects/WaterEffects.cpp"
#include "../Effects/GameEffects/PlayerMindGameBeamEffect.cpp"
#include "../Effects/RenderNodes/LightningNode.cpp"
#include "../EMPGrenade.cpp"
#include "../Environment/DoorPanel.cpp"
#include "../Environment/DoorPanelBehavior.cpp"
#include "../Environment/Ledge.cpp"
#include "../Environment/LightningArc.cpp"
#include "../Environment/MineField.cpp"
#include "../Environment/ScriptBind_LightningArc.cpp"
#include "../Environment/ScriptBind_TowerSearchLight.cpp"
#include "../Environment/SmartMine.cpp"
#include "../Environment/SmartMineBehavior.cpp"
#include "../Environment/TowerSearchLight.cpp"
#include "../Environment/VicinityDependentObjectMover.cpp"
#include "../Environment/WaterPuddle.cpp"
#include "../Environment/WaterRipplesGenerator.cpp"
#include "../ExactPositioning.cpp"
#include "../ExactPositioningTrigger.cpp"

#ifdef _DEVIRTUALIZE_
	#include <GameSDK_wrapper_includes.h>
#endif
