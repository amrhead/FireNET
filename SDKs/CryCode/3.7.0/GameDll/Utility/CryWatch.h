/*************************************************************************
Crytek Source File.
Copyright (C), Crytek Studios, 2001-2004.
-------------------------------------------------------------------------
CryWatch.cpp

Description: 
- basic onscreen watch
- in game.dll till it matures and can be moved into the engine

-------------------------------------------------------------------------
History:
-	[03/08/2009] : Created by James Bamford

*************************************************************************/

#ifndef __CRYWATCH_H__
#define __CRYWATCH_H__

#if !defined(_RELEASE)
#define CRY_WATCH_ENABLED			 (1)
#else
#define CRY_WATCH_ENABLED			 (0)
#endif

#define CryWatch(...) CryWatchFunc(string().Format(__VA_ARGS__))
#define CryWatchLog(...) CryWatchLogFunc(string().Format(__VA_ARGS__))

#if CRY_WATCH_ENABLED

SC_API int CryWatchFunc(const char * message);
SC_API int CryWatchLogFunc(const char * message);
void CryWatch3DAdd(const char * text, const Vec3 & posIn, float lifetime = 2.f, const Vec3 * velocity = NULL, float gravity = 3.f);
void CryWatch3DReset();
void CryWatch3DTick(float dt);

#else

#define CryWatchFunc(message)          (0)
#define CryWatchLogFunc(message)          (0)
#define CryWatch3DAdd(...)             ((void)0)
#define CryWatch3DReset()              ((void)0)
#define CryWatch3DTick(dt)             ((void)0)

#endif // CRY_WATCH_ENABLED

#endif // __CRYWATCH_H__