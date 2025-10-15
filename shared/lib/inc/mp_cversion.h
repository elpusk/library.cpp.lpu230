#pragma once

#include <array>
#include <cstdint>

#include <mp_type.h>
#include <mp_cconvert.h>

namespace _mp
{
	// T_EACH_VERSION_ITEM unsigned char ~ unsigned long
	template <typename T_EACH_VERSION_ITEM>
	class cversion
	{
	public:

		cversion()
		{
			reset();
		}
		cversion(const T_EACH_VERSION_ITEM* p_version)
		{
			do {
				m_major = m_minor = m_fix = m_build = 0;
				//
				if (p_version == nullptr)
					continue;

				int i(0);

				if (p_version[0] >= 0 && p_version[0] <= 9) {
					// sVer[0].sVer[1].sVer[2].sVer[3] format.
					m_major = static_cast<T_EACH_VERSION_ITEM>(p_version[i++]);
					m_minor = static_cast<T_EACH_VERSION_ITEM>(p_version[i++]);
					m_fix = static_cast<T_EACH_VERSION_ITEM>(p_version[i++]);
					m_build = static_cast<T_EACH_VERSION_ITEM>(p_version[i++]);
					continue;
				}

				//zero string format.
				T_EACH_VERSION_ITEM* p_v[] = { &m_major, &m_minor, &m_fix, &m_build };
				int j = 0;
				while (p_version[i]) {
					if (p_version[i] >= '0' && p_version[i] <= '9') {
						*p_v[j] = static_cast<T_EACH_VERSION_ITEM>(p_version[i] - '0');
						j++;
						if (j > 3)
							break;//exit while
					}
					i++;
				}//end while

			} while (false);
		}

		cversion(T_EACH_VERSION_ITEM major, T_EACH_VERSION_ITEM minor, T_EACH_VERSION_ITEM fix, T_EACH_VERSION_ITEM build)
		{
			m_major = major;	m_minor = minor;	m_fix = fix;	m_build = build;
		}

		cversion(const cversion& version)
		{
			*this = version;
		}

		virtual ~cversion(void)
		{

		}

		cversion& operator=(const cversion& src)
		{
			m_major = src.m_major;	m_minor = src.m_minor;	m_fix = src.m_fix;	m_build = src.m_build;
			return *this;
		}

		bool operator > (const cversion& rhs)
		{
			bool b_result(false);

			do {
				if (m_major > rhs.m_major){
					b_result = true;
					continue;
				}
				if (m_major < rhs.m_major)
					continue;
				//
				if (m_minor > rhs.m_minor) {
					b_result = true;
					continue;
				}
				else if (m_minor < rhs.m_minor)
					continue;
				//
				if (m_fix > rhs.m_fix) {
					b_result = true;
					continue;
				}
				else if (m_fix < rhs.m_fix)
					continue;
				//
				if (m_build > rhs.m_build) {
					b_result = true;
					continue;
				}

			} while (false);
			return b_result;
		}

		bool operator == (const cversion& rhs)
		{
			bool b_result(false);
			do {
				if (m_major != rhs.m_major)
					continue;
				if (m_minor != rhs.m_minor)
					continue;
				if (m_fix != rhs.m_fix)
					continue;
				if (m_build != rhs.m_build)
					continue;
				//
				b_result = true;
			} while (false);
			return b_result;
		}

		bool operator < (const cversion& rhs)
		{
			bool b_result(false);
			do {
				if (*this > rhs)
					continue;
				if (*this == rhs)
					continue;
				//
				b_result = true;
			} while (false);
			return b_result;
		}

		bool operator >= (const cversion& rhs)
		{
			bool b_result(true);
			do {
				if (*this == rhs)
					continue;
				if (*this > rhs)
					continue;
				b_result = false;
			} while (false);
			return b_result;
		}

		bool operator <= (const cversion& rhs)
		{
			bool b_result(true);
			do {
				if (*this == rhs)
					continue;
				if (*this < rhs)
					continue;
				b_result = false;
			} while (false);
			return b_result;
		}

		void reset()
		{
			m_major = m_minor = m_fix = m_build = 0;
		}

		bool empty() const
		{
			bool b_result(false);

			do{
				if(m_major!=0){
					continue;
				}
				if(m_minor!=0){
					continue;
				}
				if(m_fix!=0){
					continue;
				}
				if(m_build!=0){
					continue;
				}
				//
				b_result = true;
			}while(false);
			return b_result;
		}
		//getter
		T_EACH_VERSION_ITEM get_major() const { return m_major; }
		T_EACH_VERSION_ITEM get_minor() const { return m_minor; }
		T_EACH_VERSION_ITEM get_fix() const { return m_fix; }
		T_EACH_VERSION_ITEM get_build() const  { return m_build; }

		std::vector< T_EACH_VERSION_ITEM > get_by_vector() const
		{
			std::vector< T_EACH_VERSION_ITEM > v;
			v.push_back(m_major);
			v.push_back(m_minor);
			v.push_back(m_fix);
			v.push_back(m_build);
			return v;
		}

		std::wstring get_by_string() const
		{
			std::wstring s_v;
			s_v += std::to_wstring((unsigned long)m_major);
			s_v += L'.';
			s_v += std::to_wstring((unsigned long)m_minor);
			s_v += L'.';
			s_v += std::to_wstring((unsigned long)m_fix);
			s_v += L'.';
			s_v += std::to_wstring((unsigned long)m_build);
			return s_v;
		}
		//settor
		// s_version is "a.b.c.d" format std::wstring
		bool set_by_string(const std::wstring& s_version)
		{
			bool b_result(false);
			do {
				if (s_version.empty())
					continue;

				std::array<std::wstring, 4> array_version = { L"0",L"0",L"0",L"0" };
				std::wstring s(s_version);
				std::wstring s_delimiter = L".";

				bool b_error(false);
				size_t n_index(0);
				size_t pos = 0;
				while ((pos = s.find(s_delimiter)) != std::wstring::npos) {
					if (n_index >= array_version.size()) {
						b_error = true;
						break;
					}
					array_version[n_index++] = s.substr(0, pos);
					s.erase(0, pos + s_delimiter.length());
				}//end while

				if (b_error)
					continue;

				if (!s.empty()) {
					if (n_index >= array_version.size()) {
						continue;
					}
					array_version[n_index] = s;
				}

				uint32_t n_data(0);
				std::array<T_EACH_VERSION_ITEM , 4> ref_array = { m_major ,m_minor ,m_fix ,m_build };
				
				for (size_t i = 0; i < array_version.size(); i++) {
					if (!cconvert::get_value(n_data, array_version[i], 10)) {
						b_error = true;
						break;
					}
					if (n_data > (std::numeric_limits<T_EACH_VERSION_ITEM>::max)()) {
						b_error = true;
						break;
					}
					//
					ref_array[i] = (T_EACH_VERSION_ITEM)n_data;
				}//end for
				
				if (b_error)
					continue;
				//
				m_major = ref_array[0];
				m_minor = ref_array[1];
				m_fix = ref_array[2];
				m_build = ref_array[3];
				b_result = true;
			} while (false);
			return b_result;
		}
	protected:
		T_EACH_VERSION_ITEM m_major;
		T_EACH_VERSION_ITEM m_minor;
		T_EACH_VERSION_ITEM m_fix;
		T_EACH_VERSION_ITEM m_build;
	};
}
