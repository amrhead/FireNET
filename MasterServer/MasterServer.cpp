/*************************************************************************
Copyright (C), chernecoff@gmail.com, 2014-2015
-------------------------------------------------------------------------
History:

- 14.08.2014   14:07 : Created by AfroStalin(chernecoff)
- 20.06.2015   14:55 : Edited by AfroStalin(chernecoff)
-------------------------------------------------------------------------

*************************************************************************/
#include "StdAfx.h"
#include "resource.h"
#include <tchar.h>
#include <richedit.h>
#include "..\versions.h"

static TCHAR szWindowClass[] = _T("FireNET - MasterServer");

#if defined (WIN_64)
static TCHAR szTitle[] = _T("FireNET - MasterServer[x64]");
#endif
#if defined (WIN_32)
static TCHAR szTitle[] = _T("FireNET - MasterServer[x32]");
#endif




HINSTANCE hInst;
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int MasterServerInit()
{
	gEnv->startTime = clock();
	gEnv->maxPlayers = atoi(gEnv->pSettings->GetConfigValue("Server","max_players"));
	gEnv->maxGameServers = atoi(gEnv->pSettings->GetConfigValue("Server","max_gameservers"));
	gEnv->bUseXml = !!atoi(gEnv->pSettings->GetConfigValue("Server","use_xml"));
	gEnv->serverVersion = PACKET_VERSION;



	Log(LOG_INFO,"FireNET - MasterServer v.%s started!", gEnv->serverVersion);
	Log(LOG_INFO,"Copyright (C), chernecoff@gmail.com, 2014-2015");
	if(gEnv->bDebugMode)
		Log(LOG_WARNING,"************ THIS'S DEBUG MODE ************");


	if(gEnv->bUseXml)
	{
		Log(LOG_INFO,"Using XML instead of MySql...");
		gEnv->pXml->Init();
		gEnv->pServer->Start();
	}
	else
	{
		if(gEnv->pMySql->MySqlConnect())
			gEnv->pServer->Start();
		else
		{
			Log(LOG_ERROR,"MySql connection failed!!!");
			MessageBox(NULL, _T("MySql connection failed!!!"), szWindowClass, NULL);
		}
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	WNDCLASSEX wcex;

	gEnv->Init();


	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style          = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc    = WndProc;
	wcex.cbClsExtra     = 0;
	wcex.cbWndExtra     = 0;
	wcex.hInstance      = hInstance;
	wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground  = (HBRUSH)(2);
	wcex.lpszMenuName   = NULL;
	wcex.lpszClassName  = szWindowClass;
	wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ICON1));

	if (!RegisterClassEx(&wcex))
	{
		MessageBox(NULL,_T("Call to RegisterClassEx failed!"),szWindowClass,NULL);
		return 1;
	}

	hInst = hInstance;

	HWND hWnd = CreateWindow(szWindowClass, szTitle,  WS_VISIBLE| WS_SYSMENU | WS_MINIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT,750, 465,NULL, NULL, hInstance, NULL);

	if (!hWnd)
	{
		MessageBox(NULL, _T("Call to CreateWindow failed!"), szWindowClass, NULL);
		return 1;
	}

	ShowWindow(hWnd,nCmdShow);
	UpdateWindow(hWnd);

	// Init server
	std::thread serverInit(MasterServerInit);
	serverInit.detach();

	// Main message loop:
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		switch (msg.message)
		{
		case WM_KEYDOWN:
			{
				if(VK_RETURN == msg.wParam)
				{
					char buff[256];

					GetWindowText(gEnv->consoleBox,buff,256);

					gEnv->pConsole->Read(buff);

					SetWindowText(gEnv->consoleBox, "");
				}
				break;
			}
		default:
			break;
		}
		

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int) msg.wParam;
}

HWND CreateRichEdit(HWND hWndOwner, int x, int y, int width, int height)
{
    HINSTANCE hndl = LoadLibrary("riched32.dll");

	HWND hWndEdit = CreateWindowEx(WS_EX_LEFT , RICHEDIT_CLASS, "",WS_VSCROLL | WS_VISIBLE | WS_CHILD | ES_READONLY | ES_LEFT| ES_AUTOVSCROLL | ES_MULTILINE, x, y, width, height, hWndOwner, (HMENU)1, GetModuleHandle(NULL), NULL);

	SendMessage (hWndEdit, EM_SETBKGNDCOLOR, NULL, RGB(0,0,0));

	EnableScrollBar(hWndEdit,SB_VERT,true);
	ShowScrollBar(hWndEdit,SB_VERT,true);

    return hWndEdit;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_CREATE:
		{
			std::mutex g_lock;
			g_lock.lock();

			gEnv->logBox = CreateRichEdit(hWnd,0,0,735,385);

			gEnv->statusBox = CreateWindow(TEXT("EDIT"),TEXT(""), WS_VISIBLE | WS_DISABLED | WS_CHILD | ES_CENTER,0,387,750,20,hWnd,(HMENU)1,NULL,NULL);

			gEnv->consoleBox = CreateWindow(TEXT("EDIT"),TEXT(""),WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,0,407,750,20,hWnd,(HMENU)1,NULL,NULL);

			g_lock.unlock();
			break;
		}

	case WM_PAINT:
		{
			hdc = BeginPaint(hWnd, &ps);
			EndPaint(hWnd, &ps);
			break;
		}

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_CTLCOLORSTATIC:
		SetBkColor((HDC)wParam, RGB(0,0,0));
		SetTextColor((HDC)wParam, RGB(255,255,255));
		return  (LRESULT)GetStockObject(BLACK_BRUSH);
		break;

	case WM_CTLCOLOREDIT:
		SetBkColor((HDC)wParam, RGB(0,0,0));
		SetTextColor((HDC)wParam, RGB(255,255,255));
		return  (LRESULT)GetStockObject(BLACK_BRUSH);
		break;

	case WM_CLOSE:
		if(MessageBox(hWnd,"You realy want close server???",szWindowClass,MB_OKCANCEL)==IDOK)
		{
			DestroyWindow(hWnd);
			break;
		}
		else return 0;	

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
		break;
	}

	return 0;
}
