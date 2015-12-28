#ifdef _DEVIRTUALIZE_
	#include <GameSDK_devirt_defines.h>
#endif

#include "../Network/Lobby/GameBrowser.cpp"
#include "../Network/Lobby/GameLobby.cpp"
#include "../Network/Lobby/GameLobbyData.cpp"
#include "../Network/Lobby/GameServerLists.cpp"
#include "../Network/Lobby/SessionNames.cpp"
#include "../Network/Lobby/SessionSearchSimulator.cpp"
#include "../Network/Squad/SquadManager.cpp"
#include "../DownloadMgr.cpp"
#include "../CircularStatsStorage.cpp"
#include "../StatHelpers.cpp"
#include "../StatsRecordingMgr.cpp"
#include "../TelemetryCollector.cpp"
#include "../XMLStatsSerializer.cpp"
#include "../RecordingBuffer.cpp"
#include "../RecordingSystemPackets.cpp"
#include "../ReplayActor.cpp"
#include "../ReplayObject.cpp"
#include "../MultiplayerEntities/CarryEntity.cpp"
#include "../MultiplayerEntities/NetworkedPhysicsEntity.cpp"
#include "../Graphics/ColorGradientManager.cpp"
#include "../AI/AdvantagePointOccupancyControl.cpp"
#include "../AI/AIBattleFront.cpp"
#include "../AI/DeathManager.cpp"
#include "../AI/GameAIEnv.cpp"
#include "../AI/GameAIHelpers.cpp"

#ifdef _DEVIRTUALIZE_
	#include <GameSDK_wrapper_includes.h>
#endif
