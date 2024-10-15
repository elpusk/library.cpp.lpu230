#pragma once

#include <mp_type.h>
#include <set>
#include <algorithm>
#include <cstring>

namespace _mp{
	class coperation {
	public:
		template<typename T>
		static std::set<T> subtract(const std::set<T>& v1, const std::set<T>& v2)
		{
			std::set<T> set_result = v1;

			do {
				std::for_each(std::begin(v2), std::end(v2), [&](const T& s_v2) {
					auto it = set_result.find(s_v2);
					if (it != std::end(set_result)) {
						set_result.erase(it);
					}
					});
			} while (false);

			return set_result;
		}

#ifdef _WIN32
		static int get_usb_vid_from_path(const std::wstring& s_path)
		{
			int nVal = -1;// const_no_exist;

			size_t nfound = s_path.find(L"vid_"); //vid_XXXX ==> xxxx hexcimal is vender ID.
			if (nfound == std::wstring::npos)
				return nVal;

			nfound += 4;

			std::wstring sVal = s_path.substr(nfound, 4);

			if (!sVal.empty())
				nVal = std::wcstol(sVal.c_str(), NULL, 16);

			return nVal;
		}

		static int get_usb_pid_from_path(const std::wstring& s_path)
		{
			int nVal = -1;// const_no_exist;

			size_t nfound = s_path.find(L"pid_"); //vid_XXXX ==> xxxx  hexcimal is product ID.
			if (nfound == std::wstring::npos)
				return nVal;

			nfound += 4;

			std::wstring sVal = s_path.substr(nfound, 4);

			if (!sVal.empty())
				nVal = std::wcstol(sVal.c_str(), NULL, 16);

			return nVal;
		}

		static int get_usb_inf_from_path(const std::wstring& s_path)
		{
			int nVal = -1;// const_no_exist;

			size_t nfound = s_path.find(L"mi_"); //mi_XX ==> xx is interface number( interface number is started from zero-base )
			if (nfound == std::wstring::npos)
				return nVal;

			nfound += 3;

			std::wstring sVal = s_path.substr(nfound, 2);

			if (!sVal.empty())
				nVal = std::wcstol(sVal.c_str(), NULL, 16);

			return nVal;
		}

#endif
		static bool copy_btb(unsigned char* ps_dst, unsigned int n_dst, const unsigned char* ps_src, unsigned int n_src, bool b_right_arrange = false)
		{
			bool b_result(false);

			do {
				if (ps_src == nullptr || n_src == 0)
					continue;
				if (ps_dst == nullptr || n_dst == 0)
					continue;
				if (n_dst < n_src)
					continue;

				unsigned char* ps_fixed_dst(ps_dst);
				if (b_right_arrange) {
					ps_fixed_dst += (n_dst - n_src);
				}
				memcpy(ps_fixed_dst, ps_src, n_src);
				//
				b_result = true;
			} while (false);
			return b_result;
		}

		static bool copy_btb(unsigned char* ps_dst, unsigned int n_dst, const std::wstring& s_src, bool b_have_to_full_copy = false)
		{
			bool b_result(false);

			do {
				if (s_src.empty())
					continue;
				if (ps_dst == nullptr || n_dst == 0)
					continue;
				if (b_have_to_full_copy)
					if (s_src.size() > n_dst)
						continue;

				std::fill(&ps_dst[0], &ps_dst[n_dst], 0);
				if (s_src.size() < n_dst)
					n_dst = (unsigned int)s_src.size();

				std::transform(std::begin(s_src), std::begin(s_src) + n_dst, &ps_dst[0],
					[=](std::wstring::value_type c)->unsigned char {
						return (unsigned char)c;
					}
				);
				b_result = true;
			} while (false);
			return b_result;
		}
		static bool copy_btb(unsigned char* ps_dst, unsigned int n_dst, const type_v_buffer& v_src, bool b_have_to_full_copy = false)
		{
			bool b_result(false);

			do {
				if (v_src.empty())
					continue;
				if (ps_dst == nullptr || n_dst == 0)
					continue;
				if (b_have_to_full_copy)
					if (v_src.size() > n_dst)
						continue;

				std::fill(&ps_dst[0], &ps_dst[n_dst], 0);
				if (v_src.size() < n_dst)
					n_dst = (unsigned int)v_src.size();

				std::copy(std::begin(v_src), std::begin(v_src) + n_dst, &ps_dst[0]);
				b_result = true;
			} while (false);
			return b_result;
		}

