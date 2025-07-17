#include <websocket/mp_win_nt.h>

#include <cstdio>
#include <main.h>

#include <mp_coffee.h>
#include <mp_cstring.h>
#include <mp_clog.h>
#include <mp_csystem_.h>

/**
 * stop the process of "/server" and remove all installed file and paths.
 */
int main_bye(const _mp::type_set_wstring& set_parameters)
{
	int n_result(EXIT_FAILURE);

	do {
		_mp::cnamed_pipe::type_ptr ptr_ctl_pipe;
		//setup controller
		ptr_ctl_pipe = std::make_shared<_mp::cnamed_pipe>(_mp::_coffee::CONST_S_COFFEE_MGMT_CTL_PIPE_NAME, false);

		if (!ptr_ctl_pipe) {
			continue; //error
		}
		if (!ptr_ctl_pipe->is_ini()) {
			continue; //error
		}

		std::wstring s_data(_mp::_coffee::CONST_S_COFFEE_MGMT_CTL_REQ);
		if (!ptr_ctl_pipe->write(s_data)) {
			continue; //error
		}
		ptr_ctl_pipe.reset();

		n_result = EXIT_SUCCESS; // server is stopped successfully

	} while (false);

	return n_result;
}