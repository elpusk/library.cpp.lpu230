#pragma once
#include <string>

class cdef
{
public:
#ifdef _WIN32
	static std::wstring get_log_folder_except_backslash()
	{
		static std::wstring s_log_folder_except_backslash = L".";
		return s_log_folder_except_backslash;
	}
#else
	static std::wstring get_log_folder_except_backslash()
	{
		static std::wstring s_log_folder_except_backslash = L".";
		return s_log_folder_except_backslash;
	}
#endif // _WIN32


public:
	~cdef()
	{
	}
private:
	cdef();
	cdef(const cdef&);
	cdef& operator=(const cdef&);
};