#include "StdAfx.h"
#include "GameConstantCVars.h"

SGameReleaseConstantCVars* SGameReleaseConstantCVars::m_pThis = NULL;

SGameReleaseConstantCVars::SGameReleaseConstantCVars()
{
	// Only one instance of this object is allowed (stored in GameCvars)
	assert( m_pThis == NULL );

	m_pThis = this;
}

void SGameReleaseConstantCVars::Init( IConsole *pConsole )
{
	assert(pConsole != NULL);
	if (pConsole == NULL)
		return;

	// Button mashing sequence (SystemX)
	DefineGameConstFloatCVar(g_SystemX_buttonMashing_initial, VF_CHEAT, "Status progress initial value");
	DefineGameConstFloatCVar(g_SystemX_buttonMashing_attack, VF_CHEAT, "Amount to increase for every button hit");
	DefineGameConstFloatCVar(g_SystemX_buttonMashing_decayStart, VF_CHEAT, "Amount to decrease per second");
	DefineGameConstFloatCVar(g_SystemX_buttonMashing_decayMax, VF_CHEAT, "Max amount to decrease per second after time out expires");
	DefineGameConstFloatCVar(g_SystemX_buttonMashing_decayTimeOutToIncrease, VF_CHEAT, "Time out to start to increase decay towards maximum");
	DefineGameConstFloatCVar(g_SystemX_buttonMashing_decayRampUpTime, VF_CHEAT, "Time to increase from normal decay to maximum after initial time out");

	// SystemX
	DefineGameConstIntCVar(g_SystemXDebug_State, VF_CHEAT, "Debug HFSM");
	DefineGameConstIntCVar(g_SystemXDebug_HealthStatus, VF_CHEAT, "Debug drill heads status");
	DefineGameConstIntCVar(g_SystemXDebug_Movement, VF_CHEAT, "Debug movement and target tracking status");
	DefineGameConstIntCVar(g_SystemXDebug_MeleeAttacks, VF_CHEAT, "Debug melee attacks");
	DefineGameConstFloatCVar(g_SystemX_Stomp_Radius, VF_CHEAT, "Stomp attack damage area radius");
	DefineGameConstFloatCVar(g_SystemX_Sweep_Radius, VF_CHEAT, "Sweep attack damage area radius");
	DefineGameConstFloatCVar(g_SystemX_Sweep_ZOffset, VF_CHEAT, "Sweep attack damage area up offset");
	DefineGameConstFloatCVar(g_SystemX_Sweep_SideSelectionThreshold, VF_CHEAT, "Flat distance to player to select one side or the other of the platform");

	// Frontend
	DefineGameConstFloatCVar(g_loadingHintRefreshTimeSP, VF_CHEAT, "Loading hint refresh time (SP)");
	DefineGameConstFloatCVar(g_loadingHintRefreshTimeMP, VF_CHEAT, "Loading hint refresh time (MP)");
	DefineGameConstIntCVar(g_storyinfo_openitem_enabled, VF_CHEAT, "Enables story info openitem action is available after picking up story database collectable");
	DefineGameConstFloatCVar(g_storyinfo_openitem_time, VF_CHEAT, "Duration story info openitem action is available after picking up story database collectable");

	//HUD tweaking cvars
	DefineGameConstFloatCVar(hud_InterestPoints_ScanDistance_Hazards, VF_CHEAT, "Scan distance in which hazards are displayed");
	DefineGameConstFloatCVar(hud_InterestPoints_ScanDistance_Vehicles, VF_CHEAT, "Scan distance in which vehicles are displayed");
	DefineGameConstFloatCVar(hud_InterestPoints_ScanDistance_Units, VF_CHEAT, "Scan distance in which units are displayed");
	DefineGameConstFloatCVar(hud_InterestPoints_ScanDistance_Items, VF_CHEAT, "Scan distance in which items are displayed");
	DefineGameConstFloatCVar(hud_InterestPoints_ScanDistance_Ammo, VF_CHEAT, "Scan distance in which ammo crates are displayed");
	DefineGameConstFloatCVar(hud_InterestPoints_ScanDistance_Explosives, VF_CHEAT, "Scan distance in which explosives (mines) are displayed");
	DefineGameConstFloatCVar(hud_InterestPoints_ScanDistance_Story, VF_CHEAT, "Scan distance in which general story objects are displayed");

	DefineGameConstFloatCVar(hud_ondemandloading_2d, VF_CHEAT, "Enables / disabling ondemand loading for 2d hud");
	DefineGameConstFloatCVar(hud_ondemandloading_3d, VF_CHEAT, "Enables / disabling ondemand loading for 3d hud");

	//Debug cheats
	DefineGameConstIntCVar(g_infiniteSprintStamina, VF_CHEAT, "Infinite sprint");

	// Flash door panel
	DefineGameConstFloatCVar(g_flashdoorpanel_distancecheckinterval, VF_CHEAT, "Flash door panel distance check interval for visibility");

	//Achievements
	DefineGameConstFloatCVar(g_achievements_aiDetectionDelay, VF_CHEAT, "Used by certain achievement stats, defines the amount of time the ai must not be alerted by the player after performing some special kills");

	//STAP
	DefineGameConstIntCVar(g_stapEnable, VF_CHEAT, "Enables the STAP system. 0=disabled; 1=enabled; 2=force STAP to always be active");
	DefineGameConstIntCVar(g_stapLayer, VF_CHEAT, "Specifies the layer on with to apply the STAP assuming that we are using the multi-threaded, interleaved STAP system");
	DefineGameConstIntCVar(g_pwaLayer, VF_CHEAT, "Specifies the layer on with to apply Procedural Weapon Animations. Should be above g_stapLayer");
	DefineGameConstIntCVar(g_translationPinningEnable, VF_CHEAT, "Enables Translation Pinning for the client player");

	DefineGameConstFloatCVar(STAP_MF_All, VF_CHEAT, "");
	DefineGameConstFloatCVar(STAP_MF_Scope, VF_CHEAT, "");
	DefineGameConstFloatCVar(STAP_MF_ScopeVertical, VF_CHEAT, "");
	DefineGameConstFloatCVar(STAP_MF_HeavyWeapon, VF_CHEAT, "");
	DefineGameConstFloatCVar(STAP_MF_Up, VF_CHEAT, "");
	DefineGameConstFloatCVar(STAP_MF_Down, VF_CHEAT, "");
	DefineGameConstFloatCVar(STAP_MF_Left, VF_CHEAT, "");
	DefineGameConstFloatCVar(STAP_MF_Right, VF_CHEAT, "");
	DefineGameConstFloatCVar(STAP_MF_Front, VF_CHEAT, "");
	DefineGameConstFloatCVar(STAP_MF_Back, VF_CHEAT, "");
	DefineGameConstFloatCVar(STAP_MF_StrafeLeft, VF_CHEAT, "");
	DefineGameConstFloatCVar(STAP_MF_StrafeRight, VF_CHEAT, "");
	DefineGameConstFloatCVar(STAP_MF_VerticalMotion, VF_CHEAT, "");
	DefineGameConstFloatCVar(STAP_MF_VelFactorVertical, VF_CHEAT, "");
	DefineGameConstFloatCVar(STAP_MF_VelFactorHorizontal, VF_CHEAT, "");

	//Tracers
	DefineGameConstFloatCVar(g_tracers_slowDownAtCameraDistance, VF_CHEAT, "Slow down tracer speed when inside this area around the camera");
	DefineGameConstFloatCVar(g_tracers_minScale, VF_CHEAT, "Minimum scale for tracer geometry");
	DefineGameConstFloatCVar(g_tracers_minScaleAtDistance, VF_CHEAT, "Minimum scale applies at this distance from the camera");
	DefineGameConstFloatCVar(g_tracers_maxScale, VF_CHEAT, "Maximum scale for tracer geometry");
	DefineGameConstFloatCVar(g_tracers_maxScaleAtDistance, VF_CHEAT, "Maximum scale applies at this distance from the camera");
}
