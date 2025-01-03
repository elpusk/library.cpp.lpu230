#pragma once

#include <memory>
#include <utility>
#include <map>
#include <mp_type.h>
#include <mp_elpusk.h>
#include <hid/_vhid_info.h>

class _vhid_info_lpu237 : public _vhid_info
{
public:
	typedef	std::shared_ptr<_vhid_info_lpu237>	type_ptr;

private:
	//first - string : compositive additioanl path
	//second - int : open counter
	//third - bool : true(support shared open), false(support exclusive open)
	//forth - int : additional value for generating compositive map index
	typedef std::tuple<std::string, int, bool, int> _type_tuple_info_type;
	typedef	std::map<_mp::type_bm_dev,_vhid_info_lpu237::_type_tuple_info_type> _type_map_info_type;

public:
	_vhid_info_lpu237(_mp::type_bm_dev type) : 
		_vhid_info(),
		m_const_v_extra(_vhid_info::get_extra_paths(
			_mp::_elpusk::const_usb_vid,
			_mp::_elpusk::_lpu237::const_usb_pid,
			_mp::_elpusk::_lpu237::const_usb_inf_hid
		))
	{

		for (auto item : m_const_v_extra) {
			m_map_info[std::get<1>(item)] = std::make_tuple(std::get<0>(item), 0, std::get<2>(item), std::get<3>(item));
		}//end for

		//
		ajust_open_cnt(type,true);
	}

	virtual ~_vhid_info_lpu237()
	{
	}
	virtual bool can_be_open(_mp::type_bm_dev type, bool b_exclusive_open) const
	{
		bool b_can_open(false);

		do {
			auto it = m_map_info.find(type);
			if (it == m_map_info.end()) {
				continue;// not support type
			}
			if (std::get<2>(it->second)) {
				// type support shared open
				if (b_exclusive_open) {
					//but exclusive open request
					continue;//can not open
				}
				b_can_open = true; // shared open
				continue;
			}
			// type support only exclusive open case
			if (!b_exclusive_open) {
				//but shared open request
				continue;//can not open
			}
			if (std::get<1>(it->second) > 0) {
				continue;//already open. therefore can't open.
			}

			b_can_open = true;
		} while (false);
		return b_can_open;
	}

	virtual bool is_open(_mp::type_bm_dev type) const
	{
		bool b_open(false);

		do {
			auto it = m_map_info.find(type);
			if (it == m_map_info.end()) {
				continue;// not support type
			}
			if (std::get<1>(it->second) <= 0) {
				continue;
			}

			b_open = true;
		} while (false);
		return b_open;
	}

	/**
	* increase or decrease open counter of virtual device
	* return first-true(success adjustment), second-adjusted counter, third-true(all open-counter of compositive type is zero - need primitve type is closed!)
	*/
	virtual std::tuple<bool,int, bool> ajust_open_cnt(_mp::type_bm_dev type, bool b_increase)
	{
		int n_cnt = 0;
		bool b_result(false);
		bool b_all_closed(false);

		auto it = m_map_info.find(type);
		if (it == m_map_info.end()) {
			return std::make_tuple(b_result, n_cnt, b_all_closed);
		}

		n_cnt = std::get<1>(it->second);
		if (b_increase) {
			++n_cnt;
		}
		else {
			//decrease
			if (n_cnt > 0) {
				--n_cnt;
			}
		}
		std::get<1>(it->second) = n_cnt;
		b_result = true;

		if (n_cnt == 0) {
			b_all_closed = true;

			for (auto item : m_map_info) {
				if (std::get<1>(item.second) > 0) {
					b_all_closed = false;
					break;
				}
			}//end for 
		}
		return std::make_tuple(b_result, n_cnt, b_all_closed);
	}
private:
	const _vhid_info::type_set_path_type m_const_v_extra;
	_type_map_info_type m_map_info;

private://don't call these method
	_vhid_info_lpu237();
};