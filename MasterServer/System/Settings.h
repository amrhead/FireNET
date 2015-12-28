/*************************************************************************
Copyright (C), chernecoff@gmail.com, 2014-2015
---------------------------------------------------------------------------
History:

- 14.08.2014   14:07 : Created by AfroStalin(chernecoff)
- 07.03.2015   02:43 : Edited by AfroStalin(chernecoff)
-------------------------------------------------------------------------


*************************************************************************/

#ifndef _SETTING_
#define _SETTING_

class CSettings 
{
public:
	CSettings(void){}
	~CSettings(void){}

	const char* GetConfigValue(char* sectionName, char* valueName);
};

#endif