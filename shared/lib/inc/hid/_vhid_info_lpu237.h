#pragma once

#include <memory>
#include <utility>
#include <map>
#include <array>
#include <mp_type.h>
#include <mp_elpusk.h>
#include <hid/_vhid_info.h>
#include <cprotocol_lpu237.h>


/**
* this class is virtual hid device information of lpu237.
* An instance of this class corresponds to a single physical device (primitive device).
*/
class _vhid_info_lpu237 : public _vhid_info
{
public:
	/**
	 * const_msr_extension is defined for encryption msr data.
	 * If msr data is started with  tripple const_msr_extension,
	 * then msr data has a extension format.
	 * const_msr_extension is defined also in firmware code as C_MSR_EXTENSION(msrbasepbj.h, himalia).
	 * extension format :
	 * 0 offset bytes - const_msr_extension
	 * 1 offset bytes - const_msr_extension
	 * 2 offset bytes - const_msr_extension
	 * 3 ~ 6 offset bytes - data field length with hex ascii string
	 * ( converted from little endian unsigned short type).
	*/
	enum : unsigned char {
		const_msr_extension = 0xe6 //(-26) msr data format is extension format(0xe6)
	};
	enum : unsigned long
	{
		const_size_min_ms_extension_response = 3 + 4, //triple prefix(3 bytes)  + data field length(4 bytes - hex string of unsigned shotr little-endian.
		const_size_min_ms_extension_response_of_data_field = 4
	};

	enum
	{
		const_size_random_number = 16,
		const_size_ksn = 10,
		const_size_mac4 = 4
	};

	enum {
		const_max_pre_pan_value = 6,
		const_max_post_pan_value = 4,
		const_max_authenticate_error = 5
	};

	enum {
		const_size_get_random_return = 20,
		const_size_security_key = 16
	};

	enum {
		const_size_encryption_key = 16
	};

	enum {
		const_buffer_size_security_buffer = 16
	};

public:
	typedef	std::shared_ptr<_vhid_info_lpu237>	type_ptr;

private:
	//first - string : compositive additioanl path
	//second - int : open counter
	//third - bool : true(support shared open), false(support exclusive open)
	//forth - int : additional value for generating compositive map index
	typedef std::tuple<std::string, int, bool, int> _type_tuple_info_type;

	//key is type , value is tuple of extra path, open counter, support shared open, additional value
	typedef	std::map<_mp::type_bm_dev,_vhid_info_lpu237::_type_tuple_info_type> _type_map_info_type;

public:
	//static member

	/**
	* @brief check that the received data is msr extension format( for encrypted data).
	* 
	*	may be the first 3 bytes is 0xe6, 0xe6, 0xe6.
	* 
	* @return true : msr extension format.
	*/
	static bool is_rx_msr_extension(const _mp::type_v_buffer& v_rx)
	{
		bool b_result(false);

		do {
			if (v_rx.size() < _vhid_info_lpu237::const_size_min_ms_extension_response) {
				continue;
			}
			if (v_rx[0] != _vhid_info_lpu237::const_msr_extension) {
				continue;
			}
			if (v_rx[1] != _vhid_info_lpu237::const_msr_extension) {
				continue;
			}
			if (v_rx[2] != _vhid_info_lpu237::const_msr_extension) {
				continue;
			}

			b_result = true;

		} while (false);
		return b_result;
	}
	/**
	* @brief check that the received data is i-button format.
	* 
	* @param v_rx - the received data.
	* 
	* @return 
	*	first - true(the received data is i-button format). false(not~)
	* 
	*	second - 8 bytes ibutton key code(by defined i-button itself) or remove-key code(by defined user. default are 0,0,0,0,0,0,0,0) .
	*/
	static std::pair<bool, _mp::type_v_buffer> is_rx_ibutton(const _mp::type_v_buffer& v_rx)
	{
		bool b_result(false);

		const std::string s_ibutton_postfix("this_is_ibutton_data");
		const size_t n_size_button_data(8);
		const size_t n_len_bytes = 3;
		_mp::type_v_buffer v_code(0);

		do {
			if (v_rx.size() < n_len_bytes + s_ibutton_postfix.size() + n_size_button_data) {
				continue;
			}
			if (v_rx[0] != 0 || v_rx[1] != 0 || v_rx[2] != 0) {
				continue;// ibutton 데이터는 1,2,3 track 의 길이를 나타내는 값이 모두 0 임.
			}
			//
			b_result = true;

			for (auto i = 0; i < s_ibutton_postfix.size(); i++) {
				if (v_rx[n_len_bytes + n_size_button_data + i] != s_ibutton_postfix[i]) {
					b_result = false;
					break;//exit for
				}
			}//end for

			if (b_result) {
				std::copy(v_rx.begin() + n_len_bytes, v_rx.begin() + n_len_bytes + n_size_button_data, std::back_inserter(v_code));
			}
			
		} while (false);
		return std::make_pair(b_result,v_code);
	}
	static bool is_rx_pair_txrx(const _mp::type_v_buffer& v_rx)
	{
		bool b_result(false);

		do {
			if (v_rx.size() < 2) {
				continue;
			}
			if (v_rx[0] != 'R') {
				continue;
			}
			//
			b_result = true;
		} while (false);
		return b_result;
	}

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
			//key is type , value is tuple of extra path, open counter, support shared open, additional value
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
	* @brief increase or decrease open counter of virtual device
	* 
	* @return first-true(success adjustment)
	* 
	*	second-adjusted counter
	* 
	*	third-true(all open-counter of compositive type is zero - need primitve type is closed!)
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

	virtual bool is_lpu237_device()
	{
		return true;
	}
private:
	/**
	* this set is const value. initialized in constructure. 
	* the set of lpu237 primitive and compositive data.
	* tuple.first - string : compositive additioanl path
	* tuple.second - type : device type
	* tuple.third - bool : true(support shared open), false(support exclusive open)
	* tuple.forth - int : additional value for generating compositive map index
	*/
	const _vhid_info::type_set_path_type m_const_v_extra;

	//key is type , value is tuple of extra path, open counter, support shared open, additional value
	_type_map_info_type m_map_info;

private://don't call these method
	_vhid_info_lpu237();
};