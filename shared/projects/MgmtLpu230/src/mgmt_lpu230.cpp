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