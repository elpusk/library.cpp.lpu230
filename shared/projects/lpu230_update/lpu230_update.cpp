// wi_lpu230_update.cpp : This file contains the 'main' function. Program execution begins and ends there.
// Windows 10 버전 1703(Creators Update) 이상 사용 필수. ftxui 의 SetConsoleMode() 사용시 ENABLE_VIRTUAL_TERMINAL_INPUT 사용.

#include <iostream>
#include <wchar.h>
#include <string>
#include <chrono>
#include <thread>
#include <iomanip> // std::setprecision을 사용하기 위한 헤더
#include <sstream> // std::stringstream을 사용하기 위한 헤더
#include <algorithm>
#include <filesystem>

#ifdef _WIN32
#include <mp_win_console_manager.h>
#endif

#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/node.hpp>
#include <ftxui/screen/screen.hpp>

#include <mp_cconvert.h>

#include "update.h"
#include "cshare.h"

static void _print_help(const std::string& program_name);

/**
* @brief command option 유효성 검사. ONLY
* @brief dev_lib.dll(libdev_dll.so) 와 tg_rom.dll(libtg_rom.dll) 를 사용.
* @brief 여기서는 코솔 출력 금지.
* @brief 여기서 로깅도 금지.
* 
* @prarm lpCmdLine : The command line for the application, excluding the program name. To retrieve the entire command line, use the GetCommandLine function.
*/
#ifdef _WIN32
int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nShowCmd)
#else
int main(int argc, char** argv)
#endif
{
#ifdef _WIN32
	_mp::win_console_manager win_console_manager; //윈도우즈에서 자동으로 콘솔 생성 및 제거 창 관리를 위한 객체.
#endif
    int n_result(_mp::exit_error_invalid_command_line_argment);//(EXIT_FAILURE);

	bool b_help(false);
    bool b_quiet(false);
    bool b_notify(false);
    bool b_quietabsolute(false);
    bool b_mmd1100_iso_mode(false);
    bool b_mmd1100_binary_mode(false);
    bool b_rom_file(false);
    bool b_device_path(false);
    bool b_manual(false);
    cshare::Lpu237Interface lpu237_interface_after_update(cshare::Lpu237Interface::nc);
    bool b_run_by_cf(false); //executed by coffee manager 2'nd 

    unsigned long n_session = 0;
    std::string s_rom_file;
    std::string s_device_path;

    _mp::type_list_string list_parameters;
	do {
        //////////////////////////////////////////
        //get command line parameters 
#ifdef _WIN32
        int argc = 0;
        wchar_t** argv = NULL;
		std::tie(argc, argv) = win_console_manager.get_command_line_arguments();
#endif
        list_parameters = _mp::cconvert::get_command_line_parameters_by_mcsc_list(argc, argv);

        if (list_parameters.size() <= 1) {
            n_result = EXIT_SUCCESS;
            continue; // 일반 모드.
        }
        ///////////////////////////////////////////
        // -h, --help : 도움말 표시
        auto it = std::find(list_parameters.begin(), list_parameters.end(), "-h");
        if( it != std::end(list_parameters) ) {
            b_help = true;
            n_result = EXIT_SUCCESS;
            continue;
		}
        it = std::find(list_parameters.begin(), list_parameters.end(), "--help");
        if (it != std::end(list_parameters)) {
            b_help = true;
            n_result = EXIT_SUCCESS;
            continue;
        }

        ////////////////////////////////////////////////
        // -mmd1100_iso_mode, --mmd1100_iso_mode
        it = std::find(list_parameters.begin(), list_parameters.end(), "-mmd1100_iso_mode");
        if (it != std::end(list_parameters)) {
            //firmware 업데이트 후, mmd1100 을 iso  mode 로 변경.(v1.4 부터)
            // - q 또는 - qa 옵션도 같이 사용 가능.  이 option 은 lpu237 에서 만 사용 가능.
            b_mmd1100_iso_mode = true;
        }
        if (b_mmd1100_iso_mode) {
            it = std::find(list_parameters.begin(), list_parameters.end(), "--mmd1100_iso_mode");
            if (it != std::end(list_parameters)) {
                b_help = true;
                continue;//error
            }
        }
        else {
            it = std::find(list_parameters.begin(), list_parameters.end(), "--mmd1100_iso_mode");
            if (it != std::end(list_parameters)) {
                b_mmd1100_iso_mode = true;
            }
        }

        ////////////////////////////////////////////////
        // -mmd1100_binary_mode, --mmd1100_binary_mode
        it = std::find(list_parameters.begin(), list_parameters.end(), "-mmd1100_binary_mode");
        if (it != std::end(list_parameters)) {
            //firmware 업데이트 후, mmd1100 을 binary  mode 로 변경.(v1.4 부터)
            // - q 또는 - qa 옵션도 같이 사용 가능.  이 option 은 lpu237 에서 만 사용 가능.
            b_mmd1100_binary_mode = true;
        }
        if (b_mmd1100_binary_mode) {
            it = std::find(list_parameters.begin(), list_parameters.end(), "--mmd1100_binary_mode");
            if (it != std::end(list_parameters)) {
                b_help = true;
                continue;//error
            }
        }
        else {
            it = std::find(list_parameters.begin(), list_parameters.end(), "--mmd1100_binary_mode");
            if (it != std::end(list_parameters)) {
                b_mmd1100_binary_mode = true;
            }
        }

        if (b_mmd1100_binary_mode && b_mmd1100_iso_mode) {
            // 같이 사용 불가능한 옵션.
            b_help = true;
            continue;//error
        }

        ///////////////////////////////////////////
        //-q, --quiet : (quiet)Updater 동작 시, 화면 표시 없고, 로그  파일만 기록.
        it = std::find(list_parameters.begin(), list_parameters.end(), "-q");
        if (it != std::end(list_parameters)) {
            b_quiet = true;
        }

        if (b_quiet) {
            it = std::find(list_parameters.begin(), list_parameters.end(), "--quiet");
            if (it != std::end(list_parameters)) {
                b_help = true;
                continue;//error
            }
        }
        else {
            it = std::find(list_parameters.begin(), list_parameters.end(), "--quiet");
            if (it != std::end(list_parameters)) {
                b_quiet = true;
            }
        }

        ///////////////////////////////////////////
        //-qa, --quietabsolute : (quiet absolute)Updater 동작 시, 화면 표시 없고, 로그 파일도 기록하지 않음.
        it = std::find(list_parameters.begin(), list_parameters.end(), "-qa");
        if (it != std::end(list_parameters)) {
            b_quietabsolute = true;
        }

        if (b_quietabsolute) {
            it = std::find(list_parameters.begin(), list_parameters.end(), "--quietabsolute");
            if (it != std::end(list_parameters)) {
                b_help = true;
                continue;//error
            }
        }
        else {
            it = std::find(list_parameters.begin(), list_parameters.end(), "--quietabsolute");
            if (it != std::end(list_parameters)) {
                b_quietabsolute = true;
            }
        }

        ///////////////////////////////////////////
        // -f, --file : updater 동작시, rom 파일를 시작 parameter 로
        it = std::find(list_parameters.begin(), list_parameters.end(), "-f");
        if (it != std::end(list_parameters)) {
            ++it;
            if (it == std::end(list_parameters)) {
                // -f, --file 다음은 rom 파일 path.
                b_help = true;
                continue;//error
            }

            if (!std::filesystem::exists(*it)) {
                b_help = true;
                continue;//error
            }
            if (!std::filesystem::is_regular_file(*it)) {
                b_help = true;
                continue;//error
            }

            s_rom_file = *it;
            b_rom_file = true;
        }

        if (b_rom_file) {
            it = std::find(list_parameters.begin(), list_parameters.end(), "--file");
            if (it != std::end(list_parameters)) {
                b_help = true;
                continue;//error
            }
        }
        else {
            it = std::find(list_parameters.begin(), list_parameters.end(), "--file");
            if (it != std::end(list_parameters)) {
                ++it;
                if (it == std::end(list_parameters)) {
                    // -f, --file 다음은 rom 파일 path.
                    b_help = true;
                    continue;//error
                }

                if (!std::filesystem::exists(*it)) {
                    b_help = true;
                    continue;//error
                }
                if (!std::filesystem::is_regular_file(*it)) {
                    b_help = true;
                    continue;//error
                }

                s_rom_file = *it;
                b_rom_file = true;
            }
        }

        ///////////////////////////////////////////
        // -m option
        it = std::find(list_parameters.begin(), list_parameters.end(), "-m");
        if (it != std::end(list_parameters)) {
            if (b_quiet || b_quietabsolute) {
                //manual mode 랑 quiet, quietabsolute 는 동시에 불가.
                b_help = true;
                continue;//error
            }
        }

        ///////////////////////////////////////////
        // -mx option
        it = std::find(list_parameters.begin(), list_parameters.end(), "-m0");
        if (it != std::end(list_parameters)) {
            lpu237_interface_after_update = cshare::Lpu237Interface::usb_keyboard;
        }
        it = std::find(list_parameters.begin(), list_parameters.end(), "-m1");
        if (it != std::end(list_parameters)) {
            if (lpu237_interface_after_update != cshare::Lpu237Interface::nc) {
                b_help = true;
                continue;//error
            }
            lpu237_interface_after_update = cshare::Lpu237Interface::usb_hid;
        }
        it = std::find(list_parameters.begin(), list_parameters.end(), "-m2");
        if (it != std::end(list_parameters)) {
            if (lpu237_interface_after_update != cshare::Lpu237Interface::nc) {
                b_help = true;
                continue;//error
            }
            lpu237_interface_after_update = cshare::Lpu237Interface::usb_vcom;
        }
        it = std::find(list_parameters.begin(), list_parameters.end(), "-m10");
        if (it != std::end(list_parameters)) {
            if (lpu237_interface_after_update != cshare::Lpu237Interface::nc) {
                b_help = true;
                continue;//error
            }
            lpu237_interface_after_update = cshare::Lpu237Interface::uart;
        }

        //////////////////////////////////////////////////////////
        // coffee manager flag
        it = std::find(list_parameters.begin(), list_parameters.end(), "--run_by_coffee_manager_2nd");
        if (it != std::end(list_parameters)) {
			// cf2 에 의한 실행의 경우 모든 option run_by_coffee_manager_2nd 에 맞는 지 검사.
            
            // --session 필수
            it = std::find(list_parameters.begin(), list_parameters.end(), "--session");
            if (it == std::end(list_parameters)) {
                continue;// 에러
            }
            ++it;
            if (it == std::end(list_parameters)) {
                // --session 다음은 10 진수 session number.
                continue;//error
            }

            try {
                n_session = std::stoul(*it);
            }
            catch (...) {
                continue; // format 에러
            }

			// --file 필수.
            it = std::find(list_parameters.begin(), list_parameters.end(), "--file");
            if (it == std::end(list_parameters)) {
                continue;// 에러
            }
            ++it;
            if (it == std::end(list_parameters)) {
                // -f, --file 다음은 rom 파일 path.
                continue;//error
            }

            if (!std::filesystem::exists(*it)) {
                continue;//error
            }
            if (!std::filesystem::is_regular_file(*it)) {
                continue;//error
            }

            s_rom_file = *it;
            b_rom_file = true;

            // --device 필수. : s_device_path
            it = std::find(list_parameters.begin(), list_parameters.end(), "--device");
            if (it == std::end(list_parameters)) {
                continue;//error
            }
            ++it;
            if (it == std::end(list_parameters)) {
                // --device 다음은 device path.
                continue;//error
            }
            s_device_path = *it;

            b_run_by_cf = true;

            // --quiet 선택
            it = std::find(list_parameters.begin(), list_parameters.end(), "--quiet");
            if (it != std::end(list_parameters)) {
                b_quiet = true;
            }
            else {
                b_quiet = false;
            }

            // --notify 선택.
            it = std::find(list_parameters.begin(), list_parameters.end(), "--notify");
            if (it != std::end(list_parameters)) {
                b_notify = true;
            }
            else {
                b_notify = false;
            }

        }

        

        //////////////////////////////////
		n_result = EXIT_SUCCESS;
	} while (false);

    do {
        bool b_log(true);
        bool b_display(true);

        if (b_quiet) {
            b_display = false;
        }
        if (b_quietabsolute) {
            b_display = false;
            b_log = false;
        }
        //
        if (b_display) {
            if (b_help) {
                if (!list_parameters.empty()) {
                    _print_help(std::filesystem::path(*list_parameters.begin()).filename().string());
                }
                
                continue;
            }
        }
        if (n_result != EXIT_SUCCESS) {
            continue;//std::wcout << "Missing Format......." << std::endl;
        }

        // .프로그램 시작.
        n_result = update_main(
            n_session,
            s_rom_file,
            s_device_path,
            b_display,
            b_log,
            b_mmd1100_iso_mode,
            lpu237_interface_after_update,
            b_run_by_cf,
            b_notify
        );

    } while (false);

	return n_result;
}


