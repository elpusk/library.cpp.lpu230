#include "cupdater.h" // ftxui 에 min 조건부 정의가 있어서 ftxui/component/loop.hpp 보다 먼저 정의해야 함.
#include <algorithm>

#include <ftxui/component/loop.hpp>

#ifdef _WIN32
#include <atltrace.h>
#endif

#include <mp_coffee_path.h>
#include <hid/mp_clibhid.h>

#include "cshare.h"

cupdater::cupdater(_mp::clog& log,bool b_disaplay, bool b_log) :
	m_screen(ftxui::ScreenInteractive::Fullscreen())
	, m_log_ref(log)
	, m_b_display(b_disaplay)
	, m_b_log(b_log)
{
	m_n_kill_signal = m_wait.generate_new_event();
	//
	m_state = cupdater::AppState::s_ini;

	//setup boot manager
	m_p_mgmt = CHidBootManager::GetInstance();

	auto current_dir = std::filesystem::current_path();
	if(!cshare::get_instance().get_rom_file_abs_full_path().empty()) {
		current_dir = std::filesystem::path(cshare::get_instance().get_rom_file_abs_full_path()).parent_path();
	}

	cshare::get_instance().update_files_list_of_cur_dir(current_dir);//m_current_dir 에 있는 file 를 m_v_files_in_current_dir 에 설정.

	m_n_progress_min = 0;
	m_n_progress_cur = m_n_progress_min;
	m_n_progress_max = 100;

	//
	m_n_tab_index = 0; // 0: s_ini, 1: s_selfile, 2: s_selfirm, 3: s_sellast(confirm), 4: s_sellast(warning) ,5: s_ing, 6: s_scom
	m_v_tabs = {
		*_create_sub_ui0_ini(),
		*_create_sub_ui1_select_file(),
		*_create_sub_ui2_select_firmware(),
		*_create_sub_ui3_last_confirm(),
		*_create_sub_ui3_last_warning(),
		*_create_sub_ui4_updating(),
		*_create_sub_ui5_complete()
	};

	m_ptr_tab_container = std::make_shared<ftxui::Component>(ftxui::Container::Tab(m_v_tabs, &m_n_tab_index));

	// Root with title
	m_ptr_root = std::make_shared<ftxui::Component>(ftxui::Renderer(*m_ptr_tab_container, [&]() {
		return ftxui::vbox({
				   ftxui::text("lpu230_update " + 
					   _mp::cstring::get_mcsc_from_unicode(cshare::get_instance().get_version_target_device().get_by_string() )
					   + " : " 
					   + cshare::get_instance().get_rom_file_abs_full_path()
				   ) | ftxui::border,
				   (*m_ptr_tab_container)->Render() | ftxui::flex,
			}) |
			ftxui::border;
		})
	);

	m_ptr_final_root = std::make_shared<ftxui::Component>(ftxui::CatchEvent(*m_ptr_root, [&](ftxui::Event event) {
		if (event == ftxui::Event::Custom && m_state == cupdater::AppState::s_ing) {
			float progress_ratio = 0.0f;
			int total_steps = m_n_progress_max - m_n_progress_min;
			if (total_steps == 0) {
				progress_ratio = 0.0f;
			}
			else {
				progress_ratio = static_cast<float>(m_n_progress_cur - m_n_progress_min) / total_steps;
			}

			++m_n_progress_cur;
			if (m_n_progress_cur >= m_n_progress_max) {
				m_n_progress_cur = m_n_progress_max;
				_update_state(cupdater::AppEvent::e_ulstep_s);
#ifdef _WIN32
				ATLTRACE(L"Progress update complete: (%d/%d)\n", m_n_progress_cur, m_n_progress_max);
#endif
				return false;
			}
			else {
				_update_state(cupdater::AppEvent::e_ustep_s);
#ifdef _WIN32
				ATLTRACE(L"Progress update : (%d/%d)\n", m_n_progress_cur, m_n_progress_max);
#endif
			}
		}
		return false;  // Allow other events to propagate
		})
	);
}

