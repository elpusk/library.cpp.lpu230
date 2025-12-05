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

#ifndef _WIN32
		/**
		* @brief run executable file with command line arguments(linux only).
		* @param s_abs_fullpath_of_exe - exe file abs full path.
		* @param v_command_line_arguments_with_exe command line argments vector. first element must be exe full path.
		* @param b_wait_for_termination_created_process - true : wait the end of terminating new process.
		* @param b_daemonize_created_process - true : daemonize the created process.
		* @return first - true : success creating process, false - error.
		* @return second - created process id if success, otherwise -1.
		*/
		static std::pair<bool, pid_t> execute_process_on_linux(
			const std::string& s_abs_fullpath_of_exe
			, const std::vector<char*> & v_command_line_arguments_with_exe
			, bool b_wait_for_termination_created_process
			, bool b_daemonize_created_process
		);

		/**
		* @brief get terminal program path on linux.(linux only)
		* @param s_out_deb_msg - output debug message
		* @return terminal program path
		* @return "/usr/bin/xterm",
        * @return "/usr/bin/gnome-terminal",
        * @return "/usr/bin/konsole",
        * @return "/usr/bin/xfce4-terminal" or
        * @return "/usr/bin/lxterminal"
		* @return empty string - terminal program not found
		*/
		static std::string get_terminal_on_linux(std::string& s_out_deb_msg);

		/**
		* @brief make command vector from exec command string.
		* @param s_term - terminal program path
		* @param exe - executable file path
		* @return command vector
		*/
		static std::vector<std::string> make_terminal_cmd_on_linux(const std::string& s_term, const std::string& exe);
#endif //!_WIN32

	public:
		virtual ~csystem();

	private:

	private://don't call these methods
		csystem();
		csystem(const csystem&);
		csystem& operator=(const csystem&);

	};

}//the end of _mp namespace

