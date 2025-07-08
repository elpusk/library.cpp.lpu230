// coffee_service.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

#include <mp_cconvert.h>

#include "crun_service.h"


int main(int argc, char* argv[])
{
	std::wstring stemp;

	crun_service & svr = crun_service::get_instance();

	_mp::type_list_wstring list_parameters = _mp::cconvert::get_command_line_parameters_by_list(argc, argv);

	do {
		if (list_parameters.size() == 2) {
			auto it = list_parameters.begin();
			++it; // get the second parameter

			// uninstall service if switch is "-u"
			if (it->compare(L"-u") == 0) {
				std::wcout << L"Please waits!. uninstalling service." << std::endl;
				svr.uninstall();
				continue;
			}
			// install service if switch is "-i"
			if (it->compare(L"-i") == 0) {
				std::wcout << L"Please waits!. installing service." << std::endl;
				svr.install();
				continue;
			}

			// bounce service if switch is "-b"
			if (it->compare(L"-b") == 0){
				svr.kill_service();
				svr.run_service();
				continue;
			}
			// kill a service
			if (it->compare(L"-k") == 0){
				std::wcout << L"Please waits!. stopping service." << std::endl;
				if (!svr.kill_service()) {
					std::wcout << L"Failed to kill service " << std::endl;
				}
				continue;
			}
			// run a service
			if (it->compare(L"-r") == 0){
				std::wcout << L"Please waits!. starting service." << std::endl;
				if (!svr.run_service()) {
					std::wcout << L"Failed to run service " << std::endl;
				}
				continue;
			}

			// assume user is starting this service 
			// start a worker thread to check for dead programs (and restart if necessary)
			DWORD nError = svr.create_worker();
			if (nError) {
				std::wcout << L"_beginthread failed, error code = " << std::to_wstring((unsigned long long)nError) << std::endl;
			}
			else {

				// pass dispatch table to service controller
				nError = svr.launch_service();
				if (nError) {
					std::wcout << L"StartServiceCtrlDispatcher failed, error code = " << std::to_wstring((unsigned long long)nError) << std::endl;
				}
				// you don't get here unless the service is shutdown
			}


		}

	} while (false);

	return 0;
}

