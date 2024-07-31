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
			continue;
		}

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
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		} while (b_run);

		n_result = EXIT_SUCCESS;
	} while (false);

	return n_result;

}
