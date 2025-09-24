#include "cupdater.h" // ftxui 에 min 조건부 정의가 있어서 ftxui/component/loop.hpp 보다 먼저 정의해야 함.
#include <algorithm>

#include <ftxui/component/loop.hpp>

#ifdef _WIN32
#include <atltrace.h>
#endif

#include <mp_cwait.h>
#include <mp_coffee_path.h>
#include <hid/mp_clibhid.h>
#include <cprotocol_lpu237.h>

#include "HidBootManager.h"
#include "cshare.h"

cupdater::cupdater(_mp::clog& log,bool b_disaplay, bool b_log) :
	m_screen(ftxui::ScreenInteractive::Fullscreen())
	, m_log_ref(log)
	, m_b_display(b_disaplay)
	, m_b_log(b_log)
	, m_b_ini(false)
	, m_n_selected_fw_for_ui(-1)
{
	m_n_kill_signal = m_wait.generate_new_event();
	//
	m_state = cupdater::AppState::s_ini;

	//setup boot manager
	m_p_mgmt = CHidBootManager::GetInstance();

	// 현재 디렉토리를 얻음.
	auto current_dir = std::filesystem::current_path();
	if(!cshare::get_instance().get_rom_file_abs_full_path().empty()) {
		//사용자가 커맨트 라인 옵션으로 rom/bin 파일을 지정한 경우, 그 파일이 있는 디렉토리를 현재 디렉토리로 설정. 
		current_dir = std::filesystem::path(cshare::get_instance().get_rom_file_abs_full_path()).parent_path();
	}

	cshare::get_instance().update_files_list_of_cur_dir(current_dir);//m_current_dir 에 있는 file 를 m_v_files_in_current_dir 에 설정.

	//parameter valied check
	do {
		m_ptr_hid_api_briage = std::make_shared<chid_briage>();
		_mp::clibhid& mlibhid(_mp::clibhid::get_manual_instance());

		std::string s_target_dev_path = _check_target_device_path_in_initial();
		if (s_target_dev_path.empty()) {
			continue; // ERROR
		}

		// 여기 부터는 s_target_dev_path 가 실제 연결된 장비로 검증됨.
		bool b_file_is_rom_type(false);
		std::string s_target_rom_file;
		std::tie(s_target_rom_file, b_file_is_rom_type) = _check_target_file_path_in_initial();
		if (!cshare::get_instance().get_rom_file_abs_full_path().empty()) {
			if (s_target_rom_file.empty()) {
				continue; //ERROR. 주어진 파일이 있으니 실제 존재하지 않음.
			}

			// 여기는 s_target_rom_file 은 cshare::get_instance().get_rom_file_abs_full_path() 와 동일하므로, 다시 저장 불요.
			// 
			//fw check 로.
			// 여기 부터는 s_target_dev_path 가 실제 연결된 장비로 검증 & s_target_rom_file 파일 존재 검증.
			if (b_file_is_rom_type) {

				int n_updatable_index = -1;
				int n_total_fw(0);
				std::tie(n_total_fw, n_updatable_index) = _check_target_fw_of_selected_rom_in_initial();
				if (n_total_fw <= 0) {
					continue;// error잘못된 파일
				}
				if (n_updatable_index < 0) {
					// FW 있는데, 적절한 것이 없음.
					if (!m_b_display) {
						continue; // 에러 ... UI 가 숨겨져
					}
				}
			}
			else {
				// raw 파일 선택.
				// UI 가 숨겨져 있어도 사용자 설정이므로 raw(bin) 으로 계속 진행.
			}
		}
		else {
			//주어진 파일이 없고,
			if (s_target_rom_file.empty()) {
				// 현재 폴더에도 없는데.
				if (!m_b_display) {
					// UI 까지 숨겨서 선택 할 수 없으로 에러.
					continue;
				}
			}
			else {
				// 자동 선택의 경우
				if (!m_b_display && b_file_is_rom_type) {
					// UI 가 숨겨져 있고, 선택파일이 rom type 일때만 자동 선택 저장.
					cshare::get_instance().set_rom_file_abs_full_path(s_target_rom_file);
				}
			}
		}

		m_b_ini = true;
	} while (false);
	//
	m_n_progress_min = 0;
	m_n_progress_cur = m_n_progress_min;

	// 작업에 필요한 step 의 수를 계산 한다.
	m_n_progress_max = cshare::get_instance().calculate_update_step();


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
				   ftxui::text("lpu230_update : " 
					   + cshare::get_instance().get_target_device_model_name_by_string()
					   + " : "
					   + _mp::cstring::get_mcsc_from_unicode(cshare::get_instance().get_target_device_version().get_by_string() )
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
		if (!m_b_ini) {
			continue; // constructure failure
		}
		//UI ini

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

		//
		b_result = true;

		if (cshare::get_instance().is_run_by_cf()) {
			b_result = start_update_with_thread(); // run by cf 이면, 자동으로 실행되어야 한다.
		}
		else {
			if (!m_b_display) {
				b_result = start_update_with_thread(); // UI 표시가 없으므로 자동 실행되어야 한다.
			}
		}
		
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

		cshare::get_instance().set_rom_file_abs_full_path(selected_path.string());// UI 에 선택 변경을 저장.

		if (std::filesystem::path(selected_path).extension() == ".rom") {
			// rom 파일 선택 후, OK
			cshare::get_instance().update_fw_list_of_selected_rom(CHidBootManager::GetInstance()->get_rom_library());
			std::tie(m_n_selected_fw_for_ui, m_v_firmware_list_for_ui) = cshare::get_instance().get_firmware_list_of_rom_file();

			_update_state(cupdater::AppEvent::e_sfile_or, selected_path.string());
		}
		else {
			// 그 외 파일 선택 후 OK.
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
	std::shared_ptr<ftxui::Component> ptr_component;

	std::tie(m_n_selected_fw_for_ui, m_v_firmware_list_for_ui) = cshare::get_instance().get_firmware_list_of_rom_file();

	m_ptr_fw_menu = std::make_shared<ftxui::Component>(ftxui::Menu(&m_v_firmware_list_for_ui, &m_n_selected_fw_for_ui));
	m_ptr_ok_fw_button = std::make_shared<ftxui::Component>(ftxui::Button("OK", [&]() {
		cshare::get_instance().set_firmware_list_of_rom_file(m_n_selected_fw_for_ui, m_v_firmware_list_for_ui);
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
		start_update_with_thread();//최종 확인 후 실행.
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
		start_update_with_thread();//경고 후 실행.
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

std::string cupdater::_check_target_device_path_in_initial()
{
	// 현재 연결된 장비를 얻는다.
	_mp::type_set_usb_filter set_usb_filter;
	set_usb_filter.emplace(_mp::_elpusk::const_usb_vid, _mp::_elpusk::_lpu237::const_usb_pid, _mp::_elpusk::_lpu237::const_usb_inf_hid); //lpu237
	set_usb_filter.emplace(_mp::_elpusk::const_usb_vid, _mp::_elpusk::_lpu238::const_usb_pid, _mp::_elpusk::_lpu238::const_usb_inf_hid); //lpu238
	//
	_mp::clibhid& mlibhid(_mp::clibhid::get_manual_instance());
	mlibhid.set_usb_filter(set_usb_filter);

	mlibhid.update_dev_set_in_manual();
	//모든 연결된 모든 lpu237, lpu238를 얻음.
	_mp::clibhid_dev_info::type_set set_dev_info_lpu = mlibhid.get_cur_device_set();

	// prmitive 장비만 filtering....... fw update 는 primitive 에 대해서만 진행.
	_mp::clibhid_dev_info::type_set set_primitive_dev_info_lpu;
	_mp::clibhid_dev_info::filter_dev_info_set(set_primitive_dev_info_lpu, set_dev_info_lpu, { _mp::type_bm_dev_hid });

	// 연결된 hidboot 를 찾기.
	set_usb_filter.clear();
	set_usb_filter.emplace(_mp::_elpusk::const_usb_vid, _mp::_elpusk::const_usb_pid_hidbl, _mp::_elpusk::const_usb_inf_hidbl); //hidbootloader

	mlibhid.set_usb_filter(set_usb_filter);
	mlibhid.update_dev_set_in_manual();

	// hidboot는 only primitive device!
	_mp::clibhid_dev_info::type_set set_dev_info_boot = mlibhid.get_cur_device_set();

	_mp::clibhid_dev_info::type_set::iterator it_dev = std::end(set_primitive_dev_info_lpu);
	_mp::clibhid_dev_info::type_set::iterator it_boot = std::end(set_dev_info_boot);;

	/////////////////////////////////////////////////////////////////////////////////
	//주어진 path 가 연결된 path 에 있는지 확인
	std::string s_target_dev_path = cshare::get_instance().get_device_path();

	// 우선 순위는
	// 1. 주어진 device path
	// 2. 연결된 lpu237, lpu238
	// 3. 연결된 hidboot
	do {
		if (!s_target_dev_path.empty()) {
			// 주어진 장비를 연결된 lpu237, lpu238 에서 찾기
			it_dev = _mp::clibhid_dev_info::find(set_primitive_dev_info_lpu, s_target_dev_path);
			if (it_dev != std::end(set_primitive_dev_info_lpu)) {
				m_log_ref.log_fmt("[I] target is  %s\n", s_target_dev_path.c_str());
				continue; // next step
			}

			// 주어진 장비를 hidboot에서 찾기.
			it_boot = _mp::clibhid_dev_info::find(set_dev_info_boot, s_target_dev_path);
			if (it_boot != std::end(set_dev_info_boot)) {
				cshare::get_instance().set_start_from_bootloader(true); //start from boot
				m_log_ref.log_fmt("[I] target is  %s\n", s_target_dev_path.c_str());
				continue; // next step
			}
			m_log_ref.log_fmt("[E] not found %s\n", s_target_dev_path.c_str());
			s_target_dev_path.clear(); // target device 없음.
			continue; // error
		}

		// 주어진 장비가 없는 경우. 자동 선택.
		if (!set_primitive_dev_info_lpu.empty()) {
			it_dev = set_primitive_dev_info_lpu.begin();
			s_target_dev_path = it_dev->get_path_by_string();
			m_log_ref.log_fmt("[I] auto target is  %s\n", s_target_dev_path.c_str());
			continue;
		}
		if (!set_dev_info_boot.empty()) {
			it_boot = set_dev_info_boot.begin();
			s_target_dev_path = it_boot->get_path_by_string();
			cshare::get_instance().set_start_from_bootloader(true); //start from boot
			m_log_ref.log_fmt("[I] auto target is  %s\n", s_target_dev_path.c_str());
			continue;
		}

		m_log_ref.log_fmt("[E] none device.\n"); // ERROR

	} while (false);// target device finding while

	if (s_target_dev_path.empty()) {
		return s_target_dev_path; //ERROR
	}

	if (cshare::get_instance().get_start_from_bootloader()) {
		cshare::get_instance().set_device_path(s_target_dev_path);// 시작 장비가 hidboot loader 로 장비가 설정됨
		return s_target_dev_path;
	}

	//target devie 가 lpu237 or kpu238 이면 open 해서 정보를 얻어와서 
	// version & name 설정.
	// 를 해야함.
		
	// create & open clibhid_dev.
	_mp::clibhid_dev::type_ptr ptr_dev = std::make_shared<_mp::clibhid_dev>(*it_dev, m_ptr_hid_api_briage.get());
	if (!ptr_dev->is_open()) {
		m_log_ref.log_fmt("[E] open : %s\n", s_target_dev_path.c_str());
		s_target_dev_path.clear();
	}
	else {
		cshare::get_instance().set_target_lpu23x(ptr_dev);

		if (!cshare::get_instance().io_get_system_info_and_set_name_version()) {
			m_log_ref.log_fmt("[E] get system data : %s\n", s_target_dev_path.c_str());
			s_target_dev_path.clear();

			_mp::clibhid_dev::type_ptr ptr_null;
			cshare::get_instance().set_target_lpu23x(ptr_null); // clear dev
		}
		else {
			m_log_ref.log_fmt("[I] get system data : %s : (name,version) = (%s,%s).\n"
				, s_target_dev_path.c_str()
				, cshare::get_instance().get_target_device_model_name_by_string().c_str()
				, cshare::get_instance().get_target_device_model_name_by_string().c_str()
			);
			cshare::get_instance().set_device_path(s_target_dev_path);// 시작 장비가 lpu23x 로 장비가 설정됨
		}
	}

	return s_target_dev_path;
}

std::pair<std::string, bool> cupdater::_check_target_file_path_in_initial()
{
	////////////////////////////////////////////////////////////////////////////
	// 주어진 rom file 검증
	std::string s_target_rom_file = cshare::get_instance().get_rom_file_abs_full_path();
	bool b_rom_type(false);

	// 우선순위
	// 1. 주어진 파일
	// 2. 현재 실행 파일이 있는 디렉토리에서 detected rom file.
	// 3. 현재 실행 파일이 있는 디렉토리에서 detected bin file
	do {
		if (!s_target_rom_file.empty()) {
			if (!std::filesystem::exists(s_target_rom_file)) {
				m_log_ref.log_fmt("[E] not found file - %s\n", s_target_rom_file.c_str());
				s_target_rom_file.clear(); //for error maker
				continue;
			}
			if (!std::filesystem::is_regular_file(s_target_rom_file)) {
				m_log_ref.log_fmt("[E] file - %s isn't regular.\n", s_target_rom_file.c_str());
				s_target_rom_file.clear(); //for error maker
				continue;
			}

			std::filesystem::path p(s_target_rom_file);
			if (p.extension() == ".rom") {
				b_rom_type = true;
			}

			m_log_ref.log_fmt("[I] target file - %s\n", s_target_rom_file.c_str());
			continue;
		}

		// 주어진 파일이 없음(auto). 이미 읽은 리스트 있음.
		std::vector<std::string> v_s_rom = cshare::get_instance().get_vector_rom_files_in_current_dir();
		if (!v_s_rom.empty()) {
			s_target_rom_file = *v_s_rom.begin();
			b_rom_type = true;
			m_log_ref.log_fmt("[I] auto target file - %s\n", s_target_rom_file.c_str());
			continue;
		}
		std::vector<std::string> v_s_bin = cshare::get_instance().get_vector_bin_files_in_current_dir();
		if (!v_s_bin.empty()) {
			s_target_rom_file = *v_s_bin.begin();
			m_log_ref.log_fmt("[I] auto target file - %s\n", s_target_rom_file.c_str());
			continue;
		}

		m_log_ref.log_fmt("[E] none target file.\n"); // ERROR
	} while (false);
	return std::make_pair(s_target_rom_file,b_rom_type);
}

std::pair<int, int> cupdater::_check_target_fw_of_selected_rom_in_initial()
{
	int n_updatable_index = -1;
	int n_total_fw(0);
	std::string s_target_rom_file(cshare::get_instance().get_rom_file_abs_full_path());
	std::filesystem::path fp_target_file(s_target_rom_file);
	if (fp_target_file.extension() == ".rom") {
		// 이 경우 rom 내에 있는 fw 를 확인 필요.
		std::tie(n_total_fw, n_updatable_index) = cshare::get_instance().update_fw_list_of_selected_rom(CHidBootManager::GetInstance()->get_rom_library());
		if (n_updatable_index < 0) {
			m_log_ref.log_fmt("[E] not valied firmware in %s\n", s_target_rom_file.c_str());
			//ERROR
		}
		else {
			// 유효한 조건을 만족하는 firmware 가 존재
			m_log_ref.log_fmt("[I] valied firmware index (%d/%d) in %s\n", n_updatable_index, n_total_fw, s_target_rom_file.c_str());
		}
	}
	else {
		// rom 이 외의 주어진 파일 또는 자동 탐지된 bin 파일
		// 이 경우 강제로 하나의 파일이 그대로 fw 이므로, 
		n_total_fw = 1; //n_updatable_index 는 -1 이다.
	}
	return std::make_pair(n_total_fw, n_updatable_index);
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

	_mp::clibhid_dev::type_ptr ptr_null;
	cshare::get_instance().set_target_lpu23x(ptr_null);
	m_ptr_hid_api_briage.reset();
}

// Update function to be implemented by derived classes
bool cupdater::start_update_with_thread()
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

#ifdef _WIN32
			if (msg.empty()) {
				ATLTRACE("MSG - empty()\n");
			}
			else {
				ATLTRACE("MSG - %s\n", msg.c_str());
			}
#endif //_WIN32
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
	int n_cnt(0);
	cshare& sh(cshare::get_instance());

	while (m_b_is_running){
		std::this_thread::sleep_for(std::chrono::milliseconds(500)); // Sleep to avoid busy waiting

		if (m_state != cupdater::AppState::s_ing) {
			continue;
		}

		if (!m_b_is_running)
			break; // 종료 직전 체크

		/*
		do {
			++n_cnt;
			std::string s_msg("Updating...");
			s_msg += std::to_string(n_cnt);
			_push_message(s_msg);
			
		}while (false);
		*/
		//
		// 
		
		int n_step(0);

		_updates_sub_thread_backup_system_param(n_step);
		if (!m_b_is_running)
			break; // 종료 직전 체크
		//
		// 부트로더 실행
		if (!_updates_sub_thread_run_bootloader(n_step)) {
			break; // error
		}
		if (!m_b_is_running)
			break; // 종료 직전 체크
		//
		if (!_updates_sub_thread_wait_plugout_lpu23x(n_step)) {
			//error
		}
		if (!m_b_is_running)
			break; // 종료 직전 체크
		//
		if (!_updates_sub_thread_wait_plugin_bootloader(n_step)) {
			//error
		}
		if (!m_b_is_running)
			break; // 종료 직전 체크

		// 섹터 삭제
		if (!_updates_sub_thread_erase_sector(n_step)) {
			break;
		}
		if (!m_b_is_running)
			break; // 종료 직전 체크
		//

		// ..
		// 데이터 읽기
		if (!_updates_sub_thread_read_sector_from_file(n_step)) {
			break;
		}
		if (!m_b_is_running)
			break; // 종료 직전 체크

		// 데이터 쓰시 및 확인
		if (!_updates_sub_thread_write_sector(n_step)) {
			break;
		}
		if (!m_b_is_running)
			break; // 종료 직전 체크

		// ..
		// 앱실행
		if (!_updates_sub_thread_run_app(n_step)) {
			break;
		}
		if (!m_b_is_running)
			break; // 종료 직전 체크

		if (!_updates_sub_thread_wait_plugout_bootloader(n_step)) {
			break;
		}
		if (!m_b_is_running)
			break; // 종료 직전 체크

		if (!_updates_sub_thread_wait_plugin_lpu23x(n_step)) {
			break;
		}
		if (!m_b_is_running)
			break; // 종료 직전 체크

		// 시스템 파라메터 복구.
		if (!_updates_sub_thread_recover_system_param(n_step)) {
			break;
		}
		if (!m_b_is_running)
			break; // 종료 직전 체크

	}//running

#ifdef _WIN32
	ATLTRACE(L"Good bye\n");
#endif

}

bool cupdater::_updates_sub_thread_backup_system_param(int& n_step)
{
#ifdef _WIN32
	ATLTRACE(L"_updates_sub_thread_backup_system_param()\n");
#endif

	bool b_result(false);
	bool b_complete(false);
	cshare& sh(cshare::get_instance());

	do {
		// 작업을 시작한다.
		if (sh.get_start_from_bootloader()) {
			b_result = true; // bootloader 에서 시작했으므로 system parameter 를 백업 불요.
			continue;
		}

		// 시스템 파라메터 백업
		bool b_first(true);
		do {
			++n_step;
			std::tie(b_result, b_complete) = sh.io_save_all_variable_sys_parameter(b_first);
			b_first = false;
			if (b_result) {
				_push_message(std::to_string(n_step));
			}
			else {
#ifdef _WIN32
				ATLTRACE(L"ERROR : io_save_all_variable_sys_parameter().\n");
#endif
			}
		} while (!b_complete && m_b_is_running);

	} while (false);
	return b_result;
}

bool cupdater::_updates_sub_thread_run_bootloader(int& n_step)
{
#ifdef _WIN32
	ATLTRACE(L"_updates_sub_thread_run_bootloader()\n");
#endif

	bool b_result(false);
	bool b_complete(false);
	cshare& sh(cshare::get_instance());

	++n_step;

	if (sh.get_start_from_bootloader()) {
		return true; // botload 실행 필요 없음.
	}
	b_result = sh.io_run_bootloader();
	if (b_result) {
		_push_message(std::to_string(n_step));
	}
	else {
#ifdef _WIN32
		ATLTRACE(L"ERROR : io_run_bootloader().\n");
#endif
	}
	return b_result;
}

bool cupdater::_updates_sub_thread_wait_plugout_lpu23x(int& n_step)
{
#ifdef _WIN32
	ATLTRACE(L"_updates_sub_thread_wait_plugout_lpu23x()\n");
#endif

	bool b_result(false);

	do {

	} while (false);
	return b_result;
}

bool cupdater::_updates_sub_thread_wait_plugin_bootloader(int& n_step)
{
#ifdef _WIN32
	ATLTRACE(L"_updates_sub_thread_wait_plugin_bootloader()\n");
#endif

	bool b_result(false);

	do {

	} while (false);
	return b_result;
}

bool cupdater::_updates_sub_thread_erase_sector(int& n_step)
{
#ifdef _WIN32
	ATLTRACE(L"_updates_sub_thread_erase_sector()\n");
#endif

	bool b_result(false);

	do {

	} while (false);
	return b_result;
}

bool cupdater::_updates_sub_thread_read_sector_from_file(int& n_step)
{
#ifdef _WIN32
	ATLTRACE(L"_updates_sub_thread_read_sector_from_file()\n");
#endif

	bool b_result(false);

	do {

	} while (false);
	return b_result;
}

bool cupdater::_updates_sub_thread_write_sector(int& n_step)
{
#ifdef _WIN32
	ATLTRACE(L"_updates_sub_thread_write_sector\n");
#endif

	bool b_result(false);

	do {

	} while (false);
	return b_result;
}

bool cupdater::_updates_sub_thread_run_app(int& n_step)
{
#ifdef _WIN32
	ATLTRACE(L"_updates_sub_thread_run_app\n");
#endif

	bool b_result(false);

	do {

	} while (false);
	return b_result;
}

bool cupdater::_updates_sub_thread_wait_plugout_bootloader(int& n_step)
{
#ifdef _WIN32
	ATLTRACE(L"_updates_sub_thread_wait_plugout_bootloader\n");
#endif

	bool b_result(false);

	do {

	} while (false);
	return b_result;
}

bool cupdater::_updates_sub_thread_wait_plugin_lpu23x(int& n_step)
{
#ifdef _WIN32
	ATLTRACE(L"_updates_sub_thread_wait_plugin_lpu23x\n");
#endif

	bool b_result(false);

	do {

	} while (false);
	return b_result;
}

bool cupdater::_updates_sub_thread_recover_system_param(int& n_step)
{
#ifdef _WIN32
	ATLTRACE(L"_updates_sub_thread_recover_system_param\n");
#endif

	bool b_result(false);

	do {

	} while (false);
	return b_result;
}
