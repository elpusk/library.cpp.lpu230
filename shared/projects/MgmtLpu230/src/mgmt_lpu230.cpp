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




int main(int argc, char* argv[])
{
	(void)argc;
	(void)argv;

	return _test::tp_hid::test_msr();
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