#pragma once

#include <memory>
#include <string>
#include <thread>
#include <mutex>
#include <atomic>
#include <filesystem>

#include <mp_cversion.h>
#include <mp_cwait.h>

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

class cupdater {

public:
	typedef std::shared_ptr<cupdater> type_ptr;
	typedef _mp::cversion<unsigned char>	type_version;
public:
	enum class AppState { Init, FileBrowser, FirmwareSelect, UpdateProgress };

public:
	cupdater();
	virtual ~cupdater();
	// Update function to be implemented by derived classes
	bool start_update();
	// Function to check if an update is available
	bool is_update_available() const;

	//setters for rom file and target device information
	cupdater& set_rom_file(const std::wstring& s_abs_full_path_of_rom);
	cupdater& set_index_of_fw_in_rom(int n_index_of_fw_in_rom);
	cupdater& set_update_condition_of_fw(const std::wstring& s_update_condition_of_fw);

	cupdater& set_device_path(const std::wstring& s_device_path);
	cupdater& set_device_version(const std::wstring& s_device_version);

	// Getters for rom file and target device information
	const std::wstring& get_rom_file() const;
	int get_index_of_fw_in_rom() const;
	const std::wstring& get_update_condition_of_fw() const;
	bool is_fw_file_in_rom_format() const;

	const std::wstring& get_device_path() const;
	const type_version& get_device_version() const;
	//
	void update_files_list();
	void main_loop();

private:
	void _updates_thread_function();

private:
	cupdater::AppState m_state;
	bool m_b_show_change_rom_modal;
	bool m_b_show_compatibility_modal;

	int m_n_selected_file_in_v_files_in_current_dir;
	std::vector<std::string> m_v_files_in_current_dir;
	std::filesystem::path m_current_dir;

	float m_f_progress;
	std::atomic_bool m_b_update_done;
	std::atomic_bool m_b_started_update;

	ftxui::ScreenInteractive m_screen;

	std::shared_ptr<ftxui::Component> m_ptr_update_button;
	std::shared_ptr<ftxui::Component> m_ptr_init_container;
	std::shared_ptr<ftxui::Component> m_ptr_init_component;
	std::shared_ptr<ftxui::Component> m_ptr_file_menu;
	std::shared_ptr<ftxui::Component> m_ptr_select_file_button;
	std::shared_ptr<ftxui::Component> m_ptr_cancel_file_button;
	std::shared_ptr<ftxui::Component> m_ptr_file_container;
	std::shared_ptr<ftxui::Component> m_ptr_file_component;

	int m_n_selected_fw;
	std::vector<std::string> m_v_firmware_list;
	std::shared_ptr<ftxui::Component> m_ptr_fw_menu;
	std::shared_ptr<ftxui::Component> m_ptr_ok_fw_button;
	std::shared_ptr<ftxui::Component> m_ptr_cancel_fw_button;
	std::shared_ptr<ftxui::Component> m_ptr_fw_container;
	std::shared_ptr<ftxui::Component> m_ptr_fw_component;
	
	std::shared_ptr<ftxui::Component> m_ptr_exit_button;
	std::shared_ptr<ftxui::Component> m_ptr_progress_component;

	// Tab container for switching screens
	int m_n_tab_index;  // 0: Init, 1: FileBrowser, 2: FirmwareSelect, 3: UpdateProgress
	std::vector<ftxui::Component> m_v_tabs;
	std::shared_ptr<ftxui::Component> m_ptr_tab_container;

	void _update_tab_index();
	//
	// Change ROM modal
	std::shared_ptr<ftxui::Component> m_ptr_yes_change_button;
	std::shared_ptr<ftxui::Component> m_ptr_no_change_button;
	std::shared_ptr<ftxui::Component> m_ptr_change_modal_container;
	std::shared_ptr<ftxui::Component> m_ptr_change_modal_component;

	// Compatibility modal
	std::shared_ptr<ftxui::Component> m_ptr_yes_compat_button;
	std::shared_ptr<ftxui::Component> m_ptr_no_compat_button;
	std::shared_ptr<ftxui::Component> m_ptr_compat_modal_container;
	std::shared_ptr<ftxui::Component> m_ptr_compat_modal_component;

	// Root with title
	std::shared_ptr<ftxui::Component> m_ptr_root;
	std::shared_ptr<ftxui::Component> m_ptr_root_with_modals;
	std::shared_ptr<ftxui::Component> m_ptr_final_root;// Event handler for progress animation


	//////////////////////////////////
	//rom file section
	std::wstring m_s_abs_full_path_of_rom;
	int m_n_index_of_fw_in_rom;
	std::wstring m_s_update_condition_of_fw;
	bool m_b_fw_file_is_rom_format;

	//target device section
	std::wstring m_s_device_path;
	cupdater::type_version m_version_of_device;

	// Mutex for thread safety
	std::mutex m_mutex;
	_mp::cwait m_wait; // Wait object for synchronization
	int m_n_kill_signal; // Signal to stop the updater
	std::atomic_bool m_b_is_running; // Atomic boolean to check if the updater is running
	std::shared_ptr<std::thread> m_ptr_thread_update; // Thread for update process

private:// don't call these functions
	cupdater(const cupdater&) = delete; // Disable copy constructor
	cupdater& operator=(const cupdater&) = delete; // Disable assignment operator
};