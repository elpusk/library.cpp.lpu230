#include <cstdlib>
#include <string>

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <chrono>

#include <mp_cstring.h>

#include "update.h"
#include "cupdater.h"

int update_main(const std::wstring & s_abs_full_rom_file)
{
    std::string selected_rom = _mp::cstring::get_mcsc_from_unicode(s_abs_full_rom_file);

	cupdater updater;

	updater.start_update();

	updater.main_loop();

	return EXIT_SUCCESS;
}