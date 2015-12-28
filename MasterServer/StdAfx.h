/*************************************************************************
"BeatGames" Source File.
Copyright (C), BeatGames, 2014-2015
-------------------------------------------------------------------------
History:

- 14.01.2015   00:19 : Created by AfroStalin(chernecoff)
- 10.03.2015   00:16 : Edited by AfroStalin(chernecoff)

-------------------------------------------------------------------------


*************************************************************************/
#define WIN32_LEAN_AND_MEAN 

#include <windows.h>
#include <string>
#include <map>
#include <vector>

#include <thread>
#include <mutex>

#include "System\Global.h"

inline void Log(EMessageType type, char* format,...)
{
	char textBuffer[2048];

	va_list args;
	va_start(args, format);
	vsprintf(textBuffer, format, args);
	va_end(args);

	gEnv->pLog->Write(type,textBuffer);
}