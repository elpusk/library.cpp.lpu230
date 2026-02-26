#pragma once

#include <filesystem>
#include <atomic>
#include <mp_type.h>
#include <mp_cversion.h>
#include <hid/mp_clibhid_dev.h>
#include <cprotocol_lpu237.h>

#include "tg_rom.h"

class cshare {
public:
	enum : size_t {
		const_size_max_firmware = 90 * 1024 * 1024
	};

	enum class Lpu237Interface : unsigned char {
		nc = 0xFF, //no consideration
		usb_keyboard = 0,//0
		usb_hid = 1,//1
		usb_vcom = 2,//2
		uart = 10
	};

	typedef _mp::cversion<unsigned char>	type_version;

	//1'st - true : executed stop_use_target_dev- request. 2'nd - stopped usb vid, 3'rd - stopped usb pid, 4'th - stopped usb pis.
	typedef std::tuple<bool, int, int, std::wstring> type_tuple_executed_server_stop_use_target_dev;
	typedef std::vector<cshare::type_tuple_executed_server_stop_use_target_dev> type_v_tuple_executed_server_stop_use_target_dev;

public:
	_mp::type_pair_bool_result_bool_complete io_save_all_variable_sys_parameter(bool b_first);
	bool io_load_basic_sys_parameter(_mp::clibhid_dev::type_ptr& ptr_recoverd_dev);
	bool io_set_interface(_mp::clibhid_dev::type_ptr& ptr_recoverd_dev);
	bool io_set_mmd1100_to_iso_mode(_mp::clibhid_dev::type_ptr& ptr_recoverd_dev);

	_mp::type_pair_bool_result_bool_complete io_recover_all_variable_sys_parameter(bool b_first, _mp::clibhid_dev::type_ptr & ptr_recoverd_dev);

	/**
	* @brief from ptr_dev, get system information( name, version, type , and so on )
	* 
	*	set system name & version information.
	*/
	bool io_get_system_info_and_set_name_version();
	bool io_run_bootloader();

	static bool io_write_sync(_mp::clibhid_dev::type_ptr & ptr_dev,const _mp::type_v_buffer& v_tx);
	static bool io_read_sync(_mp::clibhid_dev::type_ptr& ptr_dev, _mp::type_v_buffer& v_rx, int n_report_size = -1);
	static bool io_write_read_sync(
		_mp::clibhid_dev::type_ptr& ptr_dev
		, const _mp::type_v_buffer& v_tx
		, _mp::type_v_buffer& v_rx
		, int n_report_size = -1
	);

	static std::string get_string(cshare::Lpu237Interface inf);

	static cshare& get_instance();
	
	~cshare();

	cshare& set_target_lpu23x(_mp::clibhid_dev::type_ptr& ptr_dev);
	cshare &set_start_from_bootloader(bool b_start_from_bootloader);

	cshare& set_executed_server_stop_use_target_dev(bool b_executed, int n_vid, int n_pid,const std::wstring& s_pis);
	void clear_executed_server_stop_use_target_dev();

	/**
	* @brief this updater is run by coffee manager 2nd.
	*/
	cshare& set_run_by_cf(bool b_yes);

	/**
	* @brief 롬파일의 절대 경로를 설정.(m_s_rom_file_abs_full_path)
	* 
	*   롬파일의 절대 경로가 유효하면, m_current_dir 를 롬파일의 directory 로 설정.
	* 
	*   롬파일의 절대 경로가 유효라지 않으면	, m_current_dir 는 현재 실행 파일의 절대 경로로 설정.
	* 
	*   롬파일이 binary(raw) 파일이면, 펨웨어 크기 설정(m_n_size_fw).
	* 
	*   롬파일이 rom 구조의 파일이면, 펨웨어 크기를 0 으로 설정.(선택되는 fw 마다 펨웨어 크기가 달라지기 때문.)
	* 
	* @param s_abs_full_rom_file - 롬파일의 절대 경로. 예시) "C:\update\romfile.rom"
	*   
	*   s_abs_full_rom_file 가 empty 이면, 아무 것도 하지 않는다.
	*/
	cshare& set_rom_file_abs_full_path(const std::string &s_abs_full_rom_file);
	cshare& set_firmware_index_by_user(int n_fw_index); // fw index by user assigned value.
	cshare& set_device_path(const std::string& s_device_path);
	cshare& set_bootloader_path(const std::string& s_bootloader_path);
	cshare& set_target_vid(int n_vid = 0); //default is reset
	cshare& set_target_pid(int n_pid = 0); //default is reset
	cshare& set_target_pis(const std::wstring& s_pis = std::wstring());//default is reset

