#pragma once

#include <memory>
#include <utility>
#include <mp_type.h>
#include <mp_elpusk.h>

/**
* An instance of this class corresponds to a single physical device (primitive device).
*/
class _vhid_info
{
public:
	enum : int32_t {
		const_map_index_invalid = -1,
		const_map_index_inc_unit = 0x00000100, //increment unit
		const_map_index_max = _mp::type_bm_dev_hid | 0x0000FF00, //max index of primitive map
		const_map_index_min = _mp::type_bm_dev_hid | _vhid_info::const_map_index_inc_unit, //min index of primitive map
		const_map_index_mask_additional_compositive = 0x000000FF
	};

	enum : int32_t {
		const_map_index_mask_of_primitive = 0x00FFFF00
	};

public:
	typedef	std::shared_ptr<_vhid_info>	type_ptr;

	//first - string : compositive additioanl path
	//second - type : device type
	//third - bool : true(support shared open), false(support exclusive open)
	//forth - int : additional value for generating compositive map index(1~0xff)
	typedef std::tuple<std::string, _mp::type_bm_dev, bool, int> type_tuple_path_type;
	typedef	std::set<_vhid_info::type_tuple_path_type> type_set_path_type;

public:
	/**
	* @brief get extra path of lpu237 virtual.
	* 
	*	including empty for primitive type.
	* 
	* @param w_vid unsigned short usb vendor id
	* 
	* @param w_pid unsigned short usb product id
	* 
	* @param n_interface int usb interface number
	* 
	* @return set of lpu237 primitive & compositive information
	* 
	*/
	static const _vhid_info::type_set_path_type& get_extra_paths(
		unsigned short w_vid, 
		unsigned short w_pid,
		int n_interface
	)
	{
		static const _vhid_info::type_set_path_type v_extra_empty;
		static const _vhid_info::type_set_path_type v_extra_lpu237 = {
			{std::string(),_mp::type_bm_dev_hid,false, 0}//primitive type
			,{"&msr",_mp::type_bm_dev_lpu200_msr,false, ((int)(_vhid_info::const_map_index_mask_additional_compositive)& _mp::type_bm_dev_lpu200_msr)}
			,{"&scr0",_mp::type_bm_dev_lpu200_scr0,false, ((int)(_vhid_info::const_map_index_mask_additional_compositive) & _mp::type_bm_dev_lpu200_scr0) }
			,{"&ibutton",_mp::type_bm_dev_lpu200_ibutton,true, ((int)(_vhid_info::const_map_index_mask_additional_compositive) & _mp::type_bm_dev_lpu200_ibutton)}
			,{"&switch0",_mp::type_bm_dev_lpu200_switch0,true, ((int)(_vhid_info::const_map_index_mask_additional_compositive) & _mp::type_bm_dev_lpu200_switch0)}
		};

		if (w_vid == _mp::_elpusk::const_usb_vid && w_pid == _mp::_elpusk::_lpu237::const_usb_pid && n_interface == _mp::_elpusk::_lpu237::const_usb_inf_hid) {
			return v_extra_lpu237;
		}

		return v_extra_empty;
	}

	/**
	* @brief checks the given path is primitive type or compositive type
	*
	* @param s_path - primitive or composite path
	*
	* @return tuple type
	*
	*   first - true : compositive type
	*	
	*	second - _mp::type_bm_dev, compoitive type value
	*
	*	third - std::string, primitive path of s_path
	*
	*	forth - true : support shared-open
	*
	*   second - the opened device of the index of map.(primitive index)
	*
	*   third - true(exclusive open), false(shared open or not open )
	*/
	static std::tuple<bool, _mp::type_bm_dev, std::string, bool> is_path_compositive_type(const std::string& s_path)
	{
		bool b_compositive(false);
		_mp::type_bm_dev t(_mp::type_bm_dev_hid);
		std::string s_primitive;
		bool b_support_shared_open(false);

		do {
			if (s_path.empty()) {
				continue;
			}

			for (auto item : _vhid_info::get_extra_paths(
				_mp::_elpusk::const_usb_vid,
				_mp::_elpusk::_lpu237::const_usb_pid,
				_mp::_elpusk::_lpu237::const_usb_inf_hid
			)) {
				if (s_path.size() <= std::get<0>(item).size()) {
					continue;
				}
				if (std::get<0>(item).size() == 0) {
					continue;
				}
				if (s_path.compare(s_path.size() - std::get<0>(item).size(), std::get<0>(item).size(), std::get<0>(item)) == 0) {
					// found compositive type
					b_compositive = true;
					t = std::get<1>(item);
					std::copy(s_path.begin(), s_path.end() - std::get<0>(item).size(), std::back_inserter(s_primitive));
					b_support_shared_open = std::get<2>(item);
					break; //exit for
				}
			}//end for

			if (!b_compositive)
				s_primitive = s_path;
			//
		} while (false);
		return std::make_tuple(b_compositive, t, s_primitive, b_support_shared_open);
	}

