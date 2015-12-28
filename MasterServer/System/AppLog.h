/*************************************************************************
Copyright (C), chernecoff@gmail.com, 2014-2015
--------------------------------------------------------------------------
History:

- 14.08.2014   14:07 : Created by AfroStalin(chernecoff)
- 07.03.2015   02:43 : Edited by AfroStalin(chernecoff)
-------------------------------------------------------------------------


*************************************************************************/

#ifndef _APP_LOG_
#define _APP_LOG_

enum EMessageType
{
	LOG_INFO,
	LOG_WARNING,
	LOG_ERROR,
	LOG_DEBUG,
};

class CAppLog
{
public:
	CAppLog();
	~CAppLog(){}

	void Write(EMessageType type,char* Format, ...);

private:
	void CreateLogName();
	void AddText (HWND eWnd, COLORREF color,const char* section, const char* format);
private:
	char logName[256];
	bool first;
};

#endif