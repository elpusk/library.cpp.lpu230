#pragma once

// c++ header & all inline
// support only unicode.

#include <ct_type.h>
#include <boost/beast/core/error.hpp>
#include <Windows.h>

namespace _ns_tools
{
	class ct_string
	{
	//exported methods
	public:
		static void to_upper(std::wstring & s_out, const std::wstring & s_in)
		{
			s_out = s_in;
			std::transform(std::begin(s_in), std::end(s_in), std::begin(s_out), towupper);
		}
		static void to_upper(std::wstring & s_in_out)
		{
			std::transform(std::begin(s_in_out), std::end(s_in_out), std::begin(s_in_out), towupper);
		}
		static void to_lower(std::wstring & s_out, const std::wstring & s_in)
		{
			s_out = s_in;
			std::transform(std::begin(s_in), std::end(s_in), std::begin(s_out), towlower);
		}
		static void to_lower(std::wstring & s_in_out)
		{
			std::transform(std::begin(s_in_out), std::end(s_in_out), std::begin(s_in_out), towlower);
		}

		static std::wstring get_unicode_english_error_message(boost::beast::error_code& ec)
		{
			std::wstring s_unicode;

			do {
				LANGID id = GetThreadUILanguage(); //save current language
				SetThreadUILanguage(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US)); //French
				std::string s_error = ec.message();
				SetThreadUILanguage(id); //restore previous language

				if (s_error.empty())
					continue;
				s_unicode.assign(std::begin(s_error), std::end(s_error));
			} while (false);

			return s_unicode;
		}

		static std::wstring get_unicode_from_mcsc(const std::string & s_mcsc)
		{
			std::wstring s_unicode;

			do {
				if (s_mcsc.empty())
					continue;
				//
				std::vector<wchar_t> v_wsz_buffer(s_mcsc.size() + 1, 0);
				if (::MultiByteToWideChar(CP_UTF8, 0, s_mcsc.c_str(), (int)(s_mcsc.size()) + 1, &v_wsz_buffer[0], (int)(v_wsz_buffer.size())) == 0)
					continue;
				//
				s_unicode = (wchar_t*)&v_wsz_buffer[0];

			} while (false);

			return s_unicode;
		}
		static std::wstring get_unicode_from_mcsc(const std::wstring& s_mcsc)
		{
			return s_mcsc;//no need changed. this function for simple codeing
		}

		static std::string get_mcsc_from_unicode(const std::wstring & s_unicode)
		{
			std::string s_mcsc;

			do {
				if (s_unicode.empty())
					continue;
				//
				std::vector<CHAR> v_sz_buffer(s_unicode.size() + 1, 0);
				if (::WideCharToMultiByte(CP_UTF8, 0, s_unicode.c_str(), -1, &v_sz_buffer[0],(int)( v_sz_buffer.size()), NULL, NULL) == 0)
					continue;
				s_mcsc = (CHAR*)&v_sz_buffer[0];
			} while (false);
			return s_mcsc;
		}

		static std::string get_mcsc_from_unicode(const std::string& s_unicode)
		{
			return s_unicode;//no need changed. this function for simple codeing
		}

		static void get_mcsc_from_unicode(char *s_dst_mcsc, const wchar_t *s_src_unicode)
		{
			do {
				if (s_dst_mcsc == nullptr)
					continue;
				if (s_src_unicode == nullptr)
					continue;

				int n_two = 0;
				size_t i(0);

				while (true) {
					s_dst_mcsc[i] = (char)s_src_unicode[i];
					if (s_src_unicode[i] == NULL) {
						n_two++;
						if (n_two == 2)
							break;//exit while
					}
					else {
						n_two = 0;
					}
					++i;
				}//end while
			} while (false);
		}
		static void remove_all_char_except_hex_char(std::wstring & s_out, const std::wstring & s_in)
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

		static void remove_white_space_of_prepostfix(std::wstring & s_out, const std::wstring & s_in, bool b_prefix = true, bool b_postfix = true)
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

		static void remove_white_space_of_prepostfix(std::wstring & s_in_out, bool b_prefix = true, bool b_postfix = true)
		{
			std::wstring s_out;
			ct_string::remove_white_space_of_prepostfix(s_out, s_in_out, b_prefix, b_postfix);
			s_in_out.swap(s_out);
		}
		static void remove_white_space_of_prepostfix(std::string& s_in_out, bool b_prefix = true, bool b_postfix = true)
		{
			std::string s_out;
			ct_string::remove_white_space_of_prepostfix(s_out, s_in_out, b_prefix, b_postfix);
			s_in_out.swap(s_out);
		}

		static void remove_white_space(std::wstring & s_out, const std::wstring & s_in,bool b_remove_inner_space = false)
		{
			ct_string::remove_white_space_of_prepostfix(s_out, s_in);
			ct_string::replace_all(s_out, L"\t", L"");
			ct_string::replace_all(s_out, L"\f", L"");
			ct_string::replace_all(s_out, L"\v", L"");
			ct_string::replace_all(s_out, L"\n", L"");
			ct_string::replace_all(s_out, L"\r", L"");
			if(b_remove_inner_space)
				ct_string::replace_all(s_out, L" ", L"");
		}

		static void remove_white_space(std::wstring & s_in_out, bool b_remove_inner_space = false)
		{
			ct_string::remove_white_space_of_prepostfix(s_in_out);
			ct_string::replace_all(s_in_out, L"\t", L"");
			ct_string::replace_all(s_in_out, L"\f", L"");
			ct_string::replace_all(s_in_out, L"\v", L"");
			ct_string::replace_all(s_in_out, L"\n", L"");
			ct_string::replace_all(s_in_out, L"\r", L"");
			if (b_remove_inner_space)
				ct_string::replace_all(s_in_out, L" ", L"");
		}

