#pragma once

#include <filesystem>
#include <mp_type.h>
#include <mp_cversion.h>

class cshare {
public:
	enum class Lpu237Interface {
		nc = -1, //no consideration
		usb_keyboard = 0,//0
		usb_hid = 1,//1
		usb_vcom = 2,//2
		uart = 10
	};

	typedef _mp::cversion<unsigned char>	type_version;

public:
	static std::string get_string(cshare::Lpu237Interface inf);

	static cshare& get_instance();
	
	~cshare();

	cshare &set_start_from_bootloader(bool b_start_from_bootloader);
	cshare& set_manual_mode(bool b_enable);
	cshare& set_rom_file_abs_full_path(const std::string &s_abs_full_rom_file);
	cshare& set_device_path(const std::string& s_device_path);
	cshare& set_iso_mode_after_update(bool b_enable_iso_mode_after_update);
	cshare& set_lpu23x_interface_change_after_update(cshare::Lpu237Interface inf_new);

	cshare& set_index_of_fw_in_rom(int n_index);
	cshare& set_update_condition_of_fw(const std::wstring& s_cond);
	cshare& set_fw_file_is_rom_format(bool b_yes);
	cshare& set_target_device_model_name(const _mp::type_v_buffer& v_name);
	cshare& set_version_target_device(const cshare::type_version& v);
	cshare& set_firmware_list_of_rom_file(int n_fw_index, const std::vector<std::string>& v_s_fw);

	bool get_start_from_bootloader() const;
	bool is_manual() const;
	std::string get_rom_file_abs_full_path() const;
	std::wstring get_rom_file_abs_full_wpath() const;
	std::string get_device_path() const;
	bool is_iso_mode_after_update() const;
	cshare::Lpu237Interface get_lpu23x_interface_change_after_update() const;

	int get_index_of_fw_in_rom() const;
	std::wstring get_update_condition_of_fw() const;
	bool is_fw_file_is_rom_format() const;
	_mp::type_v_buffer get_target_device_model_name() const;
	cshare::type_version get_version_target_device() const;
	bool is_update_available() const;

	/**
	* 
	* @return first(>=0 : selected fw index), second fw vector
	*/
	std::pair<int, std::vector<std::string>> get_firmware_list_of_rom_file() const;

	/**
	*
	* @return get<0> - the selected index of gt<2>
	* 
	*	get<1> - the current path
	* 
	*	get<2> - all file path vector of the current path.
	* 
	*	get<3> - rom file path vector of the current path.
	*/
	std::tuple<int, std::filesystem::path, std::vector<std::string>, std::vector<std::string>>
		get_file_list_of_selected_dir() const;

	/**
	* @brief update fw list with the selected rom file
	* @return  0>= : updatable firmware index.
	*/
	int update_fw_list_of_selected_rom();

	void update_files_list_of_cur_dir(const std::filesystem::path &current_dir);

	bool is_select_fw_updatable() const;

private:
	void _ini();
	cshare();

private:
	bool m_b_start_from_bootloader;
	bool m_b_manual_mode;
	std::string m_s_rom_file_abs_full_path;
	std::string m_s_device_path;

	bool m_b_enable_iso_mode_after_update;
	cshare::Lpu237Interface m_lpu23x_interface_after_update;
	//
	//rom file section
	int m_n_index_of_fw_in_rom;
	std::wstring m_s_update_condition_of_fw;
	bool m_b_fw_file_is_rom_format;

	// file list section.
	int m_n_selected_file_in_v_files_in_current_dir;
	std::vector<std::string> m_v_files_in_current_dir;//this include folder and ".."
	std::vector<std::string> m_v_files_in_current_dir_except_dir;//this exclude folder and ".."
	std::vector<std::string> m_v_rom_files_in_current_dir; // rom file list in the m_v_files_in_current_dir.
	std::filesystem::path m_current_dir;

	// fw section
	std::vector<std::string> m_v_firmware_list; //firmware list in the selected rom file.
	int m_n_selected_fw;
	int m_n_index_updatable_fw_in_rom;

	//target device section
	_mp::type_v_buffer m_v_target_device_model_name;
	cshare::type_version m_version_target_device;

private:
	cshare(const cshare&) = delete;
	cshare& operator=(const cshare&) = delete;
};
