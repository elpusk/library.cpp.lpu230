#pragma once
#include <iostream>
#include <sstream>
#include <iomanip> 
#include <climits>
#include <cctype>
#include <string>
#include <algorithm>
#include <set>
#include <list>
#include <cstdint>


#include <mp_type.h>
#include <mp_cstring.h>
#include <mp_coperation.h>

namespace _mp{
    class cconvert {
	public:
		typedef	enum : int {
			value_type_none,
			value_type_int,			// "123" -> 123
			value_type_string,		//"12c" -> "12c"
			value_type_hex_string,	//"1302a" -> { 0x2a, 0x30, 0x01 }
			value_type_bool,		//"true" ->true
			value_type_bcd,			//"135" ->0x87
			value_type_multi_hex_string	//"00, 23, 4b, 14f" -> { 0x00, 0x23, 0x4b, 0x14f }
		}type_value_type;

		typedef	std::pair< type_value_type, std::wstring>	type_pair_type_string;

    public:
		/*
		* p_ss_multi_dst can be null.
		* if p_ss_multi_dst is null, return the size of multi-string.( included separator null & string end mark( double nulls )
		* and unit is byte.
		* if p_ss_multi_dst isn't null, return the number of string in p_ss_multi_dst buffer.
		*/
		static size_t change(wchar_t* p_ss_multi_dst, const type_list_wstring& list_s_src)
		{
			size_t n_string(0);
			do {

				size_t n_size = 0; //unit byte
				if (list_s_src.empty())
					continue;

				n_string = list_s_src.size();

				std::for_each(std::begin(list_s_src), std::end(list_s_src), [&](const std::wstring path) {
					n_size += ((path.size() + 1) * sizeof(wchar_t));
					});

				n_size += sizeof(wchar_t);	//add multi null size

				if (p_ss_multi_dst == NULL) {
					n_string = n_size;	//return only need buffer size( BYTE unit, including NULL & NULLs )
					continue;
				}

				std::for_each(std::begin(list_s_src), std::end(list_s_src), [&](const std::wstring& s_str) {

					for (size_t i = 0; i < s_str.length(); i++) {
						p_ss_multi_dst[i] = s_str[i];
					}//end for
					p_ss_multi_dst[s_str.length()] = L'\0';

					p_ss_multi_dst = &p_ss_multi_dst[s_str.length() + 1];
					});

				*p_ss_multi_dst = L'\0'; //make multi string

			} while (false);
			return n_string;
		}

		/*
		* p_ss_multi_dst can be null.
		* if p_ss_multi_dst is null, return the size of multi-string.( included separator null & string end mark( double nulls )
		* and unit is byte.
		* if p_ss_multi_dst isn't null, return the number of string in p_ss_multi_dst buffer.
		*/
		static size_t change(char* p_ss_multi_dst, const type_list_string& list_s_src)
		{
			size_t n_string(0);
			do {

				size_t n_size = 0; //unit byte
				if (list_s_src.empty())
					continue;

				n_string = list_s_src.size();

				std::for_each(std::begin(list_s_src), std::end(list_s_src), [&](const std::string path) {
					n_size += ((path.size() + 1) * sizeof(char));
					});

				n_size += sizeof(char);	//add multi null size

				if (p_ss_multi_dst == NULL) {
					n_string = n_size;	//return only need buffer size( BYTE unit, including NULL & NULLs )
					continue;
				}

				std::for_each(std::begin(list_s_src), std::end(list_s_src), [&](const std::string& s_str) {

					for (size_t i = 0; i < s_str.length(); i++) {
						p_ss_multi_dst[i] = s_str[i];
					}//end for
					p_ss_multi_dst[s_str.length()] = '\0';

					p_ss_multi_dst = &p_ss_multi_dst[s_str.length() + 1];
					});

				*p_ss_multi_dst = '\0'; //make multi string

			} while (false);
			return n_string;
		}

		/*
		* p_ss_multi_dst can be null.
		* if p_ss_multi_dst is null, return the size of multi-string.( included separator null & string end mark( double nulls )
		* and unit is byte.
		* if p_ss_multi_dst isn't null, return the number of string in p_ss_multi_dst buffer.
		*/
		static size_t change(wchar_t* p_ss_multi_dst, const type_set_wstring& set_s_src)
		{
			size_t n_string(0);
			do {

				size_t n_size = 0; //unit byte
				if (set_s_src.empty())
					continue;

				n_string = set_s_src.size();

				std::for_each(std::begin(set_s_src), std::end(set_s_src), [&](const std::wstring path) {
					n_size += ((path.size() + 1) * sizeof(wchar_t));
					});

				n_size += sizeof(wchar_t);	//add multi null size

				if (p_ss_multi_dst == NULL) {
					n_string = n_size;	//return only need buffer size( BYTE unit, including NULL & NULLs )
					continue;
				}

				std::for_each(std::begin(set_s_src), std::end(set_s_src), [&](const std::wstring& s_str) {

					for (size_t i = 0; i < s_str.length(); i++) {
						p_ss_multi_dst[i] = s_str[i];
					}//end for
					p_ss_multi_dst[s_str.length()] = L'\0';

					p_ss_multi_dst = &p_ss_multi_dst[s_str.length() + 1];
					});

				*p_ss_multi_dst = L'\0'; //make multi string

			} while (false);
			return n_string;
		}

		static size_t change(type_list_wstring& list_s_dst, const wchar_t* p_ss_multi_src)
		{//p_ss_multi_src is multi string
			const wchar_t* pDest;
			std::wstring stemp;
			size_t n_count = 0;
			size_t nOffset = 0;

			do {
				if (p_ss_multi_src == NULL)
					continue;

				list_s_dst.clear();
				//
				while (p_ss_multi_src[nOffset] != 0) {

					pDest = &(p_ss_multi_src[nOffset]);
					stemp = pDest;
					list_s_dst.push_back(stemp);

					nOffset += stemp.length() + 1;//for passing null termination
					n_count++;
				}//while
			} while (false);
			return n_count;
		}

		static size_t change(type_set_wstring& set_s_dst, const wchar_t* p_ss_multi_src)
		{//p_ss_multi_src is multi string
			const wchar_t* pDest;
			std::wstring stemp;
			size_t n_count = 0;
			size_t nOffset = 0;

			do {
				if (p_ss_multi_src == NULL)
					continue;

				set_s_dst.clear();
				//
				while (p_ss_multi_src[nOffset] != 0) {

					pDest = &(p_ss_multi_src[nOffset]);
					stemp = pDest;
					std::pair<type_set_wstring::iterator, bool> result = set_s_dst.insert(stemp);

					nOffset += stemp.length() + 1;//for passing null termination

					if (result.second)//inserted.
						n_count++;
				}//while
			} while (false);
			return n_count;
		}

