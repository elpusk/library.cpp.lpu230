#pragma once

#include <iostream>
#include <sstream>
#include <fstream>
#include <limits>
#include <time.h>
#include <stdio.h>
#include <locale>
#include <codecvt>
#include <string>
#include <algorithm>
#include <boost/beast/core/error.hpp>

#ifdef _WIN32

#else
#include <unistd.h>
#include <stdarg.h>
#endif

#include <mp_type.h>

namespace _mp{
    class cstring {
	public:
		static void to_upper(std::wstring& s_out, const std::wstring& s_in)
		{
			s_out = s_in;
			std::transform(std::begin(s_in), std::end(s_in), std::begin(s_out), towupper);
		}
		static void to_upper(std::wstring& s_in_out)
		{
			std::transform(std::begin(s_in_out), std::end(s_in_out), std::begin(s_in_out), towupper);
		}
		static void to_lower(std::wstring& s_out, const std::wstring& s_in)
		{
			s_out = s_in;
			std::transform(std::begin(s_in), std::end(s_in), std::begin(s_out), towlower);
		}
		static void to_lower(std::wstring& s_in_out)
		{
			std::transform(std::begin(s_in_out), std::end(s_in_out), std::begin(s_in_out), towlower);
		}

		static void replace_all(std::wstring& s_in_out, const std::wstring& s_from, const std::wstring& s_to)
		{
			size_t n_pos = 0;
			while ((n_pos = s_in_out.find(s_from, n_pos)) != std::wstring::npos) {
				s_in_out.replace(n_pos, s_from.length(), s_to);
				n_pos += s_to.length();
			}//end while.
		}

		static void remove_white_space_of_prepostfix(std::wstring& s_out, const std::wstring& s_in, bool b_prefix = true, bool b_postfix = true)
		{
			do {
				s_out.clear();

				if (s_in.empty())
					continue;

				std::wstring::size_type n_found(std::wstring::npos);

				if (b_prefix) {
					n_found = s_in.find_first_not_of(L" \t\f\v\n\r");
					if (n_found != std::wstring::npos) {
						//remove prefix
						s_out = s_in.substr(n_found);
					}
					else
						s_out = s_in;
				}
				else {
					s_out = s_in;
				}
				//
				if (b_postfix) {
					n_found = s_out.find_last_not_of(L" \t\f\v\n\r");
					if (n_found != std::wstring::npos)
						s_out.erase(n_found + 1);
				}
			} while (false);
		}

		static void remove_white_space_of_prepostfix(std::string& s_out, const std::string& s_in, bool b_prefix = true, bool b_postfix = true)
		{
			do {
				s_out.clear();

				if (s_in.empty())
					continue;

				std::string::size_type n_found(std::string::npos);

				if (b_prefix) {
					n_found = s_in.find_first_not_of(" \t\f\v\n\r");
					if (n_found != std::string::npos) {
						//remove prefix
						s_out = s_in.substr(n_found);
					}
					else
						s_out = s_in;
				}
				else {
					s_out = s_in;
				}
				//
				if (b_postfix) {
					n_found = s_out.find_last_not_of(" \t\f\v\n\r");
					if (n_found != std::string::npos)
						s_out.erase(n_found + 1);
				}
			} while (false);
		}

		static void remove_white_space_of_prepostfix(std::wstring& s_in_out, bool b_prefix = true, bool b_postfix = true)
		{
			std::wstring s_out;
			cstring::remove_white_space_of_prepostfix(s_out, s_in_out, b_prefix, b_postfix);
			s_in_out.swap(s_out);
		}
		static void remove_white_space_of_prepostfix(std::string& s_in_out, bool b_prefix = true, bool b_postfix = true)
		{
			std::string s_out;
			cstring::remove_white_space_of_prepostfix(s_out, s_in_out, b_prefix, b_postfix);
			s_in_out.swap(s_out);
		}

