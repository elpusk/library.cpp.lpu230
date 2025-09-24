#pragma once

#include <cstdint>
#include <unordered_set>

#include <mp_type.h>
#include <mp_cversion.h>
#include <mp_icprotocol.h>
#include <mp_cconvert.h>
#include <mp_clog.h>
#include <ckey_map.h>

#include <lpu230/KBConst.h>
#include <lpu230/info_sys.h>
#include <lpu230/info_sys_cnst.h>


class cprotocol_lpu237 : public _mp::icprotocol
{
public:
	enum {
		msr_response_prefix = 'R'
	};
	enum : unsigned char {
		msr_warning_icc_inserted = 0xBA,	//( -70) for LPU-208D, inserted ICC
		msr_warning_icc_removed = 0xB9		//( -71) for LPU-208D, removed ICC
	};

	typedef	enum : unsigned char	//Declare MSR Response set
	{
		response_good = 0xFF,	//action done. good processing
		response_good_negative = 0x80,	//action done. negative processing

		response_error_crc = 0x01,	//CRC error
		response_error_mislength = 0x02,	//Mis-matching length of data field
		response_error_mismatching_key = 0x03,	//Mis-matching Key
		response_error_mischeck_block = 0x04,	//Wrong check block
		response_error_invalid = 0x05,	//invalied commad
		response_error_verify = 0x06	//failure verification.
	}type_msr_response;

private:
#ifdef	WIN32
#pragma pack(push,1)
#endif	//WIN32
	typedef	struct _tag_type_response
	{
		unsigned char c_prefix;
		type_msr_response c_result;
		unsigned char c_size;
		unsigned char s_data[1];
	}_PACK_BYTE _type_response;
#ifdef	WIN32
#pragma pack(pop)
#endif	//WIN32
public:
	//interface 
	~cprotocol_lpu237() {};
		
	//return tx packet.
	//v_out_packet[0] - report id
	_mp::icprotocol::type_get_tx_packet_result get_tx_packet(_mp::type_v_buffer& v_out_packet)
	{
		bool b_first(true);
		_mp::icprotocol::type_get_tx_packet_result result(gtp_result_more_packet_in_this_transaction);
		_mp::type_v_buffer v_report(0);

		v_out_packet.resize(0);

		do {
			result = get_tx_report(v_report, b_first);
			b_first = false;
			if (result == gtp_result_no_more_transaction)
				continue;

			if (v_report.size() > 0) {
				v_out_packet.resize(v_out_packet.size() + v_report.size());
				std::copy(std::begin(v_report), std::end(v_report), std::end(v_out_packet) - v_report.size());
			}
		} while (result == gtp_result_more_packet_in_this_transaction);
		return result;
	}
		
	//save rx packet.
	//v_in_packet[0] - report id
	type_get_rx_packet_result set_rx_packet(const _mp::type_v_buffer& v_in_packet)
	{
		return set_rx_report(v_in_packet, true);
	}

	size_t get_tx_transaction(_mp::type_v_buffer& v_out_packet)
	{
		do {
			v_out_packet.resize(0);
			if (m_dequeu_v_tx.empty())
				continue;
			v_out_packet = m_dequeu_v_tx.front();
			m_dequeu_v_tx.pop_front();
		} while (false);
		return m_dequeu_v_tx.size();
	}
	bool set_rx_transaction(const _mp::type_v_buffer& v_in_packet)
	{
		bool b_result(false);
		do {
			if (v_in_packet.empty())
				continue;
			m_dequeu_v_rx.push_back(v_in_packet);
			b_result = true;
		} while (false);
		return b_result;
	}

public:
	cprotocol_lpu237()
	{
		reset();
	}

	size_t size_tx_transaction() const
	{
		return m_dequeu_v_tx.size();
	}
	size_t size_rx_transaction() const
	{
		return m_dequeu_v_rx.size();
	}

	void clear_tx_transaction()
	{
		m_dequeu_v_tx.clear();
	}
	void clear_rx_transaction()
	{
		m_dequeu_v_rx.clear();
	}

	//object reset.
	void clear_transaction()
	{
		m_dequeu_v_tx.clear();
		m_dequeu_v_rx.clear();
		m_deque_generated_tx.clear();
	}

	void reset()
	{
		m_dw_scr_transaction_counter = 0;

		clear_transaction();

		m_set_change_parameter.clear();

		m_b_global_pre_postfix_send_condition = true;
		m_manufacture = mf_elpusk;

		m_v_uid.resize(0);
		m_device_function = fun_none;
		m_version.reset();
		m_version_structure.reset();

		//device information
		m_b_is_hid_boot = false;
		m_b_removed_key_map_table = false;

		///////////////////////////////
		//device parameters
		m_b_device_is_mmd1000 = false;
		m_b_device_is_ibutton_only = false;
		m_b_device_is_standard = false;

		m_interface = system_interface_usb_keyboard;
		m_dw_buzzer_frequency = default_buzzer_frequency;
		m_dw_boot_run_time = 15000;
		m_language_index = language_map_index_english;

		for (int i = 0; i < the_number_of_reack; i++ ) {
			m_b_enable_iso[i] = true;
			m_direction[i] = dir_bidectional;
		}//end for

		m_v_global_prefix.resize(0);
		m_v_global_postfix.resize(0);

		for (int i = 0; i < the_number_of_reack; i++) {
			m_v_private_prefix[i].resize(0);
			m_v_private_postfix[i].resize(0);
		}//end for

		//i-button
		m_v_prefix_ibutton.resize(0);
		m_v_postfix_ibutton.resize(0);
		m_b_f12_ibutton = false;
		m_b_zeros_ibutton = true;
		m_b_zeros_7times_ibutton = false;
		m_b_addmit_code_stick_ibutton = false;

		m_v_ibutton_remove.resize(0);

		m_v_prefix_ibutton_remove.resize(0);
		m_v_postfix_ibutton_remove.resize(0);

		for (int i = 0; i < the_size_of_system_blank; i++) {
			m_c_blank[i] = 0;
		}//end for

		//rs232
		m_v_prefix_uart.resize(0);
		m_v_postfix_uart.resize(0);
		//
		m_token_format = ef_decimal;
		m_v_name.resize(0);
	}

	uint32_t get_current_scr_transaction_counter() const
	{
		return m_dw_scr_transaction_counter;
	}

public:
	_mp::icprotocol::type_get_tx_packet_result get_tx_report(_mp::type_v_buffer& v_out_report_with_id, bool b_first_packet )
	{
		_mp::icprotocol::type_get_tx_packet_result result(gtp_result_no_more_transaction);
		static size_t n_remainder(0);
		static size_t n_offset(0);

		do {
			v_out_report_with_id.resize(0);

			//
			if (b_first_packet) {
				n_offset = n_remainder = 0;
				//the first send
				if (m_dequeu_v_tx.empty())
					continue;//gtp_result_no_more_transaction
				if (m_dequeu_v_tx.front().size() == 0) {
					m_dequeu_v_tx.pop_front();
					result = gtp_result_no_more_packet_in_this_transaction;
					continue;
				}
				//
				n_remainder = m_dequeu_v_tx.front().size();
				v_out_report_with_id.resize(65, 0); v_out_report_with_id.assign(v_out_report_with_id.size(), 0);

				if (n_remainder > (v_out_report_with_id.size() - 1)) {
					std::copy(std::begin(m_dequeu_v_tx.front()), std::begin(m_dequeu_v_tx.front()) + v_out_report_with_id.size() - 1, std::begin(v_out_report_with_id) + 1);
					n_offset += (v_out_report_with_id.size() - 1);
					n_remainder -= (v_out_report_with_id.size() - 1);
				}
				else {
					std::copy(std::begin(m_dequeu_v_tx.front()), std::end(m_dequeu_v_tx.front()), std::begin(v_out_report_with_id) + 1);
					n_offset = n_remainder = 0;
					m_dequeu_v_tx.pop_front();

				}
				if (n_remainder == 0) 	result = gtp_result_no_more_packet_in_this_transaction;
				else					result = gtp_result_more_packet_in_this_transaction;
				continue;
			}

			if (n_remainder == 0) {
				result = gtp_result_no_more_packet_in_this_transaction;
				continue;
			}
			//the partial send
			v_out_report_with_id.resize(65, 0); v_out_report_with_id.assign(v_out_report_with_id.size(), 0);
			if (n_remainder >= v_out_report_with_id.size()) {
				std::copy(std::begin(m_dequeu_v_tx.front()) + n_offset, std::begin(m_dequeu_v_tx.front()) + n_offset + v_out_report_with_id.size() - 1, std::begin(v_out_report_with_id) + 1);
				n_offset += (v_out_report_with_id.size() - 1);
				n_remainder -= (v_out_report_with_id.size() - 1);

				if (n_remainder == 0) 	result = gtp_result_no_more_packet_in_this_transaction;
				else					result = gtp_result_more_packet_in_this_transaction;
				continue;
			}

			// the last packet
			std::copy(std::begin(m_dequeu_v_tx.front()) + n_offset, std::end(m_dequeu_v_tx.front()), std::begin(v_out_report_with_id) + 1);
			n_offset = n_remainder = 0;
			m_dequeu_v_tx.pop_front();
			result = gtp_result_no_more_packet_in_this_transaction;
		} while (false);
		return result;
	}

	//v_in_report[0] - report id
	type_get_rx_packet_result set_rx_report(const _mp::type_v_buffer& v_in_report, bool b_first_packet)
	{
		type_get_rx_packet_result result(grp_result_unknown_packet);
		_mp::type_v_buffer v_rx(3 + 76 + 37 + 104, 0);
		do {
			if (v_in_report.size() != v_rx.size() + 1) {
				v_rx.resize(0);
				continue;//unknown report size
			}
			if (!b_first_packet) {
				v_rx.resize(0);
				continue;//unknown report size
			}

			std::copy(std::begin(v_in_report) + 1, std::end(v_in_report), std::begin(v_rx));
			if (v_rx[0] != msr_response_prefix) {
				v_rx.resize(0);
				continue;//unknown report size
			}
			//
			result = grp_result_complete_packet_for_this_transaction;
		} while (false);
		if (v_rx.size() > 0) {
			m_dequeu_v_rx.push_back(v_rx);
		}
		return result;
	}
private:
	enum _type_format {
		ef_decimal,
		ef_heximal,
		ef_ascii
	};

public:
	typedef	enum :unsigned char	//Declare host commnad set
	{
		cmd_change_authentificate_key = 'C',	//0x43 this command changes encryption key
									//that is used in encryption of Cmd_ChangeEnKey New Key field.
		cmd_change_encrypt_key = 'K',	//this command changes encryption key
								//that is used in encryption of magnetic card data.
		cmd_change_status = 'M',	//0x4D change status ( Wait-MS, No-Wait-MS )
		cmd_change_sn = 'S',	//this command updates serial number
		cmd_config = 'A',	//0x41 system configuration command
		cmd_apply = 'B',	//0x42 apply EEPROM data to system

		cmd_enter_cs = 'X',	//0x58 enter configuration mode.......
							// you must execute this command before do other command.
		cmd_leave_cs = 'Y',	//0x59 leave configuration mode.......
							// you must execute this command when your command processing.
		cmd_goto_boot_loader = 'G',	//goto boot loader.

		cmd_enter_opos = 'I',	//enter opos mode.
		cmd_leave_opos = 'J',	//leave opos mode.

		cmd_start_ibutton = 'F',	//start i-button. from callisto v3.24, ganymede v5.23, himalia version 2.4, europa version 1.2
		cmd_stop_ibutton = 'H',	//stop i-button. from callisto v3.24, ganymede v5.23, himalia version 2.4, europa version 1.2

		cmd_hw_is_standard = 'D',	//current model is standard.
		cmd_hw_is_only_ibutton = 'W',	//current model support only i-button.
		cmd_read_uid = 'U',	//read UID
		cmd_hw_is_mmd1000 = 'N',	//Hardware is MMD1000
		cmd_deb_interface = 'Z',	//debugging
		cmd_gen_head = 'E',	//generate head signal for magnetic card simulator.
		cmd_uart_bypass = 'T'	//bypass data to UART. supported in version 10.0 greater then equal

	}type_cmd;

	typedef enum  : unsigned char{

		request_config_set = 200,
		request_config_get = 201,
	}type_system_request_config;

	typedef enum{
		mf_elpusk,
		mf_btc
	}type_manufacturer;

	typedef enum{
		fun_none,
		fun_msr,
		fun_msr_ibutton,
		fun_ibutton
	}type_function;

	enum {
		the_number_of_reack = 3,
		the_size_of_uid = 4 * 4
	};
	enum {
		the_size_of_system_blank = 4,
		the_size_of_host_packet_header =3,	//the sum of size(cCmd, cSub ,cLen)
		the_size_of_out_report_except_id = 64,
		the_size_of_in_report_except_id = 220,
	};

	typedef enum{

		system_interface_usb_keyboard = 0,	//system interface is USB keyboard.
		system_interface_usb_msr = 1,	//system interface is USB MSR(generic HID interface).
		system_interface_uart = 10,	//system interface is uart.
		system_interface_ps2_stand_alone = 20,	//system interface is PS2 stand along mode.
		system_interface_ps2_bypass = 21,	//system interface is bypass mode.
		system_interface_by_hw_setting = 100,	//system interface is determined by HW Dip switch
	}type_system_interface;

	typedef enum{
		language_map_index_english = 0,//U.S English
		language_map_index_spanish = 1,
		language_map_index_danish = 2,
		language_map_index_french = 3,
		language_map_index_german = 4,
		language_map_index_italian = 5,
		language_map_index_norwegian = 6,
		language_map_index_swedish = 7,
		language_map_index_uk_english = 8,
		language_map_index_israel = 9,
		language_map_index_turkey = 10
	}type_keyboard_language_index;

	typedef enum{
		iso1_track = 0,
		iso2_track = 1,
		iso3_track = 2,
		iso_global = 10
	}type_msr_track_Numer ;

	typedef enum{
		ibutton_zeros = 0,
		ibutton_f12 = 1,
		ibutton_zeros7 = 2,
		ibutton_addmit = 3,
		ibutton_none = 4
	}type_ibutton_mode;

	enum {
		the_number_of_frequency = 22,
		the_frequency_of_off_buzzer = 5000,
		the_frequency_of_on_buzzer = 26000,
		the_number_of_support_language = 11,	//the number of supported language.

		address_system_hid_key_map_offset = 0x400,	//size 1K
		address_system_ps2_key_map_offset = 0x800,	//size 1K
		default_buzzer_frequency = 25000,	// default buzzer frequency.
		default_buzzer_frequency_for_wiznova_board = 16000	// default buzzer frequency. ganymede.

	};

	typedef enum  : unsigned char {//reading diection
		dir_bidectional = 0,	//reading direction forward & backward
		dir_forward = 1,	//reading direction forward
		dir_backward = 2	//reading direction backward
	}type_direction;

