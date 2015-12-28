#pragma once

struct Timer
{
	Timer()
		: endTime(-1.0f)
	{
	}

	void Reset( float duration, float variation = 0.0f )
	{
		endTime = gEnv->pSystem->GetITimer()->GetFrameStartTime() + CTimeValue(duration) + CTimeValue(cry_frand() * variation);
	}

	bool Elapsed() const
	{
		return endTime >= 0.0f && gEnv->pSystem->GetITimer()->GetFrameStartTime() >= endTime;
	}

	float GetSecondsLeft() const
	{
		return (endTime - gEnv->pSystem->GetITimer()->GetFrameStartTime()).GetSeconds();
	}

	CTimeValue endTime;
};