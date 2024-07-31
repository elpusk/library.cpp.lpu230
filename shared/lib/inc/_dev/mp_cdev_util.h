#pragma once

#include <vector>
#include <memory>

#include <mp_type.h>

namespace _mp {
	class cdev_util{
	public:
		/**
		* filter is ?- lpu200 device
		* return first - true : lpu200 filter
		* return second - wstring first argument
		* return third - wstring second argument
		*/
		static std::tuple<bool, std::wstring, std::wstring> is_lpu200_filter(const type_list_wstring& list_wstring_filter)
		{
			bool b_result(false);
			std::wstring s_first, s_second;

			do {
				type_list_wstring::const_iterator cit = std::cbegin(list_wstring_filter);
				if (cit == std::cend(list_wstring_filter))
					continue;
				if (cit->compare(L"lpu200") != 0)
					continue;

				s_first = *cit;

				++cit;
				if (cit == std::cend(list_wstring_filter)) {
					b_result = true;
					continue;//filter for component of lpu200
				}
				if (!cdev_util::is_lpu200_component_string(*cit)) {
					continue;
				}
				s_second = *cit;

				b_result = true;
			} while (false);
			return std::make_tuple(b_result, s_first, s_second);
		}
		static bool is_lpu200_component_string(const std::wstring& s_data)
		{
			bool b_result(true);
			do {
				if (s_data.compare(L"msr") == 0) {
					continue;
				}
				if (s_data.compare(L"scr0") == 0) {
					continue;
				}
				if (s_data.compare(L"ibutton") == 0) {
					continue;
				}
				if (s_data.compare(L"switch0") == 0) {
					continue;
				}
				b_result = false;
			} while (false);
			return b_result;
		}
		static type_v_bm_dev get_lpu200_component_type(const std::wstring& s_data)
		{
			type_v_bm_dev v_type;
			do {
				if (s_data.compare(L"msr") == 0) {
					v_type.push_back(type_bm_dev_lpu200_msr);
					continue;
				}
				if (s_data.compare(L"scr0") == 0) {
					v_type.push_back(type_bm_dev_lpu200_scr0);
					continue;
				}
				if (s_data.compare(L"ibutton") == 0) {
					v_type.push_back(type_bm_dev_lpu200_ibutton);
					continue;
				}
				if (s_data.compare(L"switch0") == 0) {
					v_type.push_back(type_bm_dev_lpu200_switch0);
					continue;
				}

				//here all sun component type
				v_type.push_back(type_bm_dev_lpu200_msr);
				v_type.push_back(type_bm_dev_lpu200_scr0);
				v_type.push_back(type_bm_dev_lpu200_ibutton);
				v_type.push_back(type_bm_dev_lpu200_switch0);
			} while (false);
			return v_type;
		}

		static type_bm_dev get_device_type_from_lpu200_filter(const std::wstring& s_component)
		{
			type_v_bm_dev v_type(cdev_util::get_lpu200_component_type(s_component));
			if (v_type.empty())
				return type_bm_dev_unknown;
			return v_type[0];
		}

		static type_v_bm_dev get_device_type_from_device_filter(const type_set_wstring& set_wstring_data_field)
		{
			static const std::wstring s_json_df_hid(L"hid");
			static const std::wstring s_json_df_winusb(L"usb");//winusb
			static const std::wstring s_json_df_virtual(L"virtual");

			type_v_bm_dev v_type;
			unsigned long dev_types(0);

			for (std::wstring s_filter : set_wstring_data_field) {
				if (s_filter.find(s_json_df_hid) != std::wstring::npos) {
					v_type.push_back(type_bm_dev_hid);
				}
				if (s_filter.find(s_json_df_winusb) != std::wstring::npos) {
					v_type.push_back(type_bm_dev_winusb);
				}
				if (s_filter.find(s_json_df_virtual) != std::wstring::npos) {
					v_type.push_back(type_bm_dev_virtual);
				}

			}//end for

			return v_type;
		}

	public:
		~cdev_util() {}
	private:
		cdev_util();
		cdev_util(const cdev_util&);
		cdev_util& operator=(const cdev_util&);

	};

}//the end of _mp namespace