		static size_t change(type_list_string& list_s_dst, const char* p_ss_multi_src)
		{//p_ss_multi_src is multi string
			const char* pDest;
			std::string stemp;
			size_t n_count = 0;
			size_t nOffset = 0;

			do {
				if (p_ss_multi_src == NULL)
					continue;

				list_s_dst.clear();
				//
				while (p_ss_multi_src[nOffset] != 0) {

					pDest = &(p_ss_multi_src[nOffset]);
					stemp = pDest;
					list_s_dst.push_back(stemp);

					nOffset += stemp.length() + 1;//for passing null termination
					n_count++;
				}//while
			} while (false);
			return n_count;
		}

		static size_t change(type_set_string& set_s_dst, const char* p_ss_multi_src)
		{//p_ss_multi_src is multi string
			const char* pDest;
			std::string stemp;
			size_t n_count = 0;
			size_t nOffset = 0;

			do {
				if (p_ss_multi_src == NULL)
					continue;

				set_s_dst.clear();
				//
				while (p_ss_multi_src[nOffset] != 0) {

					pDest = &(p_ss_multi_src[nOffset]);
					stemp = pDest;
					std::pair<type_set_string::iterator, bool> result = set_s_dst.insert(stemp);

					nOffset += stemp.length() + 1;//for passing null termination

					if (result.second)//inserted.
						n_count++;
				}//while
			} while (false);
			return n_count;
		}

		static size_t change(type_set_wstring& set_s_dst, unsigned long n_src, const unsigned char* s_src)
		{
			std::wstring stemp;

			do {
				set_s_dst.clear();

				if (s_src == NULL)
					continue;
				if (n_src == 0)
					continue;

				for (unsigned long i = 0; i < n_src; i++) {
					if (std::isprint(s_src[i]) == 0) {
						if (s_src[i] != 0x00) {
							set_s_dst.clear();
							break;//error exit for
						}
						//null terminatred string
						if (stemp.empty())
							continue;
						set_s_dst.insert(stemp);
						stemp.clear();
						continue;
					}

					stemp.push_back((std::wstring::value_type)s_src[i]);
				}//end for
				//
			} while (false);
			return set_s_dst.size();
		}

		static size_t change(type_set_wstring& set_s_dst, const type_v_buffer& v_src)
		{
			size_t n_item(0);
			do {
				set_s_dst.clear();

				if (v_src.empty())
					continue;
				n_item = cconvert::change(set_s_dst, (unsigned long)v_src.size(), &v_src[0]);
			} while (false);
			return n_item;
		}
		static size_t tokenizer(type_list_wstring& list_s_token, const std::wstring& s_src, const std::wstring& s_separator, bool b_if_token_empty_then_ignore = true)
		{
			do {
				list_s_token.clear();
				//
				if (s_src.empty())
					continue;
				if (s_separator.empty()) {
					list_s_token.push_back(s_src);
					continue;
				}
				//
				bool b_run(true);
				std::wstring::size_type n_start(0), n_separator(0), n_token(0);

				do {
					n_token = s_src.find_first_not_of(s_separator, n_start);
					n_separator = s_src.find_first_of(s_separator, n_start);

					if (n_token == std::wstring::npos) {
						//from n_start, all separator
						if (!b_if_token_empty_then_ignore) {
							size_t n_empty = s_src.size() - n_start + 1;
							for (size_t i = 0; i < n_empty; i++) {
								list_s_token.push_back(std::wstring());
							}//end for
						}
						//end tokenizer
						b_run = false;
						continue;
					}
					if (n_separator == std::wstring::npos) {
						list_s_token.push_back(s_src.substr(n_start));
						//end tokenizer
						b_run = false;
						continue;
					}

					if (n_token > n_separator) {
						if (!b_if_token_empty_then_ignore) {
							size_t n_empty = n_token - n_separator;
							for (size_t i = 0; i < n_empty; i++) {
								list_s_token.push_back(std::wstring());
							}//end for
						}
						n_start = n_token;
						continue;
					}

					list_s_token.push_back(s_src.substr(n_token, n_separator - n_token));
					n_start = (n_separator + 1);
				} while (b_run);

			} while (false);
			return list_s_token.size();
		}

		static std::wstring get_string(const type_v_buffer& v_src, int n_base, const std::wstring& s_prefix = L"", const std::wstring& s_delimiter = L"")
		{
			std::wstring s_out;

			do {
				if (v_src.empty())
					continue;

				std::wstring s_c;

				for (size_t i = 0; i < v_src.size(); i++) {
					s_c = get_string(v_src[i], n_base, s_prefix);
					if (i != v_src.size() - 1)
						s_out += (s_c + s_delimiter);
					else
						s_out += s_c;
				}//end for

			} while (false);
			return s_out;
		}

		static std::wstring get_string(unsigned char c_src, int n_base, const std::wstring& s_prefix = L"")
		{
			std::wostringstream ss_out;
			do {
				switch (n_base) {
				case 10:
					ss_out << s_prefix << std::dec << (unsigned short)c_src;
					break;
				case 16:
					ss_out << s_prefix;
					ss_out << std::setfill(L'0') << std::setw(2);
					ss_out << std::hex << (unsigned short)c_src;
					break;
				default:
					continue;
				}//end switch
			} while (false);
			return ss_out.str();
		}

		static std::wstring get_string(uint32_t dw_src, int n_base, const std::wstring& s_prefix = L"")
		{
			std::wostringstream ss_out;
			do {
				switch (n_base) {
				case 10:
					ss_out << s_prefix << std::dec << (uint32_t)dw_src;
					break;
				case 16:
					ss_out << s_prefix << std::hex << (uint32_t)dw_src;
					break;
				default:
					continue;
				}//end switch
			} while (false);
			return ss_out.str();
		}

		static std::wstring get_string(bool b_value)
		{
			if (b_value) {
				return std::wstring(L"true");
			}
			else {
				return std::wstring(L"false");
			}
		}