		static void replace_all(std::wstring & s_in_out, const std::wstring & s_from, const std::wstring& s_to)
		{
			size_t n_pos = 0;
			while ((n_pos = s_in_out.find(s_from, n_pos)) != std::wstring::npos){
				s_in_out.replace(n_pos, s_from.length(), s_to);
				n_pos += s_to.length();
			}//end while.
		}

		static void format(std::string & s_out, const char *s_fmt, ...)
		{
			s_out.clear();
			va_list ap;
			va_start(ap, s_fmt);

			do {
				if (s_fmt == NULL)
					continue;
				size_t n_len = _vscprintf(s_fmt, ap) + 1; // for '\0'

				_ns_tools::type_v_s_buffer v_s_buffer(n_len, 0);
				if (vsprintf_s(&v_s_buffer[0], v_s_buffer.size(), s_fmt, ap) <= 0)
					continue;
				s_out = std::string(&v_s_buffer[0]);
			} while (false);

			va_end(ap);
		}

		static void format(std::wstring & s_out, const wchar_t *s_fmt, ...)
		{
			s_out.clear();
			va_list ap;
			va_start(ap, s_fmt);

			do {
				if (s_fmt == NULL)
					continue;
				size_t n_len = _vscwprintf(s_fmt, ap) + 1; // for '\0'

				_ns_tools::type_v_ws_buffer v_ws_buffer(n_len, 0);
				if (vswprintf_s(&v_ws_buffer[0], v_ws_buffer.size(), s_fmt, ap) <= 0)
					continue;
				s_out = std::wstring(&v_ws_buffer[0]);
			} while (false);

			va_end(ap);
		}
		static void format_c_style(std::wstring& s_out, const wchar_t* s_fmt, ...)
		{
			s_out.clear();
			va_list ap;
			va_start(ap, s_fmt);

			do {
				if (s_fmt == NULL)
					continue;
				size_t n_len = _vscwprintf(s_fmt, ap) + 1; // for '\0'

				_ns_tools::type_v_ws_buffer v_ws_buffer(n_len, 0);
				if (vswprintf_s(&v_ws_buffer[0], v_ws_buffer.size(), s_fmt, ap) <= 0)
					continue;
				s_out = std::wstring(&v_ws_buffer[0]);
			} while (false);

			va_end(ap);
		}

		static void format(std::wstring & s_out, const std::wstring & s_fmt, ...)
		{
			s_out.clear();
			va_list ap;

			const wchar_t *ps_fmt = s_fmt.c_str();
			va_start(ap, ps_fmt);

			do {
				if (s_fmt.empty())
					continue;
				size_t n_len = _vscwprintf(s_fmt.c_str(), ap) + 1; // for '\0'

				_ns_tools::type_v_ws_buffer v_ws_buffer(n_len, 0);
				if (vswprintf_s(&v_ws_buffer[0], v_ws_buffer.size(), s_fmt.c_str(), ap) <= 0)
					continue;
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
				size_t n_len = _vscwprintf(s_fmt.c_str(), ap) + 1; // for '\0'

				_ns_tools::type_v_ws_buffer v_ws_buffer(n_len, 0);
				if (vswprintf_s(&v_ws_buffer[0], v_ws_buffer.size(), s_fmt.c_str(), ap) <= 0)
					continue;
				s_out = std::wstring(&v_ws_buffer[0]);
			} while (false);

			va_end(ap);
		}

		static void format(std::wstring& s_out, const std::wstring& s_prefix, const std::wstring& s_postfix,const std::wstring& s_fmt, ...)
		{
			s_out.clear();
			va_list ap;

			const wchar_t* ps_fmt = s_fmt.c_str();
			va_start(ap, ps_fmt);

			do {
				if (s_fmt.empty())
					continue;
				size_t n_len = _vscwprintf(s_fmt.c_str(), ap) + 1; // for '\0'

				_ns_tools::type_v_ws_buffer v_ws_buffer(n_len, 0);
				if (vswprintf_s(&v_ws_buffer[0], v_ws_buffer.size(), s_fmt.c_str(), ap) <= 0)
					continue;
				s_out = std::wstring(&v_ws_buffer[0]);
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
				size_t n_len = _vscwprintf(s_fmt.c_str(), ap) + 1; // for '\0'

				_ns_tools::type_v_ws_buffer v_ws_buffer(n_len, 0);
				if (vswprintf_s(&v_ws_buffer[0], v_ws_buffer.size(), s_fmt.c_str(), ap) <= 0)
					continue;
				s_out = std::wstring(&v_ws_buffer[0]);
			} while (false);

			va_end(ap);
		}

		static _ns_tools::type_v_wstring get_command_line_parameters(int argc, char* argv[])
		{
			_ns_tools::type_v_wstring v_parameters;

			do {
				for (int i = 0; i < argc; i++) {
					if (argv[i]) {
						v_parameters.push_back(_ns_tools::ct_string::get_unicode_from_mcsc(argv[i]));
					}
				}//end for
			} while (false);
			return v_parameters;
		}

	public:
		~ct_string(){}
	
	private:

	private://don't call these methods
		ct_string();
		ct_string( const ct_string & );
		ct_string & operator=(const ct_string &);
	
	};


}
