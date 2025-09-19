#include <main.h>

#include <thread>

#include <mp_coffee.h>
#include <mp_clog.h>

/**
 * run trace console.
 */
int main_trace(const _mp::type_set_wstring& set_parameters)
{
	static _mp::cnamed_pipe::type_ptr ptr_ctl_pipe;
	int n_result(EXIT_FAILURE);

	do {
		ptr_ctl_pipe = _mp::clog::get_trace_client(_mp::_coffee::CONST_S_COFFEE_MGMT_TRACE_PIPE_NAME);
		if (!ptr_ctl_pipe) {
			n_result = _mp::exit_error_get_ctl_pipe;
			std::wcout << L"Error Server trace source open.\n";
			continue;
		}

		if(!ptr_ctl_pipe->is_ini()){
			//
			std::wcout << L"None Server trace source.\ncontinue... detecting.\n";
			std::wcout << L"For stop, Press Ctl+c for breaking.\n";

			do {
				ptr_ctl_pipe = _mp::clog::get_trace_client(_mp::_coffee::CONST_S_COFFEE_MGMT_TRACE_PIPE_NAME);
				if (!ptr_ctl_pipe) {
					std::wcout << L"Error Server trace source open.\n";
					break;
				}
			} while (!ptr_ctl_pipe->is_ini());
			if (!ptr_ctl_pipe) {
				continue;
			}
		}

		std::wcout << L"Found Server trace source.\n";

		std::wstring s_data;
		bool b_run = true;
		int i = 0;

		do {
			if (ptr_ctl_pipe->read(s_data)) {
				std::wcout << s_data;
			}
			else {
				++i;
				if (i % 10) {
					//std::wcout << L'.';
				}
				else {
					//std::wcout << std::endl;
				}
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(_mp::_coffee::CONST_N_COFFEE_MGMT_CTL_PIPE_READ_INTERVAL_MMSEC));
		} while (b_run);

		n_result = EXIT_SUCCESS;
	} while (false);

	return n_result;

}
