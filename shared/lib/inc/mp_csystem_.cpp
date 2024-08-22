
#include <mutex>
#include <utility>
#include <map>
#include <iostream>
#include <fstream>
#include <stdexcept>

#ifdef _WIN32
#include <windows.h>
#include <sddl.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>
#endif

#include <mp_csystem_.h>
#include <mp_cfile.h>

namespace _mp {

	bool is_running_as_sudo_or_admin()
	{
#ifdef _WIN32
		BOOL b_admin = FALSE;
		PSID admin_group = NULL;

		do {
			// Create a SID for the BUILTIN\Administrators group
			SID_IDENTIFIER_AUTHORITY nt_authority = SECURITY_NT_AUTHORITY;
			if (!AllocateAndInitializeSid(&nt_authority, 2,
				SECURITY_BUILTIN_DOMAIN_RID,
				DOMAIN_ALIAS_RID_ADMINS,
				0, 0, 0, 0, 0, 0,
				&admin_group)) {
				continue;
			}

			// Check if the current process is a member of the admin group
			if (!CheckTokenMembership(NULL, admin_group, &b_admin)) {
				b_admin = FALSE;
			}

			// Free the SID created
			if (admin_group) {
				FreeSid(admin_group);
			}
		} while (false);

		if (b_admin) {
			return true;
		}
		else {
			return false;
		}
#else
		uid_t uid = getuid();    // ���� ����� ID
		uid_t euid = geteuid();  // ��ȿ ����� ID
		return (uid != euid && euid == 0);
#endif
	}

	std::shared_ptr<boost::interprocess::file_lock> csystem::get_file_lock_for_single_instance(const std::wstring& s_name)
	{
		std::shared_ptr<boost::interprocess::file_lock> ptr;

		do {
			if (s_name.empty()) {
				continue;
			}

			std::wstring lock_file_path = cfile::get_temp_path();
			lock_file_path += s_name;

			std::string s(cstring::get_mcsc_from_unicode(lock_file_path));

			try {
				std::ofstream lock_file(s);
				if (!lock_file.is_open()) {
					//error
					continue;
				}

#ifndef _WIN32
				//chmod(s.c_str(), 0666);// set access permision  (all user can read/write)
#endif

				ptr = std::make_shared<boost::interprocess::file_lock>(s.c_str());
				if (!ptr->try_lock()) {
					ptr.reset();
					continue;//Another instance is already running
				}

				//unlock will be processed by OS automatically!
			}
			catch (...) {
				ptr.reset();
			}

		} while (false);

		return ptr;
	}

	/**
	* in this funcation, parent process will be terminated with _exit(0)
	* @parameter s_cur_dir_abs_with_slash - current directory with slash, abs path. 
	*/
	bool csystem::daemonize_on_linux(
		const std::wstring& s_daemon_pid_file_full_path, 
		const std::wstring& s_cur_dir_abs_with_slash,
		void(*signal_handler)(int)
	)
	{
		bool b_result(false);

		do {
#ifdef _WIN32
            b_result = true;//on windows, nothing to do.
#else
            pid_t pid;

            // Fork off the parent process
            pid = fork();
            if (pid < 0) {
                continue;//Fork failed
            }

            if (pid > 0) {
				// If we got a good PID, then we can exit the parent process
				_exit(EXIT_SUCCESS);//Don't use exit().! Use _exit()
            }

			//if pid is zero, this process is child.

			// Create a new SID for the child process
			if (setsid() < 0) {
				continue;
			}
			signal(SIGCHLD, SIG_IGN);
			signal(SIGHUP, SIG_IGN); // ignore SIGHUP

			// here
			// sid == pgid == pid & ppid == 1
			pid = fork();
			if (pid < 0) {
				continue;//Fork failed
			}
			if (pid > 0) {
				// If we got a good PID, then we can exit the parent process
				_exit(EXIT_SUCCESS);//Don't use exit().! Use _exit()
			}

			// here
			// sid == pgid != pid & ppid == 1

			// Change the file mode mask
			umask(0);

			// Change the current working directory
			if(!s_cur_dir_abs_with_slash.empty()){
				std::string s_cur = cstring::get_mcsc_from_unicode(s_cur_dir_abs_with_slash);
				if (!s_cur.empty()) {
					if (chdir(s_cur.c_str()) < 0) {
						continue;
					}
				}
			}
			else {
				if (chdir("/") < 0) {
					continue;
				}
			}

			// Close out the standard file descriptors
			close(STDIN_FILENO);
			close(STDOUT_FILENO);
			close(STDERR_FILENO);

            // Redirect standard files to /dev/null
			stdin = freopen("/dev/null", "r", stdin);
			stdout = freopen("/dev/null", "w", stdout);
			stderr = freopen("/dev/null", "w", stderr);

			signal(SIGTERM, signal_handler);
			signal(SIGHUP, signal_handler);

			if (!s_daemon_pid_file_full_path.empty()) {
				//create PID file
				std::string s_pid = cstring::get_mcsc_from_unicode(s_daemon_pid_file_full_path);
				if (!s_pid.empty()) {
					std::ofstream pid_file(s_pid.c_str());
					if (pid_file.is_open()) {
						pid_file << getpid() << std::endl;
						pid_file.close();
					}
				}
			}

			b_result = true;
#endif
		} while (false);
		return b_result;
	}

	csystem::~csystem()
	{
	}

}//the end of _mp namespace

