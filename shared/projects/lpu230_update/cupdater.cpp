#include "cupdater.h"
#include <algorithm>

#ifdef _WIN32
#include <atltrace.h>
#endif

cupdater::cupdater() : m_screen(ftxui::ScreenInteractive::Fullscreen())
{
	m_s_abs_full_path_of_rom.clear();
	m_n_index_of_fw_in_rom = -1;
	m_s_update_condition_of_fw.clear();
	m_b_fw_file_is_rom_format = false;
	m_s_device_path.clear();//
	m_n_kill_signal = m_wait.generate_new_event();
	//
	m_b_started_update = false;
	m_state = cupdater::AppState::Init;
	m_b_show_change_rom_modal = false;
	m_b_show_compatibility_modal = false;

	m_current_dir = std::filesystem::current_path();
	if(!m_s_abs_full_path_of_rom.empty()) {
		m_current_dir = std::filesystem::path(m_s_abs_full_path_of_rom).parent_path();
	}

	m_n_selected_file_in_v_files_in_current_dir = 0;

	update_files_list();
	if(m_v_files_in_current_dir.empty()) {
		m_n_selected_file_in_v_files_in_current_dir = -1;
	}

	m_f_progress = 0.0f;
	m_b_update_done = false;

	m_ptr_update_button = std::make_shared<ftxui::Component>(ftxui::Button("Update", [&]() {
		m_state = cupdater::AppState::FileBrowser;
		m_b_show_change_rom_modal = true;
		}));

	m_ptr_init_container = std::make_shared<ftxui::Component>(ftxui::Container::Vertical({ *m_ptr_update_button }));
	m_ptr_init_component = std::make_shared<ftxui::Component>(ftxui::Renderer(*m_ptr_init_container, [&]() {
		return ftxui::vbox({
				   ftxui::text("Initialization complete."),
				   (*m_ptr_update_button)->Render(),
			}) |
			ftxui::border;
		})
	);

	m_ptr_file_menu = std::make_shared<ftxui::Component>(ftxui::Menu(&m_v_files_in_current_dir, &m_n_selected_file_in_v_files_in_current_dir));

	m_ptr_select_file_button = std::make_shared<ftxui::Component>(ftxui::Button("Select", [&]() {
		if (m_v_files_in_current_dir.empty()) {
			return;
		}
		std::string selected = m_v_files_in_current_dir[m_n_selected_file_in_v_files_in_current_dir];
		std::filesystem::path selected_path = m_current_dir / selected;
		if (selected_path == "../") {
			m_current_dir = m_current_dir.parent_path();
			update_files_list();
			return;
		}
		if (std::filesystem::is_directory(selected_path)) {
			m_current_dir = std::filesystem::path(selected_path);
			update_files_list();
			return;
		}
		if (std::filesystem::path(selected_path).extension() == ".rom") {
			m_s_abs_full_path_of_rom = selected_path.wstring();//SEE
			m_state = cupdater::AppState::FirmwareSelect;
		}
		}));
	//
	m_ptr_cancel_file_button = std::make_shared<ftxui::Component>(ftxui::Button("Cancel", [&]() {
		m_state = cupdater::AppState::Init;
		}));

	m_ptr_file_container = std::make_shared<ftxui::Component>(ftxui::Container::Vertical(
		{ *m_ptr_file_menu, ftxui::Container::Horizontal({ *m_ptr_select_file_button, *m_ptr_cancel_file_button }) }));

	m_ptr_file_component = std::make_shared<ftxui::Component>(ftxui::Renderer(*m_ptr_file_container, [&]() {
		return ftxui::vbox({
				   ftxui::text("Select rom file"),
				   ftxui::text("Current directory: " + m_current_dir.string()),
				   (*m_ptr_file_menu)->Render() | ftxui::frame | ftxui::flex,
				   ftxui::hbox({(*m_ptr_select_file_button)->Render(), (*m_ptr_cancel_file_button)->Render()}),
			}) |
			ftxui::border;
		})
	);

	// Firmware select screen
	std::vector<std::string> firmware_list = { "Europa" };  // Hardcoded; in real scenario, parse from .rom file
	m_v_firmware_list = firmware_list;

	m_n_selected_fw = 0;
	m_ptr_fw_menu = std::make_shared<ftxui::Component>(ftxui::Menu(&m_v_firmware_list, &m_n_selected_fw));
	m_ptr_ok_fw_button = std::make_shared<ftxui::Component>(ftxui::Button("OK", [&]() {
		m_state = cupdater::AppState::UpdateProgress;
		m_b_show_compatibility_modal = true;
		//m_b_started_update = true;
		}));
	m_ptr_cancel_fw_button = std::make_shared<ftxui::Component>(ftxui::Button("Cancel", [&]() {
		m_state = cupdater::AppState::Init;
		}));
	m_ptr_fw_container = std::make_shared<ftxui::Component>(ftxui::Container::Vertical(
		{ *m_ptr_fw_menu, ftxui::Container::Horizontal({ *m_ptr_ok_fw_button, *m_ptr_cancel_fw_button }) }));
	m_ptr_fw_component = std::make_shared<ftxui::Component>(ftxui::Renderer(*m_ptr_fw_container, [&]() {
		return ftxui::vbox({
				   ftxui::text("Select firmware"),
				   (*m_ptr_fw_menu)->Render(),
				   ftxui::hbox({(*m_ptr_ok_fw_button)->Render(), (*m_ptr_cancel_fw_button)->Render()}),
			}) |
			ftxui::border;
		})
	);
	//
	//Update progress screen
	m_ptr_exit_button = std::make_shared<ftxui::Component>(ftxui::Button("Exit", [&]() {
		m_b_is_running = false;   // worker thread 종료 신호
		m_screen.ExitLoopClosure()();
		}));
	m_ptr_progress_component = std::make_shared<ftxui::Component>(ftxui::Renderer([&]() {
		std::string status;
		if (!m_b_update_done) {
			status = std::to_string(static_cast<int>(m_f_progress * 100)) + " %";
			return ftxui::vbox({
					   ftxui::text("Update"),
					   ftxui::gauge(m_f_progress) | ftxui::color(ftxui::Color::Blue),
					   ftxui::text(status),
					   ftxui::text(m_s_abs_full_path_of_rom),
					   ftxui::emptyElement()
				}) |
				ftxui::border;
		}
		else {
			status = "The parameter has been recovered. all complete.";
			return ftxui::vbox({
					   ftxui::text("Update"),
					   ftxui::gauge(m_f_progress) | ftxui::color(ftxui::Color::Blue),
					   ftxui::text(status),
					   ftxui::text(m_s_abs_full_path_of_rom),
					   (*m_ptr_exit_button)->Render()
				}) |
				ftxui::border;
		}
		})
	);
	//
	m_n_tab_index = 0; // 0: Init, 1: FileBrowser, 2: FirmwareSelect, 3: UpdateProgress
	m_v_tabs = {
		*m_ptr_init_component,
		*m_ptr_file_component,
		*m_ptr_fw_component,
		*m_ptr_progress_component
	};

	m_ptr_tab_container = std::make_shared<ftxui::Component>(ftxui::Container::Tab(m_v_tabs, &m_n_tab_index));

	//  Change ROM modal
	m_ptr_yes_change_button = std::make_shared<ftxui::Component>(ftxui::Button("Yes", [&]() {
		m_b_show_change_rom_modal = false;
		m_state = cupdater::AppState::FileBrowser;
		_update_tab_index();
		}));
	m_ptr_no_change_button = std::make_shared<ftxui::Component>(ftxui::Button("No", [&]() {
		m_b_show_change_rom_modal = false;
		m_state = cupdater::AppState::FirmwareSelect;
		_update_tab_index();
		}));
	m_ptr_change_modal_container = std::make_shared<ftxui::Component>(ftxui::Container::Horizontal({ *m_ptr_yes_change_button, *m_ptr_no_change_button }));
	m_ptr_change_modal_component = std::make_shared<ftxui::Component>(ftxui::Renderer(*m_ptr_change_modal_container, [&]() {
		return ftxui::vbox({
				   ftxui::text("The rom file is selected already."),
				   ftxui::text("Do you want to choose another rom file?"),
				   ftxui::hbox({(*m_ptr_yes_change_button)->Render(), (*m_ptr_no_change_button)->Render()}),
			}) |
			ftxui::border;
		})
	);

	// Compatibility modal
	m_ptr_yes_compat_button = std::make_shared<ftxui::Component>(ftxui::Button("Yes", [&]() {
		m_b_show_compatibility_modal = false;
		m_state = cupdater::AppState::UpdateProgress;
		_update_tab_index();
		m_f_progress = 0.0f;
		m_b_update_done = false;
		m_b_started_update = true;
		//m_screen.PostEvent(ftxui::Event::Custom); // Start progress update
		}));
	m_ptr_no_compat_button = std::make_shared<ftxui::Component>(ftxui::Button("No", [&]() {
		m_b_show_compatibility_modal = false;
		m_state = cupdater::AppState::Init;
		_update_tab_index();
		}));
	m_ptr_compat_modal_container = std::make_shared<ftxui::Component>(ftxui::Container::Horizontal({ *m_ptr_yes_compat_button, *m_ptr_no_compat_button }));
	m_ptr_compat_modal_component = std::make_shared<ftxui::Component>(ftxui::Renderer(*m_ptr_compat_modal_container, [&]() {
		return ftxui::vbox({
				   ftxui::text("Notice"),
				   ftxui::text("Compatibility between the selected firmware and the target"),
				   ftxui::text("device cannot be confirmed."),
				   ftxui::text("Would you like to proceed with the update?"),
				   ftxui::hbox({(*m_ptr_yes_compat_button)->Render(), (*m_ptr_no_compat_button)->Render()}),
			}) |
			ftxui::border;
		})
	);

	// Root with title
	std::wstring version = L"1.2.0.0";
	m_version_of_device.set_by_string(version);

	m_ptr_root = std::make_shared<ftxui::Component>(ftxui::Renderer(*m_ptr_tab_container, [&]() {
		return ftxui::vbox({
				   ftxui::text("lpu230_update " + 
					   _mp::cstring::get_mcsc_from_unicode(m_version_of_device.get_by_string() )
					   + " : " 
					   + _mp::cstring::get_mcsc_from_unicode(m_s_abs_full_path_of_rom)
				   ) | ftxui::border,
				   (*m_ptr_tab_container)->Render() | ftxui::flex,
			}) |
			ftxui::border;
		})
	);

	m_ptr_root_with_modals = std::make_shared<ftxui::Component>();
	(*m_ptr_root_with_modals) = 
		(*m_ptr_root) | 
		ftxui::Modal(*m_ptr_change_modal_component, &m_b_show_change_rom_modal) |
		ftxui::Modal(*m_ptr_compat_modal_component, &m_b_show_compatibility_modal);

	m_ptr_final_root = std::make_shared<ftxui::Component>(ftxui::CatchEvent(*m_ptr_root_with_modals, [&](ftxui::Event event) {
		if (event == ftxui::Event::Custom && m_state == cupdater::AppState::UpdateProgress && !m_b_update_done) {
			m_f_progress += 0.001f;  // Increment by 10% each time
			//m_screen.PostEvent(ftxui::Event::Custom);  // Schedule next update
			if (m_f_progress >= 1.0f) {
				m_f_progress = 1.0f;
				m_b_update_done = true;
			}
#ifdef _WIN32
			ATLTRACE(L"Progress update: %f\n", m_f_progress);
#endif
		}
		return false;  // Allow other events to propagate
		})
	);
}
void cupdater::_update_tab_index()
{
	switch (m_state) {
	case cupdater::AppState::Init:
		m_n_tab_index = 0;
		break;
	case cupdater::AppState::FileBrowser:
		m_n_tab_index = 1;
		break;
	case cupdater::AppState::FirmwareSelect:
		m_n_tab_index = 2;
		break;
	case cupdater::AppState::UpdateProgress:
		m_n_tab_index = 3;
		break;
	default:
		m_n_tab_index = 0; // Default to Init state
		break;
	}
}

