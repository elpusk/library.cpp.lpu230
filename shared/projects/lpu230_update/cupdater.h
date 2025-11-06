#pragma once

#include <memory>
#include <string>
#include <thread>
#include <mutex>
#include <atomic>
#include <filesystem>
#include <queue>
#include <fstream>

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>


#include <mp_cwait.h>
#include <mp_clog.h>

#include <chid_briage.h>
#include "HidBootManager.h"

class cupdater {

public:
	typedef std::shared_ptr<cupdater> type_ptr;
	
public:

	enum class AppState {
		s_ini // not initialized
		,s_selfile // selecting rom or bin file
		,s_selfirm // selecting fw in the rom file.
		,s_sellast // the last chance of cancel operaion of updating fw.(confirmation)
		,s_ing // updating ....... save system parameter, run boot. erase sector, write and verify, recover parameter.
		,s_com // complete processing. waits for exsiting.
		, s_max // the number of state. 
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
	cupdater(_mp::clog& log,bool b_disaplay, bool b_log);
	virtual ~cupdater();
	// Update function to be implemented by derived classes
	bool start_update_with_thread();

	/**
	* @brief Set the range of progress for the update process.
	* @param n_progress_min Minimum progress value.(greater than equal to 0)
	*	f_progress_min must be less than f_progress_max.
	* @param n_progress_max Maximum progress value.
	*/
	void set_range_of_progress(int n_progress_min, int n_progress_max);

	void set_pos_of_progress(int n_progress_pos);

	int get_pos_of_progress() const;

	void ui_main_loop();

	void non_ui_main_loop();

	/**
	* @brief as like initial dialog
	*/
	bool initial_update();

private:
	/**
	* @brief processing message and processing counter
	* @param n_msg - positive : normal processing counter. negative : for repaint(ignore this message)
	* @param s_msg - message of ing statues.
	* 
	*/
	void _queueed_message_process(int n_msg,const std::string& s_msg);

	void _updates_thread_function();//thread main

	bool _updates_sub_thread_backup_system_param(int &n_step);
	bool _updates_sub_thread_run_bootloader(int& n_step);

	/**
	* @return first - true : success plugout lpu23x.
	* 
	*	second - true : bootloader plugin detected also!.( if this is true, no need _updates_sub_thread_wait_plugin_bootloader() )
	*/
	std::pair<bool,bool> _updates_sub_thread_wait_plugout_lpu23x(int& n_step);
	bool _updates_sub_thread_wait_plugin_bootloader(int& n_step);

	/**
	* @brief open bootloader and get sector information.
	* 
	* @return true : open & getting success.
	*/
	bool _updates_sub_thread_setup_bootloader(int& n_step);

	bool _updates_sub_thread_erase_sector(int& n_step);
	
	_mp::type_pair_bool_result_bool_complete _updates_sub_thread_read_one_sector_from_file(
		int& n_step
		, bool b_first_read
		,_mp::type_v_buffer& v_out_sector
		, int& n_out_zero_base_sector_number
	);
	bool _updates_sub_thread_write_one_sector(
		int& n_step
		,const _mp::type_v_buffer& v_sector
		, int n_zero_base_sector_number
		, std::ofstream& opened_debug_file // only for debugging
	);
	bool _updates_sub_thread_run_app(int& n_step);

	bool _updates_sub_thread_wait_plugout_bootloader(int& n_step);
	std::pair<bool, _mp::clibhid_dev_info> _updates_sub_thread_wait_plugin_lpu23x(int& n_step);

	bool _updates_sub_thread_recover_system_param(int& n_step, const _mp::clibhid_dev_info &dev_info_after_update);

	bool _updates_sub_thread_change_interface_after_update(int& n_step, const _mp::clibhid_dev_info& dev_info_after_update);

	bool _updates_sub_thread_set_iso_mode_after_update(int& n_step, const _mp::clibhid_dev_info& dev_info_after_update);

	std::shared_ptr<ftxui::Component> _create_sub_ui0_ini();
	std::shared_ptr<ftxui::Component> _create_sub_ui1_select_file();
	std::shared_ptr<ftxui::Component> _create_sub_ui2_select_firmware();

	/**
	* @brief create last confirm dialog UI(firmware compatibility OK)
	*/
	std::shared_ptr<ftxui::Component> _create_sub_ui3_last_confirm();

	/**
	* @brief create last confirm dialog UI(firmware compatibility noknown)
	*/
	std::shared_ptr<ftxui::Component> _create_sub_ui3_last_warning();
	std::shared_ptr<ftxui::Component> _create_sub_ui4_updating();
	std::shared_ptr<ftxui::Component> _create_sub_ui5_complete();

	/**
	* @brief push message with thread safety
	* @param n_in_data : int type pushed data. -1 : no data. for repaint only.
	* @param s_in_msg : pushed string
	*/
	void _push_message(int n_in_data = -1,const std::string& s_in_msg = std::string());

