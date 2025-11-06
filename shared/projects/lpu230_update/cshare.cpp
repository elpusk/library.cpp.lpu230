
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#include <mp_cstring.h>
#include <mp_cwait.h>

#include <tg_rom.h>

#include "cshare.h"
#include "HidBootBuffer.h"

#ifdef _WIN32
#include <atltrace.h>
#endif


cshare& cshare::get_instance()
{
	static cshare obj;
	return obj;
}

int cshare::calculate_update_step(int n_the_number_of_erase_sector /*= -1*/)
{
	int n_step(0);

	do {
		if (!m_b_start_from_bootloader) {
			//lpu23x 로 부터 시작
			if (!m_ptr_dev) {
				// lpu23x 로 부터 시작하자만 선태된 장비가 없어 에러.
				continue;
			}

			// generate_get_parameters() 전에
			// generate_get_system_information_with() 
			// 로 장비로 부터 데이터를 얻어 왔어야 함.

			// system parameter 저장을 위한 step 개수를 더함.
			m_target_protocol_lpu237.clear_transaction();
			m_target_protocol_lpu237.generate_get_parameters();
			n_step += (int)m_target_protocol_lpu237.get_generated_the_number_of_tx();
			m_target_protocol_lpu237.clear_transaction();

			// system parameter 복원을 위한 step 개수를 더함.
			m_target_protocol_lpu237.set_all_parameter_to_changed();
			m_target_protocol_lpu237.generate_set_parameters(); // apply 포함됨.
			n_step += (int)m_target_protocol_lpu237.get_generated_the_number_of_tx();
			m_target_protocol_lpu237.clear_transaction();

			n_step += 1;// run boot loader
			n_step += 1;// lpu23x plugout 기다림
			n_step += 1;// bootloader plugin 기다림.
		}
		/////////////////////////////
		// 공용 step 의 개수.

		// firmware 크기로 부터 필요한 sector 의 개수를 얻는다. 
		int n_sector_w = m_n_size_fw / CHidBootBuffer::C_SECTOR_SIZE;
		if (m_n_size_fw % CHidBootBuffer::C_SECTOR_SIZE != 0) {
			++n_sector_w;
		}

		// fw 를 sector 단위로 읽기 때문에 파일에서 읽는 회수도 추가.
		n_step += n_sector_w;

		// erase 수, 
		if (n_the_number_of_erase_sector < 0) {
			n_step += n_sector_w;
			n_step += 1; // system parameter area 를 위한 sector 1개 추가.
		}
		else {
			n_step += n_the_number_of_erase_sector;
		}

		// write sector 의 수
		n_step += n_sector_w;

		n_step += 1;// run app
		n_step += 1;// bootload plugout 기다림
		n_step += 1;// lpu23x lpugin 기다림.

		if(m_b_enable_iso_mode_after_update) {
			n_step += 1; // iso mode 로 변경.(enter-config, set iso mode, leave-config)
		}
		if (m_lpu23x_interface_after_update != cshare::Lpu237Interface::nc) {
			n_step += 1; // interface 변경.(enter-config, set interface, leave-config)
		}

	} while (false);

	return n_step;
}

int cshare::get_app_area_zero_based_start_sector_from_sector_number(int n_sector_number) const
{
	int n_zero_base_sector_number(-1);

	do {
		if(n_sector_number < 0)
			continue;
		//
		if(m_v_write_sec_index.empty())
			continue;
		// 에러 확률를 높이기 위해 app area 첫 sector 번호를 wrtite 순서를 나타내는 m_v_write_sec_index 의
		// 가장 마지막에 저장하므로, 마지막 값이 곧 app area 첫 sector 번호가 된다.
		n_zero_base_sector_number = n_sector_number - m_v_write_sec_index[m_v_write_sec_index.size()-1];
	} while (false);
	return n_zero_base_sector_number;
}

void cshare::_ini()
{
	m_b_display = true;
	m_b_is_possible_exit = true;

	m_b_executed_server_stop_use_target_dev = false;
	m_n_stopped_usb_vid = m_n_stopped_usb_pid = 0;

	m_b_run_by_cf = false;

	m_b_start_from_bootloader = false;

	m_b_enable_iso_mode_after_update = false;
	m_lpu23x_interface_after_update = cshare::Lpu237Interface::nc;

	m_v_firmware_list.resize(0);
	m_n_selected_fw_in_firmware_list = -1;
	m_n_index_updatable_fw_in_firmware_list = -1;
	memset(&m_rom_header, 0, sizeof(m_rom_header));

	//
	m_n_selected_file_in_v_files_in_current_dir = -1;
	m_v_files_in_current_dir.resize(0);
	m_v_files_in_current_dir_except_dir.resize(0);
	m_v_rom_files_in_current_dir.resize(0);
	m_v_bin_files_in_current_dir.resize(0);
	m_current_dir = std::filesystem::current_path();

	m_n_size_fw = 0;
	m_v_erase_sec_index.resize(0);
	m_v_write_sec_index.resize(0);
}

