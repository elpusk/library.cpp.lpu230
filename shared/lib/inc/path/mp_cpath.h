#pragma once
#include <string>

#include <mp_cfile.h>

namespace _mp {
	//the defined path
	class cpath
	{
	public:
		static void replace_backslashes_by_os(std::wstring& str) 
		{
#ifndef _WIN32
			std::replace(str.begin(), str.end(), L'\\', L'/');
#endif
		}

		static void replace_backslashes_by_os(std::string& str) 
		{
#ifndef _WIN32
			std::replace(str.begin(), str.end(), '\\', '/');
#endif
		}

		virtual ~cpath() {}
	private:
		cpath() = delete;
		cpath(const cpath&) = delete;
		cpath& operator=(const cpath&) = delete;
	};

}