#pragma once

#include <memory>
#include <string>
#include <thread>
#include <mutex>
#include <atomic>
#include <filesystem>
#include <queue>

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

#include <mp_cversion.h>
#include <mp_cwait.h>
#include <mp_clog.h>

#include "HidBootManager.h"

class cupdater {

public:
	typedef std::shared_ptr<cupdater> type_ptr;
	typedef _mp::cversion<unsigned char>	type_version;
public:
	enum class Lpu237Interface {
		nc = -1, //no consideration
		usb_keyboard = 0,//0
		usb_hid = 1,//1
		usb_vcom = 2,//2
		uart = 10
	};
	static std::string get_string(Lpu237Interface inf)
	{
		std::string s;
		switch (inf) {
		case cupdater::Lpu237Interface::nc: s = "no consideration"; break;
		case cupdater::Lpu237Interface::usb_keyboard: s = "usb_keyboard"; break;
		case cupdater::Lpu237Interface::usb_hid: s = "usb_hid"; break;
		case cupdater::Lpu237Interface::usb_vcom: s = "usb_vcom"; break;
		case cupdater::Lpu237Interface::uart: s = "uart"; break;
		default:
			s = "unknown";
			break;
		}
		return s;
	}

	enum class AppState {
		s_ini, s_selfile, s_selfirm, s_sellast, s_ing, s_com
		, s_max //
	};

	static std::string get_string(AppState st)
	{
		std::string s;
		switch (st) {
		case cupdater::AppState::s_ini: s = "s_ini";	break;
		case cupdater::AppState::s_selfile: s = "s_selfile";	break;
		case cupdater::AppState::s_selfirm: s = "s_selfirm";	break;
		case cupdater::AppState::s_sellast: s = "s_sellast";	break;
		case cupdater::AppState::s_ing: s = "s_ing";	break;
		case cupdater::AppState::s_com: s = "s_com";	break;
		case cupdater::AppState::s_max: s = "s_max";	break;
		default:
			s = "unknown";	
			break;
		}//end switch
		return s;
	}
	enum class AppEvent {
		e_start // press update button
		, e_sfile_or // in open file select dialog, select rom file and press ok button.
		, e_sfile_ob // in open file select dialog, select bin file and press ok button.
		, e_sfile_c // in open file select dialog, press cancel button.
		, e_sfirm_o // in firmware select dialog, select firmware and press ok button.
		, e_sfirm_c // in firmware select dialog, press cancel button.
		, e_slast_o // in last confirm dialog, press yes button.
		, e_slast_c // in last confirm dialog, press no button.
		, e_ustep_s // update a step processing success.
		, e_ustep_e // update a step processing error.
		, e_ulstep_s // update last step processing success.
		, e_ulstep_e // update last step processing error.
		, e_exit // press exit button.
		, event_max // not used, just for max event count.
	};

	static std::string get_string(AppEvent ev)
	{
		std::string s;
		switch (ev) {
		case cupdater::AppEvent::e_start: s = "e_start";	break;
		case cupdater::AppEvent::e_sfile_or: s = "e_sfile_or";	break;
		case cupdater::AppEvent::e_sfile_ob: s = "e_sfile_ob";	break;
		case cupdater::AppEvent::e_sfile_c: s = "e_sfile_c";	break;
		case cupdater::AppEvent::e_sfirm_o: s = "e_sfirm_o";	break;
		case cupdater::AppEvent::e_sfirm_c: s = "e_sfirm_c";	break;
		case cupdater::AppEvent::e_slast_o: s = "e_slast_o";	break;
		case cupdater::AppEvent::e_slast_c: s = "e_slast_c";	break;
		case cupdater::AppEvent::e_ustep_s: s = "e_ustep_s";	break;
		case cupdater::AppEvent::e_ustep_e: s = "e_ustep_e";	break;
		case cupdater::AppEvent::e_ulstep_s: s = "e_ulstep_s";	break;
		case cupdater::AppEvent::e_ulstep_e: s = "e_ulstep_e";	break;
		case cupdater::AppEvent::e_exit: s = "e_exit";	break;
		case cupdater::AppEvent::event_max: s = "event_max";	break;
		default:
			s = "unknown";
			break;
		}//end switch
		return s;
	}
public:
	cupdater(_mp::clog& log,bool b_disaplay, bool b_log, cupdater::Lpu237Interface lpu237_interface_after_update);
	virtual ~cupdater();
	// Update function to be implemented by derived classes
	bool start_update();
	// Function to check if an update is available
	bool is_update_available() const;

	//setters for rom file and target device information
	cupdater& set_rom_file(const std::string& s_abs_full_path_of_rom);
	cupdater& set_index_of_fw_in_rom(int n_index_of_fw_in_rom);
	cupdater& set_update_condition_of_fw(const std::wstring& s_update_condition_of_fw);

	cupdater& set_device_path(const std::string& s_device_path);
	cupdater& set_device_version(const std::wstring& s_device_version);
	cupdater& set_mmd1100_iso_mode(bool b_enable);

	// Getters for rom file and target device information
	const std::string& get_rom_file() const;
	int get_index_of_fw_in_rom() const;
	const std::wstring& get_update_condition_of_fw() const;
	bool is_fw_file_in_rom_format() const;

	const std::string& get_device_path() const;
	const type_version& get_device_version() const;

	bool is_mmd1100_iso_mode() const;
	//

	/**
	* @brief Set the range of progress for the update process.
	* @param n_progress_min Minimum progress value.(greater than equal to 0)
	*	f_progress_min must be less than f_progress_max.
	* @param n_progress_max Maximum progress value.
	*/
	void set_range_of_progress(int n_progress_min, int n_progress_max);