		static bool copy_btb(type_ptr_v_buffer& ptr_v_dst, const type_ptr_v_buffer& ptr_v_src)
		{	//deep copy
			bool b_result(false);

			do {
				if (ptr_v_src == nullptr)
					continue;
				if (ptr_v_src->empty())
					continue;
				//
				if (ptr_v_dst) {
					ptr_v_dst->resize(ptr_v_src->size());
				}
				else {
					ptr_v_dst = std::make_shared<type_v_buffer>(ptr_v_src->size());
				}
				std::copy(std::begin(*ptr_v_src), std::end(*ptr_v_src), std::begin(*ptr_v_dst));
				//
				b_result = true;
			} while (false);
			return b_result;
		}

		static bool copy_btb(type_v_buffer& v_dst, const type_ptr_v_buffer& ptr_v_src)
		{	//deep copy
			bool b_result(false);

			do {
				if (ptr_v_src == nullptr)
					continue;
				if (ptr_v_src->empty())
					continue;
				//
				v_dst.resize(ptr_v_src->size());
				std::copy(std::begin(*ptr_v_src), std::end(*ptr_v_src), std::begin(v_dst));
				//
				b_result = true;
			} while (false);
			return b_result;
		}

		static bool copy_btb(type_ptr_v_buffer& ptr_v_dst, const type_v_buffer& v_src)
		{	//deep copy
			bool b_result(false);

			do {
				if (v_src.empty())
					continue;
				//
				if (ptr_v_dst) {
					ptr_v_dst->resize(v_src.size());
				}
				else {
					ptr_v_dst = std::make_shared<type_v_buffer>(v_src.size());
				}
				std::copy(std::begin(v_src), std::end(v_src), std::begin(*ptr_v_dst));
				//
				b_result = true;
			} while (false);
			return b_result;
		}

		static bool copy_btb(type_v_buffer& v_dst, const type_v_buffer& v_src)
		{	//deep copy
			v_dst = v_src;
			return true;
		}

		static bool copy_btb(type_v_buffer& v_dst, const type_v_buffer& v_src, size_t n_start_pos, size_t n_size)
		{
			bool b_result(false);

			do {
				if (v_src.empty())
					continue;
				if (n_start_pos >= v_src.size())
					continue;
				if ((n_start_pos + n_size) > v_src.size())
					continue;
				//
				v_dst.resize(n_size);
				std::copy(std::begin(v_src) + n_start_pos, std::begin(v_src) + n_start_pos + n_size, std::begin(v_dst));
				//
				b_result = true;
			} while (false);
			return b_result;

		}
		static bool copy_btb(type_v_buffer& v_dst, const type_v_buffer& v_src, size_t n_start_pos)
		{
			bool b_result(false);

			do {
				if (v_src.empty())
					continue;
				if (n_start_pos >= v_src.size())
					continue;
				//
				v_dst.resize(v_src.size() - n_start_pos);
				std::copy(std::begin(v_src) + n_start_pos, std::end(v_src), std::begin(v_dst));
				//
				b_result = true;
			} while (false);
			return b_result;

		}

		/**
		 * copy_btb.
		 *
		 * \param v_dst - out, v_dst is to be multi string type.
		 * \param v_src - in, the set of wstring
		 * \return none
		 */
		static void copy_btb(type_v_ws_buffer& v_dst, const type_set_wstring& v_src)
		{
			do {
				v_dst.clear();

				if (v_src.empty())
					continue;
				//
				std::for_each(std::begin(v_src), std::end(v_src), [&](const std::wstring& s_data) {
					size_t n_old = v_dst.size();
					v_dst.resize(n_old + s_data.size() + 1);
					type_v_ws_buffer::iterator it(std::begin(v_dst));
					std::fill(it + n_old, std::end(v_dst), 0);

					it = it + n_old;
					std::for_each(std::begin(s_data), std::end(s_data), [&](std::wstring::value_type c) {
						*it = c;
						++it;
						});
					//
					});

				// add double nulls for the end of multi string
				v_dst.push_back((wchar_t)0);
				//
			} while (false);
		}