	typedef	_mp::type_v_buffer	type_tag;
	typedef	type_tag::iterator			type_tag_iter;
	typedef	type_tag::const_iterator	type_tag_const_iter;
	typedef	_mp::type_v_buffer	type_uid;
	typedef	_mp::type_v_buffer	type_name;

	typedef	_mp::cversion<unsigned char>	type_version;
		
private:
	//setter
	void _set_manufacture(const type_manufacturer manufacture) { m_manufacture = manufacture; }
	void _set_uid(const type_uid& v_uid) { m_v_uid = v_uid; }
	void _set_version( const cprotocol_lpu237::type_version& ver) { m_version = ver; }
	void _set_version_structure(const cprotocol_lpu237::type_version& ver) { m_version_structure = ver; }
	void _set_token_format(_type_format token_format) { m_token_format = token_format; }
	void _set_name(const cprotocol_lpu237::type_name& v_name) { m_v_name = v_name; }
	void _set_device_function(type_function device_function) { m_device_function = device_function; }
	void _set_hid_boot(bool b_hid_boot = false) { m_b_is_hid_boot = b_hid_boot; }
	void _set_device_is_mmd1000(bool b_device_is_mmd1000 = true) { m_b_device_is_mmd1000 = b_device_is_mmd1000; }
	void _set_device_is_standard(bool b_device_is_standard = true) { m_b_device_is_standard = b_device_is_standard; }
	void _set_device_is_ibutton_only(bool b_device_is_ibutton_only = true) { m_b_device_is_ibutton_only = b_device_is_ibutton_only; }
	void _set_removed_key_map_table(bool b_removed_key_map_table) { m_b_removed_key_map_table = b_removed_key_map_table; }

	enum _change_parameter {//public setter method
		cp_GlobalPrePostfixSendCondition = 0,
		cp_EnableiButton,
		cp_Interface,
		cp_BuzzerFrequency,
		cp_BootRunTime,
		cp_Language,
		cp_EnableISO1, cp_EnableISO2, cp_EnableISO3,
		cp_Direction1, cp_Direction2, cp_Direction3,
		cp_GlobalPrefix, cp_GlobalPostfix,
		cp_PrivatePrefix1, cp_PrivatePrefix2, cp_PrivatePrefix3,
		cp_PrivatePostfix1, cp_PrivatePostfix2, cp_PrivatePostfix3,
		cp_Prefix_iButton, cp_Postfix_iButton,
		cp_Prefix_Uart, cp_Postfix_Uart,
		cp_BtcConfigData, cp_EnableF12iButton, cp_EnableZerosiButton, cp_EnableZeros7TimesiButton, cp_EnableAddmitCodeStickiButton,
		cp_iButton_Remove,
		cp_Prefix_iButton_Remove, cp_Postfix_iButton_Remove
	};

	enum _generated_tx_type {//
		gt_read_uid = 0,
		gt_change_authkey,
		gt_change_status,
		gt_change_sn,
		gt_enter_config,
		gt_leave_config,
		gt_apply,
		gt_goto_boot,
		gt_enter_opos,
		gt_leave_opos,
		gt_start_ibutton,
		gt_stop_ibutton,
		gt_support_mmd1000,
		gt_bypass_uart,
		gt_type_ibutton,
		gt_type_device,

		gt_set_config,

		gt_get_version,
		gt_get_name,
		gt_get_global_prepostfix_send_condition,
		gt_get_interface,
		gt_get_language,
		gt_get_buzzer_frequency,
		gt_get_boot_run_time,
		gt_get_enable_iso1, gt_get_enable_iso2, gt_get_enable_iso3,
		gt_get_direction1, gt_get_direction2, gt_get_direction3,
		gt_get_global_prefix,  gt_get_global_postfix,
		gt_get_private_prefix1, gt_get_private_prefix2,  gt_get_private_prefix3,
		gt_get_private_postfix1,gt_get_private_postfix2, gt_get_private_postfix3,
		gt_get_prefix_ibutton, gt_get_postfix_ibutton,
		gt_get_prefix_uart, gt_get_postfix_uart,
		gt_get_f12_ibutton, gt_get_zeros_ibutton, gt_get_zeros7_times_ibutton, gt_get_addmit_code_stick_ibutton,
		gt_get_ibutton_remove,
		gt_get_prefix_ibutton_remove, gt_get_postfix_ibutton_remove,
		gt_get_version_structure
	};

	typedef	std::deque< _generated_tx_type >		_type_deque_generated_tx;
public:
	static bool get_track_data_from_rx(int n_iso_track, std::wstring& s_track, const _mp::type_v_buffer& v_rx)
	{
		bool b_result(false);
		int n_offset(0);
		int n_size(0);
		unsigned char cAdd(0x30);

		do {
			if (n_iso_track < 1 || n_iso_track > 3)
				continue;
			if (v_rx.size() < 3)
				continue;
			//
			s_track.clear();
			//
			signed char cIso[] = { (signed char)v_rx[0], (signed char)v_rx[1], (signed char)v_rx[2] };//get length or status.

			//
			switch (n_iso_track) {
			case 1:
				cAdd = 0x20;

				if (cIso[0] > 0) {
					n_offset = 3;
					n_size = cIso[0];	b_result = true;
				}
				else if (cIso[0] == 0) {
					s_track.clear();	b_result = true;
				}
				break;
			case 2:
				if (cIso[1] > 0) {
					n_offset = 3;

					if (cIso[0] > 0)
						n_offset += static_cast<int>(cIso[0]);

					n_size = cIso[1];	b_result = true;
				}
				else if (cIso[1] == 0) {
					s_track.clear();	b_result = true;
				}
				break;
			case 3:
				if (cIso[2] > 0) {
					n_offset = 3;
					if (cIso[0] > 0)
						n_offset += static_cast<int>(cIso[0]);
					if (cIso[1] > 0)
						n_offset += static_cast<int>(cIso[1]);

					n_size = cIso[2];	b_result = true;
				}
				else if (cIso[2] == 0) {
					s_track.clear();	b_result = true;
				}
				break;
			default:
				break;
			}//end switch

		} while (0);
		if (n_size > 0 && b_result) {
			std::for_each(std::begin(v_rx) + n_offset, std::begin(v_rx) + n_offset + n_size, [&](unsigned char cData) {
				s_track += static_cast<wchar_t>(cData + cAdd);
				});
		}

		return b_result;
	}
	bool is_changed_parameters()
	{
		if (m_set_change_parameter.empty())
			return false;
		else
			return true;
	}

	size_t get_generated_the_number_of_tx() const
	{
		return m_deque_generated_tx.size();
	}

	void set_global_pre_postfix_send_condition(bool b_all_no_error)
	{
		if (b_all_no_error != m_b_global_pre_postfix_send_condition) {
			m_b_global_pre_postfix_send_condition = b_all_no_error;
			_set_insert_change_set(cp_GlobalPrePostfixSendCondition);
		}
	}
	void set_interface(type_system_interface inf)
	{
		if (m_interface != inf) {
			m_interface = inf;
			_set_insert_change_set(cp_Interface);
		}
	}
	void set_buzzer_frequency(uint32_t dw_buzzer_frequency)
	{
		if (m_dw_buzzer_frequency != dw_buzzer_frequency) {
			m_dw_buzzer_frequency = dw_buzzer_frequency;
			_set_insert_change_set(cp_BuzzerFrequency);
		}
	}
	void set_boot_run_time(uint32_t dw_boot_run_time)
	{
		if (m_dw_boot_run_time != dw_boot_run_time) {
			m_dw_boot_run_time = dw_boot_run_time;
			_set_insert_change_set(cp_BootRunTime);
		}
	}
	void set_language(type_keyboard_language_index language_index)
	{
		if (m_language_index != language_index) {
			m_language_index = language_index;
			_set_insert_change_set(cp_Language);
		}
	}

	void set_enable_iso(type_msr_track_Numer track, bool b_enable)
	{
		int n_loop(0);
		int n_track(-1);
		_change_parameter cp(cp_EnableISO1);

		switch (track)
		{
		case iso1_track:	n_loop = 1;		n_track = 0;	cp = cp_EnableISO1;
			break;
		case iso2_track:	n_loop = 1;		n_track = 1;	cp = cp_EnableISO2;
			break;
		case iso3_track:	n_loop = 1;		n_track = 2;	cp = cp_EnableISO3;
			break;
		case iso_global:	n_loop = 3;		n_track = 0;	cp = cp_EnableISO1;
			break;
		default:
			break;
		}

		for (int i = 0; i < n_loop; i++) {
			if (m_b_enable_iso[n_track + i] != b_enable) {
				m_b_enable_iso[n_track + i] = b_enable;
				_set_insert_change_set(cp);
			}
			cp = (_change_parameter)((int)cp + 1);
		}//end for
	}

	void set_global_prefix(const type_tag & fix)
	{
		if (m_v_global_prefix != fix) {
			m_v_global_prefix = fix;
			_set_insert_change_set(cp_GlobalPrefix);
		}
	}

	void set_direction(const type_direction direction)
	{
		if (m_direction[0] != direction) {
			m_direction[0] = direction;
			m_direction[1] = direction;
			m_direction[2] = direction;
			_set_insert_change_set(cp_Direction1);
			_set_insert_change_set(cp_Direction2);
			_set_insert_change_set(cp_Direction3);
		}
	}

	void set_direction(type_msr_track_Numer track, const type_direction dir)
	{
		if (m_direction[track] != dir) {
			m_direction[track] = dir;
			switch (track) {
			case iso1_track:	_set_insert_change_set(cp_Direction1);	break;
			case iso2_track:	_set_insert_change_set(cp_Direction2);	break;
			case iso3_track:	_set_insert_change_set(cp_Direction3);	break;
			default:	break;
			}//end switch
		}
	}

	void set_global_postfix(const type_tag& fix)
	{
		if (m_v_global_postfix != fix) {
			m_v_global_postfix = fix;
			_set_insert_change_set(cp_GlobalPostfix);
		}
	}

	void set_private_prefix(type_msr_track_Numer track, const type_tag& fix)
	{
		if (m_v_private_prefix[track] != fix) {
			m_v_private_prefix[track] = fix;
			switch (track) {
			case iso1_track:	_set_insert_change_set(cp_PrivatePrefix1);	break;
			case iso2_track:	_set_insert_change_set(cp_PrivatePrefix2);	break;
			case iso3_track:	_set_insert_change_set(cp_PrivatePrefix3);	break;
			default:	break;
			}//end switch
		}
	}
	void set_private_postfix(type_msr_track_Numer track, const type_tag& fix)
	{
		if (m_v_private_postfix[track] != fix) {
			m_v_private_postfix[track] = fix;
			switch (track) {
			case iso1_track:	_set_insert_change_set(cp_PrivatePostfix1);	break;
			case iso2_track:	_set_insert_change_set(cp_PrivatePostfix2);	break;
			case iso3_track:	_set_insert_change_set(cp_PrivatePostfix3);	break;
			default:	break;
			}//end switch
		}
	}

	void set_prefix_ibutton(const type_tag& fix)
	{
		if (m_v_prefix_ibutton != fix) {
			m_v_prefix_ibutton = fix;
			_set_insert_change_set(cp_Prefix_iButton);
		}
	}
	void set_postfix_ibutton(const type_tag& fix)
	{
		if (m_v_postfix_ibutton != fix) {
			m_v_postfix_ibutton = fix;
			_set_insert_change_set(cp_Postfix_iButton);
		}
	}

	void set_prefix_uart(const type_tag& fix)
	{
		if (m_v_prefix_uart != fix) {
			m_v_prefix_uart = fix;
			_set_insert_change_set(cp_Prefix_Uart);
		}
	}
	void set_postfix_uart(const type_tag& fix)
	{
		if (m_v_postfix_uart != fix) {
			m_v_postfix_uart = fix;
			_set_insert_change_set(cp_Postfix_Uart);
		}
	}

	void set_enable_f12_ibutton(bool b_enable)
	{
		if (m_b_f12_ibutton != b_enable) {
			m_b_f12_ibutton = b_enable;
			_set_insert_change_set(cp_EnableF12iButton);
		}
	}

	void set_enable_zeros_ibutton(bool b_enable)
	{
		if (m_b_zeros_ibutton != b_enable) {
			m_b_zeros_ibutton = b_enable;
			_set_insert_change_set(cp_EnableZerosiButton);
		}
	}

	void set_enable_zeros_7times_ibutton(bool b_enable)
	{
		if (m_b_zeros_7times_ibutton != b_enable) {
			m_b_zeros_7times_ibutton = b_enable;
			_set_insert_change_set(cp_EnableZeros7TimesiButton);
		}
	}

	void set_enable_addmit_Code_stick_ibutton(bool b_enable)
	{
		if (m_b_addmit_code_stick_ibutton != b_enable) {
			m_b_addmit_code_stick_ibutton = b_enable;
			_set_insert_change_set(cp_EnableAddmitCodeStickiButton);
		}
	}

	void set_ibutton_remove(const type_tag& fix)
	{
		if (m_v_ibutton_remove != fix) {
			m_v_ibutton_remove = fix;
			_set_insert_change_set(cp_iButton_Remove);
		}
	}

	void set_prefix_ibutton_remove(const type_tag& fix)
	{
		if (m_v_prefix_ibutton_remove != fix) {
			m_v_prefix_ibutton_remove = fix;
			_set_insert_change_set(cp_Prefix_iButton_Remove);
		}
	}
	void set_postfix_ibutton_remove(const type_tag& fix)
	{
		if (m_v_postfix_ibutton_remove != fix) {
			m_v_postfix_ibutton_remove = fix;
			_set_insert_change_set(cp_Postfix_iButton_Remove);
		}
	}

	//getter
	cprotocol_lpu237::type_name get_name() const
	{
		cprotocol_lpu237::type_name v_name(m_v_name);
		return v_name;
	}

	bool get_global_pre_postfix_send_condition() const { return m_b_global_pre_postfix_send_condition; }
	bool get_device_is_mmd1000() const { return m_b_device_is_mmd1000; }

	type_manufacturer get_manufacture() const { return m_manufacture; }
	const type_uid & get_uid() const { return m_v_uid; }
	type_function get_device_function() const { return m_device_function; }
	cprotocol_lpu237::type_version get_system_version() const { return  m_version; }
	cprotocol_lpu237::type_version get_structure_version() const { return  m_version_structure; }

	bool get_removed_key_map_table() const { return m_b_removed_key_map_table; }
	bool get_hid_boot() const { return m_b_is_hid_boot; }

