/*************************************************************************
Copyright (C), chernecoff@gmail.com, 2014-2015
--------------------------------------------------------------------------
History:

- 29.08.2014   20:02 : Created by AfroStalin(chernecoff)
- 08.03.2015   16:37 : Edited by AfroStalin(chernecoff)
-------------------------------------------------------------------------


*************************************************************************/

#ifndef _CONSOLE_
#define _CONSOLE_

class CConsoleCommands 
{
public:
	CConsoleCommands(void){}
	~CConsoleCommands(void){}	

	void Read(const char* command);

private:
	void List();
	void Status();
	void SendTo();
	void SendAll();
	void ShowPlayers();
	void ShowServers();
	void SendRequest();
	void Quit();

	// Only for testing
	void Test();
};

#endif