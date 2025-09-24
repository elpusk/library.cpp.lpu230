
#include <mp_cstring.h>
#include <mp_cwait.h>

#include "cshare.h"
#include <tg_rom.h>

cshare& cshare::get_instance()
{
	static cshare obj;
	return obj;
}

int cshare::calculate_update_step()
{
	int n_step(0);

	do {
		if (!m_b_start_from_bootloader) {
			//lpu23x 로 부터 시작
			if (!m_ptr_dev) {
				// lpu23x 로 부터 시작하자만 선태괸 장비가 없어 에러.
				continue;
			}

			// generate_get_parameters() 전에
			// generate_get_system_information() ,generate_get_system_information_with() 
			// 로 장비로 부터 데이터를 얻어 왔어야 함.

			// system parameter 저장을 위한 step 개수를 더함.
			m_target_protocol_lpu237.clear_transaction();
			m_target_protocol_lpu237.generate_get_parameters();
			n_step += (int)m_target_protocol_lpu237.get_generated_the_number_of_tx();
			m_target_protocol_lpu237.clear_transaction();

			// system parameter 복원을 위한 step 개수를 더함.
			m_target_protocol_lpu237.set_all_parameter_to_changed();
			m_target_protocol_lpu237.generate_set_parameters();
			n_step += (int)m_target_protocol_lpu237.get_generated_the_number_of_tx();
			m_target_protocol_lpu237.clear_transaction();

			n_step += 1;// run boot loader
			n_step += 1;// lpu23x plugout 기다림
			n_step += 1;// bootloader plugin 기다림.
			n_step += 1;// apply step.
		}
		/////////////////////////////
		// 공용 step 의 개수.

		// erase 수, 
		// write sector 의 수
		n_step += 1;// run app
		n_step += 1;// bootload plugout 기다림
		n_step += 1;// lpu23x lpugin 기다림.

	} while (false);

	return n_step;
}

void cshare::_ini()
{
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
}

cshare::cshare()
{
	_ini();
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

cshare& cshare::set_run_by_cf(bool b_yes)
{
	m_b_run_by_cf = b_yes;
	return *this;
}

cshare& cshare::set_rom_file_abs_full_path(const std::string& s_abs_full_rom_file)
{
	m_s_rom_file_abs_full_path = s_abs_full_rom_file;
	if (!s_abs_full_rom_file.empty()) {
		m_current_dir = std::filesystem::path(s_abs_full_rom_file).parent_path();
	}
	return *this;
}
cshare& cshare::set_device_path(const std::string& s_device_path)
{
	m_s_device_path = s_device_path;
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
	return *this;
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

	if (waiter.wait_for_one_at_time(1000) >= 0) {
		return true;
	}
	else {
		return false;
	}

}

bool cshare::io_read_sync(_mp::clibhid_dev::type_ptr& ptr_dev,_mp::type_v_buffer& v_rx)
{
	_mp::cwait waiter;
	int n_w = waiter.generate_new_event();

	v_rx.resize(220,0);
	waiter.reset(n_w);
	std::tuple<int, _mp::cwait*, _mp::type_v_buffer*> param_rx(n_w, &waiter, &v_rx);
	ptr_dev->start_read(0,
		[](_mp::cqitem_dev& qi, void* p_user)->std::pair<bool, std::vector<size_t>> {
			std::tuple<int, _mp::cwait*, _mp::type_v_buffer*>* p_param = (std::tuple<int, _mp::cwait*, _mp::type_v_buffer*>*)p_user;

			if (qi.is_complete()) {

				int n = std::get<0>(*p_param);
				_mp::cwait* p_w = std::get<1>(*p_param);
				_mp::type_v_buffer* p_v_rx = std::get<2>(*p_param);
				*p_v_rx = qi.get_rx();
				p_w->set(n);//trigger
			}

			return std::make_pair(qi.is_complete(), std::vector<size_t>());
		}
	, &param_rx);

	if (waiter.wait_for_one_at_time(2000) >= 0) {
		return true;
	}
	else {
		return false;
	}
}

bool cshare::io_write_read_sync(_mp::clibhid_dev::type_ptr& ptr_dev, const _mp::type_v_buffer& v_tx, _mp::type_v_buffer& v_rx)
{
	_mp::cwait waiter;
	int n_w = waiter.generate_new_event();

	v_rx.resize(220, 0);
	waiter.reset(n_w);
	std::tuple<int, _mp::cwait*, _mp::type_v_buffer*> param_rx(n_w, &waiter, &v_rx);
	ptr_dev->start_write_read(0,v_tx,
		[](_mp::cqitem_dev& qi, void* p_user)->std::pair<bool, std::vector<size_t>> {
			std::tuple<int, _mp::cwait*, _mp::type_v_buffer*>* p_param = (std::tuple<int, _mp::cwait*, _mp::type_v_buffer*>*)p_user;

			if (qi.is_complete()) {

				int n = std::get<0>(*p_param);
				_mp::cwait* p_w = std::get<1>(*p_param);
				_mp::type_v_buffer* p_v_rx = std::get<2>(*p_param);
				*p_v_rx = qi.get_rx();
				p_w->set(n);//trigger
			}

			return std::make_pair(qi.is_complete(), std::vector<size_t>());
		}
	, &param_rx);

	if (waiter.wait_for_one_at_time(2000) >= 0) {
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
				m_v_rom_files_in_current_dir.push_back(entry.path().filename().string());
			}
			else if (path_ext == ".bin") {
				m_v_bin_files_in_current_dir.push_back(entry.path().filename().string());
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


