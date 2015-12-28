////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2004.
// -------------------------------------------------------------------------
//  File name:   Linux32Specific.h
//  Version:     v1.00
//  Created:     05/03/2004 by MarcoK.
//  Compilers:   Visual Studio.NET, GCC 3.2
//  Description: Specific to Linux declarations, inline functions etc.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////
#ifndef _CRY_COMMON_LINUX_SPECIFIC_HDR_
#define _CRY_COMMON_LINUX_SPECIFIC_HDR_

#include <stdlib.h> 
#include <time.h>
#include <pthread.h>
#include MATH_H
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <algorithm>
#include <signal.h>
#ifndef __COUNTER__
#define __COUNTER__ __LINE__
#endif
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#if defined(APPLE)
#include <dirent.h>
#endif //defined(APPLE)
#include <arpa/inet.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <vector>
#include <string>

#ifdef __FUNC__
#undef __FUNC__
#endif
#if defined(__GNUC__) || defined(__clang__)
#define __FUNC__ __func__
#else
#define __FUNC__ 																			\
	({																									\
		static char __f[sizeof(__PRETTY_FUNCTION__) + 1];	\
		strcpy(__f, __PRETTY_FUNCTION__);									\
		char* __p = (char*)strchr(__f, '(');							\
		*__p = 0;																					\
		while (*(__p)!=' ' && __p!=(__f-1)) {--__p;}			\
		(__p + 1);																				\
	})
#endif

typedef void*								LPVOID;
#define VOID            		void
#define PVOID								void*

typedef unsigned int UINT;
typedef char CHAR;
typedef float FLOAT;

#define PHYSICS_EXPORTS
// disable livecreate on Linux
#define NO_LIVECREATE
// MSVC compiler-specific keywords
#define __forceinline inline
#define _inline inline
#define __cdecl
#define _cdecl
#define __stdcall
#define _stdcall
#define __fastcall
#define _fastcall
#define IN
#define OUT

//#if !defined(_LIB)
//# define _LIB 1
//#endif

#ifdef _LIB
#if !defined(USE_STATIC_NAME_TABLE)
#define USE_STATIC_NAME_TABLE 1
#endif
#endif

#if !defined(_STLP_HASH_MAP) && !defined(USING_STLPORT)
#define _STLP_HASH_MAP 1
#endif

// Enable memory address tracing code.
#if !defined(MM_TRACE_ADDRS) // && !defined(NDEBUG) 
#define MM_TRACE_ADDRS 1
#endif

#define _ALIGN(num) __attribute__ ((aligned(num)))	
#define _PACK __attribute__ ((packed))

// Safe memory freeing
#ifndef SAFE_DELETE
#define SAFE_DELETE(p)			{ if(p) { delete (p);		(p)=NULL; } }
#endif

#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p)	{ if(p) { delete[] (p);		(p)=NULL; } }
#endif

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)			{ if(p) { (p)->Release();	(p)=NULL; } }
#endif


#ifndef SAFE_RELEASE_FORCE
#define SAFE_RELEASE_FORCE(p)		{ if(p) { (p)->ReleaseForce();	(p)=NULL; } }
#endif

#define MAKEWORD(a, b)      ((WORD)(((BYTE)((DWORD_PTR)(a) & 0xff)) | ((WORD)((BYTE)((DWORD_PTR)(b) & 0xff))) << 8))
#define MAKELONG(a, b)      ((LONG)(((WORD)((DWORD_PTR)(a) & 0xffff)) | ((DWORD)((WORD)((DWORD_PTR)(b) & 0xffff))) << 16))
#define LOWORD(l)           ((WORD)((DWORD_PTR)(l) & 0xffff))
#define HIWORD(l)           ((WORD)((DWORD_PTR)(l) >> 16))
#define LOBYTE(w)           ((BYTE)((DWORD_PTR)(w) & 0xff))
#define HIBYTE(w)           ((BYTE)((DWORD_PTR)(w) >> 8))

#define CALLBACK
#define WINAPI

#ifndef __cplusplus
#ifndef _WCHAR_T_DEFINED
typedef unsigned short wchar_t;
#define TCHAR wchar_t;
#define _WCHAR_T_DEFINED
#endif
#endif
typedef wchar_t WCHAR;    // wc,   16-bit UNICODE character
typedef WCHAR *PWCHAR;
typedef WCHAR *LPWCH, *PWCH;
typedef const WCHAR *LPCWCH, *PCWCH;
typedef WCHAR *NWPSTR;
typedef WCHAR *LPWSTR, *PWSTR;
typedef WCHAR *LPUWSTR, *PUWSTR;

