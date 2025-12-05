#include <mutex>
#include <utility>
#include <map>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <memory>
#include <thread>

#ifdef _WIN32
#include <windows.h>
#include <sddl.h>
#include <shellapi.h> // For ShellExecuteEx()
#else
#include <unistd.h>
#include <stdlib.h>
#include <termios.h>  // For terminal manipulation
#include <sys/select.h>// For select()
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <spawn.h>
#include <signal.h>
#include <fcntl.h>
#include <cstdio>  
#include <limits.h>
#endif

#include <mp_csystem_.h>
#include <mp_cfile.h>

#ifndef _WIN32
// in linux, environ is defined in unistd.h, but not declared in any header files.
extern char** environ;
#endif // !_WIN32

namespace _mp {

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
		std::string s_pid_file = cstring::get_mcsc_from_unicode(s_daemon_pid_file_full_path);
		std::shared_ptr<std::ofstream> ptr_pid_file;

		do {
#ifdef _WIN32
            b_result = true;//on windows, nothing to do.
#else
			if (!s_pid_file.empty()) {
				//create PID file
				ptr_pid_file = std::make_shared<std::ofstream>(s_pid_file.c_str(), std::ofstream::out | std::ofstream::trunc);
				if (!ptr_pid_file) {
					continue;
				}
				if (!ptr_pid_file->is_open()) {
					continue;
				}
			}

            pid_t pid;

            // Fork off the parent process
            pid = fork();
            if (pid < 0) {
				if (ptr_pid_file) {
					//remove pid file
					ptr_pid_file->close();
					remove(s_pid_file.c_str());
				}
                continue;//Fork failed
            }

            if (pid > 0) {
				if (ptr_pid_file) {
					//close pid file
					ptr_pid_file->close();
				}

				// If we got a good PID, then we can exit the parent process
				_exit(EXIT_SUCCESS);//Don't use exit().! Use _exit()
            }

			//if pid is zero, this process is child.

			// Create a new SID for the child process
			if (setsid() < 0) {
				if (ptr_pid_file) {
					//remove pid file
					ptr_pid_file->close();
					remove(s_pid_file.c_str());
				}
				continue;
			}
			signal(SIGCHLD, SIG_IGN);
			signal(SIGHUP, SIG_IGN); // ignore SIGHUP

			// here
			// sid == pgid == pid & ppid == 1
			pid = fork();
			if (pid < 0) {
				if (ptr_pid_file) {
					//remove pid file
					ptr_pid_file->close();
					remove(s_pid_file.c_str());
				}
				continue;//Fork failed
			}
			if (pid > 0) {
				if (ptr_pid_file) {
					//close pid file
					ptr_pid_file->close();
				}

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
						if (ptr_pid_file) {
							//remove pid file
							ptr_pid_file->close();
							remove(s_pid_file.c_str());
						}
						continue;
					}
				}
			}
			else {
				if (chdir("/") < 0) {
					if (ptr_pid_file) {
						//remove pid file
						ptr_pid_file->close();
						remove(s_pid_file.c_str());
					}
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

			if (ptr_pid_file) {
				*ptr_pid_file << getpid() << std::endl;
				ptr_pid_file->close();
			}

			b_result = true;
#endif
		} while (false);
		return b_result;
	}

	bool csystem::is_running_as_admin_or_root()
	{
		bool b_result(false);
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
			b_result = true;
		}
#else	//linux
		uid_t uid = getuid();    // 실제 사용자 ID
		uid_t euid = geteuid();  // 유효 사용자 ID
		if (uid == 0 || euid == 0) {
			b_result = true;
		}
#endif
		return b_result;
	}

