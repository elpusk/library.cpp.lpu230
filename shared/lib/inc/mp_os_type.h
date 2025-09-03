#pragma once

#include <cstdint>

/////////////////////////////////////////////////////
// warning - This file cannot be used with mp_type.h.
// definition of type, for supporting, multi OS.  
#ifdef _WIN32
#include <Windows.h>
#define	_CALLTYPE_	__stdcall

#else // linux

#include <dlfcn.h>

#define _PACK_BYTE  __attribute__((packed))
#define	_CALLTYPE_	__attribute__((__stdcall__))

#define PtrToUlong(_a)      ((uint32_t)(_a))
#define INVALID_HANDLE_VALUE    ((uint32_t)-1)
#define FALSE   0
#define TRUE    1

typedef uint32_t HWND;
typedef uint32_t UINT;
typedef uint32_t HANDLE;
typedef uint32_t BOOL;

typedef	void* HMODULE;


#endif //_WIN32