cupdater::~cupdater()
{
	if( m_ptr_thread_update ) {
		m_b_is_running = false;
		m_wait.set(m_n_kill_signal);
		if (m_ptr_thread_update->joinable()) {
			m_ptr_thread_update->join();
		}
	}
	m_ptr_final_root.reset();
	m_ptr_root_with_modals.reset();
	m_ptr_root.reset();
	m_ptr_compat_modal_component.reset();
	m_ptr_compat_modal_container.reset();
	m_ptr_no_compat_button.reset();
	m_ptr_yes_compat_button.reset();
	m_ptr_change_modal_component.reset();
	m_ptr_change_modal_container.reset();
	m_ptr_no_change_button.reset();
	m_ptr_yes_change_button.reset();
	m_ptr_tab_container.reset();
	m_ptr_progress_component.reset();
	m_ptr_exit_button.reset();
	m_ptr_fw_component.reset();
	m_ptr_fw_container.reset();
	m_ptr_cancel_fw_button.reset();
	m_ptr_ok_fw_button.reset();
	m_ptr_fw_menu.reset();
	m_ptr_file_component.reset();
	m_ptr_file_container.reset();
	m_ptr_cancel_file_button.reset();
	m_ptr_select_file_button.reset();
	m_ptr_file_menu.reset();
	m_ptr_init_component.reset();
	m_ptr_init_container.reset();
	m_ptr_update_button.reset();
}