	/**
	* @brief get compositive type from compositive map index
	* 
	* @return compositive or primitive type
	*/
	static _mp::type_bm_dev get_type_from_compositive_map_index(int n_map_index_compositive)
	{
		_mp::type_bm_dev type(_mp::type_bm_dev_unknown);

		do {
			if (n_map_index_compositive < 0) {
				continue;
			}
			int n_primitive = _vhid_info::const_map_index_mask_of_primitive & n_map_index_compositive;
			if (n_primitive > _vhid_info::const_map_index_max) {
				continue;
			}
			if (n_primitive < _vhid_info::const_map_index_min) {
				continue;
			}
			if (n_primitive % _vhid_info::const_map_index_inc_unit != 0) {
				continue;
			}
			//
			int n_additional_compositive = _vhid_info::const_map_index_mask_additional_compositive & n_map_index_compositive;
			_mp::type_bm_dev t_compositive = (_mp::type_bm_dev)(_mp::type_bm_dev_hid + n_additional_compositive);

			for (auto item : _vhid_info::get_extra_paths(
				_mp::_elpusk::const_usb_vid,
				_mp::_elpusk::_lpu237::const_usb_pid,
				_mp::_elpusk::_lpu237::const_usb_inf_hid
			)) {
				if (std::get<1>(item) == t_compositive) {
					type = t_compositive;
					break;
				}
			}//end for

		} while (false);
		return type;
	}

	/**
	* @brief get primitive map index from compositive type index.
	* 
	* @param n_map_index_compositive - primitive or compositive type index.
	* 
	* @return pair 
	*	
	*	first - primitive map index
	* 
	*	if n_map_index_compositive is primitive type index, return n_map_index_compositive without operating
	* 
	*	second - boolean : true (the given n_map_index_compositive is compositive type)
	* 
	*	false (the given n_map_index_compositive is primitive type)
	*	
	*/
	static std::pair<int,bool> get_primitive_map_index_from_compositive_map_index(int n_map_index_compositive)
	{
		int n_primitive_map_index(-1);
		bool b_compositive_type(false);

		do {
			if (n_map_index_compositive < 0) {
				continue;
			}

			if ((_vhid_info::const_map_index_mask_additional_compositive & n_map_index_compositive) != 0) {
				b_compositive_type = true;
			}
			int n_primitive = _vhid_info::const_map_index_max & n_map_index_compositive;
			if (n_primitive > _vhid_info::const_map_index_max) {
				continue;
			}
			if (n_primitive < _vhid_info::const_map_index_min) {
				continue;
			}
			if (n_primitive % _vhid_info::const_map_index_inc_unit != 0) {
				continue;
			}

			n_primitive_map_index = n_primitive;
			//

		} while (false);
		return std::make_pair(n_primitive_map_index, b_compositive_type);
	}

	static int get_compositive_map_index_from_primitive_map_index(_mp::type_bm_dev type,int n_map_index_primitive)
	{
		return n_map_index_primitive + _vhid_info::_get_additional_value_of_compositive_map_index(type);
	}
public:
	_vhid_info() : m_b_none_blocking(false)
	{
	}

	virtual ~_vhid_info()
	{
	}

	/**
	* type compositive device can be open?
	*/
	virtual bool can_be_open(_mp::type_bm_dev type, bool b_exclusive_open) const = 0;

	virtual bool is_open(_mp::type_bm_dev type) const = 0;

	/**
	* @brief increase or decrease open counter of virtual device
	*
	* @return first-true(success adjustment)
	*
	*	second-adjusted counter
	*
	*	third-true(all open-counter of compositive type is zero - need primitve type is closed!)
	*/
	virtual std::tuple<bool, int, bool> ajust_open_cnt(_mp::type_bm_dev type, bool b_increase) = 0;

public:
	void set_none_blocking(bool b_none_blocking)
	{
		m_b_none_blocking = b_none_blocking;
	}

	bool get_none_blocking() const
	{
		return m_b_none_blocking;
	}

protected:
	/**
	* @brief this return value will be uesed to generate compositive type index
	*/
	static int _get_additional_value_of_compositive_map_index(_mp::type_bm_dev type)
	{
		int n_add(0);

		do {
			for (auto item : _vhid_info::get_extra_paths(
				_mp::_elpusk::const_usb_vid,
				_mp::_elpusk::_lpu237::const_usb_pid,
				_mp::_elpusk::_lpu237::const_usb_inf_hid
			)) {
				if (std::get<1>(item) == type) {
					n_add = std::get<3>(item);
					break;
				}
			}//end for
		} while (false);

		return n_add;
	}

protected:
	bool m_b_none_blocking;
};