		static bool get_value(uint32_t& dw_out, const std::wstring& s_src, int n_base)
		{
			bool b_result(false);
			do {
				if (s_src.empty())
					continue;
				if (n_base != 10 && n_base != 16)
					continue;
				//
				std::wstring s_char(s_src);
				//remove space

				s_char.erase(std::remove_if(s_char.begin(), s_char.end(),
					[](std::wstring::value_type c) {
						return std::isspace(static_cast<std::wstring::value_type>(c));
					}), std::end(s_char));

				uint32_t l_value(0);
				try {
					l_value = (uint32_t)std::stoul(s_char, nullptr, n_base);
					b_result = true;
				}
				catch (...) {
					b_result = false;
				}

				if (!b_result)
					continue;
				dw_out = l_value;
			} while (false);
			return b_result;
		}

		static bool get_value(long& dw_out, const std::wstring& s_src, int n_base)
		{
			bool b_result(false);
			do {
				if (s_src.empty())
					continue;
				if (n_base != 10 && n_base != 16)
					continue;
				//
				std::wstring s_char(s_src);
				//remove space

				s_char.erase(std::remove_if(s_char.begin(), s_char.end(),
					[](std::wstring::value_type c) {
						return std::isspace(static_cast<std::wstring::value_type>(c));
					}), std::end(s_char));

				long l_value(0);
				try {
					l_value = std::stol(s_char, nullptr, n_base);
					b_result = true;
				}
				catch (...) {
					b_result = false;
				}

				if (!b_result)
					continue;
				dw_out = l_value;
			} while (false);
			return b_result;
		}

		static bool get_value(unsigned char& c_out, const std::wstring& s_src, int n_base)
		{
			bool b_result(false);
			do {
				if (s_src.empty())
					continue;
				if (n_base != 10 && n_base != 16)
					continue;
				//
				std::wstring s_char(s_src);
				//remove space
				s_char.erase(std::remove_if(s_char.begin(), s_char.end(),
					[](std::wstring::value_type c) {
						return std::isspace(static_cast<std::wstring::value_type>(c));
					}), std::end(s_char));


				uint32_t l_value(0);
				try {
					l_value = (uint32_t)std::stoul(s_char, nullptr, n_base);
					b_result = true;
				}
				catch (...) {
					b_result = false;
				}

				if (!b_result)
					continue;
				if (l_value > 0xFF)
					b_result = false;
				else
					c_out = (unsigned char)l_value;
			} while (false);
			return b_result;
		}

		static bool get_value(uint32_t& dw_out, const std::wstring& s_src)
		{
			bool b_result(false);
			do {
				if (s_src.empty())
					continue;
				int n_base(10);

				//
				std::wstring s_char(s_src);
				//remove space
				s_char.erase(std::remove_if(s_char.begin(), s_char.end(),
					[](std::wstring::value_type c) {
						return std::isspace(static_cast<std::wstring::value_type>(c));
					}), std::end(s_char));


				if (s_char.size() > 2) {
					if (s_char[0] == L'0' && s_char[1] == L'x') {
						s_char.erase(0, 2);//remove 0x
						n_base = 16;
					}
					else if (s_char[s_char.size() - 1] == L'h' || s_char[s_char.size() - 1] == L'H') {
						s_char.erase(s_char.size() - 1, 1);//remove h or H
						n_base = 16;
					}
				}
				else if (s_char.size() == 2) {
					if (s_char[s_char.size() - 1] == L'h' || s_char[s_char.size() - 1] == L'H') {
						s_char.erase(s_char.size() - 1, 1);//remove h or H
						n_base = 16;
					}
				}

				b_result = cconvert::get_value(dw_out, s_char, n_base);

			} while (false);
			return b_result;
		}

		static bool get_value(unsigned char& c_out, const std::wstring& s_src)
		{
			bool b_result(false);
			do {
				if (s_src.empty())
					continue;
				int n_base(10);

				//
				std::wstring s_char(s_src);
				//remove space
				s_char.erase(std::remove_if(s_char.begin(), s_char.end(),
					[](std::wstring::value_type c) {
						return std::isspace(static_cast<std::wstring::value_type>(c));
					}), std::end(s_char));


				if (s_char.size() > 2) {
					if (s_char[0] == L'0' && s_char[1] == L'x') {
						s_char.erase(0, 2);//remove 0x
						n_base = 16;
					}
					else if (s_char[s_char.size() - 1] == L'h' || s_char[s_char.size() - 1] == L'H') {
						s_char.erase(s_char.size() - 1, 1);//remove h or H
						n_base = 16;
					}
				}
				else if (s_char.size() == 2) {
					if (s_char[s_char.size() - 1] == L'h' || s_char[s_char.size() - 1] == L'H') {
						s_char.erase(s_char.size() - 1, 1);//remove h or H
						n_base = 16;
					}
				}
				b_result = cconvert::get_value(c_out, s_char, n_base);
			} while (false);
			return b_result;
		}

		/**
		 * get_bcd_code_from_decimal_stirng
		 *
		 * \param s_decimal : "0124578"
		 * \param n_bcd_size_by_byte_unit
		 * \param b_left_zero_padding
		 * \return 0x12, 0x45, 0x78
		 */
		static std::pair<bool, type_v_buffer> get_bcd_code_from_decimal_stirng(const std::wstring& s_decimal, size_t n_bcd_size_by_byte_unit = 0, bool b_left_zero_padding = true)
		{
			std::wstring s_in(s_decimal);
			type_v_buffer v_bcd(0), v_bcd_result(0);
			bool b_odd(true);
			wchar_t c_bcd(0), c_sum(0);
			bool b_result(false);

			if (s_in.size() % 2 != 0) {
				if (b_left_zero_padding)
					s_in = L'0' + s_in;
				else
					s_in = s_in + L'0';
			}

			std::for_each(std::begin(s_in), std::end(s_in), [&](std::wstring::value_type c) {
				if (c > L'9')	c_bcd = c - L'A' + 10;
				else			c_bcd = c - L'0';
				//
				if (b_odd) {
					c_sum = c_bcd;
				}
				else {
					c_sum <<= 4;
					c_sum += c_bcd;
					v_bcd.push_back((unsigned char)c_sum);
				}
				b_odd = !b_odd;
				});

			do {
				if (n_bcd_size_by_byte_unit > 0) {
					if (n_bcd_size_by_byte_unit < v_bcd.size())
						continue;
					//
					v_bcd_result.resize(n_bcd_size_by_byte_unit, 0); v_bcd_result.assign(v_bcd_result.size(), 0);
					if (b_left_zero_padding)
						std::copy(std::begin(v_bcd), std::end(v_bcd), std::begin(v_bcd_result) + (v_bcd_result.size() - v_bcd.size()));
					else {
						std::copy(std::begin(v_bcd), std::end(v_bcd), std::begin(v_bcd_result));
					}
				}
				else if (n_bcd_size_by_byte_unit == 0)
					v_bcd_result = v_bcd;
				//
				b_result = true;
			} while (false);

			return (std::make_pair(b_result, v_bcd_result));
		}