// Update function to be implemented by derived classes
bool cupdater::start_update()
{
	bool b_result(false);

	do {
		std::lock_guard<std::mutex> lock(m_mutex); // Lock the mutex for thread safety
		if (m_ptr_thread_update) {
			continue;
		}
		 
		m_b_is_running = true; // Set running state
		m_ptr_thread_update = std::make_shared<std::thread>(&cupdater::_updates_thread_function, this);

	} while (false); 
	return b_result;
}

// Function to check if an update is available
bool cupdater::is_update_available() const
{
	bool b_result(false);
	do {
		if (m_s_abs_full_path_of_rom.empty()) {
			continue; // No ROM file set, cannot check for updates
		}
		if(m_n_index_of_fw_in_rom < 0 && m_b_fw_file_is_rom_format) {
			continue; // Invalid index for firmware in ROM
		}

		b_result = true; // Assume update is available if conditions are met
	} while (false);
	return b_result;
}

cupdater& cupdater::set_rom_file(const std::wstring& s_abs_full_path_of_rom)
{
	m_s_abs_full_path_of_rom = s_abs_full_path_of_rom;
	return *this;
}

cupdater& cupdater::set_index_of_fw_in_rom(int n_index_of_fw_in_rom)
{
	m_n_index_of_fw_in_rom = n_index_of_fw_in_rom;
	return *this;
}