		static void remove_white_space(std::wstring& s_out, const std::wstring& s_in, bool b_remove_inner_space = false)
		{
			cstring::remove_white_space_of_prepostfix(s_out, s_in);
			cstring::replace_all(s_out, L"\t", L"");
			cstring::replace_all(s_out, L"\f", L"");
			cstring::replace_all(s_out, L"\v", L"");
			cstring::replace_all(s_out, L"\n", L"");
			cstring::replace_all(s_out, L"\r", L"");
			if (b_remove_inner_space)
				cstring::replace_all(s_out, L" ", L"");
		}

		static void remove_white_space(std::wstring& s_in_out, bool b_remove_inner_space = false)
		{
			cstring::remove_white_space_of_prepostfix(s_in_out);
			cstring::replace_all(s_in_out, L"\t", L"");
			cstring::replace_all(s_in_out, L"\f", L"");
			cstring::replace_all(s_in_out, L"\v", L"");
			cstring::replace_all(s_in_out, L"\n", L"");
			cstring::replace_all(s_in_out, L"\r", L"");
			if (b_remove_inner_space)
				cstring::replace_all(s_in_out, L" ", L"");
		}

		static void remove_all_char_except_hex_char(std::wstring& s_out, const std::wstring& s_in)
		{
			s_out.clear();

			std::for_each(std::begin(s_in), std::end(s_in), [&](wchar_t c) {
				if (c >= L'0' && c <= L'9')
					s_out.push_back(c);
				else if (c >= L'a' && c <= L'f')
					s_out.push_back(c);
				else if (c >= L'A' && c <= L'F')
					s_out.push_back(c);
				});
		}

		static void format(std::string& s_out, const char* s_fmt, ...)
		{
			s_out.clear();
			va_list ap;
			va_start(ap, s_fmt);

			do {
				if (s_fmt == NULL)
					continue;
#ifdef _WIN32
				va_list ap_copy;
				va_copy(ap_copy, ap);

				// Windows에서 _vscprintf 함수로 필요한 버퍼 크기 계산
				int n_len = _vscprintf(s_fmt, ap_copy) + 1; // for '\0'
				va_end(ap_copy);

				// 버퍼 생성 및 vsprintf_s 함수 사용
				type_v_s_buffer v_s_buffer(n_len, 0);
				if (vsprintf_s(&v_s_buffer[0], v_s_buffer.size(), s_fmt, ap) <= 0)
					continue;

#else
				// 버퍼 생성 및 vsnprintf 함수 사용
				type_v_s_buffer v_s_buffer(2048, 0);
				if (vsnprintf(&v_s_buffer[0], v_s_buffer.size(), s_fmt, ap) <= 0)
					continue;

#endif

				s_out = std::string(&v_s_buffer[0]);
			} while (false);

			va_end(ap);
		}

		static void format_c_style(std::wstring& s_out, const wchar_t* s_fmt, ...)
		{
			s_out.clear();
			va_list ap;
			va_start(ap, s_fmt);

			do {
				if (s_fmt == nullptr)
					continue;
#ifdef _WIN32
				va_list ap_copy;
				va_copy(ap_copy, ap);

				// Windows에서 _vscwprintf 함수로 필요한 버퍼 크기 계산
				int n_len = _vscwprintf(s_fmt, ap_copy) + 1; // for '\0'
				va_end(ap_copy);

				// 버퍼 생성 및 vswprintf_s 함수 사용
				type_v_ws_buffer v_ws_buffer(n_len, 0);
				if (vswprintf_s(&v_ws_buffer[0], v_ws_buffer.size(), s_fmt, ap) <= 0)
					continue;

#else
				// 버퍼 생성 및 vswprintf 함수 사용
				type_v_ws_buffer v_ws_buffer(2048, 0);
				if (vswprintf(&v_ws_buffer[0], v_ws_buffer.size(), s_fmt, ap) <= 0)
					continue;

#endif

				s_out = std::wstring(&v_ws_buffer[0]);
			} while (false);

			va_end(ap);
		}

