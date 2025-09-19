
#include <mp_cstring.h>
#include "cshare.h"
#include <tg_rom.h>
#include "HidBootManager.h"

cshare& cshare::get_instance()
{
	static cshare obj;
	return obj;
}

void cshare::_ini()
{
	m_b_start_from_bootloader = false;
	m_b_manual_mode = true;

	m_b_enable_iso_mode_after_update = false;
	m_lpu23x_interface_after_update = cshare::Lpu237Interface::nc;

	m_n_index_of_fw_in_rom = -1;
	m_s_update_condition_of_fw.clear();
	m_b_fw_file_is_rom_format = false;

	//m_v_target_device_model_name.resize(CRom::MAX_MODEL_NAME_SIZE, 0);
	m_v_target_device_model_name.resize(0);
	m_version_target_device = cshare::type_version(0, 0, 0, 0);

	m_v_firmware_list.resize(0);
	m_n_selected_fw = -1;
	m_n_index_updatable_fw_in_rom = -1;

	//
	m_n_selected_file_in_v_files_in_current_dir = -1;
	m_v_files_in_current_dir.resize(0);
	m_v_files_in_current_dir_except_dir.resize(0);
	m_v_rom_files_in_current_dir.resize(0);
	m_current_dir = std::filesystem::current_path();
}

cshare::cshare()
{
	_ini();
}
cshare::~cshare()
{

}

cshare& cshare::set_start_from_bootloader(bool b_start_from_bootloader)
{
	m_b_start_from_bootloader = b_start_from_bootloader;
	return *this;
}

cshare& cshare::set_manual_mode(bool b_enable)
{
	m_b_manual_mode = b_enable;
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

cshare& cshare::set_index_of_fw_in_rom(int n_index)
{
	m_n_index_of_fw_in_rom = n_index;
	return *this;
}

cshare& cshare::set_update_condition_of_fw(const std::wstring& s_cond)
{
	m_s_update_condition_of_fw = s_cond;
	return *this;
}
cshare& cshare::set_fw_file_is_rom_format(bool b_yes)
{
	m_b_fw_file_is_rom_format = b_yes;
	return *this;
}
cshare& cshare::set_target_device_model_name(const _mp::type_v_buffer& v_name)
{
	m_v_target_device_model_name = v_name;
	return *this;
}
cshare& cshare::set_version_target_device(const cshare::type_version& v)
{
	m_version_target_device = v;
	return *this;
}

cshare& cshare::set_firmware_list_of_rom_file(int n_fw_index, const std::vector<std::string>& v_s_fw)
{
	m_n_selected_fw = n_fw_index;
	m_v_firmware_list = v_s_fw;
	return *this;
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


bool cshare::get_start_from_bootloader() const
{
	return m_b_start_from_bootloader;
}

bool cshare::is_manual() const
{
	return m_b_manual_mode;
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

bool cshare::is_iso_mode_after_update() const
{
	return m_b_enable_iso_mode_after_update;
}

cshare::Lpu237Interface cshare::get_lpu23x_interface_change_after_update() const
{
	return m_lpu23x_interface_after_update;
}

int cshare::get_index_of_fw_in_rom() const
{
	return m_n_index_of_fw_in_rom;
}

std::wstring cshare::get_update_condition_of_fw() const
{
	return m_s_update_condition_of_fw;
}

bool cshare::is_fw_file_is_rom_format() const
{
	return m_b_fw_file_is_rom_format;
}

_mp::type_v_buffer cshare::get_target_device_model_name() const
{
	return m_v_target_device_model_name;
}

cshare::type_version cshare::get_version_target_device() const
{
	return m_version_target_device;
}

bool cshare::is_update_available() const
{
	bool b_result(false);
	do {
		if (m_s_rom_file_abs_full_path.empty()) {
			continue; // No ROM file set, cannot check for updates
		}
		if (m_n_index_of_fw_in_rom < 0 && m_b_fw_file_is_rom_format) {
			continue; // Invalid index for firmware in ROM
		}

		b_result = true; // Assume update is available if conditions are met
	} while (false);
	return b_result;

}

std::pair<int, std::vector<std::string>> cshare::get_firmware_list_of_rom_file() const
{
	return std::make_pair(m_n_selected_fw, m_v_firmware_list);
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

int cshare::update_fw_list_of_selected_rom()
{
	int n_updatable_fw_index(-1);

	do {
		auto ptr_rom_dll = CHidBootManager::GetInstance()->get_rom_library();
		if (!ptr_rom_dll) {
			continue;
		}
		if (m_s_rom_file_abs_full_path.empty()) {
			continue;
		}
		if (!m_b_fw_file_is_rom_format) {
			m_b_fw_file_is_rom_format = false;
			continue;
		}

		std::wstring ws_abs_full_path_of_rom = get_rom_file_abs_full_wpath();

		CRom::ROMFILE_HEAD rom_header{ 0, };
		CRom::type_result result_rom_dll = ptr_rom_dll->LoadHeader(ws_abs_full_path_of_rom.c_str(), &rom_header);
		if (result_rom_dll != CRom::result_success) {
			continue;
		}

		m_b_fw_file_is_rom_format = true;

		m_v_firmware_list.clear();

		for (uint32_t i = 0; i < rom_header.dwItem; ++i) {
			CRom::ROMFILE_HEAD_ITEAM& item = rom_header.Item[i];
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

		if (m_v_target_device_model_name.empty()) {
			continue;
		}
		n_updatable_fw_index = ptr_rom_dll->GetUpdatableItemIndex(
			&m_v_target_device_model_name[0],
			m_version_target_device.get_major(),
			m_version_target_device.get_minor(),
			m_version_target_device.get_fix(),
			m_version_target_device.get_build()
		);

	} while (false);

	m_n_index_updatable_fw_in_rom = n_updatable_fw_index;
	if (n_updatable_fw_index >= 0) {
		m_n_selected_fw = n_updatable_fw_index;
	}

	return n_updatable_fw_index;
}

bool cshare::is_select_fw_updatable() const
{
	if (m_n_index_updatable_fw_in_rom > 0 && m_n_selected_fw == m_n_index_updatable_fw_in_rom) {
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

	std::sort(m_v_rom_files_in_current_dir.begin(), m_v_rom_files_in_current_dir.end());
	std::sort(m_v_files_in_current_dir.begin(), m_v_files_in_current_dir.end());

	if (m_v_files_in_current_dir.empty()) {
		m_n_selected_file_in_v_files_in_current_dir = -1;
	}
	else {
		m_n_selected_file_in_v_files_in_current_dir = 0;
	}
}


