////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001-2010.
// -------------------------------------------------------------------------
//  File name:   FlashUI.h
//  Version:     v1.00
//  Created:     10/9/2010 by Paul Reindell.
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////
#ifndef __FlashUI_H__
#define __FlashUI_H__

#include <IFlashUI.h>
#include <CryExtension/Impl/ClassWeaver.h>
#include <IHardwareMouse.h>
#include <IInput.h>
#include <IGameFramework.h>
#include <ILevelSystem.h>
#include "FlashUIEventSystem.h"

#if !defined (_RELEASE) || defined(RELEASE_LOGGING)
#define UIACTION_LOGGING
#endif

#if defined (UIACTION_LOGGING)
#define UIACTION_LOG( ... )     { if (CFlashUI::CV_gfx_uiaction_log) CFlashUI::LogUIAction( IFlashUI::eLEL_Log,     __VA_ARGS__ ); }
#define UIACTION_WARNING( ... ) { CFlashUI::LogUIAction( IFlashUI::eLEL_Warning, __VA_ARGS__ ); }
#define UIACTION_ERROR( ... )   { CFlashUI::LogUIAction( IFlashUI::eLEL_Error,   __VA_ARGS__ ); }
#else
#define UIACTION_LOG     (void)
#define UIACTION_WARNING (void)
#define UIACTION_ERROR   (void)
#endif



class CAutoRegUIFlowNode;
struct CUIActionManager;
class CFlashUIActionEvents;