		/**
		 * copy_btb.( a1,a2 ), (b1,b2 ) -> a1=a2 b1=b2
		 *
		 * \param v_dst -  out, v_dst is to be multi string type.
		 * \param map_src- in. map , key is wstirng, value wstring
		 * \param c_separator - separator of between key and value.
		 */
		static void copy_btb(type_v_ws_buffer& v_dst, const type_map_wstring_wstring& map_src, const wchar_t c_separator)
		{
			do {
				v_dst.clear();

				if (map_src.empty())
					continue;
				//
				std::for_each(std::begin(map_src), std::end(map_src), [&](const type_map_wstring_wstring::value_type& pair_data) {
					std::for_each(std::begin(pair_data.first), std::end(pair_data.first), [&](std::wstring::value_type c) {
						v_dst.push_back(c);
						});

					v_dst.push_back(c_separator);

					std::for_each(std::begin(pair_data.second), std::end(pair_data.second), [&](std::wstring::value_type c) {
						v_dst.push_back(c);
						});
					//
					v_dst.push_back(0);//null terminate string
					});

				// add double nulls for the end of multi string
				v_dst.push_back((wchar_t)0);
				//
			} while (false);
		}

		static bool append_btb(type_ptr_v_buffer& ptr_v_dst, const type_ptr_v_buffer& ptr_v_src)
		{	//deep copy
			bool b_result(false);

			do {
				if (ptr_v_src == nullptr)
					continue;
				if (ptr_v_src->empty())
					continue;
				//
				size_t n_org(0);

				if (ptr_v_dst == nullptr)
					ptr_v_dst = std::make_shared<type_v_buffer>(ptr_v_src->size());
				else {
					n_org = ptr_v_dst->size();
					ptr_v_dst->resize(ptr_v_dst->size() + ptr_v_src->size());
				}
				//
				std::copy(std::begin(*ptr_v_src), std::end(*ptr_v_src), std::begin(*ptr_v_dst) + n_org);
				//
				b_result = true;
			} while (false);
			return b_result;
		}

		static bool append_btb(type_v_buffer& v_dst, const type_ptr_v_buffer& ptr_v_src)
		{	//deep copy
			bool b_result(false);

			do {
				if (ptr_v_src == nullptr)
					continue;
				if (ptr_v_src->empty())
					continue;
				//
				size_t n_org(v_dst.size());

				v_dst.resize(v_dst.size() + ptr_v_src->size());

				std::copy(std::begin(*ptr_v_src), std::end(*ptr_v_src), std::begin(v_dst) + n_org);
				//
				b_result = true;
			} while (false);
			return b_result;
		}

		static bool append_btb(type_ptr_v_buffer& ptr_v_dst, const type_v_buffer& v_src)
		{	//deep copy
			bool b_result(false);

			do {
				if (v_src.empty())
					continue;
				//
				size_t n_org(0);

				if (ptr_v_dst == nullptr) {
					ptr_v_dst = std::make_shared<type_v_buffer>(v_src.size());
				}
				else {
					n_org = ptr_v_dst->size();
					ptr_v_dst->resize(ptr_v_dst->size() + v_src.size());
				}
				std::copy(std::begin(v_src), std::end(v_src), std::begin(*ptr_v_dst) + n_org);
				//
				b_result = true;
			} while (false);
			return b_result;
		}

		static bool append_btb(type_v_buffer& v_dst, const type_v_buffer& v_src)
		{	//deep copy
			bool b_result(false);

			do {
				if (v_src.empty())
					continue;
				//
				size_t n_org(v_dst.size());
				v_dst.resize(v_dst.size() + v_src.size());
				std::copy(std::begin(v_src), std::end(v_src), std::begin(v_dst) + n_org);
				//
				b_result = true;
			} while (false);
			return b_result;
		}

		static bool is_equal(const type_set_wstring& v1, const type_set_wstring& v2)
		{
			bool b_equal(false);
			do {
				if (v1.size() != v2.size())
					continue;

				b_equal = true;
				std::for_each(std::begin(v1), std::end(v1), [&](const std::wstring& s_v1) {
					type_set_wstring::const_iterator it = v2.find(s_v1);
					if (it == std::end(v2)) {
						b_equal = false;
					}
					});
			} while (false);

			return b_equal;
		}

		static bool is_equal(const type_v_buffer& v1, const type_v_buffer& v2)
		{
			bool b_result(false);
			do {
				if (v1.size() != v2.size())
					continue;

				b_result = true;
				for (size_t i = 0; i < v1.size(); i++) {
					if (v1[i] != v2[i]) {
						b_result = false;
						break;
					}
				}//end for

			} while (false);
			return b_result;
		}