	bool get_device_is_ibutton_only() const { return m_b_device_is_ibutton_only; }
	bool get_device_is_standard() { return m_b_device_is_standard; }
	type_system_interface get_interface() const { return m_interface; }
	uint32_t get_buzzer_frequency() const { return m_dw_buzzer_frequency; }
	uint32_t get_boot_run_time() const { return m_dw_boot_run_time; }
	type_keyboard_language_index get_language() const { return m_language_index; }
	bool get_enable_iso(type_msr_track_Numer track) const
	{ 
		bool b_enable(false);

		switch (track) {
		case iso1_track:	b_enable = m_b_enable_iso[0];	break;
		case iso2_track:	b_enable = m_b_enable_iso[1];	break;
		case iso3_track:	b_enable = m_b_enable_iso[2];	break;
		default:	break;
		}//end switch

		return b_enable;
	}

	const type_direction get_direction(type_msr_track_Numer track) const { return m_direction[track]; }
	const type_direction get_direction() const { return m_direction[0]; }

	const type_tag& get_global_prefix() const { return m_v_global_prefix; }
	const type_tag& get_global_postfix() const { return m_v_global_postfix; }

	const type_tag& get_private_prefix(type_msr_track_Numer track) const { return m_v_private_prefix[track]; }
	const type_tag& get_private_postfix(type_msr_track_Numer track) const { return m_v_private_postfix[track]; }

	const type_tag& get_prefix_ibutton() const { return m_v_prefix_ibutton; }
	const type_tag& get_postfix_ibutton() const { return m_v_postfix_ibutton; }

	const type_tag& get_prefix_uart() const { return m_v_prefix_uart; }
	const type_tag& get_postfix_uart() const { return m_v_postfix_uart; }

	bool get_enable_f12_ibutton() const { return m_b_f12_ibutton; }
	bool get_enable_zeros_ibutton() const { return m_b_zeros_ibutton; }
	bool get_enable_zeros_7times_ibutton() const { return m_b_zeros_7times_ibutton; }
	bool get_enable_addmit_code_stick_ibutton() const { return m_b_addmit_code_stick_ibutton; }

	const type_tag& get_ibutton_remove() const { return m_v_ibutton_remove; }

	const type_tag& get_prefix_ibutton_remove() const { return m_v_prefix_ibutton_remove; }
	const type_tag& get_postfix_ibutton_remove() const { return m_v_postfix_ibutton_remove; }

	//IO data generates function
	//using call these funtions and receive report and call set_rx_packet and call is_success_x
	bool generate_bypass_uart(const _mp::type_v_buffer& v_tx)
	{
		bool b_result(false);
		_mp::type_v_buffer _v_tx(the_size_of_out_report_except_id - the_size_of_host_packet_header, 0);
		unsigned char c_chain(0);
		uint32_t dw_total((uint32_t)v_tx.size());
		uint32_t n_remainder(dw_total);
		uint32_t dw_offset(0);
		uint32_t n_data(0);
		uint32_t n_tx(0);
		uint32_t dw_scr_transaction_counter(_get_new_scr_transaction_counter());

		memcpy(&_v_tx[0], &dw_scr_transaction_counter, sizeof(uint32_t));
		memcpy(&_v_tx[sizeof(uint32_t)], &dw_total, sizeof(uint32_t));

		n_data = (uint32_t)_v_tx.size() - (sizeof(uint32_t) * 2);
		if (n_data > (uint32_t)v_tx.size())
			n_data = (uint32_t)v_tx.size();
		memcpy(&_v_tx[sizeof(uint32_t) * 2], &v_tx[dw_offset], n_data);
		n_tx = n_data + (sizeof(uint32_t) * 2);
		do {
			b_result = _generate_request(cmd_uart_bypass, c_chain, n_tx, &_v_tx[0]);
			if (!b_result) {
				break;//exit while
			}
			else {
				m_deque_generated_tx.push_back(cprotocol_lpu237::gt_bypass_uart);
			}

			dw_offset += n_data;
			n_remainder -= n_data;
			c_chain++;

			if (n_remainder > 0) {
				n_data = (uint32_t)_v_tx.size() - sizeof(uint32_t);
				if (n_data > n_remainder)
					n_data = n_remainder;
				n_tx = n_data + sizeof(uint32_t);

				std::fill(std::begin(_v_tx), std::end(_v_tx), 0);
				memcpy(&_v_tx[0], &dw_scr_transaction_counter, sizeof(uint32_t));
				memcpy(&_v_tx[sizeof(uint32_t)], &v_tx[dw_offset], n_data);
			}
		} while (n_remainder > 0);

		if (!b_result) {
			clear_transaction();
		}
		return b_result;
	}

	bool generate_start_ibutton()
	{
		if (_generate_request(cmd_start_ibutton, 0, 0, NULL)) {
			m_deque_generated_tx.push_back(gt_start_ibutton);
			return true;
		}
		return false;
	}

	bool is_success_start_ibutton()
	{
		return is_success_rx();
	}

	bool generate_stop_ibutton()
	{
		if (_generate_request(cmd_start_ibutton, 0, 0, NULL)) {
			m_deque_generated_tx.push_back(gt_stop_ibutton);
			return true;
		}
		return false;
	}

	bool is_success_stop_ibutton()
	{
		return is_success_rx();
	}

	bool generate_enter_opos_mode()
	{
		if (_generate_request(cmd_enter_opos, 0, 0, NULL)) {
			m_deque_generated_tx.push_back(gt_enter_opos);
			return true;
		}
		return false;
	}
	bool is_success_enter_opos_mode()
	{
		return is_success_rx();
	}

	bool generate_leave_opos_mode()
	{
		if (_generate_request(cmd_leave_opos, 0, 0, NULL)) {
			m_deque_generated_tx.push_back(gt_leave_opos);
			return true;
		}
		return false;
	}
	bool is_success_leave_opos_mode()
	{
		return is_success_rx();
	}

	bool generate_enter_config_mode()
	{
		if (_generate_request(cmd_enter_cs, 0, 0, NULL)) {
			m_deque_generated_tx.push_back(gt_enter_config);
			return true;
		}
		return false;
	}
	bool is_success_enter_config_mode()
	{
		return is_success_rx();
	}

	bool generate_leave_config_mode()
	{
		if (_generate_request(cmd_leave_cs, 0, 0, NULL)) {
			m_deque_generated_tx.push_back(gt_leave_config);
			return true;
		}
		return false;
	}
	bool is_success_leave_config_mode()
	{
		return is_success_rx();
	}

	bool generate_apply_config_mode()
	{
		if (_generate_request(cmd_apply, 0, 0, NULL)) {
			m_deque_generated_tx.push_back(gt_apply);
			return true;
		}
		return false;
	}
	bool is_success_apply_config_mode()
	{
		return is_success_rx();
	}

	bool generate_run_boot_loader()
	{
		if (_generate_request(cmd_goto_boot_loader, 0, 0, NULL)) {
			m_deque_generated_tx.push_back(gt_goto_boot);
			return true;
		}
		return false;
	}
	bool is_success_run_boot_loader()
	{
		return is_success_rx();
	}

	bool generate_get_uid()
	{
		bool b_result(false);

		do {
			if (!_generate_get_uid())//must be second
				continue;
			b_result = true;
		} while (false);
		return b_result;
	}

	bool generate_get_system_information()
	{
		bool b_result(false);

		do {
			if (!generate_enter_config_mode())//must be first
				continue;
			if (!_generate_get_version())//must be second
				continue;
			if (!_generate_get_device_type())//must be third
				continue;
			if (!_generate_get_version_strcuture())// must be ?
				continue;
			if (!generate_leave_config_mode())//must be last.
				continue;
			b_result = true;
		} while (false);
		return b_result;
	}
	bool generate_get_system_information_with_name()
	{
		bool b_result(false);

		do {
			if (!generate_enter_config_mode())//must be first
				continue;
			if (!_generate_get_version())//must be second
				continue;
			if (!_generate_get_device_type())//must be third
				continue;
			if (!_generate_get_version_strcuture())// must be ?
				continue;
			if (!_generate_get_name())
				continue;
			if (!generate_leave_config_mode())//must be last.
				continue;
			b_result = true;
		} while (false);
		return b_result;
	}

	bool generate_get_parameters()
	{
		bool b_result(false);

		std::vector< type_msr_track_Numer> v_track = { iso1_track ,iso2_track ,iso3_track };
		do {
			if (!generate_enter_config_mode())//must be first
				continue;

			//m_version, m_b_device_is_standard and m_b_device_is_ibutton_only settting by get_system_info.
			if (m_b_device_is_standard) {
				if (m_version > cprotocol_lpu237::type_version(3, 6, 0, 4)) {//i-button support from greater then v3.6.0.4
					if (!_generate_get_device_ibutton_type())
						continue;
				}
			}

			if (m_version > cprotocol_lpu237::type_version(3, 6, 0, 4)) {
				//i-button support from greater then v3.6.0.4
				if (!_generate_get_uid())
					continue;
			}

			if (!_generate_get_name())
				continue;

			/////////////////////////////////////////////////////////////////////
			// setting detail
			if (!_generate_get_global_pre_postfix_send_condition())
				continue;
			if (!_generate_get_device_support_mmd1000())	// . get support MMD1000 decoder.
				continue;
			if (!_generate_get_interface())		// . get interface
				continue;
			if (!_generate_get_language())		// . get language
				continue;
			if (!_generate_get_buzzer_frequency())		// . get buzzer
				continue;
			if (!_generate_get_boot_run_time())		// . get boot run time
				continue;

			b_result = true;
			for (auto track : v_track) {
				if (!_generate_get_direction(track)) {
					b_result = false;
					break;
				}//end for
			}
			if (!b_result)
				continue;

			b_result = false;

			if (m_version > cprotocol_lpu237::type_version(3, 0, 0, 0)) {
				if (!_generate_get_ibutton_prefix())		// . get iButton Pretag
					continue;
				if (!_generate_get_ibutton_postfix())		// . get iButton Posttag
					continue;
				if (!_generate_get_uart_prefix())		// . get Uart Pretag
					continue;
				if (!_generate_get_uart_postfix())		// . get Uart Posttag
					continue;

				if (!_generate_get_f12_ibutton())
					continue;
				if (!_generate_get_zeros_ibutton())
					continue;

				if (m_version < cprotocol_lpu237::type_version(4, 0, 0, 0)) {
					if (m_version > cprotocol_lpu237::type_version(3, 15, 0, 0)) {
						if (!_generate_get_zeros_7times_ibutton())
							continue;
					}

					if (m_version > cprotocol_lpu237::type_version(3, 16, 0, 0)) {
						if (!_generate_get_addmit_code_stick_ibutton())
							continue;
					}
				}
				else {
					if (m_version > cprotocol_lpu237::type_version(5, 7, 0, 0)) {
						if (!_generate_get_zeros_7times_ibutton())
							continue;
					}

					if (m_version > cprotocol_lpu237::type_version(5, 8, 0, 0)) {
						if (!_generate_get_addmit_code_stick_ibutton())
							continue;
					}
				}

				//
				if (m_version_structure >= cprotocol_lpu237::type_version(4, 0, 0, 0)) {
					if (!_generate_get_ibutton_remove())		// . get iButton remove
						continue;
					if (!_generate_get_ibutton_prefix_remove())		// . get iButton Pretag remove
						continue;
					if (!_generate_get_ibutton_postfix_remove())		// . get iButton Posttag remove
						continue;
				}
			}

			b_result = true;
			for (auto track : v_track) {
				if (!_generate_get_enable_track(track)) {	//enable track.
					b_result = false;
					break;
				}
				if (!_generate_get_private_prefix(track)){		// . private prefix 1
					b_result = false;
					break;
				}
				if (!_generate_get_private_postfix(track)){	// . private postfix 1
					b_result = false;
					break;
				}
			}//end for
			if (!b_result)
				continue;
			b_result = false;

			if (!_generate_get_global_prefix())		// . global prefix
				continue;
			if (!_generate_get_global_postfix())		// . global postfix
				continue;
			//
			if (!generate_leave_config_mode())
				continue;
			//
			b_result = true;
		} while (false);
		return b_result;
	}

	void set_all_parameter_to_changed()
	{
		// . set iButton Pretag
		m_set_change_parameter.insert(cp_Prefix_iButton);
		m_set_change_parameter.insert(cp_Postfix_iButton);
		m_set_change_parameter.insert(cp_Prefix_Uart);
		m_set_change_parameter.insert(cp_Postfix_Uart);
		m_set_change_parameter.insert(cp_EnableF12iButton);
		m_set_change_parameter.insert(cp_EnableZerosiButton);
		m_set_change_parameter.insert(cp_EnableZeros7TimesiButton);
		m_set_change_parameter.insert(cp_EnableAddmitCodeStickiButton);
		m_set_change_parameter.insert(cp_iButton_Remove);
		m_set_change_parameter.insert(cp_Prefix_iButton_Remove);
		m_set_change_parameter.insert(cp_Postfix_iButton_Remove);
		m_set_change_parameter.insert(cp_GlobalPrePostfixSendCondition);
		m_set_change_parameter.insert(cp_Interface);
		m_set_change_parameter.insert(cp_Language);
		m_set_change_parameter.insert(cp_BuzzerFrequency);
		m_set_change_parameter.insert(cp_EnableISO1);
		m_set_change_parameter.insert(cp_EnableISO2);
		m_set_change_parameter.insert(cp_EnableISO3);
		m_set_change_parameter.insert(cp_Direction1);
		m_set_change_parameter.insert(cp_Direction2);
		m_set_change_parameter.insert(cp_Direction3);
		m_set_change_parameter.insert(cp_GlobalPrefix);
		m_set_change_parameter.insert(cp_GlobalPostfix);
		m_set_change_parameter.insert(cp_PrivatePrefix1);
		m_set_change_parameter.insert(cp_PrivatePostfix1);
		m_set_change_parameter.insert(cp_PrivatePrefix2);
		m_set_change_parameter.insert(cp_PrivatePostfix2);
		m_set_change_parameter.insert(cp_PrivatePrefix3);
		m_set_change_parameter.insert(cp_PrivatePostfix3);
	}

