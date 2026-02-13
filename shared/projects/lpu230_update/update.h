#pragma once
#include <string>

#include "cupdater.h"
#include "cshare.h"

int update_main
(
	unsigned long n_session,
	const std::string& s_abs_full_rom_file,
	int n_fw_index,
	const std::string& s_device_path,
	bool b_display,
	bool b_log_file,
	bool b_mmd1100_iso_mode,
	cshare::Lpu237Interface lpu237_interface_after_update,
	bool b_run_by_cf,
	bool b_notify_progress_to_server,
	const _mp::type_list_string& list_commandline_options_for_debug
);