		static void format_stl_style(std::wstring& s_out, const std::wstring& s_fmt, ...)
		{
			s_out.clear();
			va_list ap;
			const wchar_t* ps_fmt = s_fmt.c_str();

			va_start(ap, ps_fmt);

			do {
				if (s_fmt.empty())
					continue;

#ifdef _WIN32
				va_list ap_copy;
				va_copy(ap_copy, ap);

				// Windows에서 _vscwprintf 함수로 필요한 버퍼 크기 계산
				int n_len = _vscwprintf(s_fmt.c_str(), ap_copy) + 1; // for '\0'
				va_end(ap_copy);

				// 버퍼 생성 및 vswprintf_s 함수 사용
				type_v_ws_buffer v_ws_buffer(n_len, 0);
				if (vswprintf_s(&v_ws_buffer[0], v_ws_buffer.size(), s_fmt.c_str(), ap) <= 0)
					continue;

#else
				// 버퍼 생성 및 vswprintf 함수 사용
				type_v_ws_buffer v_ws_buffer(2048, 0);
				if (vswprintf(&v_ws_buffer[0], v_ws_buffer.size(), s_fmt.c_str(), ap) <= 0)
					continue;

#endif

				s_out = std::wstring(&v_ws_buffer[0]);
			} while (false);

			va_end(ap);
		}
		static void format(std::wstring& s_out, const std::wstring& s_fmt, ...)
		{
			s_out.clear();
			va_list ap;
			const wchar_t* ps_fmt = s_fmt.c_str();

			va_start(ap, ps_fmt);

			do {
				if (s_fmt.empty())
					continue;
				//
#ifdef _WIN32
				va_list ap_copy;
				va_copy(ap_copy, ap);

				// Windows에서 _vscwprintf 함수로 필요한 버퍼 크기 계산
				int n_len = _vscwprintf(s_fmt.c_str(), ap_copy) + 1; // for '\0'
				va_end(ap_copy);

				// 버퍼 생성 및 vswprintf_s 함수 사용
				type_v_ws_buffer v_ws_buffer(n_len, 0);
				vswprintf_s(&v_ws_buffer[0], v_ws_buffer.size(), s_fmt.c_str(), ap);
				s_out = std::wstring(&v_ws_buffer[0]);
#else
				type_v_ws_buffer v_ws_buffer(2048, 0);
				if (vswprintf(&v_ws_buffer[0], v_ws_buffer.size(), s_fmt.c_str(), ap) > 0) {
					s_out = std::wstring(&v_ws_buffer[0]);
				}
				
#endif
			} while (false);
			va_end(ap);
		}

		static void format_with_pre_post(std::wstring& s_out, const std::wstring& s_prefix, const std::wstring& s_postfix, const std::wstring& s_fmt, ...)
		{
			s_out.clear();
			va_list ap;
			const wchar_t* ps_fmt = s_fmt.c_str();

			va_start(ap, ps_fmt);

			do {
				if (s_fmt.empty())
					continue;

#ifdef _WIN32
				va_list ap_copy;
				va_copy(ap_copy, ap);

				// Windows에서 _vscwprintf 함수로 필요한 버퍼 크기 계산
				int n_len = _vscwprintf(s_fmt.c_str(), ap_copy) + 1; // for '\0'
				va_end(ap_copy);

				// 버퍼 생성 및 vswprintf_s 함수 사용
				type_v_ws_buffer v_ws_buffer(n_len, 0);
				if (vswprintf_s(&v_ws_buffer[0], v_ws_buffer.size(), s_fmt.c_str(), ap) <= 0)
					continue;

#else
				// 버퍼 생성 및 vswprintf 함수 사용
				type_v_ws_buffer v_ws_buffer(2048, 0);
				if (vswprintf(&v_ws_buffer[0], v_ws_buffer.size(), s_fmt.c_str(), ap) <= 0)
					continue;

#endif

				// 전위 및 후위 문자열과 포맷된 문자열을 합쳐서 s_out에 할당
				std::wstring prefix = s_prefix;
				std::wstring postfix = s_postfix;
				s_out = prefix + std::wstring(&v_ws_buffer[0]) + postfix;

			} while (false);

			va_end(ap);
		}

