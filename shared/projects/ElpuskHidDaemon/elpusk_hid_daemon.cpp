
#include <websocket/mp_win_nt.h>


#include <iostream>
#include <thread>
#include <chrono>
#include <fstream>
#include <atomic>

#include <csignal>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <syslog.h>
#endif

#include <sys/stat.h>
#include <sys/types.h>


#include <stdio.h>
#include <wchar.h>
#include <string.h>
#include <stdlib.h>

#include <mp_type.h>
#include <mp_cconvert.h>

#include <main.h>
//#include <test/test_tp_hid.h>

/**
* this program must be on admin mode. 
* command line option
* "/server"(default) - run as security web socket server. the process of this option can be executed only one instance.
* "/trace" - display trace message of the process of "/server".
* "/cert" - generates the self signed certificate and register it for using in web-browser.
* "/removecert" - unregister the self signed certificate and delete it.
* "/removeall" - remove all logging files.
*/
int main(int argc, char* argv[])
{
	int n_result(EXIT_FAILURE);


	do {
		//get command line parameters 
		_mp::type_set_wstring set_parameters(_mp::cconvert::get_command_line_parameters(argc, argv));

		if (set_parameters.size()<2) {
			//run wss server
			n_result = main_wss(set_parameters);
			continue;
		}

		if (set_parameters.find(L"/server") != std::end(set_parameters)) {
			//run wss server
			n_result = main_wss(set_parameters);
			continue;
		}
		if (set_parameters.find(L"/trace") != std::end(set_parameters)) {
			n_result = main_trace(set_parameters);
			continue;
		}
		if (set_parameters.find(L"/cert") != std::end(set_parameters)) {
			n_result = main_cert(set_parameters);
			continue;
		}
		if (set_parameters.find(L"/removecert") != std::end(set_parameters)) {
			n_result = main_remove_cert(set_parameters);
			continue;
		}
		if (set_parameters.find(L"/removeall") != std::end(set_parameters)) {
			n_result = main_remove_all(set_parameters);
			continue;
		}



	} while (false);

	return n_result;
}