	bool generate_set_parameters()
	{
		bool b_result(false);

		do{
			if (!generate_enter_config_mode())
				continue;
			//
			if (m_version > cprotocol_lpu237::type_version(3, 0, 0, 0)) {
				// . set device type.
				if (!_generate_set_device_type())
					continue;

				// . set iButton Pretag
				if (m_set_change_parameter.find(cp_Prefix_iButton) != m_set_change_parameter.end())
					if (!_generate_set_ibutton_prefix())
						continue;

				// . set iButton Posttag
				if (m_set_change_parameter.find(cp_Postfix_iButton) != m_set_change_parameter.end())
					if(!_generate_set_ibutton_postfix())
						continue;

				// . set Uart Pretag
				if (m_set_change_parameter.find(cp_Prefix_Uart) != m_set_change_parameter.end())
					if(!_generate_set_uart_prefix())
						continue;

				// . set Uart Posttag
				if (m_set_change_parameter.find(cp_Postfix_Uart) != m_set_change_parameter.end())
					if(!_generate_set_uart_postfix())
						continue;

				do {//ibutton setting
					if (m_set_change_parameter.find(cp_EnableF12iButton) != m_set_change_parameter.end()) {
						_generate_set_f12_ibutton();
						continue;
					}

					if (m_set_change_parameter.find(cp_EnableZerosiButton) != m_set_change_parameter.end()) {
						_generate_set_zeros_ibutton();
						continue;
					}

					if (m_set_change_parameter.find(cp_EnableZeros7TimesiButton) != m_set_change_parameter.end()) {
						_generate_set_zeros_7times_ibutton();
						continue;
					}

					if (m_set_change_parameter.find(cp_EnableAddmitCodeStickiButton) != m_set_change_parameter.end()) {
						_generate_set_addmit_code_stick();
						continue;
					}
				} while (false);

				if (m_version_structure >= cprotocol_lpu237::type_version(4, 0, 0, 0)) {
					// . set iButton  remove
					if (m_set_change_parameter.find(cp_iButton_Remove) != m_set_change_parameter.end())
						if (!_generate_set_ibutton_remove())
							continue;

					// . set iButton Pretag remove
					if (m_set_change_parameter.find(cp_Prefix_iButton_Remove) != m_set_change_parameter.end())
						if (!_generate_set_ibutton_prefix_remove())
							continue;

					// . set iButton Posttag remove
					if (m_set_change_parameter.find(cp_Postfix_iButton_Remove) != m_set_change_parameter.end())
						if (!_generate_set_ibutton_postfix_remove())
							continue;
				}
			}
			//. set globalPrePostfixSendCondition
			if (m_set_change_parameter.find(cp_GlobalPrePostfixSendCondition) != m_set_change_parameter.end())
				if(!_generate_set_global_pre_postfix_send_condition())
					continue;

			// . set interface
			if (m_set_change_parameter.find(cp_Interface) != m_set_change_parameter.end())
				if(!_generate_set_interface())
					continue;

			// . get language
			if (m_set_change_parameter.find(cp_Language) != m_set_change_parameter.end()) {
				if(!_generate_set_language())
					continue;
				//set key map
				if (get_removed_key_map_table())
					if(!_generate_set_key_map())
						continue;
			}

			// . set buzzer
			if (m_set_change_parameter.find(cp_BuzzerFrequency) != m_set_change_parameter.end())
				if(!_generate_set_buzzer_frequency())
					continue;

			// .enable 1
			if (m_set_change_parameter.find(cp_EnableISO1) != m_set_change_parameter.end())
				if (!_generate_set_enable_track(iso1_track))
					continue;
			// .enable 2
			if (m_set_change_parameter.find(cp_EnableISO2) != m_set_change_parameter.end())
				if (!_generate_set_enable_track(iso2_track))
					continue;
			// .enable 3
			if (m_set_change_parameter.find(cp_EnableISO3) != m_set_change_parameter.end())
				if (!_generate_set_enable_track(iso3_track))
					continue;

			// direction 1
			if (m_set_change_parameter.find(cp_Direction1) != m_set_change_parameter.end())
				if (!_generate_set_direction(iso1_track))
					continue;
			if (m_set_change_parameter.find(cp_Direction2) != m_set_change_parameter.end())
				if (!_generate_set_direction(iso2_track))
					continue;
			if (m_set_change_parameter.find(cp_Direction3) != m_set_change_parameter.end())
				if (!_generate_set_direction(iso3_track))
					continue;


			// . global prefix
			if (m_set_change_parameter.find(cp_GlobalPrefix) != m_set_change_parameter.end())
				if (!_generate_set_global_prefix())
					continue;

			// . global postfix
			if (m_set_change_parameter.find(cp_GlobalPostfix) != m_set_change_parameter.end())
				if (!_generate_set_global_postfix())
					continue;

			// . private prefix 1
			if (m_set_change_parameter.find(cp_PrivatePrefix1) != m_set_change_parameter.end())
				if (!_generate_set_private_prefix(iso1_track))
					continue;
			// . private postfix 1
			if (m_set_change_parameter.find(cp_PrivatePostfix1) != m_set_change_parameter.end())
				if (!_generate_set_private_postfix(iso1_track))
					continue;

			// . private prefix 2
			if (m_set_change_parameter.find(cp_PrivatePrefix2) != m_set_change_parameter.end())
				if (!_generate_set_private_prefix(iso2_track))
					continue;
			// . private postfix 2
			if (m_set_change_parameter.find(cp_PrivatePostfix2) != m_set_change_parameter.end())
				if (!_generate_set_private_postfix(iso2_track))
					continue;

			// . private prefix 3
			if (m_set_change_parameter.find(cp_PrivatePrefix3) != m_set_change_parameter.end())
				if (!_generate_set_private_prefix(iso3_track))
					continue;
			// . private postfix 3
			if (m_set_change_parameter.find(cp_PrivatePostfix3) != m_set_change_parameter.end())
				if (!_generate_set_private_postfix(iso3_track))
					continue;

			//
			if (!generate_apply_config_mode())
				continue;
			if (!generate_leave_config_mode())
				continue;
			//
			b_result = true;
			m_set_change_parameter.clear();
		}while (false);
		return b_result;
	}

	bool is_success_rx(_mp::type_v_buffer& out_v_response,bool b_remove = true)
	{
		bool b_result(false);
		do {
			out_v_response.resize(0);

			if (m_dequeu_v_rx.empty())
				continue;
			if (m_dequeu_v_rx.front().size() != (3 + 76 + 37 + 104)) {
				m_dequeu_v_rx.pop_front();
				continue;
			}
			if (m_dequeu_v_rx.front()[0] != msr_response_prefix) {
				m_dequeu_v_rx.pop_front();
				continue;
			}

			b_result = true;
			out_v_response = m_dequeu_v_rx.front();
			if(b_remove)
				m_dequeu_v_rx.pop_front();

			if (out_v_response[1] == response_good)
				continue;
			if (out_v_response[1] == response_good_negative)
				continue;
			out_v_response.resize(0);
			b_result = false;
		} while (false);
		return b_result;
	}

	bool get_response_result(type_msr_response & out_result, bool b_remove = true)
	{
		bool b_result(false);
		do {
			out_result = response_good;

			if (m_dequeu_v_rx.empty())
				continue;
			if (m_dequeu_v_rx.front().size() != (3 + 76 + 37 + 104))
				continue;
			if (m_dequeu_v_rx.front()[0] != msr_response_prefix)
				continue;

			switch ((type_msr_response)m_dequeu_v_rx.front()[1]) {
				case response_good:
				case response_good_negative:
				case response_error_crc:
				case response_error_mislength:
				case response_error_mismatching_key:
				case response_error_mischeck_block:
				case response_error_invalid:
				case response_error_verify:
					out_result = (type_msr_response) m_dequeu_v_rx.front()[1];
					break;
				default:
					continue;
			}
			b_result = true;
		} while (false);

		if (!m_dequeu_v_rx.empty()) {
			if (b_remove)
				m_dequeu_v_rx.pop_front();
		}

		return b_result;
	}
	bool is_success_rx(bool b_remove = true)
	{
		bool b_result(false);
		do {
			if (m_dequeu_v_rx.empty())
				continue;
			if (m_dequeu_v_rx.front().size() != (3 + 76 + 37 + 104))
				continue;
			if (m_dequeu_v_rx.front()[0] != msr_response_prefix)
				continue;

			b_result = true;
			if (m_dequeu_v_rx.front()[1] == response_good)
				continue;
			if (m_dequeu_v_rx.front()[1] == response_good_negative)
				continue;
			b_result = false;
		} while (false);

		if (!m_dequeu_v_rx.empty()) {
			if(b_remove)
				m_dequeu_v_rx.pop_front();
		}

		return b_result;
	}

	bool set_from_rx()
	{
		bool b_result(false);
		do {
			if (m_deque_generated_tx.empty()) {
				_mp::clog::get_instance().log_fmt_in_debug_mode(L" : DEB : %ls : error : m_deque_generated_tx.empty().\n", __WFUNCTION__);
				continue;
			}

			_generated_tx_type tx_type(m_deque_generated_tx.front());
			m_deque_generated_tx.pop_front();
			switch (tx_type) {
			case gt_get_version://the first setting for coding rule.
				b_result = _set_version_by_rx();
				break;
			case gt_type_device://the second setting for coding rule.
				b_result = _set_device_type_by_rx();
				if (!b_result)
					break;
				if (!m_b_device_is_standard) {
					_set_device_function(type_function::fun_msr);//compact model
					break;
				}
				_set_device_function(type_function::fun_msr_ibutton);//standard model
				break;
			case gt_type_ibutton://the third setting for coding rule.
				b_result = _set_device_ibutton_type_by_rx();
				if (!b_result)
					break;
				if(m_b_device_is_ibutton_only)
					_set_device_function(type_function::fun_ibutton);//i-button only model
				break;
			case gt_get_version_structure://the ? setting for coding rule.
				b_result = _set_version_structure_by_rx();
				break;
			case gt_read_uid:	
				b_result = _set_uid_by_rx();	
				break;
			case gt_change_authkey:
			case gt_change_status:
			case gt_change_sn:
				b_result = is_success_rx();
				break;
			case gt_enter_config:
				b_result = is_success_enter_config_mode();
				break;
			case gt_leave_config:
				b_result = is_success_leave_config_mode();
				break;
			case gt_apply:
				b_result = is_success_apply_config_mode();
				break;
			case gt_goto_boot:
				b_result = is_success_run_boot_loader();
				break;
			case gt_enter_opos:
				b_result = is_success_enter_opos_mode();
				break;
			case gt_leave_opos:
				b_result = is_success_leave_opos_mode();
				break;
			case gt_start_ibutton:
				b_result = is_success_start_ibutton();
				break;
			case gt_stop_ibutton:
				b_result = is_success_stop_ibutton();
				break;
			case gt_support_mmd1000:
				b_result = _set_device_support_mmd1000_by_rx();
				break;
			case gt_get_name:
				b_result = _set_name_by_rx();
				break;
			case gt_get_global_prepostfix_send_condition:
				b_result = _set_global_pre_postfix_send_condition_by_rx();
				break;
			case gt_get_interface:
				b_result = _set_interface_by_rx();
				break;
			case gt_get_language:
				b_result = _set_language_by_rx();
				break;
			case gt_get_buzzer_frequency:
				b_result = _set_buzzer_frequency_by_rx();
				break;
			case gt_get_boot_run_time:
				b_result = _set_boot_run_time_by_rx();
				break;
			case gt_get_enable_iso1:
				b_result = _set_enable_track_by_rx(iso1_track);
				break;
			case gt_get_enable_iso2:
				b_result = _set_enable_track_by_rx(iso2_track);
				break;
			case gt_get_enable_iso3:
				b_result = _set_enable_track_by_rx(iso3_track);
				break;
			case gt_get_direction1:
				b_result = _set_direction_by_rx(iso1_track);
				break;
			case gt_get_direction2:
				b_result = _set_direction_by_rx(iso2_track);
				break;
			case gt_get_direction3:
				b_result = _set_direction_by_rx(iso3_track);
				break;
			case gt_get_global_prefix:
				b_result = _set_global_prefix_by_rx();
				break;
			case gt_get_global_postfix:
				b_result = _set_global_postfix_by_rx();
				break;
			case gt_get_private_prefix1:
				b_result = _set_private_prefix_by_rx(iso1_track);
				break;
			case gt_get_private_prefix2:
				b_result = _set_private_prefix_by_rx(iso2_track);
				break;
			case gt_get_private_prefix3:
				b_result = _set_private_prefix_by_rx(iso3_track);
				break;
			case gt_get_private_postfix1:
				b_result = _set_private_postfix_by_rx(iso1_track);
				break;
			case gt_get_private_postfix2:
				b_result = _set_private_postfix_by_rx(iso2_track);
				break;
			case gt_get_private_postfix3:
				b_result = _set_private_postfix_by_rx(iso3_track);
				break;
			case gt_get_prefix_ibutton:
				b_result = _set_ibutton_prefix_by_rx();
				break;
			case gt_get_postfix_ibutton:
				b_result = _set_ibutton_postfix_by_rx();
				break;
			case gt_get_prefix_uart:
				b_result = _set_uart_prefix_by_rx();
				break;
			case gt_get_postfix_uart:
				b_result = _set_uart_postfix_by_rx();
				break;
			case gt_get_f12_ibutton:
				b_result = _set_f12_ibutton_by_rx();
				break;
			case gt_get_zeros_ibutton:
				b_result = _set_zeros_ibutton_by_rx();
				break;
			case gt_get_zeros7_times_ibutton:
				b_result = _set_zeros_7times_ibutton_by_rx();
				break;
			case gt_get_addmit_code_stick_ibutton:
				b_result = _set_addmit_code_stick_ibutton_by_rx();
				break;
			case gt_get_ibutton_remove:
				b_result = _set_ibutton_remove_by_rx();
				break;
			case gt_get_prefix_ibutton_remove:
				b_result = _set_ibutton_prefix_remove_by_rx();
				break;
			case gt_get_postfix_ibutton_remove:
				b_result = _set_ibutton_postfix_remove_by_rx();
				break;
			case gt_set_config:
			case gt_bypass_uart:
				b_result = is_success_rx();
				break;
			default:
				continue;
			}
		} while (false);
		return b_result;

	}
private:
	void _clear_generated_tx_info()
	{
		m_deque_generated_tx.clear();
	}
	void _set_insert_change_set(_change_parameter para)
	{
		//_ATLTRACE_(L"..ChangeSet..\r\n");
		m_set_change_parameter.insert(para);
	}

	//set by response data.
	///////////////////////////////////////////////////////////////////////////////
	bool _set_version_by_rx()
	{
		bool b_result(false);

		do {
			_mp::type_v_buffer v_response;
			if (!is_success_rx(v_response))
				continue;
			_type_response* p_response = (_type_response*)&v_response[0];
			if (p_response->c_size != sizeofstructmember(SYSINFO, sSysVer))
				continue;
			m_version = cprotocol_lpu237::type_version(p_response->s_data[0], p_response->s_data[1], p_response->s_data[2], p_response->s_data[3]);

			//
			if (m_version > cprotocol_lpu237::type_version(1, 1, 0, 0)) {
				cprotocol_lpu237::type_version hid_ver(2, 2, 0, 0);

				if (m_version > cprotocol_lpu237::type_version(2, 2, 0, 0)) {
					_set_hid_boot(true);

					if (m_version > cprotocol_lpu237::type_version(3, 0, 0, 0)) {
						// From FW version 3.4.0.1, Keymapping table was removed from firmware.
						// therefor less then equal version 3.3.0.2, Don't call SetKeyMapToDevice() method of CMsrDevice class. 

						if (m_version > cprotocol_lpu237::type_version(3, 3, 0, 2)) {
							_set_removed_key_map_table(true);
						}
					}
				}
			}
			else {
				m_version.reset();
				continue;
			}

			b_result = true;
		} while (false);
		return b_result;
	}
	bool _set_version_structure_by_rx()
	{
		bool b_result(false);

		do {
			_mp::type_v_buffer v_response;
			if (!is_success_rx(v_response))
				continue;
			_type_response* p_response = (_type_response*)&v_response[0];
			if (p_response->c_size != sizeofstructmember(SYSINFO, sStrucVer))
				continue;
			m_version_structure = cprotocol_lpu237::type_version(p_response->s_data[0], p_response->s_data[1], p_response->s_data[2], p_response->s_data[3]);
			b_result = true;
		} while (false);
		return b_result;
	}

