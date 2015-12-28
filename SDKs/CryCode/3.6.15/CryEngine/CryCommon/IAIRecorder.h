#include DEVIRTUALIZE_HEADER_FIX(IAIRecorder.h)

#ifndef _IAIRECORDER_H_
#define _IAIRECORDER_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#if defined(LINUX) || defined(APPLE)
#	include "platform.h"
#endif

// These lines will enable AI signal profiling
//#ifndef AI_LOG_SIGNALS
//	#define AI_LOG_SIGNALS
//#endif

////////////////////////////////////////////////////

// Debug context for a specific AI object
struct SAIRecorderObjectDebugContext
{
	string sName;
	bool bEnableDrawing;
	bool bSetView;
	ColorB color;

	SAIRecorderObjectDebugContext() : bEnableDrawing(false), bSetView(false), color(255,255,255,255) { }
};

typedef DynArray<SAIRecorderObjectDebugContext> TDebugObjectsArray;

// Debug context information from the Editor tool
struct SAIRecorderDebugContext
{
	float fStartPos;
	float fEndPos;
	float fCursorPos;

	// List of objects to debug
	TDebugObjectsArray DebugObjects;

	SAIRecorderDebugContext() : fStartPos(0.0f), fEndPos(0.0f), fCursorPos(0.0f) { }
};

// Recorder mode options
enum EAIRecorderMode
{
	eAIRM_Off = 0,

	eAIRM_Memory,
	eAIRM_Disk,
};

UNIQUE_IFACE struct IAIRecorderListener
{

	virtual ~IAIRecorderListener(){}
	virtual void OnRecordingStart(EAIRecorderMode mode, const char *filename) {};
	virtual void OnRecordingStop(const char *filename) {};
	virtual void OnRecordingLoaded(const char *filename) {};
	virtual void OnRecordingSaved(const char *filename) {};

};

UNIQUE_IFACE struct IAIRecorder
{

	virtual ~IAIRecorder(){}
	virtual void Start(EAIRecorderMode mode, const char *filename = NULL) = 0;
	virtual void Stop(const char *filename = NULL) = 0;
	virtual void Reset(void) = 0;
	virtual bool Load(const char *filename = NULL) = 0;
	virtual bool Save(const char *filename = NULL) = 0;

	virtual bool AddListener(IAIRecorderListener *pListener) = 0;
	virtual bool RemoveListener(IAIRecorderListener *pListener) = 0;

};

class CStream;

struct IAIDebugRecord;

struct IAIRecordable
{
	struct RecorderEventData
	{
		const char* pString;
		Vec3	pos;
		float	val;
		void* pExtraData;

		RecorderEventData(const char *pStr) : pString(pStr), pos(ZERO), val(0), pExtraData(NULL) {}
		RecorderEventData(const Vec3& p) : pString(0), pos(p), val(0), pExtraData(NULL) {}
		RecorderEventData(float v) : pString(0), pos(ZERO), val(v), pExtraData(NULL) {}
	};

	enum e_AIDbgEvent{
		E_NONE,
		E_RESET,
		E_SIGNALRECIEVED,
		E_SIGNALRECIEVEDAUX,
		E_SIGNALEXECUTING,
		E_GOALPIPESELECTED,
		E_GOALPIPEINSERTED,
		E_GOALPIPERESETED,
		E_BEHAVIORSELECTED,
		E_BEHAVIORDESTRUCTOR,
		E_BEHAVIORCONSTRUCTOR,
		E_ATTENTIONTARGET,
		E_ATTENTIONTARGETPOS,
		E_REGISTERSTIMULUS,
		E_HANDLERNEVENT,
		E_ACTIONSTART,
		E_ACTIONSUSPEND,
		E_ACTIONRESUME,
		E_ACTIONEND,
		E_EVENT,
		E_REFPOINTPOS,
		E_AGENTPOS,
		E_AGENTDIR,
		E_LUACOMMENT,
		E_PERSONALLOG,
		E_HEALTH,
		E_HIT_DAMAGE,
		E_DEATH,
		E_SIGNALEXECUTEDWARNING,
		E_PRESSUREGRAPH,

		E_BOOKMARK,
		E_COUNT,
	};

	virtual ~IAIRecordable(){}
	virtual void	RecordEvent(e_AIDbgEvent event, const RecorderEventData* pEventData=NULL)=0;
	virtual void	RecordSnapshot()=0;
	virtual IAIDebugRecord* GetAIDebugRecord()=0;

};

struct IAIDebugStream
{

	virtual ~IAIDebugStream(){}
	virtual void	Seek(float whereTo)=0;
	virtual int		GetCurrentIdx()=0;	
	virtual int		GetSize()=0;
	virtual void*	GetCurrent(float &startingFrom)=0;
	virtual bool	GetCurrentString(string &sOut, float &startingFrom)=0;
	virtual void*	GetNext(float &startingFrom)=0;
	virtual float	GetStartTime()=0;
	virtual float	GetEndTime()=0;
	virtual bool	IsEmpty()=0;

	virtual char const* GetName() const=0;

};

UNIQUE_IFACE struct IAIDebugRecord
{

	virtual ~IAIDebugRecord(){}
	virtual IAIDebugStream* GetStream(IAIRecordable::e_AIDbgEvent streamTag)=0;

};


#endif //_IAIRECORDER_H_