		static std::string get_mcsc_from_unicode(const std::wstring& s_unicode) 
		{
			std::string s_mcsc;

			do {
				if (s_unicode.empty())
					continue;
				//
#ifdef _WIN32
				size_t size_needed = 0;
				wcstombs_s(&size_needed, nullptr, 0, s_unicode.c_str(), _TRUNCATE);
				if (size_needed > 0)
				{
					s_mcsc.resize(size_needed);
					wcstombs_s(&size_needed, &s_mcsc[0], size_needed, s_unicode.c_str(), _TRUNCATE);
					//mbstowcs_s function copy the last NULL. but std::wstring(or std::string) no need the last NULL
					//therefore .. need remove the last NULL from std::wstring(or std::string)
					if (!s_mcsc.empty()) {
						s_mcsc.resize(s_mcsc.size() - 1);
					}

				}
#else
				size_t size_needed = std::wcstombs(nullptr, s_unicode.c_str(), 0);
				if (size_needed != (size_t)-1)
				{
					std::vector<char> v_mcsc(size_needed+1, 0);
					if (!v_mcsc.empty()) {
						std::wcstombs(&v_mcsc[0], s_unicode.c_str(), size_needed);
						s_mcsc = &v_mcsc[0];
					}
				}
#endif
				else
				{
					s_mcsc.clear(); //default for error
				}

			} while (false);

			return s_mcsc;
		}

		static std::wstring get_unicode_from_mcsc(const std::string& s_mcsc)
		{
			std::wstring s_unicode;

			do {
				if (s_mcsc.empty())
					continue;
				//
#ifdef _WIN32
				size_t size_needed = 0;
				mbstowcs_s(&size_needed, nullptr, 0, s_mcsc.c_str(), _TRUNCATE);
				if (size_needed > 0)
				{
					s_unicode.resize(size_needed);
					mbstowcs_s(&size_needed, &s_unicode[0], size_needed, s_mcsc.c_str(), _TRUNCATE);
					//mbstowcs_s function copy the last NULL. but std::wstring(or std::string) no need the last NULL
					//therefore .. need remove the last NULL from std::wstring(or std::string)
					if (!s_unicode.empty()) {
						s_unicode.resize(s_unicode.size() - 1);
					}
				}
#else
				size_t size_needed = std::mbstowcs(nullptr, s_mcsc.c_str(), 0);
				if (size_needed != (size_t)-1)
				{
					std::vector<wchar_t> v_unicode(size_needed + 1, 0);
					if (!v_unicode.empty()) {
						std::mbstowcs(&v_unicode[0], s_mcsc.c_str(), size_needed);
						s_unicode = &v_unicode[0];
					}
				}
#endif
				else
				{
					s_unicode.clear();//default for erro
				}

			} while (false);

			return s_unicode;
		}

		static std::wstring get_unicode_english_error_message(boost::beast::error_code& ec)
		{
			std::wstring s_unicode;
			std::string error_message = ec.message();

			if (!error_message.empty())
			{
#ifdef _WIN32
				size_t size_needed = 0;
				mbstowcs_s(&size_needed, nullptr, 0, error_message.c_str(), _TRUNCATE);
				if (size_needed > 0)
				{
					s_unicode.resize(size_needed);
					mbstowcs_s(&size_needed, &s_unicode[0], size_needed, error_message.c_str(), _TRUNCATE);
					//mbstowcs_s function copy the last NULL. but std::wstring(or std::string) no need the last NULL
					//therefore .. need remove the last NULL from std::wstring(or std::string)
					if (!s_unicode.empty()) {
						s_unicode.resize(s_unicode.size() - 1);
					}
				}
#else
				size_t size_needed = std::mbstowcs(nullptr, error_message.c_str(), 0);
				if (size_needed != (size_t)-1)
				{
					s_unicode.resize(size_needed);
					std::mbstowcs(&s_unicode[0], error_message.c_str(), size_needed);
				}
#endif
				else
				{
					s_unicode = L"conversion failed(get_unicode_english_error_message())";
				}
			}
			return s_unicode;
		}

    private:
		cstring();
		cstring(const cstring&);
		cstring& operator=(const cstring&);
    };
}