	bool _set_name_by_rx()
	{
		bool b_result(false);

		do {
			_mp::type_v_buffer v_response;
			if (!is_success_rx(v_response))
				continue;
			_type_response* p_response = (_type_response*)&v_response[0];
			if (p_response->c_size != sizeofstructmember(SYSINFO, sName))
				continue;
			m_v_name.resize(p_response->c_size, 0);
			std::copy(&p_response->s_data[0], &p_response->s_data[p_response->c_size], std::begin(m_v_name));
			b_result = true;
		} while (false);
		return b_result;
	}
	bool _set_device_support_mmd1000_by_rx()
	{
		bool b_result(false);

		do {
			type_msr_response out_result;
			if (!get_response_result(out_result))
				continue;
			if (out_result == response_good)
				m_b_device_is_mmd1000 = true;
			else
				m_b_device_is_mmd1000 = false;
			//
			b_result = true;
		} while (false);
		return b_result;
	}
	bool _set_uid_by_rx()
	{
		bool b_result(false);

		do {
			_mp::type_v_buffer v_response;
			if (!is_success_rx(v_response)) {
				_mp::clog::get_instance().log_fmt_in_debug_mode(L" : DEB : %ls : error : is_success_rx().\n", __WFUNCTION__);
				continue;
			}
			_type_response* p_response = (_type_response*)&v_response[0];
			if (p_response->c_size != the_size_of_uid) {
				_mp::clog::get_instance().log_fmt_in_debug_mode(L" : DEB : %ls : error : p_response->c_size = 0x%x.\n", __WFUNCTION__, p_response->c_size);
				_mp::clog::get_instance().log_data_in_debug_mode(v_response, L"v_response = ",L"\n");
				continue;
			}
			m_v_uid.resize(p_response->c_size, 0);
			std::copy(&p_response->s_data[0], &p_response->s_data[p_response->c_size], std::begin(m_v_uid));
			b_result = true;
		} while (false);
		return b_result;
	}
	bool _set_device_ibutton_type_by_rx()
	{
		bool b_result(false);

		do {
			_mp::type_v_buffer v_response;
			if (!is_success_rx(v_response))
				continue;
			_type_response* p_response = (_type_response*)&v_response[0];
			if (p_response->c_result == response_good)
				m_b_device_is_ibutton_only = true;
			else
				m_b_device_is_ibutton_only = false;//response_good_negative
			//
			b_result = true;
		} while (false);
		return b_result;
	}
	bool _set_device_type_by_rx()
	{
		bool b_result(false);

		do {
			_mp::type_v_buffer v_response;
			if (!is_success_rx(v_response))
				continue;
			_type_response* p_response = (_type_response*)&v_response[0];
			if (p_response->c_result == response_good)
				m_b_device_is_standard = true;
			else
				m_b_device_is_standard = false;//response_good_negative
			//
			b_result = true;
		} while (false);
		return b_result;
	}
	bool _set_global_pre_postfix_send_condition_by_rx()
	{
		bool b_result(false);

		do {
			_mp::type_v_buffer v_response;
			if (!is_success_rx(v_response))
				continue;
			_type_response* p_response = (_type_response*)&v_response[0];
			if (p_response->c_size != sizeofstructmember(SYSINFO, ContainerInfoMsrObj.nGlobalTagCondition))
				continue;
			unsigned int n_condition(0);
			std::copy(&p_response->s_data[0], &p_response->s_data[p_response->c_size], (unsigned char*)&n_condition);

			if (n_condition)
				m_b_global_pre_postfix_send_condition = true;
			else
				m_b_global_pre_postfix_send_condition = false;
			//
			b_result = true;
		} while (false);
		return b_result;
	}
	bool _set_interface_by_rx()
	{
		bool b_result(false);

		do {
			_mp::type_v_buffer v_response;
			if (!is_success_rx(v_response))
				continue;
			_type_response* p_response = (_type_response*)&v_response[0];
			if (p_response->c_size != sizeofstructmember(SYSINFO, Interface))
				continue;
			m_interface = (type_system_interface)p_response->s_data[0];
			b_result = true;
		} while (false);
		return b_result;
	}
	bool _set_language_by_rx()
	{
		bool b_result(false);

		do {
			_mp::type_v_buffer v_response;
			if (!is_success_rx(v_response))
				continue;
			_type_response* p_response = (_type_response*)&v_response[0];
			if (p_response->c_size != sizeofstructmember(SYSINFO, ContainerInfoMsrObj.KeyMap.nMappingTableIndex))
				continue;
			unsigned int n_interface(0);
			std::copy(&p_response->s_data[0], &p_response->s_data[p_response->c_size], (unsigned char*)&n_interface);

			m_interface = (type_system_interface)n_interface;
			b_result = true;
		} while (false);
		return b_result;
	}
	bool _set_buzzer_frequency_by_rx()
	{
		bool b_result(false);

		do {
			_mp::type_v_buffer v_response;
			if (!is_success_rx(v_response))
				continue;
			_type_response* p_response = (_type_response*)&v_response[0];
			if (p_response->c_size != sizeofstructmember(SYSINFO, nBuzzerFrequency))
				continue;
			std::copy(&p_response->s_data[0], &p_response->s_data[p_response->c_size], (unsigned char*)&m_dw_buzzer_frequency);
			b_result = true;
		} while (false);
		return b_result;
	}
	bool _set_boot_run_time_by_rx()
	{
		bool b_result(false);

		do {
			_mp::type_v_buffer v_response;
			if (!is_success_rx(v_response))
				continue;
			_type_response* p_response = (_type_response*)&v_response[0];
			if (p_response->c_size != sizeofstructmember(SYSINFO, nBootRunTime))
				continue;
			std::copy(&p_response->s_data[0], &p_response->s_data[p_response->c_size], (unsigned char*)&m_dw_boot_run_time);
			b_result = true;
		} while (false);
		return b_result;
	}

	bool _set_enable_track_by_rx(type_msr_track_Numer track)
	{
		bool b_result(false);

		do {
			uint32_t dw_size;
			switch (track) {
			case iso1_track:
				dw_size = sizeofstructmember(SYSINFO, InfoMsr[0].cEnableTrack);
				break;
			case iso2_track:
				dw_size = sizeofstructmember(SYSINFO, InfoMsr[1].cEnableTrack);
				break;
			case iso3_track:
				dw_size = sizeofstructmember(SYSINFO, InfoMsr[2].cEnableTrack);
				break;
			default:
				continue;
			}//end switch
			_mp::type_v_buffer v_response;
			if (!is_success_rx(v_response))
				continue;
			_type_response* p_response = (_type_response*)&v_response[0];
			if (p_response->c_size != dw_size)
				continue;
			if (p_response->s_data[0])
				m_b_enable_iso[(int)track] = true;
			else
				m_b_enable_iso[(int)track] = false;
			b_result = true;
		} while (false);
		return b_result;
	}
	bool _set_direction_by_rx(type_msr_track_Numer track)
	{
		bool b_result(false);

		do {
			uint32_t dw_size;
			switch (track) {
			case iso1_track:
				dw_size = sizeofstructmember(SYSINFO, InfoMsr[0].cRDirect[0]);
				break;
			case iso2_track:
				dw_size = sizeofstructmember(SYSINFO, InfoMsr[1].cRDirect[0]);
				break;
			case iso3_track:
				dw_size = sizeofstructmember(SYSINFO, InfoMsr[2].cRDirect[0]);
				break;
			default:
				continue;
			}//end switch
			_mp::type_v_buffer v_response;
			if (!is_success_rx(v_response))
				continue;
			_type_response* p_response = (_type_response*)&v_response[0];
			if (p_response->c_size != dw_size)
				continue;
			m_direction[(int)track] = (type_direction)p_response->s_data[0];
			b_result = true;
		} while (false);
		return b_result;
	}

	bool _set_global_prefix_by_rx()
	{
		bool b_result(false);

		do {
			_mp::type_v_buffer v_response;
			if (!is_success_rx(v_response))
				continue;
			_type_response* p_response = (_type_response*)&v_response[0];
			if (p_response->c_size != sizeofstructmember(SYSINFO, ContainerInfoMsrObj.GlobalPrefix))
				continue;
			PMSR_TAG p_tag = (PMSR_TAG)p_response->s_data;
			m_v_global_prefix.resize(p_tag->cSize, 0);
			std::copy(&p_tag->sTag[0], &p_tag->sTag[p_tag->cSize], std::begin(m_v_global_prefix));
			b_result = true;
		} while (false);
		return b_result;
	}
	bool _set_global_postfix_by_rx()
	{
		bool b_result(false);

		do {
			_mp::type_v_buffer v_response;
			if (!is_success_rx(v_response))
				continue;
			_type_response* p_response = (_type_response*)&v_response[0];
			if (p_response->c_size != sizeofstructmember(SYSINFO, ContainerInfoMsrObj.GlobalPostfix))
				continue;
			PMSR_TAG p_tag = (PMSR_TAG)p_response->s_data;
			m_v_global_postfix.resize(p_tag->cSize, 0);
			std::copy(&p_tag->sTag[0], &p_tag->sTag[p_tag->cSize], std::begin(m_v_global_postfix));
			b_result = true;
		} while (false);
		return b_result;
	}

	bool _set_private_prefix_by_rx(type_msr_track_Numer track)
	{
		bool b_result(false);

		do {
			uint32_t dw_size;
			switch (track) {
			case iso1_track:
				dw_size = sizeofstructmember(SYSINFO, InfoMsr[0].PrivatePrefix[0]);
				break;
			case iso2_track:
				dw_size = sizeofstructmember(SYSINFO, InfoMsr[1].PrivatePrefix[0]);
				break;
			case iso3_track:
				dw_size = sizeofstructmember(SYSINFO, InfoMsr[2].PrivatePrefix[0]);
				break;
			default:
				continue;
			}//end switch
			_mp::type_v_buffer v_response;
			if (!is_success_rx(v_response))
				continue;
			_type_response* p_response = (_type_response*)&v_response[0];
			if (p_response->c_size != dw_size)
				continue;
			PMSR_TAG p_tag = (PMSR_TAG)p_response->s_data;
			m_v_private_prefix[(int)track].resize(p_tag->cSize, 0); m_v_private_prefix[(int)track].assign(m_v_private_prefix[(int)track].size(), 0);
			std::copy(&p_tag->sTag[0], &p_tag->sTag[p_tag->cSize], std::begin(m_v_private_prefix[(int)track]));
			b_result = true;
		} while (false);
		return b_result;
	}
	bool _set_private_postfix_by_rx(type_msr_track_Numer track)
	{
		bool b_result(false);

		do {
			uint32_t dw_size;
			switch (track) {
			case iso1_track:
				dw_size = sizeofstructmember(SYSINFO, InfoMsr[0].PrivatePostfix[0]);
				break;
			case iso2_track:
				dw_size = sizeofstructmember(SYSINFO, InfoMsr[1].PrivatePostfix[0]);
				break;
			case iso3_track:
				dw_size = sizeofstructmember(SYSINFO, InfoMsr[2].PrivatePostfix[0]);
				break;
			default:
				continue;
			}//end switch
			_mp::type_v_buffer v_response;
			if (!is_success_rx(v_response))
				continue;
			_type_response* p_response = (_type_response*)&v_response[0];
			if (p_response->c_size != dw_size)
				continue;
			PMSR_TAG p_tag = (PMSR_TAG)p_response->s_data;
			m_v_private_postfix[(int)track].resize(p_tag->cSize, 0); m_v_private_postfix[(int)track].assign(m_v_private_postfix[(int)track].size(), 0);
			std::copy(&p_tag->sTag[0], &p_tag->sTag[p_tag->cSize], std::begin(m_v_private_postfix[(int)track]));
			b_result = true;
		} while (false);
		return b_result;
	}

