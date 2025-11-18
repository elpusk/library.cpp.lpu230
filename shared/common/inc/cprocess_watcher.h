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

    void start(_mp::clog *p_log,const std::string& exe_path,
        const std::vector<std::string>& args,
        cprocess_watcher::type_callback cb)
    {
        m_p_log = p_log;
        m_b_stop_worker = false;
        m_worker = std::thread([=]() { _watch_process(exe_path, args, cb); });
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

    void _watch_process(std::string exe, std::vector<std::string> args, type_callback cb)
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
        std::vector<char*> argv;
        argv.push_back(const_cast<char*>(exe.c_str()));
        for (auto& a : args)
            argv.push_back(const_cast<char*>(a.c_str()));
        argv.push_back(nullptr);

        pid_t pid;
        extern char** environ;

        int ret = posix_spawn(
            &pid
            , exe.c_str()
            , nullptr
            , nullptr
            ,argv.data()
            , environ
        );

        if (ret != 0) {
            if (cb) {
                cb(-1);
            }
            // 프로세스 종료에 의해 중단.
            m_b_stop_worker = true;
            if (m_p_log) {
                m_p_log->log_fmt(L"[E] - %ls | %ls\n", __WFUNCTION__, L"posix_spawn");
                m_p_log->trace(L"[E] - %ls | %ls\n", __WFUNCTION__, L"posix_spawn");
            }
            return;
        }

        // 부모: polling 으로 종료 또는 stop_flag 확인
        int status = 0;
        while (!m_b_stop_worker) {
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
                }
                else {
                    if (cb) {
                        cb(127);
                    }
                    if (m_p_log) {
                        m_p_log->log_fmt(L"[E] - %ls | %ls\n", __WFUNCTION__, L"waitpid=ECHILD");
                        m_p_log->trace(L"[E] - %ls | %ls\n", __WFUNCTION__, L"waitpid=ECHILD");
                    }
                }
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        // m_b_stop_worker로 중단됨
        kill(pid, SIGTERM);
        waitpid(pid, nullptr, 0);  // 좀비 회수
        if (m_p_log) {
            m_p_log->log_fmt(L"[E] - %ls | %ls\n", __WFUNCTION__, L"stopped before process ended");
            m_p_log->trace(L"[E] - %ls | %ls\n", __WFUNCTION__, L"stopped before process ended");
        }
        //std::cout << "[Watcher] stopped before process ended.\n";
#endif
    }
};
