#include "StdAfx.h"

#if defined(CRY_ENABLE_RC_HELPER)

#include "SettingsManagerHelpers.h"
#include "EngineSettingsManager.h"

#include <windows.h>		
#include <shellapi.h> //ShellExecuteW()

bool SettingsManagerHelpers::Utf16ContainsAsciiOnly(const wchar_t* wstr)
{
	while (*wstr)
	{
		if (*wstr > 127 || *wstr < 0)
		{
			return false;
		}
		++wstr;
	}
	return true;
}


void SettingsManagerHelpers::ConvertUtf16ToUtf8(const wchar_t* src, CCharBuffer dst)
{
	if (dst.getSizeInElements() <= 0)
	{
		return;
	}

	if (src[0] == 0)
	{
		dst[0] = 0;
	}
	else
	{
		const int srclen = int(wcslen(src));
		const int dstsize = int(dst.getSizeInBytes());
		const int byteCount = WideCharToMultiByte(
			CP_UTF8, 
			0, 
			src,
			srclen,
			dst.getPtr(), // output buffer
			dstsize - 1,  // size of the output buffer in bytes
			NULL,    
			NULL);
		if (byteCount <= 0 || byteCount >= dstsize)
		{
			dst[0] = 0;
		}
		else
		{
			dst[byteCount] = 0;
		}
	}
}


void SettingsManagerHelpers::ConvertUtf8ToUtf16(const char* src, CWCharBuffer dst)
{
	if (dst.getSizeInElements() <= 0)
	{
		return;
	}

	if (src[0] == 0)
	{
		dst[0] = 0;
	}
	else
	{
		const int srclen = int(strlen(src));
		const int dstsize = int(dst.getSizeInElements());
		const int charCount = MultiByteToWideChar(
			CP_UTF8, 
			0, 
			src,
			srclen,
			dst.getPtr(), // output buffer
			dstsize - 1);  // size of the output buffer in characters
		if (charCount <= 0 || charCount >= dstsize)
		{
			dst[0] = 0;
		}
		else
		{
			dst[charCount] = 0;
		}
	}
}


void SettingsManagerHelpers::GetAsciiFilename(const wchar_t* wfilename, CCharBuffer buffer)
{
	if (buffer.getSizeInElements() <= 0)
	{
		return;
	}

	if (wfilename[0] == 0)
	{
		buffer[0] = 0;
		return;
	}

	if (Utf16ContainsAsciiOnly(wfilename))
	{
		ConvertUtf16ToUtf8(wfilename, buffer);
		return;
	}

	// The path is non-ASCII unicode, so let's resort to short filenames (they are always ASCII-only, I hope)
	wchar_t shortW[MAX_PATH];
	const int bufferCharCount = sizeof(shortW) / sizeof(shortW[0]);
	const int charCount = GetShortPathNameW(wfilename, shortW, bufferCharCount);
	if (charCount <= 0 || charCount >= bufferCharCount)
	{
		buffer[0] = 0;
		return;
	}

	shortW[charCount] = 0;
	if (!Utf16ContainsAsciiOnly(shortW))
	{
		buffer[0] = 0;
		return;
	}

	ConvertUtf16ToUtf8(shortW, buffer);
}


//////////////////////////////////////////////////////////////////////////
CSettingsManagerTools::CSettingsManagerTools(const wchar_t* szModuleName)
{
	m_pSettingsManager = new CEngineSettingsManager(szModuleName);
}

//////////////////////////////////////////////////////////////////////////
CSettingsManagerTools::~CSettingsManagerTools()
{
	delete m_pSettingsManager;
}

//////////////////////////////////////////////////////////////////////////
void CSettingsManagerTools::GetRootPathUtf16(bool pullFromRegistry, SettingsManagerHelpers::CWCharBuffer wbuffer)
{
	if (pullFromRegistry)
	{
		m_pSettingsManager->GetRootPathUtf16(wbuffer);
	}
	else
	{
		m_pSettingsManager->GetValueByRef("ENG_RootPath", wbuffer);
	}
}


void CSettingsManagerTools::GetRootPathAscii(bool pullFromRegistry, SettingsManagerHelpers::CCharBuffer buffer)
{
	wchar_t wbuffer[MAX_PATH];

	GetRootPathUtf16(pullFromRegistry, SettingsManagerHelpers::CWCharBuffer(wbuffer, sizeof(wbuffer)));

	SettingsManagerHelpers::GetAsciiFilename(wbuffer, buffer);
}