	cshare& set_iso_mode_after_update(bool b_enable_iso_mode_after_update);
	cshare& set_lpu23x_interface_change_after_update(cshare::Lpu237Interface inf_new);

	/**
	* @brief set firmware list of the selected rom file.
	* 
	* @param n_fw_index : the selected firmware index of the selected rom file. if n_fw_index is greater than or equal to 0.
	* 
	* @parm  v_s_fw : 만약 v_s_fw 가 empty 가 아니면, m_v_firmware_list 를 v_s_fw 로 설정하고, m_n_size_fw 를 선택된 펌웨어의 크기로 설정한다.
	* 
	*   empty 이면 m_v_firmware_list 는 변경하지 않고 m_n_size_fw 는 변경.
	* 
	*/
	cshare& set_firmware_list_of_rom_file(int n_fw_index, const std::vector<std::string>& v_s_fw = std::vector<std::string>(0));

	cshare& set_erase_sec_index(const std::vector<int>& v_sec_index = std::vector<int>(0));
	cshare& set_write_sec_index(const std::vector<int>& v_sec_index = std::vector<int>(0));

	cshare& set_possible_exit(bool b_possible);
	cshare& set_display_ui(bool b_display);

	cshare& set_plugin_device_info_after_update(const _mp::clibhid_dev_info& dev_info_after_update);


	bool get_display_ui() const;
	/**
	* @return get<0> of vector item - trure : executed stop_use_target_dev- request.
	* @return get<1> of vector item - stopped usb vid
	* @return get<2> of vector item - stopped usb pid
	* @return get<3> of vector item - stopped usb pis
	*/
	const cshare::type_v_tuple_executed_server_stop_use_target_dev & get_executed_server_stop_use_target_devs() const;

	std::string get_target_device_model_name_by_string() const;
	std::string get_target_device_version_by_string() const;

	bool get_start_from_bootloader() const;

	/**
	* @brief this updater is run bt coffee manager?
	*/
	bool is_run_by_cf() const;
	std::string get_rom_file_abs_full_path() const;
	std::wstring get_rom_file_abs_full_wpath() const;
	int get_firmware_index_by_user() const;
	std::string get_device_path() const;
	std::wstring get_device_wpath() const;
	int get_target_vid() const;
	int get_target_pid() const;
	std::wstring get_target_pis() const;

	std::string get_bootloader_path() const;
	std::wstring get_bootloader_wpath() const;

	bool is_iso_mode_after_update() const;
	cshare::Lpu237Interface get_lpu23x_interface_change_after_update() const;

	_mp::type_v_buffer get_target_device_model_name() const;
	cshare::type_version get_target_device_version() const;
	bool get_target_device_mmd1100_is_iso_mode() const;
	
	std::vector<std::string> get_vector_files_in_current_dir() const;
	std::vector<std::string> get_vector_files_in_current_dir_except_dir() const;
	std::vector<std::string> get_vector_rom_files_in_current_dir() const;
	std::vector<std::string> get_vector_bin_files_in_current_dir() const;

	size_t get_selected_fw_size() const;

	std::vector<int> get_erase_sec_index() const;
	std::vector<int> get_write_sec_index() const;

	bool is_possible_exit() const;

	/**
	* @brief get one sector fw data from the selected firmware of the selected rom file. or raw bin file.
	* @param ptr_rom_dll - the instance of CRom.
	* @param v_out_sector - the output sector data buffer. the size of this buffer will be resize to sector size(4K).
	* @param n_out_zero_base_sector_number - the output zero based sector number of the selected firmware.
	* @param b_first_read - if true, the read pointer of the selected firmware is set to the first sector.
	* @return first - true : read OK, false : read error.
	* 
	*	second - true : all read done, false : remain more data.
	* 
	*	third - std::string - error message when read error, or empty string when read OK.
	* 
	*/
	std::tuple<bool,bool,std::string> get_one_sector_fw_data(
		std::shared_ptr<CRom> ptr_rom_dll
		, _mp::type_v_buffer& v_out_sector
		, int& n_out_zero_base_sector_number
		,bool b_first_read = false
	);

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
	* @return  first - the number of fw in the rom file, second - 0>= : updatable firmware index.
	*/
	std::pair<int,int> update_fw_list_of_selected_rom(std::shared_ptr<CRom> ptr_rom_dll);

