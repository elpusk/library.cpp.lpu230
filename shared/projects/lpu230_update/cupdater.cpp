#include "cupdater.h"
#include <algorithm>

#include <ftxui/component/loop.hpp>

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
	m_state = cupdater::AppState::s_ini;

	m_current_dir = std::filesystem::current_path();
	if(!m_s_abs_full_path_of_rom.empty()) {
		m_current_dir = std::filesystem::path(m_s_abs_full_path_of_rom).parent_path();
	}

	update_files_list();
	m_n_selected_file_in_v_files_in_current_dir = 0;
	if(m_v_files_in_current_dir.empty()) {
		m_n_selected_file_in_v_files_in_current_dir = -1;
	}

	m_f_progress = 0.0f;

	m_n_tab_index = 0; // 0: s_ini, 1: s_selfile, 2: s_selfirm, 3: s_sellast, 4: s_ing, 5: s_scom
	m_v_tabs = {
		*_create_sub_ui0_ini(),
		*_create_sub_ui1_select_file(),
		*_create_sub_ui2_select_firmware(),
		*_create_sub_ui3_last_confirm(),
		*_create_sub_ui4_updating(),
		*_create_sub_ui5_complete()
	};

	m_ptr_tab_container = std::make_shared<ftxui::Component>(ftxui::Container::Tab(m_v_tabs, &m_n_tab_index));

	// Root with title
	std::wstring version = L"1.2.0.0";
	m_version_of_device.set_by_string(version);

	m_ptr_root = std::make_shared<ftxui::Component>(ftxui::Renderer(*m_ptr_tab_container, [&]() {
		return ftxui::vbox({
				   ftxui::text("lpu230_update " + 
					   _mp::cstring::get_mcsc_from_unicode(m_version_of_device.get_by_string() )
					   + " : " 
					   + m_s_abs_full_path_of_rom
				   ) | ftxui::border,
				   (*m_ptr_tab_container)->Render() | ftxui::flex,
			}) |
			ftxui::border;
		})
	);

	m_ptr_final_root = std::make_shared<ftxui::Component>(ftxui::CatchEvent(*m_ptr_root, [&](ftxui::Event event) {
		if (event == ftxui::Event::Custom && m_state == cupdater::AppState::s_ing) {
			m_f_progress += 0.1f;  // Increment by 10% each time
			if (m_f_progress >= 1.0f) {
				m_f_progress = 1.0f;
				_update_state(cupdater::AppEvent::e_ulstep_s);
#ifdef _WIN32
				ATLTRACE(L"Progress update complete: %f\n", m_f_progress);
#endif
				return false;
			}
			else {
				_update_state(cupdater::AppEvent::e_ustep_s);
#ifdef _WIN32
				ATLTRACE(L"Progress update : %f\n", m_f_progress);
#endif
			}
		}
		return false;  // Allow other events to propagate
		})
	);
}

std::shared_ptr<ftxui::Component> cupdater::_create_sub_ui0_ini()
{
	m_ptr_update_button = std::make_shared<ftxui::Component>(ftxui::Button("Update", [&]() {
		std::string s(m_s_abs_full_path_of_rom);
		_update_state(cupdater::AppEvent::e_start, s);
		}));

	ftxui::Component container(ftxui::Container::Vertical({ *m_ptr_update_button }));
	std::shared_ptr<ftxui::Component> ptr_component = std::make_shared<ftxui::Component>(ftxui::Renderer(container, [&]() {
		return ftxui::vbox({
				   ftxui::text("Initialization complete."),
				   (*m_ptr_update_button)->Render(),
			}) |
			ftxui::border;
		})
	);
	return ptr_component;
}

