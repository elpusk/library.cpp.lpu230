#pragma once

#include <boost/interprocess/sync/file_lock.hpp>
#include <boost/interprocess/exceptions.hpp>

#include <mp_type.h>
#include <mp_cstring.h>

namespace _mp {

	class csystem
	{
	public:

		/**
		* @brief good in Windows. none operation in debian12. 
		*/
		static std::shared_ptr<boost::interprocess::file_lock> get_file_lock_for_single_instance(const std::wstring& s_name);

		/**
		* @brief in this funcation, parent process will be terminated with _exit(0)
		* @param s_cur_dir_abs_with_slash - current directory with slash, abs path. 
		*/
		static bool daemonize_on_linux(const std::wstring  & s_daemon_pid_file_full_path, const std::wstring& s_cur_dir_abs_with_slash, void(*signal_handler)(int));

		/**
		* @brief check the process is running as admin(windows) or root(linux)
		* @return true - running as admin or root, false - not running as admin or root
		*/
		static bool is_running_as_admin_or_root();

		/**
		* @brief run executable file with command line arguments
		* @param s_abs_fullpath_of_exe - exe file abs full path.
		* @param v_command_line_arguments_except_exe command line argments vector.
		* @param b_wait_for_termination_created_process - true : wait the end of terminating new process.
		*/
		static bool execute_process(
			const std::string& s_abs_fullpath_of_exe
			, const std::vector<std::string>& v_command_line_arguments_except_exe
			, bool b_wait_for_termination_created_process
		);

	public:
		virtual ~csystem();

	private:

	private://don't call these methods
		csystem();
		csystem(const csystem&);
		csystem& operator=(const csystem&);

	};

}//the end of _mp namespace