typedef const WCHAR *LPCWSTR, *PCWSTR;
typedef const WCHAR *LPCUWSTR, *PCUWSTR;

#define MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
            ((DWORD)(BYTE)(ch0) | ((DWORD)(BYTE)(ch1) << 8) |       \
            ((DWORD)(BYTE)(ch2) << 16) | ((DWORD)(BYTE)(ch3) << 24 ))
#define FILE_ATTRIBUTE_NORMAL               0x00000080

typedef int							BOOL;
typedef int32_t					LONG;
typedef uint32_t 		    ULONG;
typedef int 						HRESULT;


typedef int32 __int32;
typedef uint32 __uint32;
typedef int64 __int64;
#if !defined(__clang__)
typedef uint64 __uint64;
#endif 

#define THREADID_NULL -1
typedef unsigned long int threadID;

#define TRUE 1
#define FALSE 0

#ifndef MAX_PATH
	#define MAX_PATH 256
#endif
#ifndef _MAX_PATH
#define _MAX_PATH MAX_PATH
#endif

#define _PTRDIFF_T_DEFINED 1


//-------------------------------------socket stuff------------------------------------------
typedef int SOCKET;
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)
#define SD_SEND SHUT_WR
#define SD_BOTH SHUT_RDWR

typedef struct in_addr_windows 
{
	union 
	{
		struct { unsigned char s_b1,s_b2,s_b3,s_b4; } S_un_b;
		struct { unsigned short s_w1,s_w2; } S_un_w;
		unsigned int S_addr;
	} S_un;
}in_addr_windows;

#define WSAEINTR EINTR
#define WSAEBADF EBADF
#define WSAEACCES EACCES
#define WSAEFAULT EFAULT
#define WSAEACCES EACCES
#define WSAEFAULT EFAULT
#define WSAEINVAL EINVAL
#define WSAEMFILE EMFILE
#define WSAEWOULDBLOCK EAGAIN
#define WSAEINPROGRESS EINPROGRESS
#define WSAEALREADY EALREADY
#define WSAENOTSOCK ENOTSOCK 
#define WSAEDESTADDRREQ EDESTADDRREQ
#define WSAEMSGSIZE EMSGSIZE
#define WSAEPROTOTYPE EPROTOTYPE
#define WSAENOPROTOOPT ENOPROTOOPT
#define WSAEPROTONOSUPPORT EPROTONOSUPPORT
#define WSAESOCKTNOSUPPORT ESOCKTNOSUPPORT
#define WSAEOPNOTSUPP EOPNOTSUPP
#define WSAEPFNOSUPPORT EPFNOSUPPORT
#define WSAEAFNOSUPPORT EAFNOSUPPORT
#define WSAEADDRINUSE EADDRINUSE
#define WSAEADDRNOTAVAIL EADDRNOTAVAIL
#define WSAENETDOWN ENETDOWN
#define WSAENETUNREACH ENETUNREACH
#define WSAENETRESET ENETRESET
#define WSAECONNABORTED ECONNABORTED
#define WSAECONNRESET ECONNRESET
#define WSAENOBUFS ENOBUFS
#define WSAEISCONN EISCONN
#define WSAENOTCONN ENOTCONN
#define WSAESHUTDOWN ESHUTDOWN
#define WSAETOOMANYREFS ETOOMANYREFS
#define WSAETIMEDOUT ETIMEDOUT
#define WSAECONNREFUSED ECONNREFUSED
#define WSAELOOP ELOOP
#define WSAENAMETOOLONG ENAMETOOLONG
#define WSAEHOSTDOWN EHOSTDOWN
#define WSAEHOSTUNREACH EHOSTUNREACH
#define WSAENOTEMPTY ENOTEMPTY
#define WSAEPROCLIM EPROCLIM
#define WSAEUSERS EUSERS
#define WSAEDQUOT EDQUOT
#define WSAESTALE ESTALE
#define WSAEREMOTE EREMOTE

#define WSAHOST_NOT_FOUND (1024 + 1)
#define WSATRY_AGAIN (1024 + 2)
#define WSANO_RECOVERY (1024 + 3)
#define WSANO_DATA (1024 + 4)
#define WSANO_ADDRESS (WSANO_DATA)

//-------------------------------------end socket stuff------------------------------------------