		/**
			 * get_bcd_code_from_decimal_stirng.
			 *
			 * \param s_decimal_stirng - '0'~'9'
			 * \param n_decimal_stirng - the size of s_decimal_stirng buffer( except null terminate )
			 * \param n_bcd_size_by_byte_unit - out BCD code size( unit - byte )
			 * \param c_filler
			 * \param b_left_justify
			 * \return
			 */
		static std::pair<bool, type_v_buffer> get_bcd_code_from_decimal_stirng(
			const char* s_decimal_stirng, size_t n_decimal_stirng
			, size_t n_bcd_size_by_byte_unit
			, unsigned char c_filler
			, bool b_left_justify)
		{
			bool b_result(false);
			type_v_buffer v_bcd_result(0);

			size_t hex_cur;
			size_t asc_cur;
			size_t adjust;
			unsigned char digit;

			size_t mod = n_decimal_stirng % 2;
			size_t hex_req = n_decimal_stirng / 2 + mod; // required size of hex array to hold converted ascii string

			do {
				// Check the size of the array to hold the result
				if (hex_req > n_bcd_size_by_byte_unit)
					continue;

				v_bcd_result.resize(n_bcd_size_by_byte_unit, c_filler); v_bcd_result.assign(v_bcd_result.size(), c_filler);
				//
				asc_cur = 0;
				adjust = 0;
				if (b_left_justify) {
					// If left justified always start with left most digit
					hex_cur = 0;
				}
				else {
					// If right justified, then calculate the start index
					hex_cur = n_bcd_size_by_byte_unit - hex_req;
					if (mod == 1)
						adjust = 1;
				}

				b_result = true;
				// For all characters in asciiStr array
				while (asc_cur < n_decimal_stirng) {
					// Calculate a binary value of the digit represented by asci character code
					digit = s_decimal_stirng[asc_cur] - '0';
					if ((char)digit < 0 || (char)digit > 9) {
						b_result = false;
						break;
					}
					// determine wich 4 bits of a byte to write
					if ((asc_cur + adjust) % 2 == 0) {
						// update 4 most significant (left most) bits
						v_bcd_result[hex_cur] = ((digit << 4) & 0xf0) | (v_bcd_result[hex_cur] & 0x0f);
					}
					else {
						// update 4 least significant (right most) bits
						v_bcd_result[hex_cur] = digit | (v_bcd_result[hex_cur] & 0xf0);
						hex_cur++;
					}
					asc_cur++; // Go to the next character
				}//end while

			} while (false);

			return (std::make_pair(b_result, v_bcd_result));
		}

		static std::pair<bool, type_v_buffer> get_bcd_code(uint32_t n_in, size_t n_bcd_size_by_byte_unit = 0, bool b_left_zero_padding = true)
		{
			std::string s_in(std::to_string(n_in));
			type_v_buffer v_bcd(0), v_bcd_result(0);
			bool b_odd(true);
			char c_bcd(0), c_sum(0);
			bool b_result(false);

			if (s_in.size() % 2 != 0) {
				if (b_left_zero_padding)
					s_in = '0' + s_in;
				else
					s_in = s_in + '0';
			}

			std::for_each(std::begin(s_in), std::end(s_in), [&](std::string::value_type c) {
				if (c > '9')	c_bcd = c - 'A' + 10;
				else			c_bcd = c - '0';
				//
				if (b_odd) {
					c_sum = c_bcd;
				}
				else {
					c_sum <<= 4;
					c_sum += c_bcd;
					v_bcd.push_back(c_sum);
				}
				b_odd = !b_odd;
				});

			do {
				if (n_bcd_size_by_byte_unit > 0) {
					if (n_bcd_size_by_byte_unit < v_bcd.size())
						continue;
					//
					v_bcd_result.resize(n_bcd_size_by_byte_unit, 0); v_bcd_result.assign(v_bcd_result.size(), 0);
					if (b_left_zero_padding)
						std::copy(std::begin(v_bcd), std::end(v_bcd), std::begin(v_bcd_result) + (v_bcd_result.size() - v_bcd.size()));
					else {
						std::copy(std::begin(v_bcd), std::end(v_bcd), std::begin(v_bcd_result));
					}
				}
				else if (n_bcd_size_by_byte_unit == 0)
					v_bcd_result = v_bcd;
				//
				b_result = true;
			} while (false);

			return (std::make_pair(b_result, v_bcd_result));
		}

		static std::pair<bool, type_v_buffer> get_bcd_code(const type_v_buffer& v_ascii_numeric)
		{
			type_v_buffer v_bcd_result(0);
			bool b_result(false);
			unsigned char c_value(0);

			do {
				if (v_ascii_numeric.empty())
					continue;
				if (v_ascii_numeric.size() % 2 != 0)
					continue;
				//
				b_result = true;

				for (size_t i = 0; i < v_ascii_numeric.size() / 2; i++) {
					if (!std::isdigit((int)v_ascii_numeric[2 * i])) {
						v_bcd_result.resize(0);
						b_result = false;
						break;
					}
					if (!std::isdigit((int)v_ascii_numeric[2 * i + 1])) {
						v_bcd_result.resize(0);
						b_result = false;
						break;
					}
					c_value = (unsigned char)((v_ascii_numeric[2 * i] - '0') << 4);
					c_value += (unsigned char)(v_ascii_numeric[2 * i + 1] - '0');
					v_bcd_result.push_back(c_value);
				}//end for

			} while (false);
			return (std::make_pair(b_result, v_bcd_result));
		}


		static bool get_value_from_bcd_code(uint32_t& dw_out, const unsigned char* ps_bcd, uint32_t n_bcd)
		{
			//18446744073709551615	- the maximum value for an unsigned long.
			bool b_result(false);
			uint32_t l_out = 0;
			unsigned char hi, low;

			do {
				if (ps_bcd == NULL || n_bcd == 0)
					continue;
				//
				b_result = true;

				for (uint32_t i = 0; i < n_bcd; i++) {
					if (l_out > (0xffffffffUL / 100)) {
						b_result = false;
						break;
					}
					l_out *= 100;
					hi = ps_bcd[i] >> 4;
					if (hi > 0x09) {
						b_result = false;
						break;
					}
					low = ps_bcd[i] & 0x0F;
					if (low > 0x09) {
						b_result = false;
						break;
					}

					l_out = l_out + hi * 10 + low;
				}//end for

			} while (false);

			if (b_result)
				dw_out = l_out;
			//
			return b_result;
		}