bool cupdater::initial_update()
{
	bool b_result(false);

	do {
		//UI ini
		set_range_of_progress(CHidBootManager::WSTEP_INI, CHidBootManager::WSTEP_DEV_IN);

		m_ptr_hid_api_briage = std::make_shared<chid_briage>();


		// 현재 연결된 장비를 얻는다.
		_mp::type_set_usb_filter set_usb_filter;
		set_usb_filter.emplace(_mp::_elpusk::const_usb_vid, _mp::_elpusk::_lpu237::const_usb_pid, _mp::_elpusk::_lpu237::const_usb_inf_hid); //lpu237
		set_usb_filter.emplace(_mp::_elpusk::const_usb_vid, _mp::_elpusk::_lpu238::const_usb_pid, _mp::_elpusk::_lpu238::const_usb_inf_hid); //lpu238
		//set_usb_filter.emplace(_mp::_elpusk::const_usb_vid, _mp::_elpusk::const_usb_pid_hidbl, _mp::_elpusk::const_usb_inf_hidbl); //hidbootloader

		_mp::clibhid& mlibhid(_mp::clibhid::get_manual_instance());
		mlibhid.set_usb_filter(set_usb_filter);

		mlibhid.update_dev_set_in_manual();
		_mp::clibhid_dev_info::type_set set_dev_info = mlibhid.get_cur_device_set();

		if (set_dev_info.empty()) {
			// 연결된 lpu237 or lpu238 없음.

			if (!cshare::get_instance().is_manual()) {
				continue;
			}
			if (cshare::get_instance().get_device_path().empty()) {
				// 주어진 device path 도 없음,
				m_log_ref.log_fmt(L"[E] not found lpu237 or lpu238 devices.\n");
				continue; // target device 없음.
			}

			//m_s_device_path 가 이미 주어진 경우.//
			
			//처음 부터 bootloader가 연결되어 있는지 검사. 
			if (m_p_mgmt->GetDeviceList() <= 0) {
				// 연결된 bootloader 도 없음.
				m_log_ref.log_fmt(L"[E] also not found bootloader devices.\n");
				continue;
			}

			// 연결된 부트로더 있으면, 주어진 path 로 부트로더 열기 시도.
			if (!m_p_mgmt->SelectDevice(cshare::get_instance().get_device_path())) {
				m_log_ref.log_fmt(L"[E] can't open bootloader.\n");
				continue;
			}

			cshare::get_instance().set_start_from_bootloader(true);
		}
		
		// here
		//exist device or (manual mode & bootloader only)


		// need setting callback to p_mgmt;
		// //////////////
		m_p_mgmt->add_notify_cb(
			[&](int n_msg, WPARAM wparm, LPARAM lparam) {
#ifdef _WIN32
				ATLTRACE(L"callback msg(%d) wparm(%u) lparam(%ld)\n", n_msg, (unsigned int)wparm, (long)lparam);
#endif
			}
			, 0
		);

		if (!cshare::get_instance().get_start_from_bootloader()) {

		}

		b_result = true;
	} while (false);

	return b_result;
}

