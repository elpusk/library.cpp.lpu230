#include <websocket/mp_win_nt.h>

#include <cstdio>
#include <main.h>
#include <iostream>

#include <mp_coffee.h>
#include <mp_cstring.h>
#include <mp_clog.h>
#include <mp_csystem_.h>

/**
 * stop the process of "/terminal" and remove all installed file and paths.
 */
int main_terminal(const _mp::type_set_wstring& set_parameters)
{
	int n_result(EXIT_FAILURE);

	do {
		_mp::cnamed_pipe::type_ptr ptr_ctl_pipe;
		//setup controller
		ptr_ctl_pipe = std::make_shared<_mp::cnamed_pipe>(_mp::_coffee::CONST_S_COFFEE_MGMT_CTL_PIPE_NAME, false);

		if (!ptr_ctl_pipe) {
			std::wcout << L"Error: cannot create named pipe for control." << std::endl;
			continue; //error
		}
		if (!ptr_ctl_pipe->is_ini()) {
			std::wcout << L"Error: cannot initialize to named pipe for control." << std::endl;
			continue; //error
		}

		//
		std::wcout << L"Ready to control the wss." << std::endl;

		bool b_run(true);

		std::wstring s_input;
		// main loop
		do {
			// 사용자가 std::cin에 입력한 내용을 읽어들임.
			s_input.clear();
			std::getline(std::wcin, s_input);

			// 입력된 내용을 소문자로 변환
			_mp::cstring::to_lower(s_input);

			if(s_input.empty()) {
				continue; // skip empty input
			}

			///////////////////////////////
			// 입력된 내용에 따라 동작 결정
			// 


			// 입력된 내용이 "bye"인 경우, b_run을 false로 설정하여 루프 종료
			if (s_input == L"bye") {
				b_run = false;
				std::wcout << L"Good Bye." << std::endl;
				continue; // exit loop
			}
			

			//
			if (!ptr_ctl_pipe->write(s_input)) {
				std::wcout << L"Error: cannot write to control pipe. retry again." << std::endl;
				continue; //error
			}

			

		} while (b_run);

		ptr_ctl_pipe.reset();

		n_result = EXIT_SUCCESS; // server is stopped successfully

	} while (false);

	return n_result;
}