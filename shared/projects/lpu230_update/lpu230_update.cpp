// wi_lpu230_update.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <wchar.h>
#include <string>
#include <chrono>
#include <thread>
#include <iomanip> // std::setprecision을 사용하기 위한 헤더
#include <sstream> // std::stringstream을 사용하기 위한 헤더

#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/node.hpp>
#include <ftxui/screen/screen.hpp>

#include <mp_cconvert.h>

int main(int argc, char** argv)
{
	int n_result(EXIT_FAILURE);
	bool b_help(false);

	do {
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


		//////////////////////////////////////////
		//get command line parameters 
		_mp::type_set_wstring set_parameters( _mp::cconvert::get_command_line_parameters(argc, argv));

		if (set_parameters.size() < 2) {
			b_help = true;
			continue;
		}


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
