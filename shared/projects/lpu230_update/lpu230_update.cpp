// wi_lpu230_update.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <wchar.h>
#include <string>
#include <chrono>
#include <thread>
#include <iomanip> // std::setprecision을 사용하기 위한 헤더
#include <sstream> // std::stringstream을 사용하기 위한 헤더
#include <algorithm>

#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/node.hpp>
#include <ftxui/screen/screen.hpp>

#include <mp_cconvert.h>

#include "update.h"

int main(int argc, char** argv)
{
	int n_result(EXIT_FAILURE);
	bool b_help(false);

	do {
        //////////////////////////////////////////
        //get command line parameters 
        _mp::type_list_wstring list_parameters(_mp::cconvert::get_command_line_parameters_by_list(argc, argv));

        auto it = std::find(list_parameters.begin(), list_parameters.end(), L"-h");
        if( it != std::end(list_parameters) ) {
            b_help = true;
            n_result = EXIT_SUCCESS;
            continue;
		}
        it = std::find(list_parameters.begin(), list_parameters.end(), L"/h");
        if (it != std::end(list_parameters)) {
            b_help = true;
            n_result = EXIT_SUCCESS;
            continue;
        }
        it = std::find(list_parameters.begin(), list_parameters.end(), L"--help");
        if (it != std::end(list_parameters)) {
            b_help = true;
            n_result = EXIT_SUCCESS;
            continue;
        }
		///////////////////////////////////////////
        if (list_parameters.size() == 3) {
            it = std::find(list_parameters.begin(), list_parameters.end(), L"--file");
            if (it != std::end(list_parameters)) {
                ++it;
                if (it != std::end(list_parameters)) {
                    // --file 옵션이 있고, 그 다음에 파일 경로가 있는 경우
                    std::wstring s_file_path = *it;
                    update_main(s_file_path);
                }
                n_result = EXIT_SUCCESS;
                continue;
            }
        }

        it = std::find(list_parameters.begin(), list_parameters.end(), L"-q");
        if (it != std::end(list_parameters)) {
            it = std::find(list_parameters.begin(), list_parameters.end(), L"-mmd1100_iso_mode");
            if (it != std::end(list_parameters)) {
                //firmware 업데이트 후, mmd1100 을 iso  mode 로 변경.(v1.4 부터)
                // - q 또는 - qa 옵션도 같이 사용 가능.  이 option 은 lpu237 에서 만 사용 가능.
                continue;
			}
			it = std::find(list_parameters.begin(), list_parameters.end(), L"-mmd1100_binary_mode");
            if (it != std::end(list_parameters)) {
                //firmware 업데이트 후, mmd1100 을 binary  mode 로 변경.(v1.4 부터)
                // - q 또는 - qa 옵션도 같이 사용 가능.  이 option 은 lpu237 에서 만 사용 가능.
                continue;
			}
            continue;
        }

        it = std::find(list_parameters.begin(), list_parameters.end(), L"-qa");
        if (it != std::end(list_parameters)) {
            it = std::find(list_parameters.begin(), list_parameters.end(), L"-mmd1100_iso_mode");
            if (it != std::end(list_parameters)) {
                //firmware 업데이트 후, mmd1100 을 iso  mode 로 변경.(v1.4 부터)
                // - q 또는 - qa 옵션도 같이 사용 가능.  이 option 은 lpu237 에서 만 사용 가능.
                continue;
            }
            it = std::find(list_parameters.begin(), list_parameters.end(), L"-mmd1100_binary_mode");
            if (it != std::end(list_parameters)) {
                //firmware 업데이트 후, mmd1100 을 binary  mode 로 변경.(v1.4 부터)
                // - q 또는 - qa 옵션도 같이 사용 가능.  이 option 은 lpu237 에서 만 사용 가능.
                continue;
            }
            continue;
        }

        it = std::find(list_parameters.begin(), list_parameters.end(), L"-mmd1100_iso_mode");
        if (it != std::end(list_parameters)) {
            //firmware 업데이트 후, mmd1100 을 iso  mode 로 변경.(v1.4 부터)
            // - q 또는 - qa 옵션도 같이 사용 가능.  이 option 은 lpu237 에서 만 사용 가능.
            continue;
        }
        it = std::find(list_parameters.begin(), list_parameters.end(), L"-mmd1100_binary_mode");
        if (it != std::end(list_parameters)) {
            //firmware 업데이트 후, mmd1100 을 binary  mode 로 변경.(v1.4 부터)
            // - q 또는 - qa 옵션도 같이 사용 가능.  이 option 은 lpu237 에서 만 사용 가능.
            continue;
        }

        b_help = true;
		n_result = EXIT_FAILURE;
	} while (false);

	if (b_help) {
		std::wcout << L"Usage: [/ibutton or /msr]" << std::endl;
		std::wcout << L"  /ibutton : run ibutton test." << std::endl;
		std::wcout << L"  /msr : run msr test." << std::endl;
		std::wcout << L"  /threadsafty : use threadsafty-code-style.(default)" << std::endl;
		std::wcout << L"  /getdataincallback : get ms-data in callback function." << std::endl;
		n_result = EXIT_SUCCESS;
	}

	return n_result;
}