void _print_help(const std::string& program_name)
{
    std::cout <<
        "Usage: v2.4\n"
        "  " << program_name << " [OPTIONS]\n"
        "\n"
        "Options:\n"
        "  --file, -f <path>\n"
        "      Specify the ROM or BIN file path to use.\n"
        "\n"
        "  --device, -d <path>\n"
        "      Specify the device path to update.\n"
        "      If not provided, the first connected device found on the PC will be selected.\n"
        "\n"
        "  --help, -h\n"
        "      Show this help message. If this option is present, all other options are ignored.\n"
        "\n"
        "  --quiet, -q\n"
        "      Suppress console output and only write to the log file.\n"
        "      Cannot be used together with --quietabsolute.\n"
        "\n"
        "  --quietabsolute, -qa\n"
        "      Suppress both console output and log file writing.\n"
        "      Cannot be used together with --quiet.\n"
        "\n"
        "  --mmd1100_iso_mode, -mmd1100_iso_mode\n"
        "      After the update, set the MMD1100 device to ISO mode.\n"
        "      Cannot be used together with --mmd1100_binary_mode.\n"
        "\n"
        "  --mmd1100_binary_mode, -mmd1100_binary_mode\n"
        "      After the update, set the MMD1100 device to Binary mode.\n"
        "      Cannot be used together with --mmd1100_iso_mode.\n"
        << std::endl;
    std::cout << "press any key to exit..." << std::endl;
    std::cin.get();
}
