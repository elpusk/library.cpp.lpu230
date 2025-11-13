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
#endif

class cprocess_watcher {
public:
    using type_callback = std::function<void(int)>;

    cprocess_watcher() = default;
    ~cprocess_watcher() {
        stop();
    }

    void start(const std::string& exe_path,
        const std::vector<std::string>& args,
        cprocess_watcher::type_callback cb)
    {
        m_b_stop_worker = false;
        m_worker = std::thread([=]() { _watch_process(exe_path, args, cb); });
    }

    void stop() {
        m_b_stop_worker = true;
        if (m_worker.joinable()) {
            m_worker.join();
        }
    }

private:
    std::atomic<bool> m_b_stop_worker{ false };
    std::thread m_worker;

    void _watch_process(std::string exe, std::vector<std::string> args, type_callback cb)
    {
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
                return;
            }
        }

        // m_b_stop_worker로 중단됨
        TerminateProcess(pi.hProcess, 1);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        //std::cout << "[Watcher] stopped before process ended.\n";

#else
        pid_t pid = fork();
        if (pid == 0) {
            // child
            std::vector<char*> argv;
            argv.push_back(const_cast<char*>(exe.c_str()));
            for (auto& a : args)
                argv.push_back(const_cast<char*>(a.c_str()));
            argv.push_back(nullptr);
            execvp(exe.c_str(), argv.data());
            _exit(127);
        }

        if (pid < 0) {
            //std::perror("fork failed");
            if (cb) {
                cb(-1);
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
                return;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        // m_b_stop_worker로 중단됨
        kill(pid, SIGTERM);
        //std::cout << "[Watcher] stopped before process ended.\n";
#endif
    }
};
