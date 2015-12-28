/*************************************************************************
"BeatGames" Source File.
Copyright (C), BeatGames, 2014.
-------------------------------------------------------------------------

-------------------------------------------------------------------------
History:

- 14.10.2014   23:07 : Created by AfroStalin(chernecoff)
- 06.02.2015   22:19 : Edited by AfroStalin(chernecoff)
-------------------------------------------------------------------------

*************************************************************************/

#include "StdAfx.h"
#include "ping.h"
#pragma hdrstop
#include <windows.h>
#include <winsock.h>
#include <stdio.h>
#include <conio.h>

typedef struct tagIPINFO
{
        u_char Ttl;
        u_char Tos; 
        u_char IPFlags;       
        u_char OptSize;      
        u_char FAR *Options;  
}IPINFO, *PIPINFO;
 
typedef struct tagICMPECHO
{
        u_long Source;           
        u_long Status;               
        u_long RTTime;                 
        u_short DataSize;          
        u_short Reserved;            
        void FAR *pData;           
        IPINFO  ipInfo;              
}ICMPECHO, *PICMPECHO;
 
 

HANDLE (WINAPI *pIcmpCreateFile)(VOID);
BOOL (WINAPI *pIcmpCloseHandle)(HANDLE);
DWORD (WINAPI *pIcmpSendEcho)
	(HANDLE,DWORD,LPVOID,WORD,PIPINFO,LPVOID,DWORD,DWORD);

//
WSADATA wsaData;                        
ICMPECHO icmpEcho;                    
HMODULE hndlIcmp;                      
HANDLE hndlFile;                     
struct in_addr iaDest;              
DWORD dwAddress;                  
IPINFO ipInfo;                      
int nRet;       
DWORD dwRet;  


int CPing::Init()
{                     
	hndlIcmp = LoadLibrary("ICMP.DLL");
	if (hndlIcmp == NULL)
	{
		CryLogAlways("Could not load ICMP.DLL");
		return -1;
	}

	pIcmpCreateFile = (HANDLE (WINAPI *)(void))
		GetProcAddress(hndlIcmp,"IcmpCreateFile");
	pIcmpCloseHandle = (BOOL (WINAPI *)(HANDLE))
		GetProcAddress(hndlIcmp,"IcmpCloseHandle");
	pIcmpSendEcho = (DWORD (WINAPI *)
		(HANDLE,DWORD,LPVOID,WORD,PIPINFO,LPVOID,DWORD,DWORD))
		GetProcAddress(hndlIcmp,"IcmpSendEcho");

	if (pIcmpCreateFile == NULL             || 
		pIcmpCloseHandle == NULL        ||
		pIcmpSendEcho == NULL)
	{
		CryLogAlways("Error getting ICMP proc address");
		FreeLibrary(hndlIcmp);
		return -1;
	}

	nRet = WSAStartup(0x0101, &wsaData );
	if (nRet)
	{
		CryLogAlways("WSAStartup() error: %d", nRet);
		WSACleanup();
		FreeLibrary(hndlIcmp);
		return -1;
	}

	if (0x0101 != wsaData.wVersion)
	{
		CryLogAlways("WinSock version 1.1 not supported");
		WSACleanup();
		FreeLibrary(hndlIcmp);
		return -1;
	}

	return 0;
}

int CPing::Ping(string ip)
{
	iaDest.s_addr = inet_addr(ip);
	dwAddress = (DWORD )(iaDest.s_addr);
     
	hndlFile = pIcmpCreateFile();

	ipInfo.Ttl = 255;
	ipInfo.Tos = 0;
	ipInfo.IPFlags = 0;
	ipInfo.OptSize = 0;
	ipInfo.Options = NULL;

	dwRet = pIcmpSendEcho(hndlFile,dwAddress,NULL,0,&ipInfo,&icmpEcho,sizeof(struct tagICMPECHO),1000);                 

	pIcmpCloseHandle(hndlFile);
	//FreeLibrary(hndlIcmp);
	//WSACleanup();
	//getch();

	return icmpEcho.RTTime;
}