#ifndef _WIN32
	std::pair<bool, pid_t> csystem::execute_process_on_linux(
		const std::string& s_abs_fullpath_of_exe
		, const std::vector<char*>& v_command_line_arguments_with_exe
		, bool b_wait_for_termination_created_process
		, bool b_daemonize_created_process
	)
	{
		bool b_reult(false);
		pid_t pid(-1);

		int n_result(-1);
		posix_spawn_file_actions_t actions;

		do {
			if(s_abs_fullpath_of_exe.empty()){
				continue;
			}

			if (!b_daemonize_created_process) {
				n_result = posix_spawn(&pid, s_abs_fullpath_of_exe.c_str(), nullptr, nullptr, v_command_line_arguments_with_exe.data(), environ);
			}
			else {
				//deamonize created process
				posix_spawn_file_actions_init(&actions);
				// /dev/null to FD rediect(stdin, stdout, stderr)
				posix_spawn_file_actions_addopen(&actions, 0, "/dev/null", O_RDONLY, 0);
				posix_spawn_file_actions_addopen(&actions, 1, "/dev/null", O_WRONLY, 0);
				posix_spawn_file_actions_addopen(&actions, 2, "/dev/null", O_WRONLY, 0);

				n_result = posix_spawn(&pid, s_abs_fullpath_of_exe.c_str(), &actions, nullptr, v_command_line_arguments_with_exe.data(), environ);
				posix_spawn_file_actions_destroy(&actions);
			}

			if (n_result != 0) {
				continue; //error
			}
			//
			b_reult = true;
		} while (false);

		if (b_reult && b_wait_for_termination_created_process) {
			do {
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				if (kill(pid, 0) == 0 || errno == EPERM) { 
					continue; // alive deamon
				}
				break;
			} while (true);
		}

		return std::make_pair(b_reult,pid);
	}

	std::string csystem::get_terminal_on_linux(std::string& s_out_deb_msg)
	{
		const std::vector<std::string> terms = {
					"/usr/bin/gnome-terminal"
					, "/usr/bin/konsole"
					, "/usr/bin/mate-terminal"
					, "/usr/bin/xfce4-terminal"
					, "/usr/bin/lxterminal"
					,"/usr/bin/xterm"
		};

		std::string s_terminal;
		s_out_deb_msg.clear();

		do {
			if (!std::getenv("DISPLAY")) {
				// daemon 에서 호출하면 항상 환경변수 없어서 GUI 터미널 못찾음.
				s_out_deb_msg = "no GUI environment";
				continue; //no GUI environment
			}
			for (const auto& s : terms) {
				if (access(s.c_str(), X_OK) == 0) {
					s_terminal = s;
					break; // exit for
				}
				else {
					s_out_deb_msg += "\n:";
					s_out_deb_msg += s;
					s_out_deb_msg += '-';
					if (errno == ENOENT) {
						s_out_deb_msg += "not found";
					}
					else if (errno == EACCES) {
						s_out_deb_msg += "none exe right";
					}
					else {
						s_out_deb_msg += "error access()";
					}
					s_out_deb_msg += L':';
				}
			}//end for
		} while (false);
		return s_terminal;
	}

	std::vector<std::string> csystem::make_terminal_cmd_on_linux(const std::string& s_term, const std::string& exe)
	{
		if (s_term.find("gnome-terminal") != std::string::npos) {
			// gnome-terminal은 -e 사용 불가
			return { s_term, "--", exe };
		}
		else if (s_term.find("konsole") != std::string::npos) {
			return { s_term, "-e", exe };
		}
		else if (s_term.find("mate-terminal") != std::string::npos) {
			return { s_term, "-e", exe };
		}
		else if (s_term.find("xfce4-terminal") != std::string::npos) {
			return { s_term, "-e", exe };
		}
		else if (s_term.find("lxterminal") != std::string::npos) {
			return { s_term, "-e", exe };
		}
		else if (s_term.find("xterm") != std::string::npos) {
			return { s_term, "-e", exe };
		}
		else {
			return { exe };//no terminal
		}
	}
#endif !_WIN32

	csystem::~csystem()
	{
	}

}//the end of _mp namespace