		static bool get_value_from_bcd_code(uint32_t& dw_out, const type_v_buffer& v_bcd)
		{
			return get_value_from_bcd_code(dw_out, &v_bcd[0], (uint32_t)v_bcd.size());
		}

		static bool get_data(type_v_buffer& vOut, const std::string& s_src, bool b_insert_null_to_tail = true)
		{
			bool b_result(false);

			do {
				vOut.resize(0);

				if (s_src.empty())
					continue;
				std::for_each(std::begin(s_src), std::end(s_src), [&](std::string::value_type c) {
					vOut.push_back(c);
					});
				//make nulls
				if (b_insert_null_to_tail)
					vOut.push_back(0);
				//
				b_result = true;
			} while (false);
			return b_result;
		}

		static bool get_data(type_v_buffer& vOut, const std::wstring& s_src, bool b_insert_null_to_tail = true)
		{
			bool b_result(false);

			do {
				vOut.resize(0);

				if (s_src.empty())
					continue;
				std::for_each(std::begin(s_src), std::end(s_src), [&](std::wstring::value_type cw) {
					unsigned char* p_c = (unsigned char*)&cw;
					//
					vOut.push_back(*p_c);
					p_c++;
					vOut.push_back(*p_c);
					});
				if (b_insert_null_to_tail) {
					//make nulls
					vOut.push_back(0);
					vOut.push_back(0);
				}
				//
				b_result = true;
			} while (false);
			return b_result;
		}

		static bool get_data(std::wstring& s_data, const std::wstring& s_src, const type_value_type type)
		{
			bool b_result(false);

			s_data.clear();

			do {
				switch (type) {
				case cconvert::value_type_string:
				case cconvert::value_type_hex_string:
				case cconvert::value_type_int:
				case cconvert::value_type_bool:
				case cconvert::value_type_bcd:
				case cconvert::value_type_multi_hex_string:
					s_data = s_src;
					break;
				default:
					continue;
				}//end switch
				//
				b_result = true;
			} while (false);
			return b_result;
		}

		static bool get_data(uint32_t& n_data, const std::wstring& s_src, const type_value_type type)
		{
			bool b_result(false);

			n_data = 0;
			wchar_t* end;
			errno = 0;  // To distinguish success/failure after call

			do {
				std::wstring s_data;
				std::pair<bool, type_v_buffer> pair_result;

				if (s_src.empty())
					continue;

				switch (type) {
				case cconvert::value_type_int:
				case cconvert::value_type_bcd:
					n_data = (uint32_t)std::wcstol(s_src.c_str(), &end, 10);
					if (errno == ERANGE)
						continue;
					break;
				case cconvert::value_type_hex_string:
					cstring::remove_all_char_except_hex_char(s_data, s_src);
					n_data = std::wcstoul(&s_data[0], nullptr, 16);
					if (errno == ERANGE)
						continue;
					break;
				case cconvert::value_type_string:
				case cconvert::value_type_bool:
				default:
					continue;
				}//end switch
				//
				b_result = true;
			} while (false);
			return b_result;
		}

		static bool get_data(int& n_data, const std::wstring& s_src, const type_value_type type)
		{
			bool b_result(false);
			wchar_t* end;
			errno = 0;  // To distinguish success/failure after call
			n_data = 0;

			do {
				std::wstring s_data;
				std::pair<bool, type_v_buffer> pair_result;

				if (s_src.empty())
					continue;

				switch (type) {
				case cconvert::value_type_int:
				case cconvert::value_type_bcd:
					n_data = (uint32_t)std::wcstol(s_src.c_str(), &end, 10);
					if (errno == ERANGE)
						continue;
					break;
				case cconvert::value_type_hex_string:
					cstring::remove_all_char_except_hex_char(s_data, s_src);
					n_data = (int)wcstol(&s_data[0], nullptr, 16);
					if (errno == ERANGE)
						continue;
					break;
				case cconvert::value_type_string:
				case cconvert::value_type_bool:
				default:
					continue;
				}//end switch
				//
				b_result = true;
			} while (false);
			return b_result;
		}

		static bool get_data(type_v_buffer& v_data, const std::wstring& s_src, const type_value_type type)
		{
			bool b_result(false);
			std::pair<bool, type_v_buffer> pair_result;

			v_data.resize(0);

			do {
				std::wstring s_data;

				switch (type) {
				case cconvert::value_type_hex_string:
					cstring::remove_all_char_except_hex_char(s_data, s_src);
					cconvert::binary_from_hex_string(v_data, s_data);
					break;
				case cconvert::value_type_bcd:
					pair_result = cconvert::get_bcd_code_from_decimal_stirng(s_src);
					b_result = pair_result.first;
					if (b_result) {
						v_data = pair_result.second;
					}
					break;
				case cconvert::value_type_int:
				case cconvert::value_type_string:
				case cconvert::value_type_bool:
				default:
					continue;
				}//end switch
				//
				b_result = true;
			} while (false);
			return b_result;
		}
		static bool get_data(unsigned char& c_data, const std::wstring& s_src, const type_value_type type)
		{
			bool b_result(false), b_value(false);
			std::pair<bool, type_v_buffer> pair_result;
			type_v_buffer v_data(0);
			int n_value(0);

			c_data = 0;

			do {
				std::wstring s_data = s_src;
				cstring::to_lower(s_data);

				switch (type) {
				case cconvert::value_type_hex_string:
					cstring::remove_all_char_except_hex_char(s_data, s_src);
					cconvert::binary_from_hex_string(v_data, s_data);
					if (!v_data.empty())
						c_data = v_data[0];
					break;
				case cconvert::value_type_bcd:
					pair_result = cconvert::get_bcd_code_from_decimal_stirng(s_src);
					b_result = pair_result.first;
					if (b_result) {
						v_data = pair_result.second;
						if (!v_data.empty())
							c_data = v_data[v_data.size() - 1];
					}
					break;
				case cconvert::value_type_bool:
					b_result = cconvert::get_data(b_value, s_src, type);
					if (b_result) {
						if (b_value)
							c_data = 1;
						else
							c_data = 0;
					}
					continue;
				case cconvert::value_type_int:
					b_result = cconvert::get_data(n_value, s_src, type);
					if (b_result) {
						c_data = (unsigned char)n_value;
					}
					continue;
				case cconvert::value_type_string:
				default:
					continue;
				}//end switch
				//
				b_result = true;
			} while (false);
			return b_result;
		}