std::shared_ptr<ftxui::Component> cupdater::_create_sub_ui1_select_file()
{
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
			m_s_abs_full_path_of_rom = selected_path.string();//SEE
			_update_state(cupdater::AppEvent::e_sfile_or, selected_path.string());
		}
		else {
			_update_state(cupdater::AppEvent::e_sfile_ob, selected_path.string());
		}
		}));
	//
	m_ptr_cancel_file_button = std::make_shared<ftxui::Component>(ftxui::Button("Cancel", [&]() {
		_update_state(cupdater::AppEvent::e_sfile_c);
		}));
	ftxui::Component container(ftxui::Container::Vertical(
		{ *m_ptr_file_menu, ftxui::Container::Horizontal({ *m_ptr_select_file_button, *m_ptr_cancel_file_button }) }));

	std::shared_ptr<ftxui::Component> ptr_component = std::make_shared<ftxui::Component>(ftxui::Renderer(container, [&]() {
		return ftxui::vbox({
				   ftxui::text("Select rom file"),
				   ftxui::text("Current directory: " + m_current_dir.string()),
				   (*m_ptr_file_menu)->Render() | ftxui::frame | ftxui::flex,
				   ftxui::hbox({(*m_ptr_select_file_button)->Render() | ftxui::flex, (*m_ptr_cancel_file_button)->Render() | ftxui::flex}),
			}) |
			ftxui::border;
		})
	);
	return ptr_component;

}
std::shared_ptr<ftxui::Component> cupdater::_create_sub_ui2_select_firmware()
{
	std::vector<std::string> firmware_list = { "Europa" };  // Hardcoded; in real scenario, parse from .rom file
	m_v_firmware_list = firmware_list;

	m_n_selected_fw = 0;
	m_ptr_fw_menu = std::make_shared<ftxui::Component>(ftxui::Menu(&m_v_firmware_list, &m_n_selected_fw));
	m_ptr_ok_fw_button = std::make_shared<ftxui::Component>(ftxui::Button("OK", [&]() {
		_update_state(cupdater::AppEvent::e_sfirm_o);
		}));
	m_ptr_cancel_fw_button = std::make_shared<ftxui::Component>(ftxui::Button("Cancel", [&]() {
		_update_state(cupdater::AppEvent::e_sfirm_c);
		}));

	ftxui::Component container(ftxui::Container::Vertical(
		{ *m_ptr_fw_menu, ftxui::Container::Horizontal({ *m_ptr_ok_fw_button, *m_ptr_cancel_fw_button }) }));

	std::shared_ptr<ftxui::Component> ptr_component = std::make_shared<ftxui::Component>(ftxui::Renderer(container, [&]() {
		return ftxui::vbox({
				   ftxui::text("Select firmware"),
				   (*m_ptr_fw_menu)->Render(),
				   ftxui::hbox({(*m_ptr_ok_fw_button)->Render() | ftxui::flex , (*m_ptr_cancel_fw_button)->Render() | ftxui::flex}),
			}) |
			ftxui::border;
		})
	);

	return ptr_component;
}
std::shared_ptr<ftxui::Component> cupdater::_create_sub_ui3_last_confirm()
{
	m_ptr_ok_confirm_button = std::make_shared<ftxui::Component>(ftxui::Button("OK", [&]() {
		_update_state(cupdater::AppEvent::e_slast_o);
		}));
	m_ptr_cancel_confirm_button = std::make_shared<ftxui::Component>(ftxui::Button("Cancel", [&]() {
		_update_state(cupdater::AppEvent::e_slast_c);
		}));

	ftxui::Component container(	ftxui::Container::Horizontal({ *m_ptr_ok_confirm_button, *m_ptr_cancel_confirm_button }));
	std::shared_ptr<ftxui::Component> ptr_component = std::make_shared<ftxui::Component>(ftxui::Renderer(container, [&]() {
		return ftxui::vbox({
				   ftxui::text("Notice"),
				   ftxui::text("Compatibility between the selected firmware and the target"),
				   ftxui::text("device cannot be confirmed."),
				   ftxui::text("Would you like to proceed with the update?"),
				   ftxui::hbox({(*m_ptr_ok_confirm_button)->Render() | ftxui::flex, (*m_ptr_cancel_confirm_button)->Render() | ftxui::flex}),
			}) |
			ftxui::border;
		})
	);

	return ptr_component;
}
std::shared_ptr<ftxui::Component> cupdater::_create_sub_ui4_updating()
{
	std::shared_ptr<ftxui::Component> ptr_component = std::make_shared<ftxui::Component>(ftxui::Renderer([&]() {
		std::string status;

		status = std::to_string(static_cast<int>(m_f_progress * 100)) + " %";
		return ftxui::vbox({
					ftxui::text("Updating"),
					ftxui::gauge(m_f_progress) | ftxui::color(ftxui::Color::Blue),
					ftxui::text(status),
					ftxui::text(m_s_abs_full_path_of_rom),
					ftxui::emptyElement()
			}) |
			ftxui::border;
		})
	);

	return ptr_component;

}