class CFlashUI 
	: public IFlashUI
	, public IHardwareMouseEventListener
	, public IInputEventListener
	, public IGameFrameworkListener
	, public ILevelSystemListener
	, public ISystemEventListener
	, public ILoadtimeCallback
{
	CRYINTERFACE_BEGIN()
	CRYINTERFACE_ADD( IFlashUI )
	CRYINTERFACE_END()

	CRYGENERATE_SINGLETONCLASS( CFlashUI, IFlashUIExtensionName, 0x35AE7F0FBB13437B, 0x9C5FFCD2568616A5 )

public:
	// IFlashUI
	virtual void Init();
	virtual bool PostInit();
	virtual void Update(float fDeltatime);
	virtual void Reload();
	virtual void Shutdown();

	virtual bool LoadElementsFromFile( const char* sFileName );
	virtual bool LoadActionFromFile( const char* sFileName, IUIAction::EUIActionType type );

	virtual IUIElement* GetUIElement( const char* sName ) const { return const_cast<IUIElement*>(m_elements(sName)); };
	virtual IUIElement* GetUIElement( int index ) const { return index < m_elements.size() ? const_cast<IUIElement*>(m_elements[index]) : NULL; };
	virtual int GetUIElementCount() const { return m_elements.size(); };
	virtual IUIElement* GetUIElementByInstanceStr( const char* sUIInstanceStr) const;

	virtual IUIAction* GetUIAction( const char* sName ) const { return const_cast<IUIAction*>(m_actions(sName)); };
	virtual IUIAction* GetUIAction( int index ) const { return index < m_actions.size() ? const_cast<IUIAction*>(m_actions[index]) : NULL; };
	virtual int GetUIActionCount() const { return m_actions.size(); }

	virtual IUIActionManager* GetUIActionManager() const;
	virtual void UpdateFG();
	virtual void EnableEventStack( bool bEnable );
	virtual void RegisterModule( IUIModule* pModule, const char* name );
	virtual void UnregisterModule( IUIModule* pModule );

	virtual void SetHudElementsVisible(bool bVisible);

	virtual IUIEventSystem* CreateEventSystem( const char* sName, IUIEventSystem::EEventSystemType eType );
	virtual IUIEventSystem* GetEventSystem( const char* name, IUIEventSystem::EEventSystemType eType );
	virtual IUIEventSystemIteratorPtr CreateEventSystemIterator( IUIEventSystem::EEventSystemType eType ); 

	virtual void DispatchControllerEvent( IUIElement::EControllerInputEvent event, IUIElement::EControllerInputState state, float value );
	virtual void SendFlashMouseEvent( SFlashCursorEvent::ECursorState evt, int iX, int iY, int iButton = 0, float wheel = 0.f, bool bFromController = false );
	virtual bool DisplayVirtualKeyboard( unsigned int flags, const char* title, const char* initialInput, int maxInputLength, IVirtualKeyboardEvents *pInCallback );
	virtual bool IsVirtualKeyboardRunning();
	virtual bool CancelVirtualKeyboard();

	virtual void GetScreenSize(int &width, int &height);
	virtual void SetEditorScreenSizeCallback(TEditorScreenSizeCallback& cb); 
	virtual void RemoveEditorScreenSizeCallback();

	virtual void SetEditorUILogEventCallback(TEditorUILogEventCallback& cb);
	virtual void RemoveEditorUILogEventCallback();

	virtual void SetEditorPlatformCallback(TEditorPlatformCallback& cb);
	virtual void RemoveEditorPlatformCallback();

	virtual bool UseSharedRT(const char* instanceStr, bool defVal) const;

	virtual void CheckPreloadedTexture(ITexture* pTexture) const;

	virtual void GetMemoryStatistics(ICrySizer * s) const;

#if !defined(_LIB) || defined(IS_EAAS)
	virtual SUIItemLookupSet_Impl<SUIParameterDesc>* CreateLookupParameter() { return new SUIItemLookupSet_Impl<SUIParameterDesc>(); };
	virtual SUIItemLookupSet_Impl<SUIMovieClipDesc>* CreateLookupMovieClip() { return new SUIItemLookupSet_Impl<SUIMovieClipDesc>(); };
	virtual SUIItemLookupSet_Impl<SUIEventDesc>* CreateLookupEvent() { return new SUIItemLookupSet_Impl<SUIEventDesc>(); };
#endif
	// ~IFlashUI

	// IHardwareMouseEventListener
	void OnHardwareMouseEvent( int iX,int iY,EHARDWAREMOUSEEVENT eHardwareMouseEvent, int wheelDelta );
	// ~IHardwareMouseEventListener

	// IInputEventListener
	virtual bool OnInputEvent( const SInputEvent &event );
	virtual bool OnInputEventUI( const SUnicodeEvent &event );
	// ~IInputEventListener

	// ISystemEventListener
	virtual void OnSystemEvent( ESystemEvent event,UINT_PTR wparam,UINT_PTR lparam );
	// ~ISystemEventListener

	// IGameFrameworkListener
	virtual void OnPostUpdate(float fDeltaTime) {};
	virtual void OnSaveGame(ISaveGame* pSaveGame) {};
	virtual void OnLoadGame(ILoadGame* pLoadGame) {};
	virtual void OnLevelEnd(const char* nextLevel) {};
	virtual void OnActionEvent(const SActionEvent& event);
	// ~IGameFrameworkListener

	// ILevelSystemListener
	virtual void OnLevelNotFound(const char *levelName);
	virtual void OnLoadingStart(ILevelInfo *pLevel) {};
	virtual void OnLoadingLevelEntitiesStart(ILevelInfo* pLevel) {};
	virtual void OnLoadingComplete(ILevel *pLevel) {};
	virtual void OnLoadingError(ILevelInfo *pLevel, const char *error);
	virtual void OnLoadingProgress(ILevelInfo *pLevel, int progressAmount);
	virtual void OnUnloadComplete(ILevel* pLevel) {};
	// ~ILevelSystemListener

	// ILoadtimeCallback
	virtual void LoadtimeUpdate(float fDeltaTime);
	virtual void LoadtimeRender();
	// ~ILoadtimeCallback

	// logging
	static void LogUIAction( ELogEventLevel level, const char *format, ... ) PRINTF_PARAMS(2,3);

	// cvars
	DeclareStaticConstIntCVar(CV_gfx_draw, 1);
	DeclareStaticConstIntCVar(CV_gfx_debugdraw, 0);
	DeclareStaticConstIntCVar(CV_gfx_uiaction_log, 0);
	DeclareStaticConstIntCVar(CV_gfx_uiaction_enable, 1);
	DeclareStaticConstIntCVar(CV_gfx_loadtimethread, 1);
	DeclareStaticConstIntCVar(CV_gfx_reloadonlanguagechange, 1);
	DeclareStaticConstIntCVar(CV_gfx_uievents_editorenabled, 1);
	DeclareStaticConstIntCVar(CV_gfx_ampserver, 0);
	static float CV_gfx_inputevents_triggerstart;
	static float CV_gfx_inputevents_triggerrepeat;
	static ICVar* CV_gfx_uiaction_log_filter;
	static ICVar* CV_gfx_uiaction_folder;

	static void ReloadAllElements(IConsoleCmdArgs* /* pArgs */);

	void InvalidateSortedElements();

	bool IsLoadtimeThread() const { return m_bLoadtimeThread; };

	EPlatformUI GetCurrentPlatform();

	typedef std::vector< CAutoRegUIFlowNode* > TUIFlowNodes;
private:
	CFlashUI( const CFlashUI& ) : m_modules(8) {}
	void operator = ( const CFlashUI& ) {}

	void RegisterListeners();
	void UnregisterListeners();

	void ReloadAll();

	void LoadElements();
	void ClearElements();

	void LoadActions();
	void ClearActions();

	void ResetActions();
	void ReloadScripts();

	void CreateNodes();
	void ClearNodes();

	void LoadFromFile(const char* sFolderName, const char* pSearch, bool (CFlashUI::*fhFileLoader)(const char*));
	bool LoadFGActionFromFile( const char* sFileName );
	bool LoadLuaActionFromFile( const char* sFileName );

	void PreloadTextures(const char* pLevelName = NULL);
	void PreloadTexturesFromNode(const XmlNodeRef& node);
	bool PreloadTexture(const char* pFileName);
	void ReleasePreloadedTextures(bool bReleaseTextures = true);

	typedef std::multimap<int, IUIElement*> TSortedElementList;
	inline const TSortedElementList& GetSortedElements();
	inline void UpdateSortedElements();

	void CreateMouseClick(  IUIElement::EControllerInputState state  );

	void TriggerEvent( const SInputEvent & event );

	SFlashKeyEvent MapToFlashKeyEvent(const SInputEvent &inputEvent);

	TUIEventSystemMap* GetEventSystemMap( IUIEventSystem::EEventSystemType eType );

	void StartRenderThread();
	void StopRenderThread();

	inline void CheckLanguageChanged();
	inline void CheckResolutionChange();

	void ReloadAllBootStrapper();
	void ResetDirtyFlags();

	static bool CheckFilter( const string& str );

private:
	CFlashUIActionEvents* m_pFlashUIActionEvents;

	TUIEventSystemMap m_eventSystemsUiToSys;
	TUIEventSystemMap m_eventSystemsSysToUi;

 	TUIElementsLookup m_elements;
 	TUIActionsLookup m_actions;

	TSortedElementList m_sortedElements;
	bool m_bSortedElementsInvalidated;

	bool m_bLoadtimeThread;
	typedef std::vector< IFlashPlayer* > TPlayerList;
	TPlayerList m_loadtimePlayerList;

	TUIFlowNodes m_UINodes;

	typedef std::map<ITexture*, string> TTextureMap;
	TTextureMap m_preloadedTextures;

	CUIActionManager *m_pUIActionManager;

	typedef CListenerSet< IUIModule* > TUIModules;
	TUIModules m_modules;

	int m_iWidth;
	int m_iHeight;
	TEditorScreenSizeCallback m_ScreenSizeCB;
	TEditorUILogEventCallback m_LogCallback;
	TEditorPlatformCallback m_plattformCallback;

	enum ESystemState
	{
		eSS_NoLevel,
		eSS_Loading,
		eSS_LoadingDone,
		eSS_GameStarted,
		eSS_Unloading,
	};

	ESystemState m_systemState;
	float m_fLastAdvance;
	float m_lastTimeTriggered;
	bool m_bHudVisible;
};

#endif // #ifndef __FlashUI_H__