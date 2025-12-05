#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <functional>
#include <sstream>
#include <chrono>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/wait.h>
#include <spawn.h>
#include <sys/wait.h>
#endif

#include <mp_clog.h>
#include <mp_csystem_.h>

class cprocess_watcher {
public:
    using type_callback = std::function<void(int)>;

    cprocess_watcher() : 
        m_p_log(nullptr)
    {

    }
    ~cprocess_watcher() 
    {
        stop();
    }

    void start(_mp::clog *p_log,bool b_gui,const std::string& exe_path,
        const std::vector<std::string>& args,
        cprocess_watcher::type_callback cb)
    {
        m_p_log = p_log;
        m_b_stop_worker = false;
        m_worker = std::thread([=]() { _watch_process(b_gui,exe_path, args, cb); });
    }

    void stop() {
        m_b_stop_worker = true;
        if (m_worker.joinable()) {
            m_worker.join();
        }
    }

    bool is_stop_worker() const
    {
        return m_b_stop_worker;
    }

private:
    _mp::clog* m_p_log;

    std::atomic<bool> m_b_stop_worker{ false };
    std::thread m_worker;

    /**
    * @param b_enable_gui - only for linux, true - create console
    */
    void _watch_process(bool b_enable_gui, std::string exe, std::vector<std::string> args, type_callback cb)
    {
        if (m_p_log) {
            m_p_log->log_fmt(L"[I] - %ls | %ls\n", __WFUNCTION__, L"started");
            m_p_log->trace(L"[I] - %ls | %ls\n", __WFUNCTION__, L"started");
        }

#ifdef _WIN32
        std::ostringstream cmd;
        cmd << exe;
        for (auto& a : args) cmd << " " << a;

        STARTUPINFOA si{};
        PROCESS_INFORMATION pi{};
        si.cb = sizeof(si);

        std::string cmdline = cmd.str();
        if (!CreateProcessA(
            NULL,
            cmdline.data(),
            NULL, NULL, FALSE,
            0, NULL, NULL,
            &si, &pi))
        {
            //std::cerr << "CreateProcess failed: " << GetLastError() << "\n";
            if (cb) {
                cb(-1);
            }
            return;
        }

        // 프로세스 종료 대기 (polling으로 stop_flag 체크)
        while (!m_b_stop_worker) {
            DWORD result = WaitForSingleObject(pi.hProcess, 100); // 100ms 간격
            if (result == WAIT_OBJECT_0) {
                DWORD exit_code = 0;
                GetExitCodeProcess(pi.hProcess, &exit_code);
                CloseHandle(pi.hProcess);
                CloseHandle(pi.hThread);
                if (cb) {
                    cb(static_cast<int>(exit_code));
                }
                // 프로세스 종료에 의해 중단.
                m_b_stop_worker = true;
                return;
            }
        }

        // m_b_stop_worker로 중단됨
        TerminateProcess(pi.hProcess, 1);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        //std::cout << "[Watcher] stopped before process ended.\n";

#else
        bool b_daemonize = false;
        bool b_create_new_console = false;

        if (!b_enable_gui) {
            b_daemonize = true;
            b_create_new_console = false;
        }
        else {
            b_daemonize = false;
            b_create_new_console = true;
        }
        //
        std::string s_cmd(exe);
        std::vector<std::string> args_storage;
        std::vector<char*> argv_with_exe;
        size_t n_args_storage_with_exe = args.size()+1;

        if (b_create_new_console) {
            std::string _s_deb_msg;
            std::string s_new_term_exe = _mp::csystem::get_terminal_on_linux(_s_deb_msg);
            
            if (s_new_term_exe.empty()) {
                std::wstring _ws_deb_msg = _mp::cstring::get_unicode_from_mcsc(_s_deb_msg);
                if (m_p_log) {
                    if (_ws_deb_msg.empty()) {
                        m_p_log->log_fmt(L"[E] - %ls | %ls\n", __WFUNCTION__, L"not found console");
                        m_p_log->trace(L"[E] - %ls | %ls\n", __WFUNCTION__, L"not found console");
                    }
                    else {
                        m_p_log->log_fmt(L"[E] - %ls | %ls\n", __WFUNCTION__, _ws_deb_msg.c_str());
                        m_p_log->trace(L"[E] - %ls | %ls\n", __WFUNCTION__, _ws_deb_msg.c_str());
                    }
                }
                // 콘솔이 있어서 그냥 실행.
				// b_create_new_console 이 true 이므로 b_demonize 은 false 임.
                args_storage.reserve(n_args_storage_with_exe);
                args_storage.push_back(exe);
                for (auto& a : args) {
                    args_storage.push_back(std::move(a));
                }
            }
            else {
				s_cmd = s_new_term_exe;
                std::vector<std::string> v_cmd = _mp::csystem::make_terminal_cmd_on_linux(s_new_term_exe, exe);

                if (m_p_log) {
                    std::wstring _ws_cmd = _mp::cstring::get_unicode_from_mcsc(s_cmd);
                    m_p_log->log_fmt(L"[I] - %ls | new console - %ls\n", __WFUNCTION__, _ws_cmd.c_str());
                    m_p_log->trace(L"[I] - %ls | new console - %ls\n", __WFUNCTION__, _ws_cmd.c_str());
                }

				n_args_storage_with_exe = v_cmd.size() + args.size(); 
                args_storage.reserve(n_args_storage_with_exe);
                for (auto& s : v_cmd) {
                    args_storage.push_back(std::move(s));
                }
                for (auto& a : args) {
                    args_storage.push_back(std::move(a));
				}

            }
        }
        else {//데몬의 경우.
            if (m_p_log) {
                m_p_log->log_fmt(L"[I] - %ls | %ls\n", __WFUNCTION__, L"self-daemonize");
                m_p_log->trace(L"[I] - %ls | %ls\n", __WFUNCTION__, L"self-daemonize");
            }
            args_storage.reserve(n_args_storage_with_exe);
            args_storage.push_back(exe);
            for (auto& a : args) {
                args_storage.push_back(std::move(a));
			}
        }
       
        argv_with_exe.reserve(args_storage.size() + 1);// +1 for last null
        for (auto& s : args_storage) {
            argv_with_exe.push_back(const_cast<char*>(s.c_str()));
        }
        argv_with_exe.push_back(nullptr);

        bool b_result = false;
        pid_t pid;

        std::tie(b_result,pid) = _mp::csystem::execute_process_on_linux( s_cmd,argv_with_exe,false,b_daemonize);
        if (!b_result) {
            if (cb) {
                cb(-1);
            }
            // 프로세스 실행 실패.
            m_b_stop_worker = true;
            if (m_p_log) {
                m_p_log->log_fmt(L"[E] - %ls | %ls\n", __WFUNCTION__, L"execute_process_on_linux");
                m_p_log->trace(L"[E] - %ls | %ls\n", __WFUNCTION__, L"execute_process_on_linux");
            }
			return;
        }

        m_p_log->log_fmt(L"[I] - %ls | run process %d.\n", __WFUNCTION__, pid);
        m_p_log->trace(L"[I] - %ls | run process %d.\n", __WFUNCTION__, pid);

        if (pid <= 0) {
            // deamon 죽음.
            if (cb) {
                cb(127);
            }
            m_b_stop_worker = true;//exit while
            if (m_p_log) {
                m_p_log->log_fmt(L"[E] - %ls | killed updater | %d\n", __WFUNCTION__, pid);
                m_p_log->trace(L"[E] - %ls | killed updater | %d\n", __WFUNCTION__, pid);
            }
            return;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        while (!m_b_stop_worker) {
            if (kill(pid, 0) == 0 || errno == EPERM) {   // EPERM도 살아 있다는 뜻
				continue; // alive deamon
            }

			// dead deamon
            if (cb) {
                cb(0); // 정상 종료
			}
            m_b_stop_worker = true;//exit while
            if (m_p_log) {
                m_p_log->log_fmt(L"[I] - %ls | gohome updater | %d\n", __WFUNCTION__, pid);
                m_p_log->trace(L"[I] - %ls | gohome updater | %d\n", __WFUNCTION__, pid);
                return;
            }
        } // end while

        /*
		// posix_spawn()이 반환된 직후에 waitpid(..., WNOHANG)을 하면, race condition(반환 -1 + errno = ECHILD) 이 발생할 수 있음.
        // 부모: polling 으로 종료 또는 stop_flag 확인
		int n_max_try_cnt = 50;
		int n_try_cnt(n_max_try_cnt); // 100msec * 50 = 5초

        int status = 0;
        while (!m_b_stop_worker) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            pid_t result = waitpid(pid, &status, WNOHANG);
            if (result == pid) {
                int exit_code = (WIFEXITED(status)) ? WEXITSTATUS(status) : -1;

                if (cb) {
                    cb(exit_code);
                }
                // 프로세스 종료에 의해 중단.
                m_b_stop_worker = true;
                if (m_p_log) {
                    m_p_log->log_fmt(L"[E] - %ls | %ls\n", __WFUNCTION__, L"terminated updater");
                    m_p_log->trace(L"[E] - %ls | %ls\n", __WFUNCTION__, L"terminated updater");
                }
                return;
            }
            else if (result == -1) {
                m_b_stop_worker = true;
                // waitpid 오류 (예: 자식이 이미 사라짐)
                if (errno != ECHILD) {
                    if (cb) {
                        cb(-2);
                    }
                    if (m_p_log) {
                        m_p_log->log_fmt(L"[E] - %ls | %ls\n", __WFUNCTION__, L"waitpid");
                        m_p_log->trace(L"[E] - %ls | %ls\n", __WFUNCTION__, L"waitpid");
                    }
                    break;//exit while
                }

                //posix_spawn() 에 의해 실행된 프로세스가 데몬의 경우는 항상 여기옴.
				// 부모, 자식 관계가 끊어졌기 때문.
                if (n_try_cnt > 0) {
                    --n_try_cnt;
					// posix_spawn() + waitpid(pid, &status, WNOHANG) 의 조합에 의한 race condition 대응
                    m_b_stop_worker = false; // retry
                    continue;
                }
                if (cb) {
                    cb(127);
                }
                if (m_p_log) {
                    m_p_log->log_fmt(L"[E] - %ls | %ls\n", __WFUNCTION__, L"waitpid=ECHILD");
                    m_p_log->trace(L"[E] - %ls | %ls\n", __WFUNCTION__, L"waitpid=ECHILD");
                }
                break;
            }
            else if (result == 0) {
                n_try_cnt = n_max_try_cnt; //reset retry counter
            }
        }//end while
        */
        // m_b_stop_worker로 중단됨
        kill(pid, SIGTERM);
        waitpid(pid, nullptr, 0);  // 좀비 회수
        if (m_p_log) {
            m_p_log->log_fmt(L"[E] - %ls | %ls | %d\n", __WFUNCTION__, L"stopped before process ended", pid);
            m_p_log->trace(L"[E] - %ls | %ls | %d\n", __WFUNCTION__, L"stopped before process ended", pid);
        }
        //std::cout << "[Watcher] stopped before process ended.\n";
#endif
    }
};