	bool _set_ibutton_prefix_by_rx()
	{
		bool b_result(false);

		do {
			_mp::type_v_buffer v_response;
			if (!is_success_rx(v_response))
				continue;
			_type_response* p_response = (_type_response*)&v_response[0];
			if (p_response->c_size != sizeofstructmember(SYSINFO_STD, InfoIButton.GlobalPrefix))
				continue;
			PMSR_TAG p_tag = (PMSR_TAG)p_response->s_data;
			m_v_prefix_ibutton.resize(p_tag->cSize, 0); m_v_prefix_ibutton.assign(m_v_prefix_ibutton.size(), 0);
			std::copy(&p_tag->sTag[0], &p_tag->sTag[p_tag->cSize], std::begin(m_v_prefix_ibutton));
			b_result = true;
		} while (false);
		return b_result;
	}
	bool _set_ibutton_postfix_by_rx()
	{
		bool b_result(false);

		do {
			_mp::type_v_buffer v_response;
			if (!is_success_rx(v_response))
				continue;
			_type_response* p_response = (_type_response*)&v_response[0];
			if (p_response->c_size != sizeofstructmember(SYSINFO_STD, InfoIButton.GlobalPostfix))
				continue;
			PMSR_TAG p_tag = (PMSR_TAG)p_response->s_data;
			m_v_postfix_ibutton.resize(p_tag->cSize, 0); m_v_postfix_ibutton.assign(m_v_postfix_ibutton.size(), 0);
			std::copy(&p_tag->sTag[0], &p_tag->sTag[p_tag->cSize], std::begin(m_v_postfix_ibutton));
			b_result = true;
		} while (false);
		return b_result;
	}
	bool _set_uart_prefix_by_rx()
	{
		bool b_result(false);

		do {
			_mp::type_v_buffer v_response;
			if (!is_success_rx(v_response))
				continue;
			_type_response* p_response = (_type_response*)&v_response[0];
			if (p_response->c_size != sizeofstructmember(SYSINFO_STD, InfoUart.GlobalPrefix))
				continue;
			PMSR_TAG p_tag = (PMSR_TAG)p_response->s_data;
			m_v_prefix_uart.resize(p_tag->cSize, 0); m_v_prefix_uart.assign(m_v_prefix_uart.size(), 0);
			std::copy(&p_tag->sTag[0], &p_tag->sTag[p_tag->cSize], std::begin(m_v_prefix_uart));
			b_result = true;
		} while (false);
		return b_result;
	}
	bool _set_uart_postfix_by_rx()
	{
		bool b_result(false);

		do {
			_mp::type_v_buffer v_response;
			if (!is_success_rx(v_response))
				continue;
			_type_response* p_response = (_type_response*)&v_response[0];
			if (p_response->c_size != sizeofstructmember(SYSINFO_STD, InfoUart.GlobalPostfix))
				continue;
			PMSR_TAG p_tag = (PMSR_TAG)p_response->s_data;
			m_v_postfix_uart.resize(p_tag->cSize, 0); m_v_postfix_uart.assign(m_v_postfix_uart.size(), 0);
			std::copy(&p_tag->sTag[0], &p_tag->sTag[p_tag->cSize], std::begin(m_v_postfix_uart));
			b_result = true;
		} while (false);
		return b_result;
	}
	bool _set_f12_ibutton_by_rx()
	{
		bool b_result(false);

		do {
			_mp::type_v_buffer v_response;
			if (!is_success_rx(v_response))
				continue;
			_type_response* p_response = (_type_response*)&v_response[0];
			if (p_response->c_size != sizeofstructmember(SYSINFO, cBlank))
				continue;
			std::copy(&p_response->s_data[0], &p_response->s_data[p_response->c_size], (unsigned char*)&m_c_blank[0]);
			if (m_c_blank[2] & 0x01)
				m_b_f12_ibutton = true;
			else
				m_b_f12_ibutton = false;
			//
			b_result = true;
		} while (false);
		return b_result;
	}
	bool _set_zeros_ibutton_by_rx()
	{
		bool b_result(false);

		do {
			_mp::type_v_buffer v_response;
			if (!is_success_rx(v_response))
				continue;
			_type_response* p_response = (_type_response*)&v_response[0];
			if (p_response->c_size != sizeofstructmember(SYSINFO, cBlank))
				continue;
			std::copy(&p_response->s_data[0], &p_response->s_data[p_response->c_size], (unsigned char*)&m_c_blank[0]);
			if (m_c_blank[2] & 0x02)
				m_b_zeros_ibutton = false;
			else
				m_b_zeros_ibutton = true;
			//
			b_result = true;
		} while (false);
		return b_result;
	}
	bool _set_zeros_7times_ibutton_by_rx()
	{
		bool b_result(false);

		do {
			_mp::type_v_buffer v_response;
			if (!is_success_rx(v_response))
				continue;
			_type_response* p_response = (_type_response*)&v_response[0];
			if (p_response->c_size != sizeofstructmember(SYSINFO, cBlank))
				continue;
			std::copy(&p_response->s_data[0], &p_response->s_data[p_response->c_size], (unsigned char*)&m_c_blank[0]);
			if (m_c_blank[2] & 0x04)
				m_b_zeros_7times_ibutton = true;
			else
				m_b_zeros_7times_ibutton = false;
			//
			b_result = true;
		} while (false);
		return b_result;
	}
	bool _set_addmit_code_stick_ibutton_by_rx()
	{
		bool b_result(false);

		do {
			_mp::type_v_buffer v_response;
			if (!is_success_rx(v_response))
				continue;
			_type_response* p_response = (_type_response*)&v_response[0];
			if (p_response->c_size != sizeofstructmember(SYSINFO, cBlank))
				continue;
			std::copy(&p_response->s_data[0], &p_response->s_data[p_response->c_size], (unsigned char*)&m_c_blank[0]);
			if (m_c_blank[2] & 0x08)
				m_b_addmit_code_stick_ibutton = true;
			else
				m_b_addmit_code_stick_ibutton = false;
			//
			b_result = true;
		} while (false);
		return b_result;
	}

	bool _set_ibutton_remove_by_rx()
	{
		bool b_result(false);

		do {
			_mp::type_v_buffer v_response;
			if (!is_success_rx(v_response))
				continue;
			_type_response* p_response = (_type_response*)&v_response[0];
			if (p_response->c_size != sizeofstructmember(SYSINFO_STD, RemoveItemTag))
				continue;
			PMSR_TAG p_tag = (PMSR_TAG)p_response->s_data;
			m_v_ibutton_remove.resize(p_tag->cSize, 0); m_v_ibutton_remove.assign(m_v_ibutton_remove.size(), 0);
			std::copy(&p_tag->sTag[0], &p_tag->sTag[p_tag->cSize], std::begin(m_v_ibutton_remove));
			b_result = true;
		} while (false);
		return b_result;
	}

	bool _set_ibutton_prefix_remove_by_rx()
	{
		bool b_result(false);

		do {
			_mp::type_v_buffer v_response;
			if (!is_success_rx(v_response))
				continue;
			_type_response* p_response = (_type_response*)&v_response[0];
			if (p_response->c_size != sizeofstructmember(SYSINFO_STD, InfoIButtonRemove.GlobalPrefix))
				continue;
			PMSR_TAG p_tag = (PMSR_TAG)p_response->s_data;
			m_v_prefix_ibutton_remove.resize(p_tag->cSize, 0); m_v_prefix_ibutton_remove.assign(m_v_prefix_ibutton_remove.size(), 0);
			std::copy(&p_tag->sTag[0], &p_tag->sTag[p_tag->cSize], std::begin(m_v_prefix_ibutton_remove));
			b_result = true;
		} while (false);
		return b_result;
	}
	bool _set_ibutton_postfix_remove_by_rx()
	{
		bool b_result(false);

		do {
			_mp::type_v_buffer v_response;
			if (!is_success_rx(v_response))
				continue;
			_type_response* p_response = (_type_response*)&v_response[0];
			if (p_response->c_size != sizeofstructmember(SYSINFO_STD, InfoIButtonRemove.GlobalPostfix))
				continue;
			PMSR_TAG p_tag = (PMSR_TAG)p_response->s_data;
			m_v_postfix_ibutton_remove.resize(p_tag->cSize, 0); m_v_postfix_ibutton_remove.assign(m_v_postfix_ibutton_remove.size(), 0);
			std::copy(&p_tag->sTag[0], &p_tag->sTag[p_tag->cSize], std::begin(m_v_postfix_ibutton_remove));
			b_result = true;
		} while (false);
		return b_result;
	}

	///////////////////////////////////////////////////////////////////////////////
		

	//generate basic IO pattern . save to m_dequeu_v_tx
	bool _generate_request(const type_cmd cmd, const unsigned char c_sub, const uint32_t dw_data, const unsigned char* s_data)
	{
		bool b_result = false;

		do {
			_mp::type_v_buffer v_tx(0);

			uint32_t nOffset = 0;
			_mp::type_v_buffer v_packet(65, 0);
				
			if (dw_data > 0 && s_data == nullptr)
				continue;

			v_tx.resize(3 + dw_data, 0);//cmd, sub, data field length, data field
			v_tx.assign(v_tx.size(), 0);
			v_tx[0] = (unsigned char)cmd;
			v_tx[1] = c_sub;
			v_tx[2] = (unsigned char)dw_data;

			if (dw_data > 0)
				std::copy(&s_data[0], &s_data[dw_data], std::begin(v_tx) + 3);

			m_dequeu_v_tx.push_back(v_tx);
			b_result = true;
		} while (false);
		return b_result;
	}
		
	//generate IO pattern with _generate_request()
	bool _generate_config_get(uint32_t dw_offset, uint32_t dw_size)//GetFromDevice
	{
		_mp::type_v_buffer v_data(sizeof(uint32_t) + sizeof(uint32_t), 0);////offset and size
		unsigned char* p_data = (unsigned char*)&dw_offset;
		std::copy(&p_data[0], &p_data[sizeof(dw_offset)], std::begin(v_data));

		p_data = (unsigned char*)&dw_size;
		std::copy(&p_data[0], &p_data[sizeof(dw_size)], std::begin(v_data)+ sizeof(dw_offset));

		return _generate_request(cmd_config, (const unsigned char)request_config_get, (uint32_t)(v_data.size()), &v_data[0]);
	}

	//generate IO pattern with _generate_config_get()
	bool _generate_get_version()
	{
		if (_generate_config_get(offsetof(SYSINFO, sSysVer), sizeofstructmember(SYSINFO, sSysVer))) {
			m_deque_generated_tx.push_back(gt_get_version);
			return true;
		}
		return false;
	}
	bool _generate_get_version_strcuture()
	{
		if (_generate_config_get(offsetof(SYSINFO, sStrucVer), sizeofstructmember(SYSINFO, sStrucVer))) {
			m_deque_generated_tx.push_back(gt_get_version_structure);
			return true;
		}
		return false;
	}
	bool _generate_get_name()
	{
		if( _generate_config_get(offsetof(SYSINFO, sName), sizeofstructmember(SYSINFO, sName))) {
			m_deque_generated_tx.push_back(gt_get_name);
			return true;
		}
		return false;
	}
	bool _generate_get_device_support_mmd1000()
	{
		if( _generate_request(cmd_hw_is_mmd1000, 0, 0, nullptr) ){
			m_deque_generated_tx.push_back(gt_support_mmd1000);
				return true;
		}
		return false;
	}
	bool _generate_get_uid()
	{
		if( _generate_request(cmd_read_uid, 0, 0, nullptr)) {
			m_deque_generated_tx.push_back(gt_read_uid);
			return true;
		}
		return false;
	}
	bool _generate_get_device_ibutton_type()
	{
		if( _generate_request(cmd_hw_is_only_ibutton, 0, 0, nullptr)) {
			m_deque_generated_tx.push_back(gt_type_ibutton);
			return true;
		}
		return false;
	}
	bool _generate_get_device_type()
	{
		if( _generate_request(cmd_hw_is_standard, 0, 0, nullptr)) {
			m_deque_generated_tx.push_back(gt_type_device);
			return true;
		}
		return false;
	}
	bool _generate_get_global_pre_postfix_send_condition()
	{
		if( _generate_config_get(offsetof(SYSINFO, ContainerInfoMsrObj.nGlobalTagCondition), sizeofstructmember(SYSINFO, ContainerInfoMsrObj.nGlobalTagCondition))) {
			m_deque_generated_tx.push_back(gt_get_global_prepostfix_send_condition);
			return true;
		}
		return false;
	}
	bool _generate_get_interface()
	{
		if( _generate_config_get(offsetof(SYSINFO, Interface), sizeofstructmember(SYSINFO, Interface))) {
			m_deque_generated_tx.push_back(gt_get_interface);
			return true;
		}
		return false;
	}
	bool _generate_get_language()
	{
		if( _generate_config_get(offsetof(SYSINFO, ContainerInfoMsrObj.KeyMap.nMappingTableIndex), sizeofstructmember(SYSINFO, ContainerInfoMsrObj.KeyMap.nMappingTableIndex))) {
			m_deque_generated_tx.push_back(gt_get_language);
			return true;
		}
		return false;
	}
	bool _generate_get_buzzer_frequency()
	{
		if( _generate_config_get(offsetof(SYSINFO, nBuzzerFrequency), sizeofstructmember(SYSINFO, nBuzzerFrequency))) {
			m_deque_generated_tx.push_back(gt_get_buzzer_frequency);
			return true;
		}
		return false;
	}
	bool _generate_get_boot_run_time()
	{
		if( _generate_config_get(offsetof(SYSINFO, nBootRunTime), sizeofstructmember(SYSINFO, nBootRunTime))) {
			m_deque_generated_tx.push_back(gt_get_boot_run_time);
			return true;
		}
		return false;
	}

	bool _generate_get_enable_track(type_msr_track_Numer track)
	{
		bool b_result(false);

		do {
			uint32_t dw_offset;
			uint32_t dw_size;

			switch (track) {
			case iso1_track:
				dw_offset = offsetof(SYSINFO, InfoMsr[0].cEnableTrack);
				dw_size = sizeofstructmember(SYSINFO, InfoMsr[0].cEnableTrack);
				break;
			case iso2_track:
				dw_offset = offsetof(SYSINFO, InfoMsr[1].cEnableTrack);
				dw_size = sizeofstructmember(SYSINFO, InfoMsr[1].cEnableTrack);
				break;
			case iso3_track:
				dw_offset = offsetof(SYSINFO, InfoMsr[2].cEnableTrack);
				dw_size = sizeofstructmember(SYSINFO, InfoMsr[2].cEnableTrack);
				break;
			default:
				continue;
			}//end switch

			if (!_generate_config_get(dw_offset, dw_size))
				continue;
			switch (track) {
			case iso1_track:
				m_deque_generated_tx.push_back(gt_get_enable_iso1);
				break;
			case iso2_track:
				m_deque_generated_tx.push_back(gt_get_enable_iso2);
				break;
			case iso3_track:
				m_deque_generated_tx.push_back(gt_get_enable_iso3);
				break;
			default:
				continue;
			}//end switch

			b_result = true;
		} while (false);
		return b_result;
	}
	bool _generate_get_direction(type_msr_track_Numer track)
	{
		bool b_result(false);

		do {
			uint32_t dw_offset;
			uint32_t dw_size;

			switch (track) {
			case iso1_track:
				dw_offset = offsetof(SYSINFO, InfoMsr[0].cRDirect[0]);
				dw_size = sizeofstructmember(SYSINFO, InfoMsr[0].cRDirect[0]);
				break;
			case iso2_track:
				dw_offset = offsetof(SYSINFO, InfoMsr[1].cRDirect[0]);
				dw_size = sizeofstructmember(SYSINFO, InfoMsr[1].cRDirect[0]);
				break;
			case iso3_track:
				dw_offset = offsetof(SYSINFO, InfoMsr[2].cRDirect[0]);
				dw_size = sizeofstructmember(SYSINFO, InfoMsr[2].cRDirect[0]);
				break;
			default:
				continue;
			}//end switch

			if (!_generate_config_get(dw_offset, dw_size))
				continue;
			switch (track) {
			case iso1_track:
				m_deque_generated_tx.push_back(gt_get_direction1);
				break;
			case iso2_track:
				m_deque_generated_tx.push_back(gt_get_direction2);
				break;
			case iso3_track:
				m_deque_generated_tx.push_back(gt_get_direction3);
				break;
			default:
				continue;
			}//end switch

			b_result = true;
		} while (false);
		return b_result;
	}

	bool _generate_get_global_prefix()
	{
		if( _generate_config_get(offsetof(SYSINFO, ContainerInfoMsrObj.GlobalPrefix), sizeofstructmember(SYSINFO, ContainerInfoMsrObj.GlobalPrefix))) {
			m_deque_generated_tx.push_back(gt_get_global_prefix);
			return true;
		}
		return false;
	}
	bool _generate_get_global_postfix()
	{
		if( _generate_config_get(offsetof(SYSINFO, ContainerInfoMsrObj.GlobalPostfix), sizeofstructmember(SYSINFO, ContainerInfoMsrObj.GlobalPostfix))) {
			m_deque_generated_tx.push_back(gt_get_global_postfix);
			return true;
		}
		return false;
	}

