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
    // �ܼ� â�� ����ϴ�.
    HWND hwnd = GetConsoleWindow();
    ShowWindow(hwnd, SW_HIDE);

    // ���α׷� ����
    while (true) {
        // ���α׷� ������ ���⿡ �߰��ϼ���
        Sleep(1000); // ���÷� 1�� ���
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

    // �θ� ���μ����� �����Ͽ� �ڽ� ���μ����� ��׶���� ����
    pid = fork();
    if (pid < 0) {
        exit(EXIT_FAILURE);
    }
    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    // ���� ��� ����ũ�� ����
    umask(0);

    // ���ο� ���� ID�� ����
    sid = setsid();
    if (sid < 0) {
        exit(EXIT_FAILURE);
    }

    // ���� �۾� ���丮�� ��Ʈ�� ����
    if ((chdir("/")) < 0) {
        exit(EXIT_FAILURE);
    }

    // ǥ�� ���� ��ũ���͸� ����
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
}

int main() {
    daemonize();

    // ���α׷� ����
    while (1) {
        // ���α׷� ������ ���⿡ �߰��ϼ���
        sleep(1); // ���÷� 1�� ���
    }

    return 0;
}
*/