cshare::cshare()
{
	_ini();
}
void cshare::_set_firmware_size(size_t n_size_fw)
{
	m_n_size_fw = n_size_fw;
}

cshare::~cshare()
{

}

cshare& cshare::set_target_lpu23x(_mp::clibhid_dev::type_ptr& ptr_dev)
{
	m_ptr_dev = ptr_dev;
	return *this;
}

cshare& cshare::set_start_from_bootloader(bool b_start_from_bootloader)
{
	m_b_start_from_bootloader = b_start_from_bootloader;
	return *this;
}

cshare& cshare::set_executed_server_stop_use_target_dev(bool b_executed, int n_vid, int n_pid)
{
	m_b_executed_server_stop_use_target_dev = b_executed;
	m_n_stopped_usb_vid = n_vid;
	m_n_stopped_usb_pid = n_pid;
	return *this;
}

void cshare::clear_executed_server_stop_use_target_dev()
{
	m_b_executed_server_stop_use_target_dev = false;
	m_n_stopped_usb_vid = m_n_stopped_usb_pid = 0;
}

cshare& cshare::set_run_by_cf(bool b_yes)
{
	m_b_run_by_cf = b_yes;
	return *this;
}

cshare& cshare::set_rom_file_abs_full_path(const std::string& s_abs_full_rom_file)
{
	m_s_rom_file_abs_full_path = s_abs_full_rom_file;
	if (!s_abs_full_rom_file.empty()) {
		std::filesystem::path p(s_abs_full_rom_file);

		std::filesystem::path current_dir = p.parent_path();
		if (current_dir.empty()) {
			m_current_dir = std::filesystem::current_path();
			auto f = m_current_dir / s_abs_full_rom_file; /// std::filesystem::path::preferred_separator
			m_s_rom_file_abs_full_path = f.string();
		}
		else {
			m_current_dir = current_dir;
		}
		if (p.extension() != ".rom") {
			_set_firmware_size((size_t)std::filesystem::file_size(p));
		}
		else {
			_set_firmware_size(0);
		}
	}
	return *this;
}
cshare& cshare::set_device_path(const std::string& s_device_path)
{
	m_s_device_path = s_device_path;
	return *this;
}

cshare& cshare::set_bootloader_path(const std::string& s_bootloader_path)
{
	m_s_bootloader_path = s_bootloader_path;
	return *this;
}

cshare& cshare::set_iso_mode_after_update(bool b_enable_iso_mode_after_update)
{
	m_b_enable_iso_mode_after_update = b_enable_iso_mode_after_update;
	return *this;
}

cshare& cshare::set_lpu23x_interface_change_after_update(cshare::Lpu237Interface inf_new)
{
	m_lpu23x_interface_after_update = inf_new;
	return *this;
}

cshare& cshare::set_firmware_list_of_rom_file(int n_fw_index, const std::vector<std::string>& v_s_fw)
{
	m_n_selected_fw_in_firmware_list = n_fw_index;
	m_v_firmware_list = v_s_fw;
	_set_firmware_size(m_rom_header.Item[n_fw_index].dwSize);
	return *this;
}

cshare& cshare::set_erase_sec_index(const std::vector<int>& v_sec_index/* = std::vector<int>(0) */)
{
	m_v_erase_sec_index = v_sec_index;
	return *this;
}

cshare& cshare::set_write_sec_index(const std::vector<int>& v_sec_index /* = std::vector<int>(0) */)
{
	m_v_write_sec_index = v_sec_index;
	return *this;
}

cshare& cshare::set_possible_exit(bool b_possible)
{
	m_b_is_possible_exit = b_possible;
	return *this;
}

cshare& cshare::set_display_ui(bool b_display)
{
	m_b_display = b_display;
	return *this;
}

bool cshare::get_display_ui() const
{
	return m_b_display;
}

std::tuple<bool,int,int> cshare::is_executed_server_stop_use_target_dev() const
{
	return std::make_tuple( m_b_executed_server_stop_use_target_dev, m_n_stopped_usb_vid, m_n_stopped_usb_pid);
}

