// stdafx.h : 자주 사용하지만 자주 변경되지는 않는
// 표준 시스템 포함 파일 및 프로젝트 관련 포함 파일이
// 들어 있는 포함 파일입니다.

#pragma once

#ifndef STRICT
#define STRICT
#endif

#include "targetver.h"

#define _ATL_APARTMENT_THREADED

#define _ATL_NO_AUTOMATIC_NAMESPACE

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// 일부 CString 생성자는 명시적으로 선언됩니다.


#define ATL_NO_ASSERT_ON_DESTROY_NONEXISTENT_WINDOW

#include "resource.h"
#include <atlbase.h>
#include <atlcom.h>
#include <atlctl.h>
#include <atlstr.h>


#undef	_YSS_ENABLE_STDOUT_FOR_DEBUG_

#if defined(_UNICODE) || defined(UNICODE)
	# define LOCALE std::wcerr.imbue(std::locale("")); \
		 std::wcin.imbue(std::locale("")); \
		 std::wclog.imbue(std::locale("")); \
		 std::wcout.imbue(std::locale(""));
	# define _terr		std::wcerr
	# define _tin		std::wcin
	# define _tlog		std::wclog
	# define _tout		std::wcout
	# define _tstring	std::wstring
#define	_tfilebuf		wfilebuf
#define	_tfstream		wfstream
#define	_tifstream		wifstream
#define	_tofstream	wofstream
#define	to_tstring		to_wstring

	# define PTCHAR		TCHAR*

#define	_tcout		wcout
#define	_tstringstream	wstringstream

#else
	# define _terr		std::cerr
	# define _tin		std::cin
	# define _tlog		std::clog
	# define _tout		std::cout
	# define _tstring	std::string
#define	_tfilebuf		filebuf
#define	_tfstream		fstream
#define	_tifstream		ifstream
#define	_tofstream	ofstream
#define	to_tstring		to_string

# define PTCHAR		char*

#define	_tcout		cout

#define	_tstringstream	stringstream

#endif

