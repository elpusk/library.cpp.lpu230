#pragma once
#include <string>
#include <memory>
#include <vector>
#include <mutex>

#include <mp_cstring.h>
#include <mp_cio_packet.h>
#include <mp_clog.h>

#include <cprocess_watcher.h>

/**
* parameters stroage of lpu230_update 
*/
class cupdater_param {
public:
	typedef std::shared_ptr< cupdater_param > type_ptr;
private:
	typedef std::map< std::wstring, std::wstring > _type_map;
public:
	cupdater_param(unsigned long n_session_number);
	virtual ~cupdater_param();

	void set_log_obj(_mp::clog* p_log);

	/**
	* @brief check bootloader request parameter existence.
	* @return first - true : exist bootloader request parameter.
	* @return second - parameter info
	*/
	std::pair<bool,std::wstring> can_be_start_firmware_update() const;

	bool insert(const std::wstring& s_key, const std::wstring& s_value);

	/**
	* @brief insert file path parameter(key).
	*/
	bool insert_file(const std::wstring& s_value_file_full_abs_path);

	/**
	* @brief insert device path parameter(key).
	*/
	bool insert_device(const std::wstring& s_value_device_path);

	/**
	* @brief insert run by cf2 mode indicator(key).
	*/
	bool insert_run_by_cf2_mode();

	/**
	* @set information for generating response packet of progress nortification.
	*/
	void set_packet_info_for_notify(const _mp::cio_packet& req_act_dev_sub_bootloader);

	/**
	* @brief remove all item of map.
	*/

	bool start_update();

	/**
	* @brief called by cprocess_watcher() thread.
	*/
	void callback_update_end(int n_exit_code);

	/**
	* @brief set executable file path.
	* @param s_exe_full_abs_path - abs full path
	*/
	cupdater_param& set_exe_full_abs_path(const std::wstring& s_exe_full_abs_path);

	std::wstring get_exe_full_abs_path_by_wstring() const;

	std::string get_exe_full_abs_path_by_string() const;

	/**
	* @brief generate command line arguments from stored parameters.
	*/
	std::wstring generate_command_line_arguments_except_exe_by_wstring() const;
	std::string generate_command_line_arguments_except_exe_by_string() const;
	std::wstring generate_command_line_arguments_with_exe_by_wstring() const;
	std::string generate_command_line_arguments_with_exe_by_string() const;

	std::vector<std::wstring> generate_command_line_arguments_except_exe_by_vector_wstring() const;
	std::vector<std::string> generate_command_line_arguments_except_exe_by_vector_string() const;
	std::vector<std::wstring> generate_command_line_arguments_with_exe_by_vector_wstring() const;
	std::vector<std::string> generate_command_line_arguments_with_exe_by_vector_string() const;

	_mp::cio_packet& get_rsp_packet_before_setting();


private:
	/**
	* @brief delete firmware file which is created for updating temporarily.
	* @brief this function is guard by mutex.
	* @return true : delete success.
	*/
	bool _delete_firmware();

private:
	_mp::clog* m_p_log;

	unsigned long m_n_session_number;

	std::mutex m_mutex; //used in _delete_firmware()
	cupdater_param::_type_map m_map;
	std::wstring m_s_abs_full_exe_path;

	_mp::cio_packet m_rsp;

	std::shared_ptr<cprocess_watcher> m_ptr_runner;

private:
	cupdater_param() = delete;
};
