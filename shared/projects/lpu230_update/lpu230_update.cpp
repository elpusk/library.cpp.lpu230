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
        /*
        if (list_parameters.size() == 1) {
            // none option case 
            // 옵션 없이 updater 를 실행 하면, 동작 상태를 화면에 표시하고, 로그 파일도 기록한다.
            continue;
        }
        */
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

        //////////////////////////////////////////
        /*
        double progress = 0.0;

        // 화면 생성
        auto screen = ftxui::Screen::Create(ftxui::Dimension::Full());

        while (progress < 1.0) {
            // 진행률 증가
            progress += 0.01;

            // 진행률을 문자열로 변환 (예: 50.0%)
            std::stringstream ss;
            ss << std::fixed << std::setprecision(1) << progress * 100.0 << "%";

            // 게이지 바 생성
            auto gauge_bar = ftxui::gauge(progress);

            // 진행률 텍스트 생성 (진행률을 굵은 글씨로 표시)
            auto percentage_text = ftxui::text(ss.str()) | ftxui::bold;

            // 게이지 바와 텍스트를 수평으로 정렬
            auto document = ftxui::hbox({
                percentage_text,
                ftxui::filler(), // 게이지 바와 텍스트 사이에 공간을 채워줌
                gauge_bar
                });

            // 화면 렌더링
            screen.Clear();
            ftxui::Render(screen, document.get());
            screen.Print();

            // 잠시 대기
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        // 최종적으로 100% 진행률 표시
        screen.Clear();

        std::stringstream final_ss;
        final_ss << std::fixed << std::setprecision(1) << 100.0 << "%";

        auto final_gauge = ftxui::gauge(1.0);
        auto final_text = ftxui::text(final_ss.str()) | ftxui::bold;

        auto final_document = ftxui::hbox({
            final_gauge,
            ftxui::filler(),
            final_text
            });

        ftxui::Render(screen, final_document.get());
        screen.Print();
        */



		b_help = true;
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