/* 
아래 코드는 화면표시는 되는데, m_ptr_exit_button 의 click handler 가 호출되지 않는다.
std::shared_ptr<ftxui::Component> cupdater::_create_sub_ui5_complete()
{
	m_ptr_exit_button = std::make_shared<ftxui::Component>(ftxui::Button("Exit", [&]() {
		m_b_is_running = false;   // worker thread 종료 신호
		_update_state(cupdater::AppEvent::e_exit);
		m_screen.ExitLoopClosure()();
		}));
	std::shared_ptr<ftxui::Component> ptr_component = std::make_shared<ftxui::Component>(ftxui::Renderer([&]() {
		std::string status;
		status = "The parameter has been recovered. all complete.";
		return ftxui::vbox({
				   ftxui::text("SUCCESS"),
				   ftxui::text(status),
				   ftxui::text(m_s_abs_full_path_of_rom),
				   (*m_ptr_exit_button)->Render()
			}) |
			ftxui::border;
		})
	);

	return ptr_component;
}
*/
std::shared_ptr<ftxui::Component> cupdater::_create_sub_ui5_complete()
{
	m_ptr_exit_button = std::make_shared<ftxui::Component>(ftxui::Button("Exit", [&]() {
		//std::cout << "Exit button clicked, current state: " << get_string(m_state) << std::endl;
		m_b_is_running = false;
		_update_state(cupdater::AppEvent::e_exit);
		m_screen.ExitLoopClosure()();
		}));

	//m_ptr_exit_button->TakeFocus(); // 포커스 설정
	m_ptr_exit_container = std::make_shared<ftxui::Component>(
		ftxui::Container::Vertical({
		ftxui::Renderer([] { return ftxui::text("SUCCESS"); }),
		ftxui::Renderer([] { return ftxui::text("The parameter has been recovered. all complete."); }),
		ftxui::Renderer([this] { return ftxui::text(m_s_abs_full_path_of_rom); }),
		*m_ptr_exit_button
		})
	);

	std::shared_ptr<ftxui::Component> ptr_component = std::make_shared<ftxui::Component>(ftxui::Renderer(*m_ptr_exit_container, [&]() {
		//std::cout << "Rendering s_com UI, tab_index: " << m_n_tab_index << std::endl;
		return (*m_ptr_exit_container)->Render() | ftxui::border;
		}));
	return ptr_component;
}