cupdater& cupdater::set_update_condition_of_fw(const std::wstring& s_update_condition_of_fw)
{
	m_s_update_condition_of_fw = s_update_condition_of_fw;
	return *this;
}

cupdater& cupdater::set_device_path(const std::wstring& s_device_path)
{
	m_s_device_path = s_device_path;
	return *this;
}

cupdater& cupdater::set_device_version(const std::wstring& s_device_version)
{
	m_version_of_device.set_by_string(s_device_version);
	return *this;
}

const std::wstring& cupdater::get_rom_file() const
{ 
	return m_s_abs_full_path_of_rom;

}
int cupdater::get_index_of_fw_in_rom() const
{
	return m_n_index_of_fw_in_rom;
}
const std::wstring& cupdater::get_update_condition_of_fw() const
{ 
	return m_s_update_condition_of_fw;
}
bool cupdater::is_fw_file_in_rom_format() const
{ 
	return m_b_fw_file_is_rom_format;
}

const std::wstring& cupdater::get_device_path() const
{ 
	return m_s_device_path;
}
const cupdater::type_version& cupdater::get_device_version() const
{ 
	return m_version_of_device;
}

void cupdater::update_files_list()
{
	m_v_files_in_current_dir.clear();
	m_v_files_in_current_dir.push_back("../");
	for (const auto& entry : std::filesystem::directory_iterator(m_current_dir)) {
		std::string name = entry.path().filename().string();
		if (entry.is_directory()) {
			name += "/";
		}
		m_v_files_in_current_dir.push_back(name);
	}
	std::sort(m_v_files_in_current_dir.begin(), m_v_files_in_current_dir.end());

}


void cupdater::main_loop()
{
	m_screen.Loop(*m_ptr_final_root);
}

void cupdater::_updates_thread_function()
{
	while (m_b_is_running){
		std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Sleep to avoid busy waiting
		if (!m_b_started_update) {
			continue;
		}
		if (m_b_update_done) {
			continue; // If update is done, skip further processing
		}

		if (!m_b_is_running) break; // 종료 직전 체크

		try {
			m_screen.PostEvent(ftxui::Event::Custom);
		}
		catch (...) {
			break; // ScreenInteractive 파괴 직후 방어
		}
	}//running
}