		static bool is_equal(const type_v_buffer& v1, const std::string& v2)
		{
			bool b_result(false);
			do {
				if (v1.size() != v2.size())
					continue;

				b_result = true;
				for (size_t i = 0; i < v1.size(); i++) {
					if (v1[i] != (unsigned char)v2[i]) {
						b_result = false;
						break;
					}
				}//end for

			} while (false);
			return b_result;
		}

		static bool is_equal(const unsigned char* p_v1, size_t n_v1, const unsigned char* p_v2, size_t n_v2)
		{
			bool b_result(false);
			do {
				if (p_v1 == 0 || n_v2 == 0)
					continue;
				//
				if (n_v1 != n_v2)
					continue;
				//
				b_result = true;

				for (size_t i = 0; i < n_v1; i++) {
					if (p_v1[i] != p_v2[i]) {
						b_result = false;
						break;
					}
				}//end for

			} while (false);

			return b_result;
		}

		static bool is_equal(const std::string& v1, const type_v_buffer& v2)
		{
			return coperation::is_equal(v2, v1);
		}

		static bool is_equal(const unsigned char* p_v1, size_t n_v1, const std::string& v2)
		{
			bool b_result(false);
			do {
				if (p_v1 == nullptr)
					continue;
				if (n_v1 < v2.size())
					continue;

				b_result = true;
				for (size_t i = 0; i < v2.size(); i++) {
					if (p_v1[i] != (unsigned char)v2[i]) {
						b_result = false;
						break;
					}
				}//end for

			} while (false);
			return b_result;
		}

		static bool is_partial_equal(const type_v_buffer& v1, const type_v_buffer& v2)
		{
			bool b_result(false);
			do {
				if (v1.empty())
					continue;
				if (v2.empty())
					continue;

				size_t n_loop(0);
				if (v1.size() <= v2.size())
					n_loop = v1.size();
				else
					n_loop = v2.size();

				b_result = true;
				for (size_t i = 0; i < n_loop; i++) {
					if (v1[i] != v2[i]) {
						b_result = false;
						break;
					}
				}//end for

			} while (false);
			return b_result;
		}


		/**
		 * @brief is_all_utf8 - if all characters of string is utf-8 format, return true
		 *
		 * @param s_data - zero string format( n_data is zero ). or not zero string format( n_data is greater then zero.)
		 * @param n_data - from zero.
		 * @return true - all characters of s_data is utf-8 format
		 * @return false - else( including empty-string )
		 */
		static bool is_all_utf8(const char* s_data, size_t n_data)
		{
			bool b_result(false);
			size_t n_byte(0);
			size_t n_offset(0);

			char c_one(0);
			size_t n_char(0);//the size of one character( 1~4 bytes)

			do {
				if (s_data == NULL)
					continue;

				if (n_data == 0) {
					n_byte = strlen(s_data);
				}
				else {
					n_byte = n_data;
				}

				if (n_byte == 0)
					continue;

				b_result = true;

				while (n_offset < n_byte) {
					c_one = s_data[n_offset];
					n_char = 0;
					//
					if ((c_one & 0x80) == 0) {
						// 0xxxxxxx
						++n_offset;
					}
					else if ((c_one & 0xF8) == 0xF0) {
						// 11110uuu 10uuzzzz 10yyyyyy 10xxxxxx
						++n_offset;
						n_char = 3;
					}
					else if ((c_one & 0xE0) == 0xE0) {
						// 1110zzzz 10yyyyyy 10xxxxxx
						++n_offset;
						n_char = 2;
					}
					else if ((c_one & 0xE0) == 0xC0) {
						// 110yyyyy 10xxxxxx
						++n_offset;
						n_char = 1;
					}
					else {
						b_result = false;
						continue;//not utf8 format
					}

					while (b_result && n_char--) {
						if (n_offset >= n_byte) {
							b_result = false;
						}
						else if ((s_data[n_offset++] & 0xC0) != 0x80) {
							// 10uuzzzz
							b_result = false;
						}
					}

				}//end while

			} while (false);

			return b_result;
		}

	private://don't call these function
		coperation() = delete;
		coperation(const coperation&) = delete;
		coperation& operator=(const coperation&) = delete;
	};
}