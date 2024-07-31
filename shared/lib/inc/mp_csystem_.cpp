
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
		uid_t uid = getuid();    // 실제 사용자 ID
		uid_t euid = geteuid();  // 유효 사용자 ID
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
	*/
	bool csystem::daemonize_on_linux(const std::wstring& s_daemon_name,void(*signal_handler)(int))
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

			// Change the current working directory
			if ((chdir("/")) < 0) {
				continue;
			}

			// Close out the standard file descriptors
			close(STDIN_FILENO);
			close(STDOUT_FILENO);
			close(STDERR_FILENO);

            // Change the file mode mask
            umask(0);

            // Redirect standard files to /dev/null
			stdin = freopen("/dev/null", "r", stdin);
			stdout = freopen("/dev/null", "w", stdout);
			stderr = freopen("/dev/null", "w", stderr);

			signal(SIGTERM, signal_handler);
			signal(SIGHUP, signal_handler);

			if (!s_daemon_name.empty()) {
				//create PID file

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