	/**
	* @brief pop message with thread safety
	* @param n_out_data : poped ,int
	* @param s_out_msg : poped string
	* @param b_remove_after_pop  true : if a item is poped,remove item from q.
	* @return true : pop OK. false : none item in the q.
	*/
	bool _pop_message(int &n_out_data, std::string& s_out_msg,bool b_remove_after_pop = true);

	std::vector<std::filesystem::path> _find_rom_files();

	/**
	* @brief cheks tagrget device validation.
	* 
	* @return target device. if return is empty, error
	*/
	std::string _check_target_device_path_in_initial();

	/**
	* @brief cheks tagrget file validation.
	* 
	*	cshare::get_instance().set_x() isn't used. 
	*
	* @return first - target file path. if return is empty, error.
	* 
	*	second - true : rom type, false : raw(bin or ..... )
	*/
	std::pair<std::string,bool> _check_target_file_path_in_initial();


	/**
	* @brief cheks fw index of tagrget file validation.
	* 
	* @param s_target_rom_file - valid firmware file.(rom or raw)
	* 
	* @return first - total fw in the selected file.( if the selected firmware isn't rom, this value is 1 and second is -1 )
	* 
	*	second - updatable fw index of the selected rom file.
	*/
	std::pair<int, int> _check_target_fw_of_selected_rom_in_initial();

private:
	bool m_b_ini;

	CHidBootManager* m_p_mgmt;
	chid_briage::type_ptr m_ptr_hid_api_briage;

	std::atomic<cupdater::AppState> m_state;

	std::string m_s_message_in_ing_state;
	int m_n_progress_cur, m_n_progress_min, m_n_progress_max;

	ftxui::ScreenInteractive m_screen;

	std::shared_ptr<ftxui::Component> m_ptr_update_button; // update start button
	std::shared_ptr<ftxui::Component> m_ptr_update_exit_button; // update exit button

	std::shared_ptr<ftxui::Component> m_ptr_file_menu; // list UI in file selection dialog
	std::shared_ptr<ftxui::Component> m_ptr_select_file_button; // select button in file selection dialog
	std::shared_ptr<ftxui::Component> m_ptr_cancel_file_button; // cancel button in file selection dialog

	std::shared_ptr<ftxui::Component> m_ptr_fw_menu; // list UI by m_v_firmware_list
	std::shared_ptr<ftxui::Component> m_ptr_ok_fw_button; // ok(selection) button of firmware select dialog.
	std::shared_ptr<ftxui::Component> m_ptr_cancel_fw_button; // cancel button of firmware select dialog.

	std::vector<std::string> m_v_firmware_list_for_ui; //firmware list in the selected rom file.
	int m_n_selected_fw_for_ui;

	std::shared_ptr<ftxui::Component> m_ptr_ok_confirm_button;
	std::shared_ptr<ftxui::Component> m_ptr_cancel_confirm_button;

	std::shared_ptr<ftxui::Component> m_ptr_ok_warning_button;
	std::shared_ptr<ftxui::Component> m_ptr_cancel_warning_button;

	std::shared_ptr<ftxui::Component> m_ptr_exit_button; //exit button in complete dialog
	std::shared_ptr<ftxui::Component> m_ptr_exit_container;

	// Tab container for switching screens
	int m_n_tab_index;  // 0: s_ini, 1: s_selfile, 2: s_selfirm, 3: s_sellast, 4: s_ing, 5: s_scom
	std::vector<ftxui::Component> m_v_tabs;
	std::shared_ptr<ftxui::Component> m_ptr_tab_container;

	// Root with title
	std::shared_ptr<ftxui::ComponentBase> m_ptr_root;
	std::shared_ptr<ftxui::ComponentBase> m_ptr_final_root;// Event handler for progress animation

	/**
	* @brief Update the application state based on the given event.
	* @param event The event that triggers the state update.
	* @param s_rom_or_bin_file rom or bin file abs full path.
	*   if s_rom_or_bin_file isn't empty, s_rom_or_bin_file file ,must exist.
	* @return 1'st - true : state is changed, false : state is not changed.
	* @return 2'nd - The new application state after processing the event.
	* @return 3'th - message to be displayed to the user (empty if no message).
	*/
	std::tuple<bool,cupdater::AppState,std::string> _update_state(cupdater::AppEvent event,const std::string & s_rom_or_bin_file = std::string());

	_mp::clog &m_log_ref;
	//////////////////////////////////
	// UI option
	bool m_b_display; //CUI enable or disable
	bool m_b_log; //log file generation

	// Mutex for thread safety
	std::mutex m_mutex;
	std::queue<std::pair<int,std::string>> m_q_messages; // Queue for messages from the update thread

	_mp::cwait m_wait; // Wait object for synchronization
	int m_n_kill_signal; // Signal to stop the updater
	std::atomic_bool m_b_is_running; // Atomic boolean to check if the updater is running
	std::shared_ptr<std::thread> m_ptr_thread_update; // Thread for update process
	

private:// don't call these functions
	cupdater() = delete;
	cupdater(const cupdater&) = delete; // Disable copy constructor
	cupdater& operator=(const cupdater&) = delete; // Disable assignment operator
};