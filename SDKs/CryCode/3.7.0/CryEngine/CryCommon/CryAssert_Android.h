/*************************************************************************
Crytek Source File.
Copyright (C), Crytek Studios, 2001-2005.
-------------------------------------------------------------------------
$Id$
$DateTime$
Description:
    Assert dialog box for android.

-------------------------------------------------------------------------
History:
*************************************************************************/

#if !defined(__CRYASSERT_ANDROID_H__) && defined(USE_CRY_ASSERT) && defined(ANDROID)
#define __CRYASSERT_ANDROID_H__

static char gs_szMessage[MAX_PATH];

void CryAssertTrace(const char * szFormat,...)
{
    if (gEnv == 0)
    {
        return;
    }

    if (!gEnv->bIgnoreAllAsserts || gEnv->bTesting)
    {
        if(szFormat == NULL)
        {
            gs_szMessage[0] = '\0';
        }
        else
        {
            va_list args;
            va_start(args,szFormat);
            vsnprintf(gs_szMessage, sizeof(gs_szMessage), szFormat, args);
            va_end(args);
        }
    }
}

bool CryAssert(const char* szCondition, const char* szFile, unsigned int line, bool *pIgnore)
{
    if (!gEnv)
		return true;

    gEnv->pSystem->OnAssert(szCondition, gs_szMessage, szFile, line);

    if (!gEnv->bNoAssertDialog && !gEnv->bIgnoreAllAsserts)
    {
		return true;
    }
	else
	{
		return false;
	}

}

#endif // __CRYASSERT_ANDROID_H__
