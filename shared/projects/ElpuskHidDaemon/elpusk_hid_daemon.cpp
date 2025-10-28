
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


#ifdef _WIN32
// for libusb
#pragma comment(lib, "hid.lib")
#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "Cfgmgr32.lib")
#endif //_WIN32


/**
* 2025.08.08 - v2.0
* 2025.10.28 - v2.1 : lpu230_update 지원을 위함.
*/

/**
* @bref display help message.
*/
static void _display_help();

/**
* this program must be on admin mode. 
* command line option
* "/server" or "--server"(default) - run as security web socket server. the process of this option can be executed only one instance.
* "/bye" or "--bye" - stop the process of "/server".
* "/trace" or "--trace" - display trace message of the process of "/server".
* "/cert" or "--cert"- generates the self signed certificate and register it for using in web-browser.
* "/removecert" or "--removecert" - unregister the self signed certificate and delete it.
* "/removeall" or "--removeall" - remove all logging files.
* "/terminal" or "--terminal" - run terminal for adjusting the server.
* "/help" or "--help" - display this help message.
* "/version" or "--version" - display elpusk-hid-d version.
* 
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
		//
		if (set_parameters.find(L"/server") != std::end(set_parameters)) {
			//run wss server
			n_result = main_wss(set_parameters);
			continue;
		}
		if (set_parameters.find(L"--server") != std::end(set_parameters)) {
			//run wss server
			n_result = main_wss(set_parameters);
			continue;
		}
		//
		if (set_parameters.find(L"/bye") != std::end(set_parameters)) {
			//stop wss server
			n_result = main_bye(set_parameters);
			continue;
		}
		if (set_parameters.find(L"--bye") != std::end(set_parameters)) {
			//stop wss server
			n_result = main_bye(set_parameters);
			continue;
		}
		//
		if (set_parameters.find(L"/trace") != std::end(set_parameters)) {
			n_result = main_trace(set_parameters);
			continue;
		}
		if (set_parameters.find(L"--trace") != std::end(set_parameters)) {
			n_result = main_trace(set_parameters);
			continue;
		}
		//
		if (set_parameters.find(L"/cert") != std::end(set_parameters)) {
			n_result = main_cert(set_parameters);
			continue;
		}
		if (set_parameters.find(L"--cert") != std::end(set_parameters)) {
			n_result = main_cert(set_parameters);
			continue;
		}
		//
		if (set_parameters.find(L"/removecert") != std::end(set_parameters)) {
			n_result = main_remove_cert(set_parameters);
			continue;
		}
		if (set_parameters.find(L"--removecert") != std::end(set_parameters)) {
			n_result = main_remove_cert(set_parameters);
			continue;
		}
		//
		if (set_parameters.find(L"/removeall") != std::end(set_parameters)) {
			n_result = main_remove_all(set_parameters);
			continue;
		}
		if (set_parameters.find(L"--removeall") != std::end(set_parameters)) {
			n_result = main_remove_all(set_parameters);
			continue;
		}
		//
		if (set_parameters.find(L"/terminal") != std::end(set_parameters)) {
			n_result = main_terminal(set_parameters);
			continue;
		}
		if (set_parameters.find(L"--terminal") != std::end(set_parameters)) {
			n_result = main_terminal(set_parameters);
			continue;
		}
		//
		if (set_parameters.find(L"/help") != std::end(set_parameters)) {
			_display_help();
			n_result = EXIT_SUCCESS;
			continue;
		}
		if (set_parameters.find(L"--help") != std::end(set_parameters)) {
			_display_help();
			n_result = EXIT_SUCCESS;
			continue;
		}
		//
		std::wstring ws_version(L"2.1");

		if (set_parameters.find(L"/version") != std::end(set_parameters)) {
			std::wcout << ws_version << std::endl;
			n_result = EXIT_SUCCESS;
			continue;
		}
		if (set_parameters.find(L"--version") != std::end(set_parameters)) {
			std::wcout << ws_version << std::endl;
			n_result = EXIT_SUCCESS;
			continue;
		}

		std::wcout << L"unknown option." << std::endl;


	} while (false);

	return n_result;
}

void _display_help()
{
	std::wcout << L" * Execution Format\n\n";
	std::wcout << L"elpusk-hid-d [option]\n\n";

	std::wcout << L" * Available Options\n\n";
	std::wcout << L"1. /server or --server (default)\n"
		<< L"   Runs in secure WebSocket server (WSS) mode.\n"
		<< L"   Note: Only a single instance is supported in this mode.\n\n";

	std::wcout << L"2. /bye or --bye\n"
		<< L"   Sends a termination request to the currently running instance in secure WebSocket server mode.\n\n";

	std::wcout << L"3. /trace or --trace\n"
		<< L"   Runs in trace mode.\n"
		<< L"   Displays messages generated by the instance running in WSS mode in real time.\n\n";

	std::wcout << L"4. /cert or --cert\n"
		<< L"   Issues and registers a self-signed root certificate.\n\n";

	std::wcout << L"5. /removecert or --removecert\n"
		<< L"   Deletes and unregisters the previously registered self-signed root certificate.\n\n";

	std::wcout << L"6. /removeall or --removeall\n"
		<< L"   Deletes all log files.\n\n";

	std::wcout << L"7. /terminal or --terminal\n"
		<< L"   Launches a console that can send requests to the instance running in secure WebSocket server mode.\n\n";

	std::wcout << L"8. /help or --help\n"
		<< L"   Displays help information.\n\n";

	std::wcout << L"9. /version or --version\n"
		<< L"   Displays version.\n";

}