		static bool get_data(bool& b_data, const std::wstring& s_src, const type_value_type type)
		{
			bool b_result(false);

			b_data = false;
			do {
				if (s_src.empty())
					continue;

				std::wstring s_data = s_src;
				cstring::to_lower(s_data);

				switch (type) {
				case cconvert::value_type_bool:
					if (s_data == L"0")
						b_data = false;
					else if (s_data == L"1")
						b_data = true;
					else if (s_data == L"true")
						b_data = true;
					else if (s_data == L"false")
						b_data = false;
					else
						continue;
					break;
				case cconvert::value_type_int:
				case cconvert::value_type_string:
				case cconvert::value_type_hex_string:
				case cconvert::value_type_bcd:
				default:
					continue;
				}//end switch
				//
				b_result = true;
			} while (false);
			return b_result;
		}

		static bool get_data(type_v_ul_buffer& v_ul_data, const std::wstring& s_src, const type_value_type type, const std::wstring& s_separator)
		{
			bool b_result(false);

			v_ul_data.resize(0);

			do {
				if (s_src.empty()) {
					b_result = true;
					continue;
				}

				std::wstring s_data = s_src;
				cstring::to_lower(s_data);

				type_list_wstring list_s_token;
				type_list_wstring::iterator it_token;
				uint32_t n_data(0);

				switch (type) {
				case cconvert::value_type_multi_hex_string:
					cconvert::tokenizer(list_s_token, s_data, s_separator);
					if (list_s_token.empty())
						continue;
					//
					b_result = true;
					it_token = std::begin(list_s_token);
					for (; it_token != std::end(list_s_token); ++it_token) {
						if (!cconvert::get_data(n_data, *it_token, cconvert::value_type_hex_string)) {
							b_result = false;
							break;
						}
						v_ul_data.push_back(n_data);
					}//end for

					if (!b_result)
						continue;
					break;
				case cconvert::value_type_bool:
				case cconvert::value_type_int:
				case cconvert::value_type_string:
				case cconvert::value_type_hex_string:
				case cconvert::value_type_bcd:
				default:
					continue;
				}//end switch
				//
				b_result = true;
			} while (false);
			return b_result;
		}
		/**
		 * @brief change_from_utf8_multistring : get unicode strings(dequeu tpye) from utf8 multi string.
		 *
		 * @param q_s_dst - output , double linked queue strings
		 * @param n_total_characters - the number of total characters. ex) list_s_dst = {"01","2","467"}, then n_total_characters is 6.
		 * @param p_ss_multi_utf8 - utf8 multi string. the tail must be 0x00, 0x00
		 * @param n_ss_multi_utf8 - the size of p_ss_multi_utf8 (including double null termination)
		 * @return size_t - the number of string in list_s_dst.
		 */
		static size_t change_from_utf8_multistring(type_deque_wstring& q_s_dst, size_t& n_total_characters, const char* p_ss_multi_utf8, size_t n_ss_multi_utf8)
		{
			type_list_wstring list_s_dst;
			size_t n_string = cconvert::change_from_utf8_multistring(list_s_dst, n_total_characters, p_ss_multi_utf8, n_ss_multi_utf8);
			do {
				q_s_dst.clear();
				if (n_string == 0) {
					continue;
				}

				std::for_each(std::begin(list_s_dst), std::end(list_s_dst), [&](const std::wstring& s_data) {
					q_s_dst.push_back(s_data);
					});

				n_string = q_s_dst.size();
			} while (false);
			return n_string;
		}

		/**
		 * @brief change_from_utf8_multistring : get unicode strings(dequeu tpye) from utf8 multi string.
		 *
		 * @param list_s_dst - output , list strings
		 * @param n_total_characters - the number of total characters. ex) list_s_dst = {"01","2","467"}, then n_total_characters is 6.
		 * @param p_ss_multi_utf8 - utf8 multi string. the tail must be 0x00, 0x00
		 * @param n_ss_multi_utf8 - the size of p_ss_multi_utf8 (including double null termination)
		 * @return size_t - the number of string in list_s_dst.
		 */
		static size_t change_from_utf8_multistring(type_list_wstring& list_s_dst, size_t& n_total_characters, const char* p_ss_multi_utf8, size_t n_ss_multi_utf8)
		{
			size_t n_item(0);

			do {
				list_s_dst.clear();
				n_total_characters = 0;

				if (p_ss_multi_utf8 == nullptr)
					continue;
				if (n_ss_multi_utf8 == 0)
					continue;
				std::vector< std::pair<const char*, size_t > > v_pair_string(0);
				//get the number of string from multi string type.
				char* ps_cur((char*)p_ss_multi_utf8);
				char* ps_st(ps_cur);
				size_t n_size(0);//included one null of tail.

				for (size_t i = 0; i < n_ss_multi_utf8; i++, ps_cur++) {
					if (*ps_cur) {
						++n_size;
						continue;
					}
					//current is null
					if ((i + 1) >= n_ss_multi_utf8) {
						//the last is one null. format error.
						n_item = 0;
						break;//exit for
					}

					v_pair_string.push_back(std::make_pair(ps_st, n_size + 1));

					if (*(ps_cur + 1) == 0) {
						//the last is double null. the end of multi string.
						n_item = v_pair_string.size();
						break;//exit for
					}

					// one null, the end of one string.
					// new start stirng
					ps_st = ps_cur + 1;
					n_size = 0;
				}//end for

				if (n_item == 0) {
					continue;//none string
				}

				//check uft8 format.
				std::vector< std::pair<const char*, size_t > >::iterator it_info = std::begin(v_pair_string);
				for (; it_info != std::end(v_pair_string); ++it_info) {
					if (!coperation::is_all_utf8(it_info->first, it_info->second)) {
						n_item = 0;
						break;//format error
					}
				}//end for

				if (n_item == 0) {
					continue;//none string
				}

				//convert utf8 to unicode.
				int n_len(0);
				it_info = std::begin(v_pair_string);
				type_v_ws_buffer v_ws_data(0);
				std::wstring s_data;

				for (; it_info != std::end(v_pair_string); ++it_info) {

					s_data = cstring::get_unicode_from_mcsc(std::string(it_info->first));
					if (!s_data.empty()) {
						list_s_dst.push_back(s_data);
						n_total_characters += s_data.size();
					}
				}//end for
			} while (false);
			return n_item;
		}

