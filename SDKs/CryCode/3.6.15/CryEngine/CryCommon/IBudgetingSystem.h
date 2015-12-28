#include DEVIRTUALIZE_HEADER_FIX(IBudgetingSystem.h)

#ifndef _I_BUDGETING_SYSTEM_
#define _I_BUDGETING_SYSTEM_


UNIQUE_IFACE struct IBudgetingSystem
{

	// set budget
	virtual void SetSysMemLimit( int sysMemLimitInMB ) = 0;
	virtual void SetVideoMemLimit( int videoMemLimitInMB ) = 0;
	virtual void SetFrameTimeLimit( float frameLimitInMS ) = 0;
	virtual void SetSoundChannelsPlayingLimit( int soundChannelsPlayingLimit ) = 0;
	virtual void SetSoundMemLimit( int SoundMemLimit ) = 0;
	virtual void SetSoundCPULimit( int SoundCPULimit ) = 0;
	virtual void SetNumDrawCallsLimit( int numDrawCallsLimit ) = 0;
	virtual void SetStreamingThroughputLimit( float streamingThroughputLimit ) = 0;
	virtual void SetBudget( int sysMemLimitInMB, int videoMemLimitInMB, float frameTimeLimitInMS, int soundChannelsPlayingLimit, int SoundMemLimitInMB, int SoundCPULimit, int numDrawCallsLimit ) = 0;
	
	// get budget
	virtual int GetSysMemLimit() const = 0;
	virtual int GetVideoMemLimit() const = 0;
	virtual float GetFrameTimeLimit() const = 0;
	virtual int GetSoundChannelsPlayingLimit() const = 0;
	virtual int GetSoundMemLimit() const = 0;
	virtual int GetSoundCPULimit() const = 0;
	VIRTUAL int GetNumDrawCallsLimit() const = 0;
	VIRTUAL float GetStreamingThroughputLimit() const = 0;
	virtual void GetBudget( int& sysMemLimitInMB, int& videoMemLimitInMB, float& frameTimeLimitInMS, int& soundChannelsPlayingLimit, int& SoundMemLimitInMB, int& SoundCPULimitInPercent, int& numDrawCallsLimit ) const = 0;

	// Return estimated memory that level will be use on Xbox360 (running on PC)
	virtual void GetXbox360MemoryEstimation( int &systemMemInMB,int &videoMemTextureInMB,int &videoMemMeshInMB ) = 0;

	// monitoring
	virtual void MonitorBudget() = 0;
	virtual void Render(float x, float y) = 0;

	// destruction
	virtual void Release() = 0;


protected:
	virtual ~IBudgetingSystem() {}
};


#endif // #ifndef _I_BUDGETING_SYSTEM_