	void set_pos_of_progress(int n_progress_pos);

	int get_pos_of_progress() const;

	/**
	* @brief Update the list of files in the current directory.
	*   update m_v_files_in_current_dir and m_v_rom_files_in_current_dir with m_current_dir.
	*/
	void update_files_list_of_cur_dir();
	void update_fw_list_of_selected_rom();

	void ui_main_loop();

	/**
	* @brief as like initial dialog
	*/
	bool initial_update();

private:
	void _updates_thread_function();

	std::shared_ptr<ftxui::Component> _create_sub_ui0_ini();
	std::shared_ptr<ftxui::Component> _create_sub_ui1_select_file();
	std::shared_ptr<ftxui::Component> _create_sub_ui2_select_firmware();
	std::shared_ptr<ftxui::Component> _create_sub_ui3_last_confirm();
	std::shared_ptr<ftxui::Component> _create_sub_ui4_updating();
	std::shared_ptr<ftxui::Component> _create_sub_ui5_complete();

	/**
	* @brief push message with thread safety
	* @param s_in_msg : pushed string
	*/
	void _push_message(const std::string& s_in_msg);

	/**
	* @brief pop message with thread safety
	* @param s_out_msg : poped string
	* @param b_remove_after_pop  true : if a item is poped,remove item from q.
	* @return true : pop OK. false : none item in the q.
	*/
	bool _pop_message(std::string& s_out_msg,bool b_remove_after_pop = true);

private:
	CHidBootManager* m_p_mgmt;

	std::atomic<cupdater::AppState> m_state;

	int m_n_selected_file_in_v_files_in_current_dir;
	std::vector<std::string> m_v_files_in_current_dir;
	std::vector<std::string> m_v_rom_files_in_current_dir; // rom file list in the m_v_files_in_current_dir.
	std::filesystem::path m_current_dir;

	int m_n_progress_cur, m_n_progress_min, m_n_progress_max;

	ftxui::ScreenInteractive m_screen;

	std::shared_ptr<ftxui::Component> m_ptr_update_button; // update start button

	std::shared_ptr<ftxui::Component> m_ptr_file_menu; // list UI in file selection dialog
	std::shared_ptr<ftxui::Component> m_ptr_select_file_button; // select button in file selection dialog
	std::shared_ptr<ftxui::Component> m_ptr_cancel_file_button; // cancel button in file selection dialog

	int m_n_selected_fw;
	std::vector<std::string> m_v_firmware_list; //firmware list in the selected rom file.
	std::shared_ptr<ftxui::Component> m_ptr_fw_menu; // list UI by m_v_firmware_list
	std::shared_ptr<ftxui::Component> m_ptr_ok_fw_button; // ok(selection) button of firmware select dialog.
	std::shared_ptr<ftxui::Component> m_ptr_cancel_fw_button; // cancel button of firmware select dialog.

	std::shared_ptr<ftxui::Component> m_ptr_ok_confirm_button;
	std::shared_ptr<ftxui::Component> m_ptr_cancel_confirm_button;
	

	std::shared_ptr<ftxui::Component> m_ptr_exit_button; //exit button in complete dialog
	std::shared_ptr<ftxui::Component> m_ptr_exit_container;

	// Tab container for switching screens
	int m_n_tab_index;  // 0: s_ini, 1: s_selfile, 2: s_selfirm, 3: s_sellast, 4: s_ing, 5: s_scom
	std::vector<ftxui::Component> m_v_tabs;
	std::shared_ptr<ftxui::Component> m_ptr_tab_container;

	// Root with title
	std::shared_ptr<ftxui::Component> m_ptr_root;
	std::shared_ptr<ftxui::Component> m_ptr_final_root;// Event handler for progress animation

	/**
	* @brief Update the application state based on the given event.
	* @param event The event that triggers the state update.
	* @param s_rom_or_bin_file rom or bin file abs full path.
	*   if s_rom_or_bin_file isn't empty, s_rom_or_bin_file file ,must exist.
	* @return 
	*    1'st - true : state is changed, false : state is not changed.
	*    2'nd - The new application state after processing the event.
	*    3'th - message to be displayed to the user (empty if no message).
	*/
	std::tuple<bool,cupdater::AppState,std::string> _update_state(cupdater::AppEvent event,const std::string & s_rom_or_bin_file = std::string());

	_mp::clog &m_log_ref;
	//////////////////////////////////
	// UI option
	bool m_b_display; //CUI enable or disable
	bool m_b_log; //log file generation

	// MMD1100 option
	bool m_b_mmd1100_iso_mode;

	cupdater::Lpu237Interface m_lpu237_interface_after_update; // after updating, interface change value.

	//////////////////////////////////
	//rom file section
	std::string m_s_abs_full_path_of_rom;
	int m_n_index_of_fw_in_rom;
	std::wstring m_s_update_condition_of_fw;
	bool m_b_fw_file_is_rom_format;

	//target device section
	std::string m_s_device_path;
	cupdater::type_version m_version_of_device;

	// Mutex for thread safety
	std::mutex m_mutex;
	std::queue<std::string> m_q_messages; // Queue for messages from the update thread

	_mp::cwait m_wait; // Wait object for synchronization
	int m_n_kill_signal; // Signal to stop the updater
	std::atomic_bool m_b_is_running; // Atomic boolean to check if the updater is running
	std::shared_ptr<std::thread> m_ptr_thread_update; // Thread for update process
	

private:// don't call these functions
	cupdater() = delete;
	cupdater(const cupdater&) = delete; // Disable copy constructor
	cupdater& operator=(const cupdater&) = delete; // Disable assignment operator
};