//////////////////////////////////////////////////////////////////////////
bool CSettingsManagerTools::GetInstalledBuildPathUtf16(const int index, SettingsManagerHelpers::CWCharBuffer name, SettingsManagerHelpers::CWCharBuffer path)
{
	return m_pSettingsManager->GetInstalledBuildRootPathUtf16(index, name, path);
}


bool CSettingsManagerTools::GetInstalledBuildPathAscii(const int index, SettingsManagerHelpers::CCharBuffer name, SettingsManagerHelpers::CCharBuffer path)
{
	wchar_t wName[MAX_PATH];
	wchar_t wPath[MAX_PATH];
	if( GetInstalledBuildPathUtf16(index, SettingsManagerHelpers::CWCharBuffer(wName, sizeof(wName)), SettingsManagerHelpers::CWCharBuffer(wPath, sizeof(wPath))) )
	{
		SettingsManagerHelpers::GetAsciiFilename(wName, name);
		SettingsManagerHelpers::GetAsciiFilename(wPath, path);
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
void CSettingsManagerTools::CallSettingsManagerExe( void* hParent )
{
	m_pSettingsManager->CallSettingsDialog(hParent);
}


//////////////////////////////////////////////////////////////////////////
static bool FileExists(const wchar_t* filename)
{
	const DWORD dwAttrib = GetFileAttributesW(filename);
	return dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
}


//////////////////////////////////////////////////////////////////////////
void CSettingsManagerTools::GetEditorExecutable(SettingsManagerHelpers::CWCharBuffer wbuffer)
{
	if (wbuffer.getSizeInElements() <= 0)
	{
		return;
	}

	m_pSettingsManager->GetRootPathUtf16(wbuffer);

	SettingsManagerHelpers::CFixedString<wchar_t, 1024> editorExe;
	editorExe = wbuffer.getPtr();

	if (editorExe.length() <= 0)
	{
		MessageBoxA(NULL, "Can't Find the Editor.\nPlease, setup correct CryENGINE root path in the engine settings dialog", "Error", MB_ICONERROR | MB_OK);
		CallSettingsManagerExe(0);
		m_pSettingsManager->GetRootPathUtf16(wbuffer);
		editorExe = wbuffer.getPtr();

		if (editorExe.length() <= 0)
		{
			wbuffer[0] = 0;
			return;
		}
	}

	editorExe.appendAscii("/Bin64/Editor.exe");
	const bool bFound = FileExists(editorExe.c_str());

	const size_t sizeToCopy = (editorExe.length() + 1) * sizeof(wbuffer[0]);
	if (!bFound || sizeToCopy > wbuffer.getSizeInBytes())
	{
		wbuffer[0] = 0;
	}
	else
	{
		memcpy(wbuffer.getPtr(), editorExe.c_str(), sizeToCopy);
	}
}


//////////////////////////////////////////////////////////////////////////
bool CSettingsManagerTools::CallEditor(void** pEditorWindow, void* hParent, const char* pWindowName, const char* pFlag)
{
	HWND window = ::FindWindowA(NULL, pWindowName);
	if (window)
	{
		*pEditorWindow = window;
		return true;
	}
	else
	{
		*pEditorWindow = 0;

		wchar_t buffer[512] = { L'\0' };
		GetEditorExecutable(SettingsManagerHelpers::CWCharBuffer(buffer, sizeof(buffer)));

		SettingsManagerHelpers::CFixedString<wchar_t, 256> wFlags;
		SettingsManagerHelpers::ConvertUtf8ToUtf16(pFlag, wFlags.getBuffer());
		wFlags.setLength(wcslen(wFlags.c_str()));

		if (buffer[0] != '\0')		
		{
			INT_PTR hIns = (INT_PTR)ShellExecuteW(NULL, L"open", buffer, wFlags.c_str(), NULL, SW_SHOWNORMAL);
			if(hIns > 32)
			{
				return true;
			}
			else
			{
				MessageBoxA(0,"Editor.exe was not found.\n\nPlease verify CryENGINE root path.","Error",MB_ICONERROR|MB_OK);
				CallSettingsManagerExe(hParent);
				GetEditorExecutable(SettingsManagerHelpers::CWCharBuffer(buffer, sizeof(buffer)));
				ShellExecuteW(NULL, L"open", buffer, wFlags.c_str(), NULL, SW_SHOWNORMAL);
			}
		}
	}

	return false;
}

#endif // #if defined(CRY_ENABLE_RC_HELPER)

// eof
