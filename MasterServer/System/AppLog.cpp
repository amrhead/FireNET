/*************************************************************************
Copyright (C), chernecoff@gmail.com, 2014-2015
--------------------------------------------------------------------------
History:

- 14.08.2014   14:07 : Created by AfroStalin(chernecoff)
- 12.03.2015   17:01 : Edited by AfroStalin(chernecoff)
-------------------------------------------------------------------------


*************************************************************************/
#include "StdAfx.h"
#include <time.h>
#include <Richedit.h>

#define White RGB(255,255,255)
#define Red RGB(255,0,0)
#define Yellow RGB(255,255,0)
#define Green RGB(0,255,0)

CAppLog::CAppLog()
{
	first = true;
	CreateLogName();
}

void CAppLog::CreateLogName()
{
	char LogTime[256];
	time_t seconds = time(NULL);
	tm* timeinfo = localtime(&seconds);
	char* LogNameFormat = "%d.%m.%Y %H-%M";
	strftime(LogTime, 256, LogNameFormat, timeinfo);
	sprintf(logName, "ServerLog\\MasterServer[%s].log", LogTime);
}

void CAppLog::AddText (HWND eWnd, COLORREF color,const char* section, const char* text) 
{
	CHARFORMAT cf;
	memset(&cf, 0, sizeof(CHARFORMAT));
	cf.crTextColor = color;
	cf.dwMask = CFM_COLOR;
	cf.cbSize = sizeof(cf);
	//
	HDC hdc = GetDC(eWnd);
	int ndx = GetWindowTextLength (eWnd);
	//
	char logText[2048];
	sprintf(logText,"[%s] %s\r\n",section,text);
	//
	
	SendMessage (eWnd, EM_SETSEL, (WPARAM)ndx, (LPARAM)ndx);
	//SendMessage(eWnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
	SendMessage(eWnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
	SendMessage (eWnd, EM_REPLACESEL, FALSE, reinterpret_cast<LPARAM>(logText)); 
	SendMessage(eWnd, EM_SCROLL, SB_LINEDOWN, 0);
}

void CAppLog::Write(EMessageType type, char* Format, ...)
{
	if(first)
		CreateDirectory("ServerLog",NULL);

	char buffer[256];
	char logText[1024];
	time_t seconds = time(NULL);
	tm* timeinfo = localtime(&seconds);
	char* format = "%d.%m.%Y <%H:%M:%S> ";

	strftime(buffer, 256, format, timeinfo);

	if (FILE *file = fopen(logName, first ? "w" : "a"))
	{
		first = false;

		if(!gEnv->bDebugMode && type == LOG_DEBUG)
		{
			fclose(file);
			return;
		}


		va_list args;

		va_start(args, buffer);  
		vfprintf(file, buffer, args); // System time and date
		va_end(args);


		switch (type)
		{
		case LOG_INFO:
			{		
				va_start(args, Format);

				vfprintf(file, Format, args);	
				vsprintf(logText, Format, args);
				AddText(gEnv->logBox, White, "Info", logText);

				va_end(args);
				break;
			}
		case LOG_WARNING:
			{
				va_start(args, "[WARNING] ");
				vfprintf(file, "[WARNING] ", args);
				va_end(args);

				va_start(args, Format);

				vfprintf(file, Format, args);
				vsprintf(logText, Format, args);
				AddText(gEnv->logBox, Yellow, "Warning", logText);

				va_end(args);
				break;
			}
		case LOG_ERROR:
			{
				va_start(args, "[ERROR] ");
				vfprintf(file, "[ERROR] ", args);
				va_end(args);

				va_start(args, Format);

				vfprintf(file, Format, args);
				vsprintf(logText, Format, args);	
				AddText(gEnv->logBox, Red, "Error", logText);

				va_end(args);			
				break;
			}
		case LOG_DEBUG:
			{
				va_start(args, "[DEBUG] ");
				vfprintf(file, "[DEBUG] ", args);
				va_end(args);

				va_start(args, Format);

				vfprintf(file, Format, args);	
				vsprintf(logText, Format, args);
				AddText(gEnv->logBox, Green, "DEBUG", logText);

				va_end(args);		
				break;
			}
		}

		//
		va_start(args, "\n");
		vfprintf(file, "\n", args);
		va_end(args);

		fclose(file);
	}
	else
		AddText(gEnv->logBox,Red,"Error","Error open log file!");
}