	bool _generate_get_private_prefix(type_msr_track_Numer track)
	{
		bool b_result(false);

		do {
			uint32_t dw_offset;
			uint32_t dw_size;

			switch (track) {
			case iso1_track:
				dw_offset = offsetof(SYSINFO, InfoMsr[0].PrivatePrefix[0]);
				dw_size = sizeofstructmember(SYSINFO, InfoMsr[0].PrivatePrefix[0]);
				break;
			case iso2_track:
				dw_offset = offsetof(SYSINFO, InfoMsr[1].PrivatePrefix[0]);
				dw_size = sizeofstructmember(SYSINFO, InfoMsr[1].PrivatePrefix[0]);
				break;
			case iso3_track:
				dw_offset = offsetof(SYSINFO, InfoMsr[2].PrivatePrefix[0]);
				dw_size = sizeofstructmember(SYSINFO, InfoMsr[2].PrivatePrefix[0]);
				break;
			default:
				continue;
			}//end switch

			if(!_generate_config_get(dw_offset, dw_size))
				continue;
			switch (track) {
			case iso1_track:
				m_deque_generated_tx.push_back(gt_get_private_prefix1);
				break;
			case iso2_track:
				m_deque_generated_tx.push_back(gt_get_private_prefix2);
				break;
			case iso3_track:
				m_deque_generated_tx.push_back(gt_get_private_prefix3);
				break;
			default:
				continue;
			}//end switch

			b_result = true;
		} while (false);
		return b_result;
	}
	bool _generate_get_private_postfix(type_msr_track_Numer track)
	{
		bool b_result(false);

		do {
			uint32_t dw_offset;
			uint32_t dw_size;

			switch (track) {
			case iso1_track:
				dw_offset = offsetof(SYSINFO, InfoMsr[0].PrivatePostfix[0]);
				dw_size = sizeofstructmember(SYSINFO, InfoMsr[0].PrivatePostfix[0]);
				break;
			case iso2_track:
				dw_offset = offsetof(SYSINFO, InfoMsr[1].PrivatePostfix[0]);
				dw_size = sizeofstructmember(SYSINFO, InfoMsr[1].PrivatePostfix[0]);
				break;
			case iso3_track:
				dw_offset = offsetof(SYSINFO, InfoMsr[2].PrivatePostfix[0]);
				dw_size = sizeofstructmember(SYSINFO, InfoMsr[2].PrivatePostfix[0]);
				break;
			default:
				continue;
			}//end switch

			if(!_generate_config_get(dw_offset, dw_size))
				continue;
			switch (track) {
			case iso1_track:
				m_deque_generated_tx.push_back(gt_get_private_postfix1);
				break;
			case iso2_track:
				m_deque_generated_tx.push_back(gt_get_private_postfix2);
				break;
			case iso3_track:
				m_deque_generated_tx.push_back(gt_get_private_postfix3);
				break;
			default:
				continue;
			}//end switch

			b_result = true;
		} while (false);
		return b_result;
	}

	bool _generate_get_ibutton_prefix()
	{
		if( _generate_config_get(offsetof(SYSINFO_STD, InfoIButton.GlobalPrefix), sizeofstructmember(SYSINFO_STD, InfoIButton.GlobalPrefix))) {
			m_deque_generated_tx.push_back(gt_get_prefix_ibutton);
			return true;
		}
		return false;
	}
	bool _generate_get_ibutton_postfix()
	{
		if( _generate_config_get(offsetof(SYSINFO_STD, InfoIButton.GlobalPostfix), sizeofstructmember(SYSINFO_STD, InfoIButton.GlobalPostfix))) {
			m_deque_generated_tx.push_back(gt_get_postfix_ibutton);
			return true;
		}
		return false;
	}
	bool _generate_get_uart_prefix()
	{
		if( _generate_config_get(offsetof(SYSINFO_STD, InfoUart.GlobalPrefix), sizeofstructmember(SYSINFO_STD, InfoUart.GlobalPrefix))) {
			m_deque_generated_tx.push_back(gt_get_prefix_uart);
			return true;
		}
		return false;
	}
	bool _generate_get_uart_postfix()
	{
		if( _generate_config_get(offsetof(SYSINFO_STD, InfoUart.GlobalPostfix), sizeofstructmember(SYSINFO_STD, InfoUart.GlobalPostfix))) {
			m_deque_generated_tx.push_back(gt_get_postfix_uart);
			return true;
		}
		return false;
	}
	bool _generate_get_f12_ibutton()
	{
		if( _generate_config_get(offsetof(SYSINFO, cBlank), sizeofstructmember(SYSINFO, cBlank))) {
			m_deque_generated_tx.push_back(gt_get_f12_ibutton);
			return true;
		}
		return false;
	}
	bool _generate_get_zeros_ibutton()
	{
		if( _generate_config_get(offsetof(SYSINFO, cBlank), sizeofstructmember(SYSINFO, cBlank))) {
			m_deque_generated_tx.push_back(gt_get_zeros_ibutton);
			return true;
		}
		return false;
	}
	bool _generate_get_zeros_7times_ibutton()
	{
		if( _generate_config_get(offsetof(SYSINFO, cBlank), sizeofstructmember(SYSINFO, cBlank))) {
			m_deque_generated_tx.push_back(gt_get_zeros7_times_ibutton);
			return true;
		}
		return false;
	}
	bool _generate_get_addmit_code_stick_ibutton()
	{
		if( _generate_config_get(offsetof(SYSINFO, cBlank), sizeofstructmember(SYSINFO, cBlank))) {
			m_deque_generated_tx.push_back(gt_get_addmit_code_stick_ibutton);
			return true;
		}
		return false;
	}

	bool _generate_get_ibutton_remove()
	{
		if (_generate_config_get(offsetof(SYSINFO_STD, RemoveItemTag), sizeofstructmember(SYSINFO_STD, RemoveItemTag))) {
			m_deque_generated_tx.push_back(gt_get_ibutton_remove);
			return true;
		}
		return false;
	}

	bool _generate_get_ibutton_prefix_remove()
	{
		if (_generate_config_get(offsetof(SYSINFO_STD, InfoIButtonRemove.GlobalPrefix), sizeofstructmember(SYSINFO_STD, InfoIButtonRemove.GlobalPrefix))) {
			m_deque_generated_tx.push_back(gt_get_prefix_ibutton_remove);
			return true;
		}
		return false;
	}
	bool _generate_get_ibutton_postfix_remove()
	{
		if (_generate_config_get(offsetof(SYSINFO_STD, InfoIButtonRemove.GlobalPostfix), sizeofstructmember(SYSINFO_STD, InfoIButtonRemove.GlobalPostfix))) {
			m_deque_generated_tx.push_back(gt_get_postfix_ibutton_remove);
			return true;
		}
		return false;
	}

	//generate IO pattern with _generate_request()
	bool _generate_config_set(uint32_t dw_offset, uint32_t dw_size, unsigned char* ps_data)//SetToDevice
	{
		_mp::type_v_buffer v_data(sizeof(uint32_t) + sizeof(uint32_t) + dw_size, 0);////offset and size and data field
		unsigned char* p_data = (unsigned char*)&dw_offset;
		std::copy(&p_data[0], &p_data[sizeof(dw_offset)], std::begin(v_data));

		p_data = (unsigned char*)&dw_size;
		std::copy(&p_data[0], &p_data[sizeof(dw_size)], std::begin(v_data) + sizeof(dw_offset));

		if (dw_size > 0) {
			std::copy(&ps_data[0], &ps_data[dw_size], std::begin(v_data) + sizeof(dw_offset) + sizeof(dw_size));
		}

		if(_generate_request(cmd_config, (const unsigned char)request_config_set, (uint32_t)(v_data.size()), &v_data[0])) {
			m_deque_generated_tx.push_back(gt_set_config);
			return true;
		}
		return false;
	}

	//generate IO pattern with _generate_config_set()
	bool _generate_set_global_pre_postfix_send_condition()
	{
		uint32_t n_condition(0);

		if (get_global_pre_postfix_send_condition())
			n_condition = 1;
		return _generate_config_set(
			offsetof(SYSINFO, ContainerInfoMsrObj.nGlobalTagCondition),
			sizeofstructmember(SYSINFO, ContainerInfoMsrObj.nGlobalTagCondition),
			reinterpret_cast<unsigned char*>(&n_condition)
		);
	}
	bool _generate_set_interface()
	{
		unsigned char Interface = static_cast<unsigned char>(get_interface());

		return _generate_config_set(
			offsetof(SYSINFO, Interface),
			sizeofstructmember(SYSINFO, Interface),
			static_cast<unsigned char*>(&Interface)
		);
	}
	bool _generate_set_language()
	{
		bool b_result(false);

		do {
			uint32_t n_mapping_table_index = static_cast<uint32_t>(get_language());

			b_result = _generate_config_set(
				offsetof(SYSINFO, ContainerInfoMsrObj.KeyMap.nMappingTableIndex),
				sizeofstructmember(SYSINFO, ContainerInfoMsrObj.KeyMap.nMappingTableIndex),
				reinterpret_cast<unsigned char*>(&n_mapping_table_index)
			);
			if (!b_result)
				continue;

			b_result = _generate_config_set(
				offsetof(SYSINFO, InfoMsr[0].KeyMap[0].nMappingTableIndex),
				sizeofstructmember(SYSINFO, InfoMsr[0].KeyMap[0].nMappingTableIndex),
				reinterpret_cast<unsigned char*>(&n_mapping_table_index)
			);
			if (!b_result)
				continue;

			b_result = _generate_config_set(
				offsetof(SYSINFO, InfoMsr[1].KeyMap[0].nMappingTableIndex),
				sizeofstructmember(SYSINFO, InfoMsr[1].KeyMap[0].nMappingTableIndex),
				reinterpret_cast<unsigned char*>(&n_mapping_table_index)
			);
			if (!b_result)
				continue;

			b_result = _generate_config_set(
				offsetof(SYSINFO, InfoMsr[2].KeyMap[0].nMappingTableIndex),
				sizeofstructmember(SYSINFO, InfoMsr[2].KeyMap[0].nMappingTableIndex),
				reinterpret_cast<unsigned char*>(&n_mapping_table_index)
			);
		} while (false);
		return b_result;
	}
	bool _generate_set_buzzer_frequency()
	{
		uint32_t n_freq = static_cast<uint32_t>(get_buzzer_frequency());

		if (m_b_device_is_mmd1000) {
			//change default value.
			if (n_freq == default_buzzer_frequency)
				n_freq = default_buzzer_frequency_for_wiznova_board;
		}

		if (m_version.get_major() > 4 && n_freq == default_buzzer_frequency_for_wiznova_board) {
			n_freq = default_buzzer_frequency;
		}

		return _generate_config_set(
			offsetof(SYSINFO, nBuzzerFrequency),
			sizeofstructmember(SYSINFO, nBuzzerFrequency),
			reinterpret_cast<unsigned char*>(&n_freq)
		);
	}

	bool _generate_set_enable_track(type_msr_track_Numer track)
	{
		bool b_result(false);

		do {
			uint32_t dw_offset,dw_size;
			unsigned char c_track(0);

			if (get_enable_iso(track))
				c_track = 1;

			switch (track) {
			case iso1_track:
				dw_offset = offsetof(SYSINFO, InfoMsr[0].cEnableTrack);
				dw_size = sizeofstructmember(SYSINFO, InfoMsr[0].cEnableTrack);
				break;
			case iso2_track:
				dw_offset = offsetof(SYSINFO, InfoMsr[1].cEnableTrack);
				dw_size = sizeofstructmember(SYSINFO, InfoMsr[1].cEnableTrack);
				break;
			case iso3_track:
				dw_offset = offsetof(SYSINFO, InfoMsr[2].cEnableTrack);
				dw_size = sizeofstructmember(SYSINFO, InfoMsr[2].cEnableTrack);
				break;
			default:
				continue;
			}//end switch

			b_result = _generate_config_set(
				dw_offset,
				dw_size,
				&c_track
			);
		} while (false);
		return b_result;
	}
	bool _generate_set_direction(type_msr_track_Numer track)
	{
		bool b_result(false);

		do {
			uint32_t dw_offset, dw_size;
			unsigned char c_track(0);

			if (get_enable_iso(track))
				c_track = 1;

			switch (track) {
			case iso1_track:
				dw_offset = offsetof(SYSINFO, InfoMsr[0].cRDirect[0]);
				dw_size = sizeofstructmember(SYSINFO, InfoMsr[0].cRDirect[0]);
				break;
			case iso2_track:
				dw_offset = offsetof(SYSINFO, InfoMsr[1].cRDirect[0]);
				dw_size = sizeofstructmember(SYSINFO, InfoMsr[1].cRDirect[0]);
				break;
			case iso3_track:
				dw_offset = offsetof(SYSINFO, InfoMsr[2].cRDirect[0]);
				dw_size = sizeofstructmember(SYSINFO, InfoMsr[2].cRDirect[0]);
				break;
			default:
				continue;
			}//end switch

			b_result = _generate_config_set(
				dw_offset,
				dw_size,
				&c_track
			);
		} while (false);
		return b_result;
	}

	bool _generate_set_global_prefix()
	{
		MSR_TAG Tag;

		memset(&Tag, 0, sizeof(MSR_TAG));

		_get_tag_from_type_tag(Tag, get_global_prefix());

		return _generate_config_set(
			offsetof(SYSINFO, ContainerInfoMsrObj.GlobalPrefix),
			sizeofstructmember(SYSINFO, ContainerInfoMsrObj.GlobalPrefix),
			reinterpret_cast<unsigned char*>(&Tag)
		);
	}
	bool _generate_set_global_postfix()
	{
		MSR_TAG Tag;

		memset(&Tag, 0, sizeof(MSR_TAG));

		_get_tag_from_type_tag(Tag, get_global_postfix());

		return _generate_config_set(
			offsetof(SYSINFO, ContainerInfoMsrObj.GlobalPostfix),
			sizeofstructmember(SYSINFO, ContainerInfoMsrObj.GlobalPostfix),
			reinterpret_cast<unsigned char*>(&Tag)
		);
	}

