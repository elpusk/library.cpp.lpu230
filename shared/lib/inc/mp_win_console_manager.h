#pragma once
////////////////////////////////////////////////////
// windows only

#ifdef _WIN32

#ifndef _UNICODE
    #error "ERROR _UNICODE must be defined!!!"
#endif

#define NOMINMAX
#include <stdio.h>
#include <utility>
#include <windows.h>
#include <shellapi.h>

namespace _mp{
class win_console_manager {
public:
    win_console_manager() : 
        m_b_allocated_console(false)
		, m_argc(0)
		, m_argv(nullptr)
    {
        // allocate new console for app
        if (AllocConsole()) {
            // stdout connect to the new console.
            // std::cout connect to the new console.
            FILE* pFile;
            freopen_s(&pFile, "CONOUT$", "w", stdout);

            // stdin connect to the new console.
            // std::cin connect to the new console.
            freopen_s(&pFile, "CONIN$", "r", stdin);

            // stderr connect to the new console.
            freopen_s(&pFile, "CONOUT$", "w", stderr);

            m_b_allocated_console = true;
        }
		m_argv = CommandLineToArgvW(GetCommandLineW(), &m_argc); // 프로제트 설정이 unicode 이어서 unicode 로 받음.
    }
    ~win_console_manager() 
    {
        if (m_b_allocated_console) {
            FreeConsole();
        }

        if (m_argv) {
            LocalFree(m_argv);
        }
	}

    std::pair<int, wchar_t**> get_command_line_arguments() const
    {
        return std::make_pair(m_argc, m_argv);
	}
private:
	bool m_b_allocated_console;
    int m_argc;
    wchar_t** m_argv;

private:
    win_console_manager( const win_console_manager&) = delete;
    win_console_manager& operator=( const win_console_manager&) = delete;
};

} // the end of _mp
#endif _WIN32