std::tuple<bool, cupdater::AppState, std::string> cupdater::_update_state(cupdater::AppEvent event, const std::string& s_rom_or_bin_file /*= std::string()*/)
{
	bool b_changed(false);
	std::string s_message;
	cupdater::AppState old_state = m_state;
	cupdater::AppState new_state = m_state;

	static const std::pair<int,std::string> _state_table[(int)cupdater::AppState::s_max][(int)cupdater::AppEvent::event_max] = {
		{
			std::make_pair((int)cupdater::AppState::s_selfile,""),//std::make_pair((int)cupdater::AppState::s_selfirm,"") or std::make_pair((int)cupdater::AppState::s_sellast,"")
			std::make_pair(-1,""),
			std::make_pair(-1,""),
			std::make_pair(-1,""),
			std::make_pair(-1,""),
			std::make_pair(-1,""),
			std::make_pair(-1,""),
			std::make_pair(-1,""),
			std::make_pair(-1,""),
			std::make_pair(-1,""),
			std::make_pair(-1,""),
			std::make_pair(-1,""),
			std::make_pair(-1,"")
		}, // <- s_ini state
		{
			std::make_pair(-1,""),
			std::make_pair((int)cupdater::AppState::s_selfirm,""),
			std::make_pair((int)cupdater::AppState::s_sellast,""),
			std::make_pair((int)cupdater::AppState::s_ini,""),
			std::make_pair(-1,""),
			std::make_pair(-1,""),
			std::make_pair(-1,""),
			std::make_pair(-1,""),
			std::make_pair(-1,""),
			std::make_pair(-1,""),
			std::make_pair(-1,""),
			std::make_pair(-1,""),
			std::make_pair(-1,"")
		}, // <-s_selfile state
		{
			std::make_pair(-1,""),
			std::make_pair(-1,""),
			std::make_pair(-1,""),
			std::make_pair(-1,""),
			std::make_pair((int)cupdater::AppState::s_sellast,"get the last confirm"),
			std::make_pair((int)cupdater::AppState::s_selfile,"select a rom file"),
			std::make_pair(-1,""),
			std::make_pair(-1,""),
			std::make_pair(-1,""),
			std::make_pair(-1,""),
			std::make_pair(-1,""),
			std::make_pair(-1,""),
			std::make_pair(-1,"")
		}, // <-s_selfirm state
		{
			std::make_pair(-1,""),
			std::make_pair(-1,""),
			std::make_pair(-1,""),
			std::make_pair(-1,""),
			std::make_pair(-1,""),
			std::make_pair(-1,""),
			std::make_pair((int)cupdater::AppState::s_ing,"start the first step"),
			std::make_pair((int)cupdater::AppState::s_selfirm,""),
			std::make_pair(-1,""),
			std::make_pair(-1,""),
			std::make_pair(-1,""),
			std::make_pair(-1,""),
			std::make_pair(-1,"")
		}, // <-s_sellast state
		{
			std::make_pair(-1,""),
			std::make_pair(-1,""),
			std::make_pair(-1,""),
			std::make_pair(-1,""),
			std::make_pair(-1,""),
			std::make_pair(-1,""),
			std::make_pair(-1,""),
			std::make_pair(-1,""),
			std::make_pair((int)cupdater::AppState::s_ing,"the current step success, go the next step."),
			std::make_pair((int)cupdater::AppState::s_com,"complete with error"),
			std::make_pair((int)cupdater::AppState::s_com,"complete with success"),
			std::make_pair((int)cupdater::AppState::s_com,"complete with error at last"),
			std::make_pair(-1,"")
		}, // <-s_ing state
		{
			std::make_pair(-1,""),
			std::make_pair(-1,""),
			std::make_pair(-1,""),
			std::make_pair(-1,""),
			std::make_pair(-1,""),
			std::make_pair(-1,""),
			std::make_pair(-1,""),
			std::make_pair(-1,""),
			std::make_pair(-1,""),
			std::make_pair(-1,""),
			std::make_pair(-1,""),
			std::make_pair(-1,""),
			std::make_pair(-1,"Terminate program")
		}, // <-s_com state

	};

	std::pair<int, std::string> new_value = _state_table[(int)old_state][(int)event];

	do {
		//special case.
		if (old_state == cupdater::AppState::s_ini && event == cupdater::AppEvent::e_start) {
			
			if (!s_rom_or_bin_file.empty()) {
				std::filesystem::path fp(s_rom_or_bin_file);
				if (fp.extension() == ".rom") {
					// s_rom_or_bin_file 확장자가 rom 인 경우.	
					new_value.first = (int)cupdater::AppState::s_selfirm;
					new_value.second = std::string("select a firmware in the selected rom file");
				}
				else if (fp.extension() == ".bin") {
					// s_rom_or_bin_file 확장자가 bin 인 경우.	
					new_value.first = (int)cupdater::AppState::s_sellast;
					new_value.second = std::string("confirm the selected firmware");
				}
				else {
					continue;
				}
			}
		}

		new_state = (cupdater::AppState)new_value.first;
		s_message = new_value.second;
		if((int)new_state >= 0 ){
			// negative state is impossible case/
			if (new_state != old_state) {
				b_changed = true;
				m_state = new_state;

				switch (m_state) {
				case cupdater::AppState::s_ini:
					m_n_tab_index = 0;
					break;
				case cupdater::AppState::s_selfile:
					m_n_tab_index = 1;
					break;
				case cupdater::AppState::s_selfirm:
					m_n_tab_index = 2;
					break;
				case cupdater::AppState::s_sellast:
					m_n_tab_index = 3;
					break;
				case cupdater::AppState::s_ing:
					m_n_tab_index = 4;
					break;
				case cupdater::AppState::s_com:
					m_n_tab_index = 5;
					break;
				default:
					m_n_tab_index = 5; // Default to s_ini state
					break;
				}
			}
		}
	} while(false);

#ifdef _WIN32
	if (b_changed) {
		if (s_message.empty()) {
			ATLTRACE("_update_state(%s)->[%d](SC,%s,)\n", cupdater::get_string(event).c_str(), m_n_tab_index, cupdater::get_string(new_state).c_str());
		}
		else {
			ATLTRACE("_update_state(%s)->[%d](SC,%s,%s)\n", cupdater::get_string(event).c_str(), m_n_tab_index, cupdater::get_string(new_state).c_str(), s_message.c_str());
		}
	}
	else {
		if (s_message.empty()) {
			ATLTRACE("_update_state(%s)->[%d](SM,%s,)\n", cupdater::get_string(event).c_str(), m_n_tab_index,cupdater::get_string(new_state).c_str());
		}
		else {
			ATLTRACE("_update_state(%s)->[%d](SM,%s,%s)\n", cupdater::get_string(event).c_str(), m_n_tab_index, cupdater::get_string(new_state).c_str(), s_message.c_str());
		}
	}
#endif

	return std::make_tuple(b_changed, new_state, s_message);
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
	/*
	m_ptr_final_root.reset();
	m_ptr_root.reset();
	m_ptr_tab_container.reset();

	m_ptr_exit_button.reset();

	m_ptr_cancel_confirm_button.reset();
	m_ptr_ok_confirm_button.reset();

	m_ptr_cancel_fw_button.reset();
	m_ptr_ok_fw_button.reset();
	m_ptr_fw_menu.reset();

	m_ptr_cancel_file_button.reset();
	m_ptr_select_file_button.reset();
	m_ptr_file_menu.reset();

	m_ptr_update_button.reset();
	*/
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

cupdater& cupdater::set_rom_file(const std::string& s_abs_full_path_of_rom)
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

const std::string& cupdater::get_rom_file() const
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
	ftxui::Loop loop(&m_screen, *m_ptr_final_root);

	while (!loop.HasQuitted()) {
		loop.RunOnce();
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		// Do something else like running a different library loop function.
#ifdef _WIN32
		//ATLTRACE("Current state: %s, tab_index: %d.\n", cupdater::get_string(m_state).c_str(), m_n_tab_index);
#endif

		do {
			std::lock_guard<std::mutex> lock(m_mutex); // Lock the mutex for thread safety
			if (m_q_messages.empty()) {
				continue;
			}
			std::string msg = m_q_messages.front();
			m_q_messages.pop();
			//
			try {
				m_screen.PostEvent(ftxui::Event::Custom);
			}
			catch (...) {
				continue;
			}

		} while (false);
	}//end while
	
}

void cupdater::_updates_thread_function()
{
	while (m_b_is_running){
		std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Sleep to avoid busy waiting

		if (m_state != cupdater::AppState::s_ing) {
			continue;
		}

		if (!m_b_is_running)
			break; // 종료 직전 체크

		do {
			std::lock_guard<std::mutex> lock(m_mutex); // Lock the mutex for thread safety
			m_q_messages.push("Updating...");
			
		}while (false);

	}//running

#ifdef _WIN32
	ATLTRACE(L"Good bye\n");
#endif

}