//#define __TIMESTAMP__ __DATE__" "__TIME__

// function renaming
#define _finite __finite
#define _snprintf snprintf
#define _isnan isnan
#define stricmp strcasecmp
#define _stricmp strcasecmp
#define strnicmp strncasecmp
#define _strnicmp strncasecmp
#define wcsicmp wcscasecmp
#define wcsnicmp wcsncasecmp


#define _vsnprintf vsnprintf
#define _wtof( str ) wcstod( str, 0 )

/*static unsigned char toupper(unsigned char c)
{
  return c & ~0x40;
}
*/
typedef union _LARGE_INTEGER
{
  struct
	{
      DWORD LowPart;
      LONG HighPart;
  };
  struct
	{
      DWORD LowPart;
      LONG HighPart;
  } u;
#if defined(PS3)
	uint64 QuadPart;
#else
  long long QuadPart;
#endif
} LARGE_INTEGER;


// stdlib.h stuff
#define _MAX_DRIVE  3   // max. length of drive component
#define _MAX_DIR    256 // max. length of path component
#define _MAX_FNAME  256 // max. length of file name component
#define _MAX_EXT    256 // max. length of extension component

// fcntl.h
#define _O_RDONLY       0x0000  /* open for reading only */
#define _O_WRONLY       0x0001  /* open for writing only */
#define _O_RDWR         0x0002  /* open for reading and writing */
#define _O_APPEND       0x0008  /* writes done at eof */
#define _O_CREAT        0x0100  /* create and open file */
#define _O_TRUNC        0x0200  /* open and truncate */
#define _O_EXCL         0x0400  /* open only if file doesn't already exist */
#define _O_TEXT         0x4000  /* file mode is text (translated) */
#define _O_BINARY       0x8000  /* file mode is binary (untranslated) */
#define _O_RAW  _O_BINARY
#define _O_NOINHERIT    0x0080  /* child process doesn't inherit file */
#define _O_TEMPORARY    0x0040  /* temporary file bit */
#define _O_SHORT_LIVED  0x1000  /* temporary storage file, try not to flush */
#define _O_SEQUENTIAL   0x0020  /* file access is primarily sequential */
#define _O_RANDOM       0x0010  /* file access is primarily random */

// curses.h stubs for PDcurses keys
#define PADENTER	KEY_MAX + 1
#define CTL_HOME	KEY_MAX + 2
#define CTL_END		KEY_MAX + 3
#define CTL_PGDN	KEY_MAX + 4
#define CTL_PGUP	KEY_MAX + 5

// stubs for virtual keys, isn't used on Linux
#define VK_UP				0
#define VK_DOWN			0
#define VK_RIGHT		0
#define VK_LEFT			0
#define VK_CONTROL	0
#define VK_SCROLL		0

// io.h stuff
typedef unsigned int _fsize_t;

#define _flushall()

struct _OVERLAPPED;

typedef void (*LPOVERLAPPED_COMPLETION_ROUTINE)(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, struct _OVERLAPPED *lpOverlapped);

typedef struct _OVERLAPPED
{
    void* pCaller;//this is orginally reserved for internal purpose, we store the Caller pointer here
    LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine; ////this is orginally ULONG_PTR InternalHigh and reserved for internal purpose
    union {
        struct {
            DWORD Offset;
            DWORD OffsetHigh;
        };
        PVOID Pointer;
    };
		DWORD dwNumberOfBytesTransfered;	//additional member temporary speciying the number of bytes to be read
    /*HANDLE*/void*  hEvent;
} OVERLAPPED, *LPOVERLAPPED;

typedef struct _SECURITY_ATTRIBUTES 
{
    DWORD nLength;
    LPVOID lpSecurityDescriptor;
    BOOL bInheritHandle;
} SECURITY_ATTRIBUTES, *PSECURITY_ATTRIBUTES, *LPSECURITY_ATTRIBUTES;

