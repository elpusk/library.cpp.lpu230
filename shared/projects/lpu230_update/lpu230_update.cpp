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
#include <filesystem>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif

#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/node.hpp>
#include <ftxui/screen/screen.hpp>

#include <mp_cconvert.h>

#include "update.h"
#include "cshare.h"

static void _hide_console();
static void _print_help(const std::string& program_name);

/**
* @brief command option 유효성 검사. ONLY
*   
*   dev_lib.dll(libdev_dll.so) 와 tg_rom.dll(libtg_rom.dll) 를 사용.
*/
int main(int argc, char** argv)
{
	int n_result(EXIT_FAILURE);

	bool b_help(false);
    bool b_quiet(false);
    bool b_quietabsolute(false);
    bool b_mmd1100_iso_mode(false);
    bool b_mmd1100_binary_mode(false);
    bool b_rom_file(false);
    bool b_device_path(false);
    bool b_manual(false);
    cshare::Lpu237Interface lpu237_interface_after_update(cshare::Lpu237Interface::nc);
    bool b_run_by_cf(false); //executed by coffee manager 2'nd 

    std::string s_rom_file;
    std::string s_device_path;

	do {
        //////////////////////////////////////////
        //get command line parameters 
        _mp::type_list_string list_parameters(_mp::cconvert::get_command_line_parameters_by_mcsc_list(argc, argv));

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

        // coffee manager flag
        it = std::find(list_parameters.begin(), list_parameters.end(), "--run_by_coffee_manager_2nd");
        if (it != std::end(list_parameters)) {
            b_run_by_cf = true;
        }

        

        //////////////////////////////////
		n_result = EXIT_SUCCESS;
	} while (false);

    do {
        if (n_result != EXIT_SUCCESS) {
            std::wcout << "Missing Format......." << std::endl;
        }
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
        if (!b_display) {
            //hide mode
            _hide_console();
        }
        else {
            if (b_help) {
                _print_help(std::filesystem::path(argv[0]).filename().string());
                continue;
            }
        }

        if (n_result != EXIT_SUCCESS) {
            continue;
        }


        // .프로그램 시작.
        n_result = update_main(
            s_rom_file,
            s_device_path,
            b_display,
            b_log,
            b_mmd1100_iso_mode,
            lpu237_interface_after_update,
            b_run_by_cf
        );

    } while (false);

	return n_result;
}


void _print_help(const std::string& program_name)
{
    std::cout <<
        "Usage:\n"
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
}

void _hide_console() 
{
#ifdef _WIN32
    FreeConsole(); // Detach console
#else
    // 이후 출력 억제
    std::cout.setstate(std::ios_base::failbit);
    std::cerr.setstate(std::ios_base::failbit);
#endif
}