		/**
		 * @brief change_from_utf8_multistring : get unicode strings(dequeu tpye) from utf8 multi string.
		 *
		 * @param set_s_dst - output , set strings
		 * @param n_total_characters - the number of total characters. ex) list_s_dst = {"01","2","467"}, then n_total_characters is 6.
		 * @param p_ss_multi_utf8 - utf8 multi string. the tail must be 0x00, 0x00
		 * @param n_ss_multi_utf8 - the size of p_ss_multi_utf8 (including double null termination)
		 * @return size_t - the number of string in list_s_dst.
		 */
		static size_t change_from_utf8_multistring(type_set_wstring& set_s_dst, size_t& n_total_characters, const char* p_ss_multi_utf8, size_t n_ss_multi_utf8)
		{
			type_list_wstring list_s_dst;
			size_t n_string = cconvert::change_from_utf8_multistring(list_s_dst, n_total_characters, p_ss_multi_utf8, n_ss_multi_utf8);
			do {
				set_s_dst.clear();
				if (n_string == 0) {
					continue;
				}

				std::for_each(std::begin(list_s_dst), std::end(list_s_dst), [&](const std::wstring& s_data) {
					set_s_dst.insert(s_data);
					});

				n_string = set_s_dst.size();
			} while (false);
			return n_string;
		}

		static void binary_from_hex_string(type_v_buffer& vOut, const std::wstring& s_hex, const std::wstring& s_seperator = std::wstring(L""))
		{
			std::wstring s_src(s_hex);

			size_t n_pos = 0;
			std::wstring s_token;

			vOut.resize(0);

			std::list<std::wstring> list_string_token;

			do {
				if (s_seperator.empty()) {// no delimiter
					if (s_src.size() % 2 != 0)
						continue;

					std::transform(std::begin(s_src), std::end(s_src), std::begin(s_src),
						[](std::wstring::value_type c)-> std::wstring::value_type {
							return std::tolower(static_cast<std::wstring::value_type>(c));
						});

					for (size_t i = 0; i < s_src.size() / 2; i++) {
						list_string_token.push_back(s_src.substr(i * 2, 2));
					}//end for
				}
				else {
					while ((n_pos = s_src.find(s_seperator)) != std::wstring::npos) {
						if (n_pos == 0 && (s_seperator.compare(L" ") == 0)) {
							s_src.erase(0, n_pos + s_seperator.length());
						}
						else {
							s_token = s_src.substr(0, n_pos);
							cstring::remove_white_space_of_prepostfix(s_token);
							std::transform(std::begin(s_token), std::end(s_token), std::begin(s_token),
								[](std::wstring::value_type c)-> std::wstring::value_type {
									return std::tolower(static_cast<std::wstring::value_type>(c));
								});

							list_string_token.push_back(s_token);
							s_src.erase(0, n_pos + s_seperator.length());
						}
					}//end while

					std::transform(std::begin(s_src), std::end(s_src), std::begin(s_src),
						[](std::wstring::value_type c)-> std::wstring::value_type {
							return std::tolower(static_cast<std::wstring::value_type>(c));
						});

					list_string_token.push_back(s_src);
				}
				//
				if (list_string_token.empty())
					continue;

				std::list<std::wstring>::iterator it_token = std::begin(list_string_token);
				for (; it_token != std::end(list_string_token); ++it_token) {

					if (it_token->size() < 1 || it_token->size() > 2) {
						vOut.resize(0);
						break;//error
					}

					unsigned c_digit(0);
					bool b_error(false);

					for (size_t j = 0; j < it_token->size(); j++) {
						c_digit <<= 4;
						unsigned c_nibble = (unsigned char)((*it_token)[j]);
						if (c_nibble >= '0' && c_nibble <= '9') {
							c_nibble -= '0';
							c_digit += c_nibble;
							continue;
						}
						if (c_nibble >= 'a' && c_nibble <= 'f') {
							c_nibble -= 'a';
							c_digit += (c_nibble + 10);
							continue;
						}
						b_error = true;
						break;
					}//end for j

					if (b_error) {
						vOut.resize(0);
						break;//error
					}

					vOut.push_back(c_digit);
				}//end for

			} while (false);
		}

		static void binary_from_hex_string(type_v_buffer& vOut, const std::string& s_hex, const std::string& s_seperator = std::string(""))
		{
			std::string s_src(s_hex);

			size_t n_pos = 0;
			std::string s_token;

			vOut.resize(0);

			std::list<std::string> list_string_token;

			do {
				if (s_seperator.empty()) {// no delimiter
					if (s_src.size() % 2 != 0)
						continue;
					std::transform(std::begin(s_src), std::end(s_src), std::begin(s_src),
						[](std::string::value_type c)-> std::string::value_type {
							return std::tolower(static_cast<std::string::value_type>(c));
						});

					for (size_t i = 0; i < s_src.size() / 2; i++) {
						list_string_token.push_back(s_src.substr(i * 2, 2));
					}//end for
				}
				else {
					while ((n_pos = s_src.find(s_seperator)) != std::wstring::npos) {
						if (n_pos == 0 && (s_seperator.compare(" ") == 0)) {
							s_src.erase(0, n_pos + s_seperator.length());
						}
						else {
							s_token = s_src.substr(0, n_pos);
							cstring::remove_white_space_of_prepostfix(s_token);
							std::transform(std::begin(s_token), std::end(s_token), std::begin(s_token),
								[](std::string::value_type c)-> std::string::value_type {
									return std::tolower(static_cast<std::string::value_type>(c));
								});

							list_string_token.push_back(s_token);
							s_src.erase(0, n_pos + s_seperator.length());
						}
					}//end while

					std::transform(std::begin(s_src), std::end(s_src), std::begin(s_src),
						[](std::string::value_type c)-> std::string::value_type {
							return std::tolower(static_cast<std::string::value_type>(c));
						});
					list_string_token.push_back(s_src);
				}
				//
				if (list_string_token.empty())
					continue;

				std::list<std::string>::iterator it_token = std::begin(list_string_token);
				for (; it_token != std::end(list_string_token); ++it_token) {

					if (it_token->size() < 1 || it_token->size() > 2) {
						vOut.resize(0);
						break;//error
					}

					unsigned c_digit(0);
					bool b_error(false);

					for (size_t j = 0; j < it_token->size(); j++) {
						c_digit <<= 4;
						unsigned c_nibble = (unsigned char)((*it_token)[j]);
						if (c_nibble >= '0' && c_nibble <= '9') {
							c_nibble -= '0';
							c_digit += c_nibble;
							continue;
						}
						if (c_nibble >= 'a' && c_nibble <= 'f') {
							c_nibble -= 'a';
							c_digit += (c_nibble + 10);
							continue;
						}
						b_error = true;
						break;
					}//end for j

					if (b_error) {
						vOut.resize(0);
						break;//error
					}

					vOut.push_back(c_digit);
				}//end for

			} while (false);
		}