#ifdef __cplusplus
	extern bool QueryPerformanceCounter(LARGE_INTEGER *);

	static pthread_mutex_t mutex_t;
	template<typename T>
	const volatile T InterlockedIncrement(volatile T* pT)
	{
		pthread_mutex_lock(&mutex_t);
		++(*pT);
		pthread_mutex_unlock(&mutex_t);
		return *pT;
	}

	template<typename T>
	const volatile T InterlockedDecrement(volatile T* pT)
	{
		pthread_mutex_lock(&mutex_t);
		--(*pT);
		pthread_mutex_unlock(&mutex_t);
		return *pT;
	}















	template<typename S, typename T>
	inline S __min(const S& rS, const T& rT)
	{
		return std::min(rS, rT);
	}

	template<typename S, typename T>
	inline S __max(const S& rS, const T& rT)
	{
		return std::max(rS, rT);
	}


	typedef enum {INVALID_HANDLE_VALUE = -1l}INVALID_HANDLE_VALUE_ENUM;
	//for compatibility reason we got to create a class which actually contains an int rather than a void* and make sure it does not get mistreated
	template <class T, T U>//U is default type for invalid handle value, T the encapsulated handle type to be used instead of void* (as under windows and never linux)
	class CHandle
	{
	public:
		typedef T			HandleType;
		typedef void* PointerType;	//for compatibility reason to encapsulate a void* as an int

		static const HandleType sciInvalidHandleValue = U;

		CHandle(const CHandle<T,U>& cHandle) : m_Value(cHandle.m_Value){}
		CHandle(const HandleType cHandle = U) : m_Value(cHandle){}
		CHandle(const PointerType cpHandle) : m_Value(reinterpret_cast<HandleType>(cpHandle)){}
		CHandle(INVALID_HANDLE_VALUE_ENUM) : m_Value(U){}//to be able to use a common value for all InvalidHandle - types
#if defined(LINUX64) && !defined(__clang__)
		//treat __null tyope also as invalid handle type
		CHandle(typeof(__null)) : m_Value(U){}//to be able to use a common value for all InvalidHandle - types
#endif
		operator HandleType(){return m_Value;}
		bool operator!() const{return m_Value == sciInvalidHandleValue;}
		const CHandle& operator =(const CHandle& crHandle){m_Value = crHandle.m_Value;return *this;}
		const CHandle& operator =(const PointerType cpHandle){m_Value = (HandleType)reinterpret_cast<UINT_PTR>(cpHandle);return *this;}
		const bool operator ==(const CHandle& crHandle)		const{return m_Value == crHandle.m_Value;}
		const bool operator ==(const HandleType cHandle)	const{return m_Value == cHandle;}
		const bool operator ==(const PointerType cpHandle)const{return m_Value == (HandleType)reinterpret_cast<UINT_PTR>(cpHandle);}
		const bool operator !=(const HandleType cHandle)	const{return m_Value != cHandle;}
		const bool operator !=(const CHandle& crHandle)		const{return m_Value != crHandle.m_Value;}
		const bool operator !=(const PointerType cpHandle)const{return m_Value != (HandleType)reinterpret_cast<UINT_PTR>(cpHandle);}
		const bool operator <	(const CHandle& crHandle)		const{return m_Value < crHandle.m_Value;}
		HandleType Handle()const{return m_Value;}

	private:
		HandleType m_Value;	//the actual value, remember that file descriptors are ints under linux

		typedef void	ReferenceType;//for compatibility reason to encapsulate a void* as an int
		//forbid these function which would actually not work on an int
		PointerType operator->();
    PointerType operator->() const;
    ReferenceType operator*();
    ReferenceType operator*() const;
		operator PointerType();
	};

	typedef CHandle<int, (int)-1l> HANDLE;

	typedef HANDLE EVENT_HANDLE;
	typedef pid_t THREAD_HANDLE;

	inline int64 CryGetTicks()
	{
		LARGE_INTEGER counter;
		QueryPerformanceCounter(&counter);
		return counter.QuadPart;
	}

#endif //__cplusplus

inline int _CrtCheckMemory() { return 1; };

inline char *_fullpath(char *absPath, const char *relPath, size_t maxLength)
{
  char path[PATH_MAX];

  if (realpath(relPath, path) == NULL)
    return NULL;
  strncpy(absPath, path, maxLength - 1);
  absPath[maxLength - 1] = 0;
  return absPath;
}

typedef void *HGLRC;
typedef void *HDC;
typedef void *PROC;
typedef void *PIXELFORMATDESCRIPTOR;

#define SCOPED_ENABLE_FLOAT_EXCEPTIONS

// Linux_Win32Wrapper.h now included directly by platform.h
//#include "Linux_Win32Wrapper.h"

#define closesocket close
inline int WSAGetLastError()
{
    return errno;
}
     

#endif //_CRY_COMMON_LINUX_SPECIFIC_HDR_

// vim:ts=2:sw=2:tw=78