	bool _generate_set_private_prefix(type_msr_track_Numer track)
	{
		bool b_result(false);
		do {
			MSR_TAG Tag;

			memset(&Tag, 0, sizeof(MSR_TAG));

			_get_tag_from_type_tag(Tag, get_private_prefix(track));

			uint32_t dw_offset, dw_size;

			switch (track) {
			case iso1_track:
				dw_offset = offsetof(SYSINFO, InfoMsr[0].PrivatePrefix[0]);
				dw_size = sizeofstructmember(SYSINFO, InfoMsr[0].PrivatePrefix[0]);
				break;
			case iso2_track:
				dw_offset = offsetof(SYSINFO, InfoMsr[1].PrivatePrefix[0]);
				dw_size = sizeofstructmember(SYSINFO, InfoMsr[1].PrivatePrefix[0]);
				break;
			case iso3_track:
				dw_offset = offsetof(SYSINFO, InfoMsr[2].PrivatePrefix[0]);
				dw_size = sizeofstructmember(SYSINFO, InfoMsr[2].PrivatePrefix[0]);
				break;
			default:
				continue;
			}//end switch

			b_result = _generate_config_set(dw_offset,dw_size,reinterpret_cast<unsigned char*>(&Tag));
		} while (false);
		return b_result;
	}
	bool _generate_set_private_postfix(type_msr_track_Numer track)
	{
		bool b_result(false);
		do {
			MSR_TAG Tag;

			memset(&Tag, 0, sizeof(MSR_TAG));

			_get_tag_from_type_tag(Tag, get_private_postfix(track));

			uint32_t dw_offset, dw_size;

			switch (track) {
			case iso1_track:
				dw_offset = offsetof(SYSINFO, InfoMsr[0].PrivatePostfix[0]);
				dw_size = sizeofstructmember(SYSINFO, InfoMsr[0].PrivatePostfix[0]);
				break;
			case iso2_track:
				dw_offset = offsetof(SYSINFO, InfoMsr[1].PrivatePostfix[0]);
				dw_size = sizeofstructmember(SYSINFO, InfoMsr[1].PrivatePostfix[0]);
				break;
			case iso3_track:
				dw_offset = offsetof(SYSINFO, InfoMsr[2].PrivatePostfix[0]);
				dw_size = sizeofstructmember(SYSINFO, InfoMsr[2].PrivatePostfix[0]);
				break;
			default:
				continue;
			}//end switch

			b_result = _generate_config_set(dw_offset, dw_size, reinterpret_cast<unsigned char*>(&Tag));
		} while (false);
		return b_result;
	}

	bool _generate_set_key_map()
	{
		bool b_result(false);

		do {
			uint32_t n_mapping_table_index = static_cast<uint32_t>(get_language());
			uint32_t dw_offset, dw_size;
			unsigned char* p_data;

			//USB map
			dw_offset = address_system_hid_key_map_offset;
			dw_size = FOR_CVT_MAX_ASCII_CODE;
			p_data = (unsigned char*)(ckey_map::get_ascii_to_hid_key_map(n_mapping_table_index));
			b_result = _generate_config_set(dw_offset, dw_size, p_data);

			dw_offset = address_system_hid_key_map_offset + FOR_CVT_MAX_ASCII_CODE;
			dw_size = FOR_CVT_MAX_ASCII_CODE;
			p_data = (unsigned char*)(ckey_map::get_ascii_to_hid_key_map(n_mapping_table_index,FOR_CVT_MAX_ASCII_CODE / 2));
			b_result = _generate_config_set(dw_offset, dw_size, p_data);

			//PS2 map
			dw_offset = address_system_ps2_key_map_offset;
			dw_size = FOR_CVT_MAX_ASCII_CODE;
			p_data = (unsigned char*)(ckey_map::get_ascii_to_ps2_key_map(n_mapping_table_index));
			b_result = _generate_config_set(dw_offset, dw_size, p_data);

			dw_offset = address_system_ps2_key_map_offset + FOR_CVT_MAX_ASCII_CODE;
			dw_size = FOR_CVT_MAX_ASCII_CODE;
			p_data = (unsigned char*)(ckey_map::get_ascii_to_ps2_key_map(n_mapping_table_index, FOR_CVT_MAX_ASCII_CODE / 2));
			b_result = _generate_config_set(dw_offset, dw_size, p_data);
		} while (false);
		return b_result;
	}
	bool _generate_set_device_type()
	{
		//impossible ....... designed by Hardwired-logic.
		//Therefore is function is dummy.
		return true;
	}
	bool _generate_set_ibutton_prefix()
	{
		MSR_TAG Tag;

		memset(&Tag, 0, sizeof(MSR_TAG));

		_get_tag_from_type_tag(Tag, get_prefix_ibutton());

		return _generate_config_set(
			offsetof(SYSINFO_STD, InfoIButton.GlobalPrefix),
			sizeofstructmember(SYSINFO_STD, InfoIButton.GlobalPrefix),
			reinterpret_cast<unsigned char*>(&Tag)
		);
	}
	bool _generate_set_ibutton_postfix()
	{
		MSR_TAG Tag;

		memset(&Tag, 0, sizeof(MSR_TAG));

		_get_tag_from_type_tag(Tag, get_postfix_ibutton());

		return _generate_config_set(
			offsetof(SYSINFO_STD, InfoIButton.GlobalPostfix),
			sizeofstructmember(SYSINFO_STD, InfoIButton.GlobalPostfix),
			reinterpret_cast<unsigned char*>(&Tag)
		);
	}
	bool _generate_set_uart_prefix()
	{
		MSR_TAG Tag;

		memset(&Tag, 0, sizeof(MSR_TAG));

		_get_tag_from_type_tag(Tag, get_prefix_uart());

		return _generate_config_set(
			offsetof(SYSINFO_STD, InfoUart.GlobalPrefix),
			sizeofstructmember(SYSINFO_STD, InfoUart.GlobalPrefix),
			reinterpret_cast<unsigned char*>(&Tag)
		);
	}
	bool _generate_set_uart_postfix()
	{
		MSR_TAG Tag;

		memset(&Tag, 0, sizeof(MSR_TAG));

		_get_tag_from_type_tag(Tag, get_postfix_uart());

		return _generate_config_set(
			offsetof(SYSINFO_STD, InfoUart.GlobalPostfix),
			sizeofstructmember(SYSINFO_STD, InfoUart.GlobalPostfix),
			reinterpret_cast<unsigned char*>(&Tag)
		);
	}
	bool _generate_set_f12_ibutton()
	{
		unsigned char s_blank[SYSTEM_SIZE_BLANK];
		memcpy(s_blank, m_c_blank, sizeof(s_blank));

		if (get_enable_f12_ibutton()) { SET_IBUTTON_ENABLE_F12(s_blank); }
		else { RESET_IBUTTON_ENABLE_F12(s_blank); }

		if (get_enable_zeros_ibutton()) { IBUTTON_ZERO8_ENABLE(s_blank); }
		else { IBUTTON_ZERO8_DISABLE(s_blank); }

		if (get_enable_zeros_7times_ibutton()) { IBUTTON_ZEROS_7TIMES_ENABLE(s_blank); }
		else { IBUTTON_ZEROS_7TIMES_DISABLE(s_blank); }

		if (get_enable_addmit_code_stick_ibutton()) { IBUTTON_ADDMIT_CODESTICK_ENABLE(s_blank); }
		else { IBUTTON_ADDMIT_CODESTICK_DISABLE(s_blank); }

		return _generate_config_set(
			offsetof(SYSINFO, cBlank),
			sizeofstructmember(SYSINFO, cBlank),
			reinterpret_cast<unsigned char*>(s_blank)
		);
	}
	bool _generate_set_zeros_ibutton()
	{
		return _generate_set_f12_ibutton();
	}
	bool _generate_set_zeros_7times_ibutton()
	{
		return _generate_set_f12_ibutton();
	}
	bool _generate_set_addmit_code_stick()
	{
		return _generate_set_f12_ibutton();
	}

	bool _generate_set_ibutton_remove()
	{
		MSR_TAG Tag;

		memset(&Tag, 0, sizeof(MSR_TAG));

		_get_tag_from_type_tag(Tag, get_ibutton_remove());

		return _generate_config_set(
			offsetof(SYSINFO_STD, RemoveItemTag),
			sizeofstructmember(SYSINFO_STD, RemoveItemTag),
			reinterpret_cast<unsigned char*>(&Tag)
		);
	}

	bool _generate_set_ibutton_prefix_remove()
	{
		MSR_TAG Tag;

		memset(&Tag, 0, sizeof(MSR_TAG));

		_get_tag_from_type_tag(Tag, get_prefix_ibutton_remove());

		return _generate_config_set(
			offsetof(SYSINFO_STD, InfoIButtonRemove.GlobalPrefix),
			sizeofstructmember(SYSINFO_STD, InfoIButtonRemove.GlobalPrefix),
			reinterpret_cast<unsigned char*>(&Tag)
		);
	}
	bool _generate_set_ibutton_postfix_remove()
	{
		MSR_TAG Tag;

		memset(&Tag, 0, sizeof(MSR_TAG));

		_get_tag_from_type_tag(Tag, get_postfix_ibutton_remove());

		return _generate_config_set(
			offsetof(SYSINFO_STD, InfoIButtonRemove.GlobalPostfix),
			sizeofstructmember(SYSINFO_STD, InfoIButtonRemove.GlobalPostfix),
			reinterpret_cast<unsigned char*>(&Tag)
		);
	}


	//////////////////////////////////////////////////////////////////////
	// etc funciton
	bool _get_tag_from_string(MSR_TAG& out_tag, const std::wstring& s_src)
	{
		bool b_result(false);
		do {
			if (s_src.empty())
				continue;

			//fill m_vKey vector from m_sKey
			_mp::type_list_wstring list_s_token;
			_mp::cconvert::tokenizer(list_s_token, s_src, std::wstring(L" "));
			int n_val(0);
			out_tag.cSize = 0;

			b_result = true;
			for (auto s_token : list_s_token) {
				if (_is_avalied_format(s_token)) {
					b_result = false;
					break;
				}
				n_val = _get_byte_from_token(s_token);
				if (m_token_format != ef_heximal) {
					b_result = false;
					break;
				}
				out_tag.sTag[out_tag.cSize++] = static_cast<unsigned char>(n_val);
			}//end for
		} while (false);
		return b_result;
	}
	bool _get_tag_from_type_tag(MSR_TAG& out_tag, const type_tag& v_in_tag)
	{
		bool b_result(false);

		do {
			if (v_in_tag.empty())
				continue;
			//
			out_tag.cSize = 0;

			for (type_tag::const_iterator iter = v_in_tag.begin(); iter != v_in_tag.end(); ++iter) {
				out_tag.sTag[out_tag.cSize++] = *iter;
			}//end for
			b_result = true;
		} while (false);
		return b_result;
	}
	bool _is_avalied_format(const std::wstring& sToken)
	{
		bool b_result(false);
		do {
			int n_result = _get_byte_from_token(sToken);
			if (n_result < 0)
				continue;
			b_result = true;
		} while (false);
		return b_result;
	}

	//get one byte data from token
	// return negative is error.
	// this method set a token format( m_token_format memeber ) 
	int _get_byte_from_token(const std::wstring& sToken)
	{
		int n_result = -1;

		// case 1 : 0~9
		// case 2 : starting with 0x, 0~9, a~f, A~F, max size 4
		// case 3 : start and end with ' , max size 3 
		// case 4 : raw ascii code. max size 1
		do {
			std::wstring::size_type Pos = sToken.find_first_not_of(std::wstring(L"0123456789"));

			if (Pos == std::wstring::npos) {
				//case 1 : decimal
				n_result = stoi(sToken);

				if (n_result > 255 || n_result < 0) {
					n_result = -1;
				}
				else {
					//success
					m_token_format = ef_decimal;
				}
				continue;
			}

			Pos = sToken.find_first_not_of(std::wstring(L"0123456789abcdefABCDEF"));

			if (Pos == 1) {	//'x' of "0x"

				if (sToken[Pos] == L'x') {
					// case 2 : starting with 0x, 0~9, a~f, A~F, max size 4
					std::wstring stemp = sToken.substr(2);
					n_result = stoi(stemp, 0, 16);

					if (n_result > 255 || n_result < 0) {
						n_result = -1;
					}
					else {
						//success
						m_token_format = ef_heximal;
					}
				}
				else
					n_result = -1;

				continue;
			}

			//
			if (sToken.size() == 3) {

				//case 3
				std::wstring::const_reference cFirst = sToken[0];
				std::wstring::const_reference cLast = sToken[2];

				if ((cFirst != L'\'') || (cLast != L'\''))
					n_result = -1;
				else {
					n_result = static_cast<uint32_t>(sToken[1]);
					if (n_result > 255)
						n_result = -1;
					else {
						//success
						m_token_format = ef_ascii;
					}
				}
				continue;
			}

			if (sToken.size() == 1) {
				// case 4
				n_result = static_cast<uint32_t>(sToken[0]);
				if (n_result > 255)
					n_result = -1;
				else {
					//success
					m_token_format = ef_ascii;
				}
			}
			else {
				n_result = -1;
			}
		} while (false);

		return n_result;
	}

	uint32_t _get_new_scr_transaction_counter()
	{
		++m_dw_scr_transaction_counter;
		uint32_t n_new = m_dw_scr_transaction_counter;
		return n_new;
	}
private:
	uint32_t m_dw_scr_transaction_counter;
	_type_deque_generated_tx m_deque_generated_tx;

	_mp::type_dequeu_v_buffer m_dequeu_v_tx;//not included report id
	_mp::type_dequeu_v_buffer m_dequeu_v_rx;//not included report id

	std::unordered_set<_change_parameter> m_set_change_parameter;

	bool m_b_global_pre_postfix_send_condition;
	type_manufacturer m_manufacture;

	type_uid m_v_uid;
	type_function m_device_function;
	cprotocol_lpu237::type_version m_version;
	cprotocol_lpu237::type_version m_version_structure;

	//device information
	bool m_b_is_hid_boot;
	bool m_b_removed_key_map_table;

	///////////////////////////////
	//device parameters
	bool m_b_device_is_mmd1000;
	bool m_b_device_is_ibutton_only;
	bool m_b_device_is_standard;

	type_system_interface m_interface;
	uint32_t m_dw_buzzer_frequency;
	uint32_t m_dw_boot_run_time;
	type_keyboard_language_index m_language_index;

	bool m_b_enable_iso[the_number_of_reack];

	type_direction m_direction[the_number_of_reack];

	type_tag m_v_global_prefix;
	type_tag m_v_global_postfix;

	type_tag m_v_private_prefix[the_number_of_reack];
	type_tag m_v_private_postfix[the_number_of_reack];

	//i-button
	type_tag m_v_prefix_ibutton;
	type_tag m_v_postfix_ibutton;
	bool m_b_f12_ibutton;
	bool m_b_zeros_ibutton;
	bool m_b_zeros_7times_ibutton;
	bool m_b_addmit_code_stick_ibutton;
	unsigned char m_c_blank[the_size_of_system_blank];

	type_tag m_v_ibutton_remove;
	type_tag m_v_prefix_ibutton_remove;
	type_tag m_v_postfix_ibutton_remove;


	//rs232
	type_tag m_v_prefix_uart;
	type_tag m_v_postfix_uart;
	//
	_type_format m_token_format;
	cprotocol_lpu237::type_name m_v_name;
};
