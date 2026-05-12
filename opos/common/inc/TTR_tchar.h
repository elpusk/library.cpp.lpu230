
#if !defined(__TTRL_TCHAR_HEADER_20091116__)
#define __TTRL_TCHAR_HEADER_20091116__

///////////////////////////////////////////////
//TTR unicode converting
//////////////////////////////////////////////	
#include "TTR_def.h"
#include <iostream>
#include <string>


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

	#define PTCHAR		TCHAR*
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

	#define PTCHAR		char*
	#define _tstrtol		strtol
#endif


/////////////////////////////////////////

#endif	//__TTRL_TCHAR_HEADER_20091116__
