#pragma once
#include <string>
#include <memory>
#include <vector>

#include <mp_cstring.h>

namespace _mp {

	/**
	* parameters stroage of lpu230_update 
	*/
	class cupdater_param {
	public:
		typedef std::shared_ptr< _mp::cupdater_param > type_ptr;
	private:
		typedef std::map< std::wstring, std::wstring > _type_map;
	public:
		cupdater_param(unsigned long n_session_number);
		virtual ~cupdater_param();

		/**
		* @brief get value of the given key.
		* @param s_key - key of map.
		* @param b_remove_after_found - remove value of key in the map after returning value.
		* @return first - true : found
		* @return second - the value of key.
		*/
		std::pair<bool, std::wstring> find(const std::wstring& s_key, bool b_remove_after_found);

		bool is_valid(const std::wstring& s_key) const;

		/**
		* @brief check bootloader request parameter existence.
		* @return true - exist bootloader request parameter.
		*/
		bool can_be_start_firmware_update() const;

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

		bool erase(const std::wstring& s_key);

		/**
		* @brief remove all item of map.
		*/
		void clear();
		bool empty() const;
		size_t size() const;

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

	private:
		unsigned long m_n_session_number;
		_mp::cupdater_param::_type_map m_map;
		std::wstring m_s_abs_full_exe_path;

	private:
		cupdater_param() = delete;
	};

} //end _mp