	/**
	* @brief set current file lists with the given current directory.
	* 
	*	changed : m_current_dir, m_n_selected_file_in_v_files_in_current_dir
	*
	*	changed : m_v_files_in_current_dir, m_v_files_in_current_dir_except_dir.
	* 
	*	changed : m_v_rom_files_in_current_dir, m_v_bin_files_in_current_dir.
	* 
	*/
	void update_files_list_of_cur_dir(const std::filesystem::path &current_dir);

	bool is_select_fw_updatable() const;

	/**
	* @brief calculate the number of update step.
	* @param n_the_number_of_erase_sector : the numb of erase sector
	* 
	*	if int n_the_number_of_erase_sector  is negative, the number of erase sector is equal to writing it.
	* 
	* @return the number of update step.
	*/
	int calculate_update_step(int n_the_number_of_erase_sector = -1);

	/**
	* @brief convert sector number to app area zero based sector number.
	* @param n_sector_number - sector number.
	* @return -1 : error, 0>= : app area zero based sector number.
	*/
	int get_app_area_zero_based_start_sector_from_sector_number( int n_sector_number) const;

	_mp::clibhid_dev_info get_plugin_device_info_after_update() const;

private:
	void _ini();
	cshare();

	void _set_firmware_size(size_t n_size_fw);

private:
	_mp::clibhid_dev_info m_plugin_dev_info_after_update; // device info after update.

	bool m_b_display; // display(default) ui or not
	// exitable or not
	std::atomic_bool m_b_is_possible_exit;

	// devices of executed stop_use_target_dev- request.
	// Indicate that a stop request for device usage has been sent to the server.
	// If this first value is true, request to resume usage once the update is completed.
	cshare::type_v_tuple_executed_server_stop_use_target_dev m_v_tuple_executed_server_stop_use_target_dev; 

	bool m_b_run_by_cf; // this updater is run by coffee manager.
	bool m_b_start_from_bootloader;
	std::string m_s_rom_file_abs_full_path;//rom or bin file path.
	int m_n_fw_index_by_user; // fw index by user assigned value. -1 means not assigned.(auto)
	std::string m_s_device_path;// if the given device is bootloader, then m_s_device_path can be eqaul to m_s_bootloader_path.
	std::string m_s_bootloader_path;// if the given device is bootloader, then m_s_device_path can be eqaul to m_s_bootloader_path.
	int m_n_target_vid;
	int m_n_target_pid;
	std::wstring m_s_target_pis; // usb port id string

	bool m_b_enable_iso_mode_after_update;
	cshare::Lpu237Interface m_lpu23x_interface_after_update;
	
	// file list section.
	int m_n_selected_file_in_v_files_in_current_dir;
	std::vector<std::string> m_v_files_in_current_dir;//this include folder and ".."
	std::vector<std::string> m_v_files_in_current_dir_except_dir;//this exclude folder and ".."
	std::vector<std::string> m_v_rom_files_in_current_dir; // rom file list in the m_v_files_in_current_dir.(full path)
	std::vector<std::string> m_v_bin_files_in_current_dir; // bin file list in the m_v_files_in_current_dir.(full path)
	std::filesystem::path m_current_dir;

	//fw section of the rom file
	std::vector<std::string> m_v_firmware_list; //firmware list in the selected rom file.
	int m_n_selected_fw_in_firmware_list;
	int m_n_index_updatable_fw_in_firmware_list;
	CRom::ROMFILE_HEAD m_rom_header;

	//target device section
	cprotocol_lpu237 m_target_protocol_lpu237;
	_mp::clibhid_dev::type_ptr m_ptr_dev;

	//erase / write section
	size_t m_n_size_fw; // the selected firmware size or raw firmware file size
	std::vector<int> m_v_erase_sec_index; //app area(erase sequence). except system variable , local storage control and area 
	std::vector<int> m_v_write_sec_index; //write sequence
	std::vector<int> m_v_write_sec_index_for_bin_file; //for reading sequence of bin file 

private:
	cshare(const cshare&) = delete;
	cshare& operator=(const cshare&) = delete;
};
