#include <websocket/mp_win_nt.h>

#ifdef _WIN32
    #include <boost/asio.hpp>
    #include <windows.h>
#endif // _WIN32

#include <iostream>
#include <thread>
#include <chrono>

#include <stdio.h>
#include <wchar.h>
#include <string.h>
#include <stdlib.h>

#include <test/test_tp_hid.h>


static void _display_option()
{
    std::wcout << L" = input option!" << std::endl;
    std::wcout << L" = filter - filter test." << std::endl;
    std::wcout << L" = hid - tx-rx test." << std::endl;
    std::wcout << L" = msr - reading msr test." << std::endl;
    std::wcout << L" = ibutton - reading ibutton test." << std::endl;
}


int main(int argc, char* argv[])
{
	//(void)argc;
	//(void)argv;

    int n_result(0);

    auto list_option = _mp::cconvert::get_command_line_parameters_by_list(argc, argv);

    do {
        if (list_option.size() < 1) {
            std::wcout << L" = BYE - Error option." <<std::endl;
            std::wcout << L" = filter - filter test." << std::endl;
            std::wcout << L" = hid - tx-rx test." << std::endl;
            std::wcout << L" = msr - reading msr test." << std::endl;
            std::wcout << L" = ibutton - reading ibutton test." << std::endl;
            continue;
        }

        if (list_option.size() < 2) {
            _display_option();
            continue;
        }

        auto it = std::begin(list_option);
        ++it;
        std::wstring s_test(*it);
        //
        if (s_test.compare(L"filter") == 0) {
            n_result = _test::tp_hid::test_filtering();
            continue;
        }
        if (s_test.compare(L"hid") == 0) {
            n_result = _test::tp_hid::test7();
            continue;
        }
        if (s_test.compare(L"msr") == 0) {
            n_result = _test::tp_hid::test_msr();
            continue;
        }
        if (s_test.compare(L"ibutton") == 0) {
            n_result = _test::tp_hid::test_ibutton();
            continue;
        }

        _display_option();
    } while (false);

    return n_result;
	
}



/*
#include <windows.h>

int main() {
    // 콘솔 창을 숨깁니다.
    HWND hwnd = GetConsoleWindow();
    ShowWindow(hwnd, SW_HIDE);

    // 프로그램 로직
    while (true) {
        // 프로그램 로직을 여기에 추가하세요
        Sleep(1000); // 예시로 1초 대기
    }

    return 0;
}
*/

/*
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

void daemonize() {
    pid_t pid, sid;

    // 부모 프로세스를 종료하여 자식 프로세스를 백그라운드로 실행
    pid = fork();
    if (pid < 0) {
        exit(EXIT_FAILURE);
    }
    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    // 파일 모드 마스크를 설정
    umask(0);

    // 새로운 세션 ID를 생성
    sid = setsid();
    if (sid < 0) {
        exit(EXIT_FAILURE);
    }

    // 현재 작업 디렉토리를 루트로 변경
    if ((chdir("/")) < 0) {
        exit(EXIT_FAILURE);
    }

    // 표준 파일 디스크립터를 닫음
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
}

int main() {
    daemonize();

    // 프로그램 로직
    while (1) {
        // 프로그램 로직을 여기에 추가하세요
        sleep(1); // 예시로 1초 대기
    }

    return 0;
}
*/