		static void hex_string_from_binary(std::wstring& sOut, const unsigned char* s_In, int n_In, const std::wstring& s_delimiter = L"")
		{
			do {
				sOut.clear();
				if (n_In <= 0)
					continue;
				if (s_In == nullptr)
					continue;

				for (int i = 0; i < n_In; i++) {
					sOut += cconvert::_get_string(s_In[i], 16,L"");
					if (i != (n_In - 1))
						sOut += s_delimiter;
				}//end for

			} while (false);
		}

		static std::wstring hex_string_from_binary(const type_v_buffer& vIn, const std::wstring& s_delimiter = L"")
		{
			std::wstring s_out;
			cconvert::hex_string_from_binary(s_out, vIn, s_delimiter);
			if (s_out.empty())
				s_out = L"";
			return s_out;
		}
		static void hex_string_from_binary(std::wstring& sOut, const type_v_buffer& vIn, const std::wstring& s_delimiter = L"")
		{
			sOut.clear();

			for (size_t i = 0; i < vIn.size(); i++) {//don't use for_each, for_each will be problemed in VC2010.
				std::wstring s_hex = cconvert::_get_string((unsigned char)vIn[i], 16,L"");
				sOut += s_hex;
				if (i != (vIn.size() - 1))
					sOut += s_delimiter;
			}//end for
		}

		static void hex_string_from_binary(std::string& sOut, const type_v_buffer& vIn, const std::string& s_delimiter = "")
		{
			sOut.clear();

			for (size_t i = 0; i < vIn.size(); i++) {//don't use for_each, for_each will be problemed in VC2010.
				std::string s_hex = cconvert::_get_string((unsigned char)vIn[i], 16, "");
				sOut += s_hex;
				if (i != (vIn.size() - 1))
					sOut += s_delimiter;
			}//end for
		}

		static void hex_string_from_binary(std::string& sOut, const unsigned char* s_In, int n_In, const std::string& s_delimiter = "")
		{
			do {
				sOut.clear();
				if (n_In <= 0)
					continue;
				if (s_In == nullptr)
					continue;

				for (int i = 0; i < n_In; i++) {
					sOut += cconvert::_get_string(s_In[i], 16,"");
					if (i != (n_In - 1))
						sOut += s_delimiter;
				}//end for

			} while (false);
		}

		/**
		* @brief Don't use this function. this function must be depricated. only for old code.
		* Use get_command_line_parameters_by_set() or get_command_line_parameters_by_list()
		*/
		static type_set_wstring get_command_line_parameters(int argc, char* argv[])
		{
			type_set_wstring set_parameters;

			do {
				for (int i = 0; i < argc; i++) {
					if (argv[i]) {
						set_parameters.insert(cstring::get_unicode_from_mcsc(argv[i]));
					}
				}//end for
			} while (false);
			return set_parameters;
		}

		static type_set_wstring get_command_line_parameters(int argc, wchar_t* argv[])
		{
			type_set_wstring set_parameters;

			do {
				for (int i = 0; i < argc; i++) {
					if (argv[i]) {
						set_parameters.insert(argv[i]);
					}
				}//end for
			} while (false);
			return set_parameters;
		}

		static type_set_wstring get_command_line_parameters_by_set(int argc, char* argv[])
		{
			return _mp::cconvert::get_command_line_parameters(argc, argv);
		}

		static type_list_wstring get_command_line_parameters_by_list(int argc, char* argv[])
		{
			type_list_wstring list_parameters;

			do {
				for (int i = 0; i < argc; i++) {
					if (argv[i]) {
						list_parameters.push_back(cstring::get_unicode_from_mcsc(argv[i]));
					}
				}//end for
			} while (false);
			return list_parameters;
		}

		static type_list_string get_command_line_parameters_by_mcsc_list(int argc, char* argv[])
		{
			type_list_string list_parameters;

			do {
				for (int i = 0; i < argc; i++) {
					if (argv[i]) {
						list_parameters.push_back(argv[i]);
					}
				}//end for
			} while (false);
			return list_parameters;
		}

		static type_list_string get_command_line_parameters_by_mcsc_list(int argc, wchar_t* argv[])
		{
			type_list_string list_parameters;
			std::string s;

			do {
				for (int i = 0; i < argc; i++) {
					if (argv[i]) {
						s = _mp::cstring::get_mcsc_from_unicode(argv[i]);
						list_parameters.push_back(s);
					}
				}//end for
			} while (false);
			return list_parameters;
		}

		//
    private:
		static std::string _get_string(const type_v_buffer& v_src, int n_base, const std::string& s_prefix = "", const std::string& s_delimiter = "")
		{
			std::string s_out;

			do {
				if (v_src.empty())
					continue;

				std::string s_c;

				for (size_t i = 0; i < v_src.size(); i++) {
					s_c = cconvert::_get_string(v_src[i], n_base, s_prefix);
					if (i != v_src.size() - 1)
						s_out += (s_c + s_delimiter);
					else
						s_out += s_c;
				}//end for

			} while (false);
			return s_out;
		}
		static std::string _get_string(unsigned char c_src, int n_base, const std::string& s_prefix)
		{
			std::ostringstream ss_out;
			do {
				switch (n_base) {
				case 10:
					ss_out << s_prefix << std::dec << (unsigned short)c_src;
					break;
				case 16:
					ss_out << s_prefix;
					ss_out << std::setfill('0') << std::setw(2);
					ss_out << std::hex << (unsigned short)c_src;
					break;
				default:
					continue;
				}//end switch
			} while (false);
			return ss_out.str();
		}
		static std::wstring _get_string(unsigned char c_src, int n_base, const std::wstring& s_prefix)
		{
			std::wostringstream ss_out;
			do {
				switch (n_base) {
				case 10:
					ss_out << s_prefix << std::dec << (unsigned short)c_src;
					break;
				case 16:
					ss_out << s_prefix;
					ss_out << std::setfill(L'0') << std::setw(2);
					ss_out << std::hex << (unsigned short)c_src;
					break;
				default:
					continue;
				}//end switch
			} while (false);
			return ss_out.str();
		}

	private:
        cconvert();
        cconvert(const cconvert&);
        cconvert& operator=(const cconvert&);
    };
}