_mp::type_pair_bool_result_bool_complete cshare::io_save_all_variable_sys_parameter(bool b_first)
{
	bool b_result(false);
	bool b_complete(true);

	if (!m_ptr_dev) {
		return std::make_pair(b_result,b_complete);
	}
	//
	if (b_first) {
		m_target_protocol_lpu237.clear_transaction();
		m_target_protocol_lpu237.generate_get_parameters();
	}

	_mp::type_v_buffer v_tx;
	_mp::type_v_buffer v_rx;
	size_t n_remainder_transaction(0);

	do {
		b_result = true;

		n_remainder_transaction = m_target_protocol_lpu237.get_tx_transaction(v_tx);
		if (v_tx.size() == 0)
			continue;
		//
		if (!cshare::get_instance().io_write_read_sync(m_ptr_dev, v_tx, v_rx)) {
			b_result = false;
			break;
		}

		//waits the complete of tx.
		if (!m_target_protocol_lpu237.set_rx_transaction(v_rx)) {
			b_result = false;
			break;
		}
		if (!m_target_protocol_lpu237.set_from_rx()) {
			b_result = false;
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	} while (false);

	if (n_remainder_transaction <= 0) {
		m_target_protocol_lpu237.clear_transaction();
	}

	if (b_result && n_remainder_transaction > 0) {
		b_complete = false;
	}
	return std::make_pair(b_result, b_complete);
}

bool cshare::io_load_basic_sys_parameter(_mp::clibhid_dev::type_ptr& ptr_recoverd_dev)
{
	bool b_result(false);
	bool b_complete(false);

	if (!ptr_recoverd_dev) {
		return b_result;
	}
	//
	m_target_protocol_lpu237.clear_transaction();
	m_target_protocol_lpu237.generate_get_system_information_with_name();

	_mp::type_v_buffer v_tx;
	_mp::type_v_buffer v_rx;
	size_t n_remainder_transaction(0);

	b_result = true;

	do {
		n_remainder_transaction = m_target_protocol_lpu237.get_tx_transaction(v_tx);
		if (v_tx.size() == 0) {
			b_complete = true;
			continue;
		}
		//
		if (!cshare::get_instance().io_write_read_sync(ptr_recoverd_dev, v_tx, v_rx)) {
			b_result = false;
			break;
		}

		//waits the complete of tx.
		if (!m_target_protocol_lpu237.set_rx_transaction(v_rx)) {
			b_result = false;
			break;
		}
		if (!m_target_protocol_lpu237.set_from_rx()) {
			b_result = false;
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	} while (!b_complete);

	if (n_remainder_transaction <= 0) {
		m_target_protocol_lpu237.clear_transaction();
	}

	return b_result;
}

bool cshare::io_set_interface(_mp::clibhid_dev::type_ptr& ptr_recoverd_dev)
{
	bool b_result(false);
	bool b_complete(false);

	if (!ptr_recoverd_dev) {
		return b_result;
	}
	//
	m_target_protocol_lpu237.set_interface((cprotocol_lpu237::type_system_interface)m_lpu23x_interface_after_update);
	m_target_protocol_lpu237.clear_transaction();
	if (!m_target_protocol_lpu237.generate_set_interface()) {
		return b_result;
	}

	_mp::type_v_buffer v_tx;
	_mp::type_v_buffer v_rx;
	size_t n_remainder_transaction(0);

	b_result = true;

	do {
		n_remainder_transaction = m_target_protocol_lpu237.get_tx_transaction(v_tx);
		if (v_tx.size() == 0) {
			b_complete = true;
			continue;
		}
		//
		if (!cshare::get_instance().io_write_read_sync(ptr_recoverd_dev, v_tx, v_rx)) {
			b_result = false;
			break;
		}

		//waits the complete of tx.
		if (!m_target_protocol_lpu237.set_rx_transaction(v_rx)) {
			b_result = false;
			break;
		}
		if (!m_target_protocol_lpu237.set_from_rx()) {
			b_result = false;
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	} while (!b_complete);

	if (n_remainder_transaction <= 0) {
		m_target_protocol_lpu237.clear_transaction();
	}

	return b_result;
}

bool cshare::io_set_mmd1100_to_iso_mode(_mp::clibhid_dev::type_ptr& ptr_recoverd_dev)
{
	bool b_result(false);
	bool b_complete(false);

	if (!ptr_recoverd_dev) {
		return b_result;
	}
	//
	m_target_protocol_lpu237.set_enable_mmd1100_iso_mode(true);
	m_target_protocol_lpu237.clear_transaction();
	if (!m_target_protocol_lpu237.generate_mmd1100_to_iso_mode()) {
		return b_result;//not support iso mode change
	}

	_mp::type_v_buffer v_tx;
	_mp::type_v_buffer v_rx;
	size_t n_remainder_transaction(0);

	b_result = true;

	do {
		n_remainder_transaction = m_target_protocol_lpu237.get_tx_transaction(v_tx);
		if (v_tx.size() == 0) {
			b_complete = true;
			continue;
		}
		//
		if (!cshare::get_instance().io_write_read_sync(ptr_recoverd_dev, v_tx, v_rx)) {
			b_result = false;
			break;
		}

		//waits the complete of tx.
		if (!m_target_protocol_lpu237.set_rx_transaction(v_rx)) {
			b_result = false;
			break;
		}
		if (!m_target_protocol_lpu237.set_from_rx()) {
			b_result = false;
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	} while (!b_complete);

	if (n_remainder_transaction <= 0) {
		m_target_protocol_lpu237.clear_transaction();
	}

	return b_result;
}

_mp::type_pair_bool_result_bool_complete cshare::io_recover_all_variable_sys_parameter(bool b_first, _mp::clibhid_dev::type_ptr& ptr_recoverd_dev)
{
	bool b_result(false);
	bool b_complete(true);

	if (!ptr_recoverd_dev) {
		return std::make_pair(b_result, b_complete);
	}
	//
	if (b_first) {
		m_target_protocol_lpu237.clear_transaction();
		m_target_protocol_lpu237.set_all_parameter_to_changed();
		if (!m_target_protocol_lpu237.generate_set_parameters()) {
			return std::make_pair(b_result, b_complete);
		}
	}

	_mp::type_v_buffer v_tx;
	_mp::type_v_buffer v_rx;
	size_t n_remainder_transaction(0);

	do {
		b_result = true;

		n_remainder_transaction = m_target_protocol_lpu237.get_tx_transaction(v_tx);
		if (v_tx.size() == 0)
			continue;
		//
		if (!cshare::get_instance().io_write_read_sync(ptr_recoverd_dev, v_tx, v_rx)) {
			b_result = false;
			break;
		}

		//waits the complete of tx.
		if (!m_target_protocol_lpu237.set_rx_transaction(v_rx)) {
			b_result = false;
			break;
		}
		if (!m_target_protocol_lpu237.is_success_rx()) {
			b_result = false;
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	} while (false);

	if (n_remainder_transaction <= 0) {
		m_target_protocol_lpu237.clear_transaction();
	}

	if (b_result && n_remainder_transaction > 0) {
		b_complete = false;
	}
	return std::make_pair(b_result, b_complete);
}

bool cshare::io_get_system_info_and_set_name_version()
{
	if (!m_ptr_dev) {
		return false;
	}
	/////////////////
	m_target_protocol_lpu237.reset();
	m_target_protocol_lpu237.generate_get_system_information_with_name();

	_mp::type_v_buffer v_tx;
	_mp::type_v_buffer v_rx;
	size_t n_remainder_transaction(0);

	bool b_result = true;
	do {
		b_result = true;

		n_remainder_transaction = m_target_protocol_lpu237.get_tx_transaction(v_tx);
		if (v_tx.size() == 0)
			continue;
		//
		if (!cshare::get_instance().io_write_read_sync(m_ptr_dev, v_tx, v_rx)) {
			b_result = false;
			break;
		}

		//waits the complete of tx.
		if (!m_target_protocol_lpu237.set_rx_transaction(v_rx)) {
			b_result = false;
			break;
		}
		if (!m_target_protocol_lpu237.set_from_rx()) {
			b_result = false;
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	} while (n_remainder_transaction > 0);

	m_target_protocol_lpu237.clear_transaction();

	return b_result;
}

bool cshare::io_run_bootloader()
{
	if (!m_ptr_dev) {
		return false;
	}

	size_t n_remainder_transaction(0);
	
	m_target_protocol_lpu237.generate_enter_config_mode();
	m_target_protocol_lpu237.generate_run_boot_loader();

	_mp::type_v_buffer v_tx;
	_mp::type_v_buffer v_rx;

	bool b_result(true);
	do {
		n_remainder_transaction = m_target_protocol_lpu237.get_tx_transaction(v_tx);
		if (v_tx.size() == 0)
			continue;
		//
		if (!cshare::get_instance().io_write_read_sync(m_ptr_dev, v_tx, v_rx)) {
			b_result = false;
			break;
		}
		//waits the complete of tx.
		if (!m_target_protocol_lpu237.set_rx_transaction(v_rx)) {
			b_result = false;
			break;
		}
		if (!m_target_protocol_lpu237.set_from_rx()) {
			b_result = false;
			break;
		}

	} while (n_remainder_transaction > 0);

	m_target_protocol_lpu237.clear_transaction();

	return b_result;
}

bool cshare::io_write_sync(_mp::clibhid_dev::type_ptr& ptr_dev,const _mp::type_v_buffer& v_tx)
{
	_mp::cwait waiter;
	int n_w = waiter.generate_new_event();

	std::pair<int, _mp::cwait*> param(n_w, &waiter);
	ptr_dev->start_write(
		0, v_tx,
		[](_mp::cqitem_dev& qi, void* p_user)->std::pair<bool, std::vector<size_t> > {
			std::pair<int, _mp::cwait*>* p_param = (std::pair<int, _mp::cwait*>*)p_user;
			p_param->second->set(p_param->first);//trigger
			return std::make_pair(qi.is_complete(), std::vector<size_t>());
		},
		&param
	);

	if (waiter.wait_for_one_at_time(5000) >= 0) {
		return true;
	}
	else {
		return false;
	}

}

bool cshare::io_read_sync(_mp::clibhid_dev::type_ptr& ptr_dev,_mp::type_v_buffer& v_rx, int n_report_size /*= -1*/)
{
	_mp::cwait waiter;
	int n_w = waiter.generate_new_event();

	if (n_report_size < 0) {
		v_rx.resize(220, 0);
	}
	else {
		v_rx.resize(n_report_size, 0);
	}

	waiter.reset(n_w);
	std::tuple<int, _mp::cwait*, _mp::type_v_buffer*> param_rx(n_w, &waiter, &v_rx);
	ptr_dev->start_read(0,
		[](_mp::cqitem_dev& qi, void* p_user)->std::pair<bool, std::vector<size_t>> {
			if (qi.get_result() == _mp::cqitem_dev::result_error) {
				return std::make_pair(true, std::vector<size_t>()); // error
			}

			std::tuple<int, _mp::cwait*, _mp::type_v_buffer*>* p_param = (std::tuple<int, _mp::cwait*, _mp::type_v_buffer*>*)p_user;

			if (qi.is_complete()) {

				int n = std::get<0>(*p_param);
				_mp::cwait* p_w = std::get<1>(*p_param);
				_mp::type_v_buffer* p_v_rx = std::get<2>(*p_param);
				if (p_v_rx) {
					*p_v_rx = qi.get_rx();
				}
				if (p_w) {
					p_w->set(n);//trigger
				}
			}

			return std::make_pair(qi.is_complete(), std::vector<size_t>());
		}
	, &param_rx);

	if (waiter.wait_for_one_at_time(5000) >= 0) {
		return true;
	}
	else {
		return false;
	}
}

bool cshare::io_write_read_sync(
	_mp::clibhid_dev::type_ptr& ptr_dev
	, const _mp::type_v_buffer& v_tx
	, _mp::type_v_buffer& v_rx
	, int n_report_size /*= -1*/
)
{
	_mp::cwait waiter;
	int n_w = waiter.generate_new_event();

	if (n_report_size < 0) {
		v_rx.resize(220, 0);
	}
	else {
		v_rx.resize(n_report_size, 0);
	}
	waiter.reset(n_w);
	std::tuple<int, _mp::cwait*, _mp::type_v_buffer*> param_rx(n_w, &waiter, &v_rx);
	ptr_dev->start_write_read(0,v_tx,
		[](_mp::cqitem_dev& qi, void* p_user)->std::pair<bool, std::vector<size_t>> {
			if (qi.get_result() == _mp::cqitem_dev::result_error) {
				return std::make_pair(true, std::vector<size_t>()); // error
			}
			std::tuple<int, _mp::cwait*, _mp::type_v_buffer*>* p_param = (std::tuple<int, _mp::cwait*, _mp::type_v_buffer*>*)p_user;

			if (qi.is_complete()) {

				int n = std::get<0>(*p_param);
				_mp::cwait* p_w = std::get<1>(*p_param);
				_mp::type_v_buffer* p_v_rx = std::get<2>(*p_param);
				if (p_v_rx) {
					*p_v_rx = qi.get_rx();
				}
				if (p_w) {
					p_w->set(n);//trigger
				}
			}

			return std::make_pair(qi.is_complete(), std::vector<size_t>());
		}
	, &param_rx);

	if (waiter.wait_for_one_at_time(5000) >= 0) {
		return true;
	}
	else {
		return false;
	}
}

std::string cshare::get_string(cshare::Lpu237Interface inf)
{
	std::string s;
	switch (inf) {
	case cshare::Lpu237Interface::nc: s = "no consideration"; break;
	case cshare::Lpu237Interface::usb_keyboard: s = "usb_keyboard"; break;
	case cshare::Lpu237Interface::usb_hid: s = "usb_hid"; break;
	case cshare::Lpu237Interface::usb_vcom: s = "usb_vcom"; break;
	case cshare::Lpu237Interface::uart: s = "uart"; break;
	default:
		s = "unknown";
		break;
	}
	return s;
}


std::string cshare::get_target_device_model_name_by_string() const
{
	std::string s;
	_mp::type_v_buffer v = m_target_protocol_lpu237.get_name();
	for (auto item : v) {
		if (item == ' ' || item == 0) {
			break;
		}
		s.push_back((char)item);
	}

	return s;
}

std::string cshare::get_target_device_version_by_string() const
{
	return _mp::cstring::get_mcsc_from_unicode(m_target_protocol_lpu237.get_system_version().get_by_string());
}

bool cshare::get_start_from_bootloader() const
{
	return m_b_start_from_bootloader;
}

bool cshare::is_run_by_cf() const
{
	return m_b_run_by_cf;
}

std::string cshare::get_rom_file_abs_full_path() const
{
	return m_s_rom_file_abs_full_path;
}

std::wstring cshare::get_rom_file_abs_full_wpath() const
{
	return _mp::cstring::get_unicode_from_mcsc(m_s_rom_file_abs_full_path);
}

std::string cshare::get_device_path() const
{
	return m_s_device_path;
}

std::wstring cshare::get_device_wpath() const
{
	return _mp::cstring::get_unicode_from_mcsc(m_s_device_path);
}

std::string cshare::get_bootloader_path() const
{
	return m_s_bootloader_path;
}

std::wstring cshare::get_bootloader_wpath() const
{
	return _mp::cstring::get_unicode_from_mcsc(m_s_bootloader_path);
}

bool cshare::is_iso_mode_after_update() const
{
	return m_b_enable_iso_mode_after_update;
}

cshare::Lpu237Interface cshare::get_lpu23x_interface_change_after_update() const
{
	return m_lpu23x_interface_after_update;
}

_mp::type_v_buffer cshare::get_target_device_model_name() const
{
	return m_target_protocol_lpu237.get_name();
}

cshare::type_version cshare::get_target_device_version() const
{
	return m_target_protocol_lpu237.get_system_version();
}

bool cshare::get_target_device_mmd1100_is_iso_mode() const
{
	return m_target_protocol_lpu237.get_enable_mmd1100_is_iso_mode();
}

std::vector<std::string> cshare::get_vector_files_in_current_dir() const
{
	return m_v_files_in_current_dir;
}

std::vector<std::string> cshare::get_vector_files_in_current_dir_except_dir() const
{
	return m_v_files_in_current_dir_except_dir;
}

std::vector<std::string> cshare::get_vector_rom_files_in_current_dir() const
{
	return m_v_rom_files_in_current_dir;
}

std::vector<std::string> cshare::get_vector_bin_files_in_current_dir() const
{
	return m_v_bin_files_in_current_dir;
}

size_t cshare::get_selected_fw_size() const
{
	return m_n_size_fw;
}

std::vector<int> cshare::get_erase_sec_index() const
{
	return m_v_erase_sec_index;
}

std::vector<int> cshare::get_write_sec_index() const
{
	return m_v_write_sec_index;
}

bool cshare::is_possible_exit() const
{
	return m_b_is_possible_exit;
}

_mp::type_pair_bool_result_bool_complete cshare::get_one_sector_fw_data
(
	std::shared_ptr<CRom> ptr_rom_dll
	, _mp::type_v_buffer& v_out_sector
	, int& n_out_zero_base_sector_number
	,bool b_first_read /*=false*/
)
{
	static uint32_t dw_offset(0);
	static int n_write_sector_index(0);
	static uint32_t dw_last_readdone(CHidBootBuffer::C_SECTOR_SIZE);

	bool b_result(false), b_complete(true);

	
	uint32_t dw_read(CHidBootBuffer::C_SECTOR_SIZE);
	int n_write_sector(0);

	do {
		if (!ptr_rom_dll) {
			continue;
		}
		if (m_s_rom_file_abs_full_path.empty()) {
			continue;
		}
		if (m_v_write_sec_index.empty()) {
			continue;
		}

		if (b_first_read) {
			dw_offset = 0;
			n_write_sector_index = 0;
			dw_last_readdone = CHidBootBuffer::C_SECTOR_SIZE;
		}
		if (n_write_sector_index >= m_v_write_sec_index.size()) {
			continue;//모두 읽음.
		}

		// sector 번호
		n_out_zero_base_sector_number = m_v_write_sec_index[n_write_sector_index];
		v_out_sector.resize(CHidBootBuffer::C_SECTOR_SIZE);
		v_out_sector.assign(CHidBootBuffer::C_SECTOR_SIZE, 0xff);

		if (m_v_write_sec_index.size() > 1) {
			if (n_write_sector_index == (m_v_write_sec_index.size() - 2)) {
				// 마지막 sector 는 app area 첫 sector 번호이므로,
				// 마지막 전 sector 는 진짜 마지막 secotor 이어서, 4096 byte 보다 작을 수 있다.
				dw_read = (uint32_t)(m_n_size_fw - ((m_v_write_sec_index.size()-1)*CHidBootBuffer::C_SECTOR_SIZE));
			}
			else {
				dw_read = (uint32_t)(CHidBootBuffer::C_SECTOR_SIZE);
			}
			
		}
		else {
			dw_read = (uint32_t)m_n_size_fw;
		}

		

		if (m_n_selected_fw_in_firmware_list < 0) {
			// 순수 raw binary file 인 경우.
			n_write_sector = m_v_write_sec_index[n_write_sector_index];
			if (dw_last_readdone < CHidBootBuffer::C_SECTOR_SIZE) {
				dw_offset = 0;
			}
			else {
				dw_offset = n_write_sector * CHidBootBuffer::C_SECTOR_SIZE;
			}

			std::ifstream file_raw(m_s_rom_file_abs_full_path, std::ios::binary);
			if(!file_raw){
				continue; //error
			}

			file_raw.seekg(dw_offset, std::ios::beg);
			if (!file_raw) {
				continue; //error
			}

			file_raw.read(reinterpret_cast<char*>(v_out_sector.data()), dw_read);
			//실제 읽은 바이트 수
			dw_last_readdone = (uint32_t)file_raw.gcount();
#if defined(_WIN32) && defined(_DEBUG)
			ATLTRACE(" :::::::: %u = file_raw.read([%x,%x,%x,%x],%u[read],%u[offset],item)", dw_last_readdone,v_out_sector[0], v_out_sector[1], v_out_sector[2], v_out_sector[3], dw_read, dw_offset);
#endif
			if (dw_last_readdone == 0) {
				continue; //error
			}
		}
		else {
			// rom file 인 경우.
			// 실제 sector 번호에서 app area 첫 sector 번호를 0 으로 가정한 상대 sector 번호를 얻음.
			n_write_sector = m_v_write_sec_index[n_write_sector_index]- m_v_write_sec_index[m_v_write_sec_index.size() - 1];
			if (dw_last_readdone < CHidBootBuffer::C_SECTOR_SIZE) {
				dw_offset = 0;
			}
			else {
				dw_offset = n_write_sector * CHidBootBuffer::C_SECTOR_SIZE;
			}

			dw_last_readdone = (uint32_t)ptr_rom_dll->ReadBinaryOfItem(v_out_sector.data(), dw_read, dw_offset, &m_rom_header.Item[m_n_selected_fw_in_firmware_list]);
#if defined(_WIN32) && defined(_DEBUG)
			//ATLTRACE(" :::::::: %u = ReadBinaryOfItem([%x,%x,%x,%x],%u[read],%u[offset],item)", dw_last_readdone,v_out_sector[0], v_out_sector[1], v_out_sector[2], v_out_sector[3], dw_read, dw_offset);
#endif
			if (dw_last_readdone ==0) {
				continue; //error
			}
		}

		++n_write_sector_index;
		if(n_write_sector_index < m_v_write_sec_index.size()){

			b_complete = false;
		}
		
		b_result = true;
	} while (false);

	return std::make_pair(b_result,b_complete);
}

std::pair<int, std::vector<std::string>> cshare::get_firmware_list_of_rom_file() const
{
	return std::make_pair(m_n_selected_fw_in_firmware_list, m_v_firmware_list);
}

std::tuple<int, std::filesystem::path, std::vector<std::string>, std::vector<std::string>> cshare::get_file_list_of_selected_dir() const
{

	return std::make_tuple(
		m_n_selected_file_in_v_files_in_current_dir,
		m_current_dir,
		m_v_files_in_current_dir,
		m_v_rom_files_in_current_dir
	);
}

std::pair<int, int> cshare::update_fw_list_of_selected_rom(std::shared_ptr<CRom> ptr_rom_dll)
{
	int n_updatable_fw_index(-1);
	int n_total_fw(0);

	do {
		if (!ptr_rom_dll) {
			continue;
		}
		if (m_s_rom_file_abs_full_path.empty()) {
			continue;
		}

		std::filesystem::path p(m_s_rom_file_abs_full_path);
		if (p.extension() != ".rom") {
			continue;
		}

		std::wstring ws_abs_full_path_of_rom = get_rom_file_abs_full_wpath();

		memset(&m_rom_header, 0, sizeof(m_rom_header));
		CRom::type_result result_rom_dll = ptr_rom_dll->LoadHeader(ws_abs_full_path_of_rom.c_str(), &m_rom_header);
		if (result_rom_dll != CRom::result_success) {
			continue;
		}

		m_v_firmware_list.clear();

		n_total_fw = (int)m_rom_header.dwItem;

		for (uint32_t i = 0; i < m_rom_header.dwItem; ++i) {
			CRom::ROMFILE_HEAD_ITEAM& item = m_rom_header.Item[i];
			std::string s_model = (char*)item.sModel;
			std::string s_version = std::to_string((int)item.sVersion[0]) + "." +
				std::to_string((int)item.sVersion[1]) + "." +
				std::to_string((int)item.sVersion[2]) + "." +
				std::to_string((int)item.sVersion[3]);
			std::string s_cond;
			if (item.dwUpdateCondition & CRom::condition_eq) {
				s_cond += " EQ";
			}
			if (item.dwUpdateCondition & CRom::condition_neq) {
				s_cond += " NEQ";
			}
			if (item.dwUpdateCondition & CRom::condition_gt) {
				s_cond += " GT";
			}
			if (item.dwUpdateCondition & CRom::condition_lt) {
				s_cond += " LT";
			}
			if (s_cond.empty()) {
				s_cond = " None";
			}
			std::string s_list_item = std::to_string(i) + ": " + s_model + " v" + s_version + " (" + s_cond + ")";
			m_v_firmware_list.push_back(s_list_item);
		}//end for

		if (m_v_firmware_list.empty()) {
			continue;
		}

		auto v_name = get_target_device_model_name();
		auto version = get_target_device_version();

		if (v_name.empty()) {
			continue;
		}

		n_updatable_fw_index = ptr_rom_dll->GetUpdatableItemIndex(
			&v_name[0],
			version.get_major(),
			version.get_minor(),
			version.get_fix(),
			version.get_build()
		);

	} while (false);

	m_n_index_updatable_fw_in_firmware_list = n_updatable_fw_index;
	if (n_updatable_fw_index >= 0) {
		m_n_selected_fw_in_firmware_list = n_updatable_fw_index;
		_set_firmware_size(m_rom_header.Item[n_updatable_fw_index].dwSize);
	}

	return std::make_pair(n_total_fw,n_updatable_fw_index);
}

bool cshare::is_select_fw_updatable() const
{
	if (m_n_index_updatable_fw_in_firmware_list > 0 && m_n_selected_fw_in_firmware_list == m_n_index_updatable_fw_in_firmware_list) {
		return true;
	}
	else {
		return false;
	}
}

void cshare::update_files_list_of_cur_dir(const std::filesystem::path& current_dir)
{
	m_current_dir = current_dir;
	m_v_files_in_current_dir.clear();
	m_v_files_in_current_dir_except_dir.clear();
	m_v_rom_files_in_current_dir.clear();
	m_v_bin_files_in_current_dir.clear();

#ifdef _WIN32
	m_v_files_in_current_dir.push_back("..");
#else
	m_v_files_in_current_dir.push_back("..");
#endif
	for (const auto& entry : std::filesystem::directory_iterator(current_dir)) {
		if (entry.is_regular_file()) {
			// regular file only.
			std::filesystem::path path_ext = entry.path().extension();
			if (path_ext != ".rom" && path_ext != ".bin") {
				continue;
			}
			if (path_ext == ".rom") {
				//m_v_rom_files_in_current_dir.push_back(entry.path().filename().string());
				m_v_rom_files_in_current_dir.push_back(entry.path().string());
			}
			else if (path_ext == ".bin") {
				//m_v_bin_files_in_current_dir.push_back(entry.path().filename().string());
				m_v_bin_files_in_current_dir.push_back(entry.path().string());
			}

			m_v_files_in_current_dir_except_dir.push_back(entry.path().filename().string());
		}

		std::string name = entry.path().filename().string();
		if (entry.is_directory()) {
#ifdef _WIN32
			//name += "\\";
#else
			//name += "/";
#endif
		}
		m_v_files_in_current_dir.push_back(name);
	}

	std::sort(m_v_bin_files_in_current_dir.begin(), m_v_bin_files_in_current_dir.end());
	std::sort(m_v_rom_files_in_current_dir.begin(), m_v_rom_files_in_current_dir.end());
	std::sort(m_v_files_in_current_dir.begin(), m_v_files_in_current_dir.end());

	if (m_v_files_in_current_dir.empty()) {
		m_n_selected_file_in_v_files_in_current_dir = -1;
	}
	else {
		m_n_selected_file_in_v_files_in_current_dir = 0;
	}
}


