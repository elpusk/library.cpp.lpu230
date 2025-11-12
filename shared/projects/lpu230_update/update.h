#pragma once
#include <string>

#include "cupdater.h"
#include "cshare.h"

int update_main
(
	const std::string& s_abs_full_rom_file,
	const std::string& s_device_path,
	bool b_display,
	bool b_log_file,
	bool b_mmd1100_iso_mode,
	cshare::Lpu237Interface lpu237_interface_after_update,
	bool b_run_by_cf,
	bool b_notify_progress_to_server
);