std::shared_ptr<ftxui::Component> cupdater::_create_sub_ui0_ini()
{
	m_ptr_update_button = std::make_shared<ftxui::Component>(ftxui::Button("Update", [&]() {
		std::string s(cshare::get_instance().get_rom_file_abs_full_path());
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
	static int _n_selected_file_in_v_files_in_current_dir(-1);
	static std::vector<std::string> _v_files_in_current_dir;
	static std::vector<std::string> _v_rom_files_in_current_dir; // rom file list in the _v_files_in_current_dir.
	static std::filesystem::path _current_dir;

	std::tie(
		_n_selected_file_in_v_files_in_current_dir,
		_current_dir,
		_v_files_in_current_dir,
		_v_rom_files_in_current_dir
	) = cshare::get_instance().get_file_list_of_selected_dir();

	m_ptr_file_menu = std::make_shared<ftxui::Component>(ftxui::Menu(&_v_files_in_current_dir, &_n_selected_file_in_v_files_in_current_dir));

	m_ptr_select_file_button = std::make_shared<ftxui::Component>(ftxui::Button("Select", [&]() {
		/*
		std::tie(
			_n_selected_file_in_v_files_in_current_dir,
			_current_dir,
			_v_files_in_current_dir,
			_v_rom_files_in_current_dir
		) = cshare::get_instance().get_file_list_of_selected_dir();
		*/
		if (_v_files_in_current_dir.empty()) {
			return;
		}
		std::string selected = _v_files_in_current_dir[_n_selected_file_in_v_files_in_current_dir];
		std::filesystem::path selected_path = _current_dir / selected;
		if (selected == "..") {
			_current_dir = _current_dir.parent_path();
			cshare::get_instance().update_files_list_of_cur_dir(_current_dir);

			std::tie(
				_n_selected_file_in_v_files_in_current_dir,
				std::ignore,
				_v_files_in_current_dir,
				_v_rom_files_in_current_dir
			) = cshare::get_instance().get_file_list_of_selected_dir();
			return;
		}
		if (std::filesystem::is_directory(selected_path)) {
			_current_dir = std::filesystem::path(selected_path);
			cshare::get_instance().update_files_list_of_cur_dir(_current_dir);

			std::tie(
				_n_selected_file_in_v_files_in_current_dir,
				std::ignore,
				_v_files_in_current_dir,
				_v_rom_files_in_current_dir
			) = cshare::get_instance().get_file_list_of_selected_dir();

			return;
		}
		if (std::filesystem::path(selected_path).extension() == ".rom") {
			cshare::get_instance().set_rom_file_abs_full_path(selected_path.string());
			cshare::get_instance().update_fw_list_of_selected_rom();

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
		std::filesystem::path path_cur;
		std::tie(std::ignore, path_cur, std::ignore, std::ignore) = cshare::get_instance().get_file_list_of_selected_dir();
		return ftxui::vbox({
				   ftxui::text("Select rom file"),
				   ftxui::text("Current directory: " + path_cur.string()),
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
	static std::vector<std::string> _v_firmware_list; //firmware list in the selected rom file.
	static int _n_selected_fw(-1);

	std::shared_ptr<ftxui::Component> ptr_component;

	std::tie(_n_selected_fw, _v_firmware_list) = cshare::get_instance().get_firmware_list_of_rom_file();

	m_ptr_fw_menu = std::make_shared<ftxui::Component>(ftxui::Menu(&_v_firmware_list, &_n_selected_fw));
	m_ptr_ok_fw_button = std::make_shared<ftxui::Component>(ftxui::Button("OK", [&]() {
		cshare::get_instance().set_firmware_list_of_rom_file(_n_selected_fw, _v_firmware_list);
		_update_state(cupdater::AppEvent::e_sfirm_o);
		}));
	m_ptr_cancel_fw_button = std::make_shared<ftxui::Component>(ftxui::Button("Cancel", [&]() {
		_update_state(cupdater::AppEvent::e_sfirm_c);
		}));

	ftxui::Component container(ftxui::Container::Vertical(
		{ *m_ptr_fw_menu, ftxui::Container::Horizontal({ *m_ptr_ok_fw_button, *m_ptr_cancel_fw_button }) }));

	ptr_component = std::make_shared<ftxui::Component>(ftxui::Renderer(container, [&]() {
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
				   ftxui::text("the selected firmware will be downloaded to your device."),
				   ftxui::text("Would you like to proceed with the update?"),
				   ftxui::hbox({(*m_ptr_ok_confirm_button)->Render() | ftxui::flex, (*m_ptr_cancel_confirm_button)->Render() | ftxui::flex}),
			}) |
			ftxui::border;
		})
	);

	return ptr_component;
}

std::shared_ptr<ftxui::Component> cupdater::_create_sub_ui3_last_warning()
{
	m_ptr_ok_warning_button = std::make_shared<ftxui::Component>(ftxui::Button("OK", [&]() {
		_update_state(cupdater::AppEvent::e_slast_o);
		}));
	m_ptr_cancel_warning_button = std::make_shared<ftxui::Component>(ftxui::Button("Cancel", [&]() {
		_update_state(cupdater::AppEvent::e_slast_c);
		}));

	ftxui::Component container(ftxui::Container::Horizontal({ *m_ptr_ok_warning_button, *m_ptr_cancel_warning_button }));
	std::shared_ptr<ftxui::Component> ptr_component = std::make_shared<ftxui::Component>(ftxui::Renderer(container, [&]() {
		return ftxui::vbox({
				   ftxui::text("Warning"),
				   ftxui::text("Compatibility between the selected firmware and the target"),
				   ftxui::text("device cannot be confirmed."),
				   ftxui::text("Would you like to proceed with the update?"),
				   ftxui::hbox({(*m_ptr_ok_warning_button)->Render() | ftxui::flex, (*m_ptr_cancel_warning_button)->Render() | ftxui::flex}),
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

		float a = (float)m_n_progress_cur - (float)m_n_progress_min;
		float b = (float)m_n_progress_max - (float)m_n_progress_min;
		float f = (float)(a/b);
		f = f * 100.0f;

		status = std::to_string((int)f) + " %";
		return ftxui::vbox({
					ftxui::text("Updating"),
					ftxui::gauge(static_cast<float>(m_n_progress_cur - m_n_progress_min) / (m_n_progress_max - m_n_progress_min)) | ftxui::color(ftxui::Color::Blue),
					ftxui::text(status),
					ftxui::text(cshare::get_instance().get_rom_file_abs_full_path()),
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
				   ftxui::text(cshare::get_instance().get_rom_file_abs_full_path()),
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
		ftxui::Renderer([this] { return ftxui::text(cshare::get_instance().get_rom_file_abs_full_path()); }),
		*m_ptr_exit_button
		})
	);

	std::shared_ptr<ftxui::Component> ptr_component = std::make_shared<ftxui::Component>(ftxui::Renderer(*m_ptr_exit_container, [&]() {
		//std::cout << "Rendering s_com UI, tab_index: " << m_n_tab_index << std::endl;
		return (*m_ptr_exit_container)->Render() | ftxui::border;
		}));
	return ptr_component;
}

void cupdater::_push_message(const std::string& s_in_msg)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	m_q_messages.push(s_in_msg);
}

bool cupdater::_pop_message(std::string& s_out_msg, bool b_remove_after_pop/*=true*/)
{
	bool b_result(false);

	do {
		std::lock_guard<std::mutex> lock(m_mutex);
		s_out_msg.clear();

		if (m_q_messages.empty()) {
			continue;
		}

		s_out_msg = m_q_messages.front();
		if (b_remove_after_pop) {
			m_q_messages.pop();
		}
		b_result = true;
	} while (false);
	return b_result;
}

std::vector<std::filesystem::path> cupdater::_find_rom_files()
{
	std::vector<std::filesystem::path> rom_files;

	// 현재 실행 디렉터리
	std::filesystem::path current_exe = _mp::cfile::get_cur_exe_abs_path();

	std::filesystem::path current_dir = current_exe.parent_path();

	// 디렉터리 순회
	for (const auto& entry : std::filesystem::directory_iterator(current_dir)) {
		if (entry.is_regular_file() && entry.path().extension() == ".rom") {
			rom_files.push_back(entry.path());
		}
	}

	// 내림차순 정렬
	std::sort(rom_files.begin(), rom_files.end(),
		[](const std::filesystem::path& a, const std::filesystem::path& b) {
			return a.filename().string() > b.filename().string();
		});

	return rom_files;
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
					if (cshare::get_instance().is_select_fw_updatable()) {
						// selected firmware is updatable.
						m_n_tab_index = 3;
					}
					else {
						m_n_tab_index = 4;
					}
					break;
				case cupdater::AppState::s_ing:
					m_n_tab_index = 5;
					break;
				case cupdater::AppState::s_com:
					m_n_tab_index = 6;
					break;
				default:
					m_n_tab_index = 6; // Default to s_ini state
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
		b_result = true;
	} while (false); 
	return b_result;
}

void cupdater::set_range_of_progress(int n_progress_min, int n_progress_max)
{
	m_n_progress_cur = n_progress_min;
	m_n_progress_min = n_progress_min;
	m_n_progress_max = n_progress_max;
}

void cupdater::set_pos_of_progress(int n_progress_pos)
{
	m_n_progress_cur = n_progress_pos;
}

int cupdater::get_pos_of_progress() const
{
	return m_n_progress_cur;
}

void cupdater::ui_main_loop()
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
			std::string msg;
			if (!_pop_message(msg)) {
				continue;
			}
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
			_push_message("Updating...");
			
		}while (false);

	}//running

#ifdef _WIN32
	ATLTRACE(L"Good bye\n");
#endif

}
