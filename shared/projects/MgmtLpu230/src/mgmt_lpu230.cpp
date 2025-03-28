#include <websocket/mp_win_nt.h>

#ifdef _WIN32
    #include <boost/asio.hpp>
    #include <windows.h>
#endif // _WIN32

#include <iostream>
#include <sstream>
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
            std::wcout << L" = multiibutton - reading ibutton test in multi-threading env." << std::endl;
            std::wcout << L" = msr ibutton - reading one msr and two ibutton test." << std::endl;
            std::wcout << L" = cancelmsr - cancel reading msr test." << std::endl;
            std::wcout << L" = cancelibutton - cancel reading ibutton test." << std::endl;
            std::wcout << L"ex) hid io 10 times test > mgmt_lpu230 hid 10 " << std::endl;
            std::wcout << L"ex) msr io 100 times test > mgmt_lpu230 msr 100 " << std::endl;
            std::wcout << L"ex) ibutton io 200 times test > mgmt_lpu230 ibutton 200 " << std::endl;
            std::wcout << L"ex) after default timeout, cancel the status of msr-reading 300 times test > mgmt_lpu230 cancelmsr 300 " << std::endl;
            std::wcout << L"ex) after 200 msec, cancel the status of msr-reading 100 times test > mgmt_lpu230 cancelmsr 100 200 " << std::endl;
            std::wcout << L"ex) after default timeout, cancel the status of ibutton-reading 300 times test > mgmt_lpu230 cancelibutton 300 " << std::endl;
            std::wcout << L"ex) after 200 msec, cancel the status of ibutton-reading 100 times test > mgmt_lpu230 cancelibutton 100 200 " << std::endl;
            continue;
        }

        if (list_option.size() < 2) {
            _display_option();
            continue;
        }

        auto it = std::begin(list_option);

        if (list_option.size() == 2) {
            ++it;
            std::wstring s_test(*it);
            //
            if (s_test.compare(L"filter") == 0) {
                n_result = _test::tp_hid::get_instance().test_filtering();
                continue;
            }
            if (s_test.compare(L"hid") == 0) {
                n_result = _test::tp_hid::get_instance().test_hid_io();
                continue;
            }
            if (s_test.compare(L"msr") == 0) {
                n_result = _test::tp_hid::get_instance().test_msr();
                continue;
            }
            if (s_test.compare(L"ibutton") == 0) {
                n_result = _test::tp_hid::get_instance().test_ibutton();
                continue;
            }
            if (s_test.compare(L"multiibutton") == 0) {
                n_result = _test::tp_hid::get_instance().test_multi_ibutton();
                continue;
            }
            if (s_test.compare(L"cancelmsr") == 0) {
                n_result = _test::tp_hid::get_instance().test_cancelmsr();
                continue;
            }
            if (s_test.compare(L"cancelibutton") == 0) {
                n_result = _test::tp_hid::get_instance().test_cancelibutton();
                continue;
            }

        }

        if (list_option.size() == 3) {
            ++it;
            std::wstring s_p1(*it);

            ++it;
            std::wstring s_p2(*it);

            if ((s_p1 == L"msr" && s_p2 == L"ibutton") || (s_p1 == L"ibutton" && s_p2 == L"msr")) {
                n_result = _test::tp_hid::get_instance().test_msr_ibutton();
                continue;
            }
            //
            std::wistringstream wiss(s_p2);
            int n_loop(-1);
            if (!(wiss >> n_loop)) {
                _display_option();
                continue;
            }
            // n_loop is interger
            if (n_loop <= 0) {
                _display_option();
                continue;
            }

            if (s_p1.compare(L"hid") == 0) {
                n_result = _test::tp_hid::get_instance().test_hid_io(n_loop);
                continue;
            }
            if (s_p1.compare(L"msr") == 0) {
                n_result = _test::tp_hid::get_instance().test_msr(n_loop);
                continue;
            }
            if (s_p1.compare(L"ibutton") == 0) {
                n_result = _test::tp_hid::get_instance().test_ibutton(n_loop);
                continue;
            }
            if (s_p1.compare(L"cancelmsr") == 0) {
                n_result = _test::tp_hid::get_instance().test_cancelmsr(n_loop);
                continue;
            }
            if (s_p1.compare(L"cancelibutton") == 0) {
                n_result = _test::tp_hid::get_instance().test_cancelibutton(n_loop);
                continue;
            }

        }

        if (list_option.size() == 4) {
            ++it;
            std::wstring s_p1(*it);

            ++it;
            std::wstring s_p2(*it);

            ++it;
            std::wstring s_p3(*it);

            int n_loop(-1);

            if ((s_p1 == L"msr" && s_p2 == L"ibutton") || (s_p1 == L"ibutton" && s_p2 == L"msr")) {

                if (!(std::wistringstream(s_p3) >> n_loop)) {
                    _display_option();
                    continue;
                }
                // n_loop is interger
                if (n_loop <= 0) {
                    _display_option();
                    continue;
                }

                n_result = _test::tp_hid::get_instance().test_msr_ibutton(n_loop);
                continue;
            }

            std::wistringstream wiss(s_p2);

            if (!(wiss >> n_loop)) {
                _display_option();
                continue;
            }
            // n_loop is interger
            if (n_loop <= 0) {
                _display_option();
                continue;
            }

            std::wistringstream wiss3(s_p3);
            int n_cancel_time_msec(1000);
            if (!(wiss3 >> n_cancel_time_msec)) {
                _display_option();
                continue;
            }

            if (s_p1.compare(L"cancelmsr") == 0) {
                n_result = _test::tp_hid::get_instance().test_cancelmsr(n_loop, n_cancel_time_msec);
                continue;
            }
            if (s_p1.compare(L"cancelibutton") == 0) {
                n_result = _test::tp_hid::get_instance().test_cancelibutton(n_loop, n_cancel_time_msec);
                continue;
            }

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