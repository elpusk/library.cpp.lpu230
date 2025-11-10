#include "cupdater.h" // ftxui 에 min 조건부 정의가 있어서 ftxui/component/loop.hpp 보다 먼저 정의해야 함.
#include <algorithm>
#include <fstream>
#include <vector>
#include <string>
#include <cstdint>

#include <ftxui/component/loop.hpp>

#ifdef _WIN32
#include <atltrace.h>
#endif

#include <mp_cwait.h>
#include <mp_coffee_path.h>
#include <mp_coffee_pipe.h>
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
	m_screen.ForceHandleCtrlC(false);// Disable default handling. CatchEvent() 에서 ctrl-c 처리 함.
#ifndef _WIN32
	//linux only
	m_screen.ForceHandleCtrlZ(false);// Disable default handling. CatchEvent() 에서 ctrl-z 처리 함.
#endif // !_WIN32

	

	m_n_kill_signal = m_wait.generate_new_event();
	//
	m_state = cupdater::AppState::s_ini;

	//setup boot manager
	m_p_mgmt = CHidBootManager::GetInstance();

	// 현재 디렉토리를 얻음.
	std::filesystem::path current_dir = _mp::cfile::get_cur_exe_abs_path_except_backslah_file_name_extension();
	if(!cshare::get_instance().get_rom_file_abs_full_path().empty()) {
		//사용자가 커맨트 라인 옵션으로 rom/bin 파일을 지정한 경우, 그 파일이 있는 디렉토리를 현재 디렉토리로 설정. 
		current_dir = std::filesystem::path(cshare::get_instance().get_rom_file_abs_full_path()).parent_path();
		m_log_ref.log_fmt("[I] user defined : current dir = %s\n", current_dir.string().c_str());
	}
	else {
		m_log_ref.log_fmt("[I] auto detected : current dir = %s\n", current_dir.string().c_str());
	}

	cshare::get_instance().update_files_list_of_cur_dir(current_dir);//m_current_dir 에 있는 file 를 m_v_files_in_current_dir 에 설정.

	//parameter valied check
	do {
		m_ptr_hid_api_briage = std::make_shared<chid_briage>(true); //remove_all_zero_in_report
		_mp::clibhid& mlibhid(_mp::clibhid::get_manual_instance());

		std::string s_target_dev_path = _check_target_device_path_in_initial();
		if (s_target_dev_path.empty()) {
			continue; // ERROR
		}

		int n_updatable_index = -1;
		int n_total_fw(0);

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

					std::tie(n_total_fw, n_updatable_index) = _check_target_fw_of_selected_rom_in_initial();
					if (n_total_fw <= 0) {
						continue;// error잘못된 파일
					}
					if (n_updatable_index < 0) {
						// FW 있는데, 적절한 것이 없음.
						continue; // 에러 ... UI 가 숨겨져
					}
					//
					m_n_progress_max = cshare::get_instance().calculate_update_step();
				}
			}
		}

		m_b_ini = true;
	} while (false);

	// UI part
	
	do {
		if(!m_b_ini) {
			continue; // constructure failure
		}
		//
		m_n_progress_min = 0;
		m_n_progress_cur = m_n_progress_min;

		// 작업에 필요한 step 의 수를 계산 한다.
		m_n_progress_max = cshare::get_instance().calculate_update_step();

		if (!b_disaplay) {
			continue; // UI 표시가 없으므로, UI 구성 불요.
		}

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
		m_ptr_root = ftxui::Renderer(*m_ptr_tab_container, [&]() {
			return ftxui::vbox({
					   ftxui::text("lpu230_update : "
						   + cshare::get_instance().get_target_device_model_name_by_string()
						   + " : "
						   + _mp::cstring::get_mcsc_from_unicode(cshare::get_instance().get_target_device_version().get_by_string())
						   + " : "
						   + cshare::get_instance().get_rom_file_abs_full_path()
					   ) | ftxui::border,
					   (*m_ptr_tab_container)->Render() | ftxui::flex,
				}) |
				ftxui::border;
			});

		m_ptr_final_root = ftxui::CatchEvent(m_ptr_root, [&](ftxui::Event event) {

			if (event == ftxui::Event::CtrlC) {  // Ctrl+C = ASCII 3
				m_log_ref.log_fmt("[I] Detected CTRL+C\n");
#ifdef _WIN32
				ATLTRACE("~~~~~CTRL+C\n");
#endif
				return true;  // 처리됨 -> 종료 이벤트로 전달 안 함
			}

			if (event == ftxui::Event::CtrlZ) {
				m_log_ref.log_fmt("[I] Detected CTRL+Z\n");
#ifdef _WIN32
				ATLTRACE("~~~~~CTRL+Z\n");
#endif
				return true;  // 처리됨 -> 종료 이벤트로 전달 안 함
			}

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
					//_update_state(cupdater::AppEvent::e_ulstep_s);
#ifdef _WIN32
					ATLTRACE(L"Progress update complete: (%d/%d)\n", m_n_progress_cur, m_n_progress_max);
#endif
					return false;
				}
				else {
					//_update_state(cupdater::AppEvent::e_ustep_s);
#ifdef _WIN32
					ATLTRACE(L"Progress update : (%d/%d)\n", m_n_progress_cur, m_n_progress_max);
#endif
				}
			}
			return false;  // Allow other events to propagate
			});
	} while (false);
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
			_update_state(cupdater::AppEvent::e_slast_o);
			b_result = start_update_with_thread(); // run by cf 이면, 자동으로 실행되어야 한다.
		}
		else {
			if (!m_b_display) {
				_update_state(cupdater::AppEvent::e_slast_o);
				b_result = start_update_with_thread(); // UI 표시가 없으므로 자동 실행되어야 한다.
			}
		}
		
	} while (false);

	return b_result;
}

std::shared_ptr<ftxui::Component> cupdater::_create_sub_ui0_ini()
{
	m_ptr_update_button = std::make_shared<ftxui::Component>(ftxui::Button(" Update", [&]() {
		std::string s(cshare::get_instance().get_rom_file_abs_full_path());
		_update_state(cupdater::AppEvent::e_start, s);
		}));

	m_ptr_update_exit_button = std::make_shared<ftxui::Component>(ftxui::Button(" Exit", [&]() {
		std::string s(cshare::get_instance().get_rom_file_abs_full_path());
		m_b_is_running = false;
		_update_state(cupdater::AppEvent::e_exit,s);
		m_screen.ExitLoopClosure()();
		}));
	/*
	ftxui::Component container(ftxui::Container::Vertical({ *m_ptr_update_button }));
	std::shared_ptr<ftxui::Component> ptr_component = std::make_shared<ftxui::Component>(ftxui::Renderer(container, [&]() {
		return ftxui::vbox({
				   ftxui::text("Initialization complete."),
				   (*m_ptr_update_button)->Render(),
			}) |
			ftxui::border;
		})
	);
	*/
	ftxui::Component container(ftxui::Container::Horizontal({ *m_ptr_update_button, *m_ptr_update_exit_button }));
	std::shared_ptr<ftxui::Component> ptr_component = std::make_shared<ftxui::Component>(ftxui::Renderer(container, [&]() {
		return ftxui::vbox({
				   ftxui::text(" Initialization complete."),
				   ftxui::hbox({(*m_ptr_update_button)->Render() | ftxui::flex, (*m_ptr_update_exit_button)->Render() | ftxui::flex}),
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

	m_ptr_select_file_button = std::make_shared<ftxui::Component>(ftxui::Button(" Select", [&]() {
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
		m_n_progress_max = cshare::get_instance().calculate_update_step();

		if (std::filesystem::path(selected_path).extension() == ".rom") {
			// rom 파일 선택 후, OK
			cshare::get_instance().update_fw_list_of_selected_rom(CHidBootManager::GetInstance()->get_rom_library());
			m_n_progress_max = cshare::get_instance().calculate_update_step();
			std::tie(m_n_selected_fw_for_ui, m_v_firmware_list_for_ui) = cshare::get_instance().get_firmware_list_of_rom_file();

			_update_state(cupdater::AppEvent::e_sfile_or, selected_path.string());
		}
		else {
			// 그 외 파일 선택 후 OK.
			_update_state(cupdater::AppEvent::e_sfile_ob, selected_path.string());
		}
		}));
	//
	m_ptr_cancel_file_button = std::make_shared<ftxui::Component>(ftxui::Button(" Cancel", [&]() {
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
	m_ptr_ok_fw_button = std::make_shared<ftxui::Component>(ftxui::Button(" OK", [&]() {
		cshare::get_instance().set_firmware_list_of_rom_file(m_n_selected_fw_for_ui, m_v_firmware_list_for_ui);
		m_n_progress_max = cshare::get_instance().calculate_update_step();
		_update_state(cupdater::AppEvent::e_sfirm_o);
		}));
	m_ptr_cancel_fw_button = std::make_shared<ftxui::Component>(ftxui::Button(" Cancel", [&]() {
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
	m_ptr_ok_confirm_button = std::make_shared<ftxui::Component>(ftxui::Button(" OK", [&]() {
		
		_update_state(cupdater::AppEvent::e_slast_o);
		start_update_with_thread();//최종 확인 후 실행.
		}));
	m_ptr_cancel_confirm_button = std::make_shared<ftxui::Component>(ftxui::Button(" Cancel", [&]() {
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
	m_ptr_ok_warning_button = std::make_shared<ftxui::Component>(ftxui::Button(" OK", [&]() {
		_update_state(cupdater::AppEvent::e_slast_o);
		start_update_with_thread();//경고 후 실행.
		}));
	m_ptr_cancel_warning_button = std::make_shared<ftxui::Component>(ftxui::Button(" Cancel", [&]() {
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
		cshare& sh(cshare::get_instance());
		std::string status;

		float a = (float)m_n_progress_cur - (float)m_n_progress_min;
		float b = (float)m_n_progress_max - (float)m_n_progress_min;
		float f = (float)(a/b);
		f = f * 100.0f;

		status = std::to_string((int)f) + " %";

		if (sh.is_possible_exit()) {
			return ftxui::vbox({
						ftxui::text(" Updating"),
						ftxui::gauge(static_cast<float>(m_n_progress_cur - m_n_progress_min) / (m_n_progress_max - m_n_progress_min)) | ftxui::color(ftxui::Color::Blue),
						ftxui::text(status),
						ftxui::text(m_s_message_in_ing_state),
						ftxui::emptyElement()
				}) | ftxui::border;
		}
		else {
			return ftxui::vbox({
						ftxui::text( " Updating - Don't exit!"),
						ftxui::gauge(static_cast<float>(m_n_progress_cur - m_n_progress_min) / (m_n_progress_max - m_n_progress_min)) | ftxui::color(ftxui::Color::Blue),
						ftxui::text(status),
						ftxui::text(m_s_message_in_ing_state),
						ftxui::emptyElement()
				}) | ftxui::border;
		}
		}
	)//Renderer
	);//make_shared

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
	m_ptr_exit_button = std::make_shared<ftxui::Component>(ftxui::Button(" Exit", [&]() {
		//std::cout << "Exit button clicked, current state: " << get_string(m_state) << std::endl;
		m_b_is_running = false;
		_update_state(cupdater::AppEvent::e_exit);
		m_screen.ExitLoopClosure()();
		}));

	//m_ptr_exit_button->TakeFocus(); // 포커스 설정
	m_ptr_exit_container = std::make_shared<ftxui::Component>(
		ftxui::Container::Vertical({
		ftxui::Renderer([] { return ftxui::text("THE END"); }),
		ftxui::Renderer([&] { return ftxui::text(m_s_message_in_ing_state); }),
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

void cupdater::_push_message(int n_in_data /*= -1*/,const std::string& s_in_msg/*=std::string()*/)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	m_q_messages.push(std::make_pair(n_in_data,s_in_msg));
}

bool cupdater::_pop_message(int& n_out_data,std::string& s_out_msg, bool b_remove_after_pop/*=true*/)
{
	bool b_result(false);

	do {
		std::lock_guard<std::mutex> lock(m_mutex);
		s_out_msg.clear();

		if (m_q_messages.empty()) {
			continue;
		}

		std::tie(n_out_data,s_out_msg) = m_q_messages.front();
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
		cshare::get_instance().set_bootloader_path(s_target_dev_path);
		return s_target_dev_path;
	}

	//target devie 가 lpu237 or kpu238 이면 open 해서 정보를 얻어와서 
	// version & name 설정.
	// 를 해야함.
	uint32_t n_server_rsp_check_interval_mm = 10;
	int n_n_server_rsp_check_times = 300; // n_server_rsp_check_interval_mm 를 몇번 검사 할 건지.

	// lpu237 or kpu238 생성 전에, coffee-manager-2nd(server) 이 동작 중이면, server 에서 해당 device 사용 중지를 요청해야함.
	do{
		_mp::cnamed_pipe::type_ptr ptr_tx_ctl_pipe, ptr_rx_ctl_pipe;
		//setup controller
		ptr_tx_ctl_pipe = std::make_shared<_mp::cnamed_pipe>(_mp::_coffee::CONST_S_COFFEE_MGMT_CTL_PIPE_NAME, false);
		if (!ptr_tx_ctl_pipe) {
			m_log_ref.log_fmt("[E] server control pip getting.\n");
			continue;
		}
		if (!ptr_tx_ctl_pipe->is_ini()) {
			m_log_ref.log_fmt("[E] server control pip ini.\n");
			continue;
		}

		ptr_rx_ctl_pipe = std::make_shared<_mp::cnamed_pipe>(_mp::_coffee::CONST_S_COFFEE_MGMT_CTL_PIPE_NAME_OF_SERVER_RESPONSE, false);
		if (!ptr_rx_ctl_pipe) {
			m_log_ref.log_fmt("[E] server rx control pip getting.\n");
			continue;
		}
		if (!ptr_rx_ctl_pipe->is_ini()) {
			m_log_ref.log_fmt("[E] server rx control pip ini.\n");
			continue;
		}

		// 서버 control pipe 에 연결 설공.
		std::wstring s_req_ctl_pip = _mp::ccoffee_pipe::generate_ctl_request_for_consider_to_removed(
			it_dev->get_vendor_id(), it_dev->get_product_id()
		);
		if (s_req_ctl_pip.empty()) {
			m_log_ref.log_fmt("[E] generating request for stopping %s\n", s_target_dev_path.c_str());
			continue;
		}
		if (!ptr_tx_ctl_pipe->write(s_req_ctl_pip)) {
			m_log_ref.log_fmt("[E] request is sent to server for stopping %s\n", s_target_dev_path.c_str());
			continue;
		}

		std::wstring s_rx;
		int i = 0;
		for (; i < n_n_server_rsp_check_times; i++) {
			std::this_thread::sleep_for(std::chrono::milliseconds(n_server_rsp_check_interval_mm)); // 서버의 작업 시간을 위한 sleep.
			if (!ptr_rx_ctl_pipe->read(s_rx)) {
				continue;
			}
			if (s_rx.compare(_mp::_coffee::CONST_S_COFFEE_MGMT_CTL_RSP_STOP_DEV) == 0) {
				m_log_ref.log_fmt("[I] request is sent to server for stopping %s\n", s_target_dev_path.c_str());

				cshare::get_instance().set_executed_server_stop_use_target_dev(
					true,
					(int)it_dev->get_vendor_id(),
					(int)it_dev->get_product_id()
				);
				std::this_thread::sleep_for(std::chrono::milliseconds(500)); // 서버의 작업 시간을 위한 sleep.
				break; //exit for
			}
		}//end for

		if (i == n_n_server_rsp_check_times) {
			m_log_ref.log_fmt("[E] response timeout from server for stopping %s\n", s_target_dev_path.c_str());
		}
	} while (false);

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
				, cshare::get_instance().get_target_device_version_by_string().c_str()
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
			std::make_pair(-1,"Terminate program")
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
		if (old_state == cupdater::AppState::s_ini ) {

			if (event == cupdater::AppEvent::e_start) {
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
			else if (event == cupdater::AppEvent::e_slast_o) {
				// run_by_cf 나 display 가 off 된 상태에서 자동 실행이 필요하므로.
				// 바로 실행하는 상태로 전환.
				b_changed = true;
				m_state = new_state = cupdater::AppState::s_ing;
				s_message = "run by auto";
				continue;
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
	cshare::get_instance().set_target_lpu23x(ptr_null); // clear dev
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

void cupdater::_queueed_message_process(int n_msg, const std::string& s_msg)
{
	if(n_msg < 0) {
		return;
	}

	int n_progress_cur(0), n_progress_min(0), n_progress_max(0);

	// get progress pos
	n_progress_cur = m_n_progress_cur;
	n_progress_min = m_n_progress_min;
	n_progress_max = m_n_progress_max;

	m_s_message_in_ing_state.clear();

	m_s_message_in_ing_state += "[";
	m_s_message_in_ing_state += std::to_string(n_progress_min);
	m_s_message_in_ing_state += ".";
	m_s_message_in_ing_state += std::to_string(n_progress_cur);
	m_s_message_in_ing_state += ".";
	m_s_message_in_ing_state += std::to_string(n_progress_max);
	m_s_message_in_ing_state += "]";

	m_s_message_in_ing_state += " step";
	m_s_message_in_ing_state += std::to_string(n_msg);
	m_s_message_in_ing_state += " : ";
	m_s_message_in_ing_state += s_msg;

	if (s_msg.empty()) {
		m_log_ref.log_fmt("[I] ui_main_loop : (%d,empty)\n", n_msg);
#ifdef _WIN32
		ATLTRACE("(INT,MSG) - (%d,empty)\n", n_msg);
#endif
	}
	else {
		m_log_ref.log_fmt("[I] ui_main_loop : (%d,%s)\n", n_msg, s_msg.c_str());
#ifdef _WIN32
		ATLTRACE("(INT,MSG) - (%d,%s)\n", n_msg, s_msg.c_str());
#endif
	}

}
void cupdater::ui_main_loop()
{
	int n_progress_cur(0),n_progress_min(0),n_progress_max(0);

	ftxui::Loop loop(&m_screen, m_ptr_final_root);

	while (true) {
		if (loop.HasQuitted()) {
			break; // exit while
		}
		loop.RunOnce();
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		// Do something else like running a different library loop function.
#ifdef _WIN32
		//ATLTRACE("Current state: %s, tab_index: %d.\n", cupdater::get_string(m_state).c_str(), m_n_tab_index);
#endif

		do {
			int n_msg(0);
			std::string s_msg;
			if (!_pop_message(n_msg, s_msg)) {
				continue;
			}
			_queueed_message_process(n_msg, s_msg);
			//
			try {
				m_screen.PostEvent(ftxui::Event::Custom);//for repainting
			}
			catch (...) {
				continue;
			}

		} while (false);
	}//end while
	
}

void cupdater::non_ui_main_loop()
{
	int n_progress_cur(0), n_progress_min(0), n_progress_max(0);
	bool b_run(true);
	int n_msg(0);
	std::string s_msg;

	while (b_run) {
		std::this_thread::sleep_for(std::chrono::milliseconds(10));

		if (!m_b_is_running) {
			// 작업 쓰레드 자체 종료됨. 수신된 메시지 처리 후 종료.
			b_run = false;
			//
			while (_pop_message(n_msg, s_msg)) {
				_queueed_message_process(n_msg, s_msg);
			}//end while
			continue;
		}

		do {
			if (!_pop_message(n_msg, s_msg)) {
				continue;
			}
			_queueed_message_process(n_msg, s_msg);

		} while (false);
	}//end while

}

void cupdater::_updates_thread_function()
{
	int n_cnt(0);
	cshare& sh(cshare::get_instance());

	bool b_need_close_lpu23x(false);
	bool b_need_close_bootloader(false);

	while (m_b_is_running){
		std::this_thread::sleep_for(std::chrono::milliseconds(500)); // Sleep to avoid busy waiting

		if (m_state != cupdater::AppState::s_ing) {
			continue;
		}

		if (!m_b_is_running)
			break; // 종료 직전 체크
		
		sh.set_possible_exit(false); // 종료 막기
		int n_step(1);
		////////////////////////////////////////////////////
		if (!_updates_sub_thread_backup_system_param(n_step)) {
			_update_state(cupdater::AppEvent::e_ustep_e);
			_push_message();//repainting
			m_b_is_running = false; // 에러 종료
			continue;
		}
		b_need_close_lpu23x = true;

		if (!m_b_is_running)
			break; // 종료 직전 체크

		////////////////////////////////////////////////////
		// 부트로더 실행
		b_need_close_lpu23x = false; // _updates_sub_thread_run_bootloader 을 실행하면, 항상 내부에서, close 함.
		if (!_updates_sub_thread_run_bootloader(n_step)) {
			_update_state(cupdater::AppEvent::e_ustep_e);
			_push_message();//repainting
			m_b_is_running = false; // 에러 종료
			continue;
		}
		if (!m_b_is_running)
			break; // 종료 직전 체크

		////////////////////////////////////////////////////
		auto pair_bout_bin = _updates_sub_thread_wait_plugout_lpu23x(n_step);
		if (!pair_bout_bin.first) {
			//error
		}
		if (!m_b_is_running)
			break; // 종료 직전 체크
		//
		if (pair_bout_bin.second) {
			++n_step; // 이미 plugin detected
		}
		else {
			if (!_updates_sub_thread_wait_plugin_bootloader(n_step)) {
				//error
			}
			if (!m_b_is_running)
				break; // 종료 직전 체크
		}

		////////////////////////////////////////////////////
		// 부트로더 설정 실시.
		if (!_updates_sub_thread_setup_bootloader(n_step)) {
			_update_state(cupdater::AppEvent::e_ustep_e);
			_push_message();//repainting
			m_b_is_running = false; // 에러 종료
			continue;
		}
		b_need_close_bootloader = true;
		if (!m_b_is_running)
			break; // 종료 직전 체크
		//
		////////////////////////////////////////////////////
		// 섹터 삭제
		if (!_updates_sub_thread_erase_sector(n_step)) {
			_update_state(cupdater::AppEvent::e_ustep_e);
			_push_message();//repainting
			m_b_is_running = false; // 에러 종료
			continue;
		}
		if (!m_b_is_running)
			break; // 종료 직전 체크
		//

		bool b_result(false), b_complete(false);
		_mp::type_v_buffer v_sector;
		bool b_first_read(true);
		int n_out_zero_base_sector_number(0);

#if defined(_WIN32) && defined(_DEBUG)
		std::ofstream _ofs("deb_file_dump.bin", std::ios::binary | std::ios::out | std::ios::trunc);
		std::ofstream _ofs_tx("deb_file_tx_dump.bin", std::ios::binary | std::ios::out | std::ios::trunc);
#else
		std::ofstream _ofs_tx;
#endif

		do {
			////////////////////////////////////////////////////
			// 데이터 읽기
			std::tie(b_result, b_complete) = _updates_sub_thread_read_one_sector_from_file(
				n_step
				, b_first_read
				, v_sector
				, n_out_zero_base_sector_number
			);
			b_first_read = false;
			if (!b_result) {
				break;
			}
			if (!m_b_is_running) {
				continue; // 종료 직전 체크
			}

#if defined(_WIN32) && defined(_DEBUG)
			if(_ofs.is_open()) {
				_ofs.write(reinterpret_cast<const char*>(v_sector.data()), v_sector.size());
			}
#endif
			// 읽은 하나의 sector 를 write & verify.
			b_result = _updates_sub_thread_write_one_sector(n_step, v_sector, n_out_zero_base_sector_number, _ofs_tx);
			if (!b_result) {
				break;
			}
			if (!m_b_is_running) {
				continue; // 종료 직전 체크
			}

		} while (!b_complete && m_b_is_running);
#if defined(_WIN32) && defined(_DEBUG)
		if (_ofs) {
			_ofs.close();
		}
		if (_ofs_tx) {
			_ofs_tx.close();
		}
#endif

		if (!b_result) {
			_update_state(cupdater::AppEvent::e_ustep_e);
			_push_message();//repainting
			m_b_is_running = false; // 에러 종료
			continue;
		}
		if (!m_b_is_running) {
			break; // 종료 직전 체크
		}

		////////////////////////////////////////////////////
		// 앱실행
		b_need_close_bootloader = false; // 아래 실행하면 무조건, bootloader 닫음.
		if (!_updates_sub_thread_run_app(n_step)) {
			_update_state(cupdater::AppEvent::e_ustep_e);
			_push_message();//repainting
			m_b_is_running = false; // 에러 종료
			continue;
		}
		if (!m_b_is_running)
			break; // 종료 직전 체크
		
		////////////////////////////////////////////////////
		if (!_updates_sub_thread_wait_plugout_bootloader(n_step)) {
			_update_state(cupdater::AppEvent::e_ustep_e);
			_push_message();//repainting
			m_b_is_running = false; // 에러 종료
			continue;
		}
		if (!m_b_is_running)
			break; // 종료 직전 체크

		////////////////////////////////////////////////////
		_mp::clibhid_dev_info dev_info_after_update;
		std::tie(b_result, dev_info_after_update) = _updates_sub_thread_wait_plugin_lpu23x(n_step);
		if (!b_result) {
			_update_state(cupdater::AppEvent::e_ustep_e);
			_push_message();//repainting
			m_b_is_running = false; // 에러 종료
			continue;
		}
		if (!m_b_is_running)
			break; // 종료 직전 체크
		//
		sh.set_plugin_device_info_after_update(dev_info_after_update);

		////////////////////////////////////////////////////
		// 시스템 파라메터 복구.
		if (!_updates_sub_thread_recover_system_param(n_step, dev_info_after_update)) {
			_update_state(cupdater::AppEvent::e_ustep_e);
			_push_message();//repainting
			m_b_is_running = false; // 에러 종료
			continue;
		}
		b_need_close_lpu23x = true;
		if (!m_b_is_running)
			break; // 종료 직전 체크

		////////////////////////////////////////////////////
		// 업데이트 후 인터페이스 변경.
		if (!_updates_sub_thread_change_interface_after_update(n_step, dev_info_after_update)) {
			_update_state(cupdater::AppEvent::e_ustep_e);
			_push_message();//repainting
			m_b_is_running = false; // 에러 종료
			continue;
		}
		if (!m_b_is_running)
			break; // 종료 직전 체크

		////////////////////////////////////////////////////
		// 업데이트 후 iso mode 로.
		if (!_updates_sub_thread_set_iso_mode_after_update(n_step, dev_info_after_update)) {
			_update_state(cupdater::AppEvent::e_ulstep_e);
			_push_message();//repainting
			m_b_is_running = false; // 에러 종료
			continue;
		}
		if (!m_b_is_running)
			break; // 종료 직전 체크

		///////////////////////////////////////////////////
		_update_state(cupdater::AppEvent::e_ulstep_s);
		_push_message(n_step, " * Firmware update complete. *");
		m_b_is_running = false; // 정상 종료
	}//running

	if (b_need_close_lpu23x) {
		_mp::clibhid_dev::type_ptr ptr_null;
		cshare::get_instance().set_target_lpu23x(ptr_null); // clear dev
	}
	if (b_need_close_bootloader) {
		m_p_mgmt->UnselectDevice(); 
	}

	sh.set_possible_exit(true); // 종료 막기 해제
	//
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
				_push_message(n_step,"backup system parameters");
			}
			else {
				_push_message(n_step, "ERROR - backup system parameters");
#ifdef _WIN32
				ATLTRACE(L"ERROR : io_save_all_variable_sys_parameter().\n");
#endif
				break;
			}
		} while (!b_complete && m_b_is_running);

	} while (false);

	if (!b_result) { // 실패시 통신 채널 닫기.
		_mp::clibhid_dev::type_ptr ptr_null;
		cshare::get_instance().set_target_lpu23x(ptr_null); // clear dev
	}
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

	if (sh.get_start_from_bootloader()) {
		return true; // bootload 실행 필요 없음.
	}
	++n_step;

	_mp::type_set_usb_filter set_usb_filter;
	set_usb_filter.emplace(_mp::_elpusk::const_usb_vid, _mp::_elpusk::_lpu237::const_usb_pid, _mp::_elpusk::_lpu237::const_usb_inf_hid); //lpu237
	set_usb_filter.emplace(_mp::_elpusk::const_usb_vid, _mp::_elpusk::_lpu238::const_usb_pid, _mp::_elpusk::_lpu238::const_usb_inf_hid); //lpu238
	set_usb_filter.emplace(_mp::_elpusk::const_usb_vid, _mp::_elpusk::const_usb_pid_hidbl, _mp::_elpusk::const_usb_inf_hidbl); //hidbootloader

	_mp::clibhid& mlibhid(_mp::clibhid::get_manual_instance());
	mlibhid.set_usb_filter(set_usb_filter);

	mlibhid.update_dev_set_in_manual();
	//
	b_result = sh.io_run_bootloader();
	if (b_result) {
		_push_message(n_step,"run bootloader.");
	}
	else {
		_push_message(n_step, "ERROR - run bootloader.");
#ifdef _WIN32
		ATLTRACE(L"ERROR : io_run_bootloader().\n");
#endif
	}

	_mp::clibhid_dev::type_ptr ptr_null;
	cshare::get_instance().set_target_lpu23x(ptr_null); // clear dev

	return b_result;
}

std::pair<bool, bool> cupdater::_updates_sub_thread_wait_plugout_lpu23x(int& n_step)
{
#ifdef _WIN32
	ATLTRACE(L"_updates_sub_thread_wait_plugout_lpu23x()\n");
#endif

	bool b_result(false);
	bool b_detect_plugin_hidboot(false);
	cshare& sh(cshare::get_instance());

	if (sh.get_start_from_bootloader()) {
		return std::make_pair(true, false); // bootload 실행 필요 없음.
	}
	++n_step;

	_mp::cwait w;
	int n_w = w.generate_new_event();

	bool b_wait(true);
	int n_timeout_mm_unit = 300; //300msec
	int n_total_timeout_unit = 3;
	_mp::clibhid& mlibhid(_mp::clibhid::get_manual_instance());
	_mp::clibhid_dev_info::type_set::iterator it_out, it_in;

	_push_message(n_step, "waits plugout lpu23x.");

	do {
		mlibhid.update_dev_set_in_manual();
		auto set_r = mlibhid.get_removed_device_set();
		auto set_i = mlibhid.get_inserted_device_set();

		it_out = _mp::clibhid_dev_info::find(set_r, sh.get_device_path());
		if (it_out != std::end(set_r)) {
			b_wait = false;
			b_result = true; // plug out 검출.
			_mp::clibhid_dev::type_ptr ptr_null;
			cshare::get_instance().set_target_lpu23x(ptr_null); // clear dev
			// plug in 도 검사.
			it_in = _mp::clibhid_dev_info::find(set_i, _mp::_elpusk::const_usb_vid, _mp::_elpusk::const_usb_pid_hidbl);
			if (it_in != std::end(set_i)) {
				sh.set_bootloader_path(it_in->get_path_by_string());
				b_detect_plugin_hidboot = true;
			}
			continue;
		}

		if (!m_b_is_running) {
			break;// kill thread.
		}
		// more wait
		w.wait_for_one_at_time(n_timeout_mm_unit);
		--n_total_timeout_unit;
		if (n_total_timeout_unit <= 0) {
			break; //timeout : n_timeout_mm_unit * n_total_timeout_unit mm sec.
		}

	} while (b_wait);

	if (!b_result) {
		_push_message(n_step, "ERROR - detect plugout lpu23x.");
	}
	else {
		if (b_detect_plugin_hidboot) {
			_push_message(n_step, "detected plugout lpu23x and plugin bootloader.");
		}
	}
	return std::make_pair(b_result, b_detect_plugin_hidboot);
}

bool cupdater::_updates_sub_thread_wait_plugin_bootloader(int& n_step)
{
#ifdef _WIN32
	ATLTRACE(L"_updates_sub_thread_wait_plugin_bootloader()\n");
#endif

	bool b_result(false);
	cshare& sh(cshare::get_instance());

	if (sh.get_start_from_bootloader()) {
		return true; // bootload 실행 필요 없음.
	}
	++n_step;

	_mp::cwait w;
	int n_w = w.generate_new_event();

	bool b_wait(true);
	int n_timeout_mm_unit = 300; //300msec
	int n_total_timeout_unit = 17;
	_mp::clibhid& mlibhid(_mp::clibhid::get_manual_instance());
	_mp::clibhid_dev_info::type_set::iterator it_in;

	_push_message(n_step, "waits plugin bootloader.");

	do {
		mlibhid.update_dev_set_in_manual();
		auto set_i = mlibhid.get_inserted_device_set();

		it_in = _mp::clibhid_dev_info::find(set_i, _mp::_elpusk::const_usb_vid, _mp::_elpusk::const_usb_pid_hidbl);
		if (it_in != std::end(set_i)) {
			sh.set_bootloader_path(it_in->get_path_by_string());
			b_wait = false;
			b_result = true; // plug in 검출.
			continue;
		}

		if (!m_b_is_running) {
			break;// kill thread.
		}
		// more wait
		w.wait_for_one_at_time(n_timeout_mm_unit);
		--n_total_timeout_unit;
		if (n_total_timeout_unit <= 0) {
			break; //timeout : n_timeout_mm_unit * n_total_timeout_unit mm sec.
		}

	} while (b_wait);

	if (!b_result) {
		_push_message(n_step, "ERROR - detect plugin bootloader.");
	}
	return b_result;
}

bool cupdater::_updates_sub_thread_setup_bootloader(int& n_step)
{
	bool b_result(false);
	cshare& sh(cshare::get_instance());
	bool b_need_close(false);

	do {
		if (!m_p_mgmt) {
			continue;
		}

		std::string s_path_boot = sh.get_bootloader_path();
		if (s_path_boot.empty()) {
			continue;
		}

		size_t n_fw_size = sh.get_selected_fw_size();

		m_p_mgmt->GetDeviceList();
		//open & get sector info. set setup read buffer.
		if (!m_p_mgmt->SelectDevice(s_path_boot, n_fw_size)) {
			b_need_close = true;
			continue;
		}
		// 다음 작업을 위해 open 된 상태 유지 필요,
		b_result = true;
	} while (false);

	if (b_need_close) {
		m_p_mgmt->UnselectDevice();
	}
	if (!b_result) {
		_push_message(n_step, "ERROR - setup bootloader.");
	}
	else {
		//
		_push_message(n_step, "setup bootloader.");
	}

	return b_result;
}

bool cupdater::_updates_sub_thread_erase_sector(int& n_step)
{
#ifdef _WIN32
	ATLTRACE(L"_updates_sub_thread_erase_sector()\n");
#endif

	bool b_result(true);
	cshare& sh(cshare::get_instance());
	std::string s_msg;

	do {
		std::string s_error;

		// 이미 fw 크기에 맞게 erase_sec_index 가 설정되어 있음.
		auto v_sec_index = sh.get_erase_sec_index();

		for (auto n_sec : v_sec_index) {
			++n_step;
			std::tie(b_result, s_error) = m_p_mgmt->do_erase_in_worker(n_sec);
			if (!b_result) {
				s_msg = "ERROR - erase sector ";
				s_msg += std::to_string(n_sec);
				s_msg += "(";
				s_msg += s_error;
				s_msg += ").";
				_push_message(n_step, s_msg);
				break;//exit for
			}
			else {
				s_msg = "erase sector ";
				s_msg += std::to_string(n_sec);
				s_msg += ".";
				_push_message(n_step, s_msg);
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(500));
		}//end for

	} while (false);

	return b_result;
}

_mp::type_pair_bool_result_bool_complete cupdater::_updates_sub_thread_read_one_sector_from_file(
	int& n_step
	, bool b_first_read
	,_mp::type_v_buffer& v_out_sector
	,int & n_out_zero_base_sector_number
)
{
#ifdef _WIN32
	ATLTRACE(L"_updates_sub_thread_read_one_sector_from_file()\n");
#endif

	bool b_result(false);
	bool b_complete(false);
	cshare& sh(cshare::get_instance());
	std::string s_msg;

	do {
		++n_step;

		s_msg = "read sector ";
		s_msg += std::to_string(n_out_zero_base_sector_number);
		s_msg += " from file.";
		_push_message(n_step, s_msg);

		std::tie(b_result, b_complete) = sh.get_one_sector_fw_data(
			CHidBootManager::GetInstance()->get_rom_library()
			, v_out_sector
			, n_out_zero_base_sector_number
			,b_first_read
		
		);
		if (!b_result) {
			s_msg = "ERROR - read sector ";
			s_msg += std::to_string(n_out_zero_base_sector_number);
			s_msg += " from file.";
			_push_message(n_step, s_msg);
			continue;
		}

	} while (false);

	return std::make_pair(b_result,b_complete);
}

bool cupdater::_updates_sub_thread_write_one_sector(
	int& n_step
	, const _mp::type_v_buffer& v_sector
	, int n_zero_base_sector_number
	, std::ofstream& opened_debug_file
)
{
#ifdef _WIN32
	ATLTRACE(L"_updates_sub_thread_write_one_sector : %d sec.\n",n_zero_base_sector_number);
#endif

	bool b_result(true);
	std::string s_msg;

	++n_step;

	s_msg = "write & verify sector ";
	s_msg += std::to_string(n_zero_base_sector_number);
	s_msg += "(7 seconds).";
	_push_message(n_step, s_msg);

	std::string s_error;
	// 하나의 sector 를 모두 쓴다. verify 는 fw 를 받은 마이컴에서 write 한 후 읽어서 비교해서 verify 한다.
	std::tie(b_result, s_error) = m_p_mgmt->do_write_sector(n_zero_base_sector_number, v_sector, opened_debug_file);

	if (!b_result) {
		s_msg = "ERROR - write & verify sector ";
		s_msg += std::to_string(n_zero_base_sector_number);
		s_msg += "(";
		s_msg += s_error;
		s_msg += ").";
		_push_message(n_step, s_msg);

	}

	return b_result;
}

bool cupdater::_updates_sub_thread_run_app(int& n_step)
{
#ifdef _WIN32
	ATLTRACE(L"_updates_sub_thread_run_app\n");
#endif

	bool b_result(false);
	cshare& sh(cshare::get_instance());
	std::string s_msg;


	do {
		if (!m_p_mgmt->GotoApp()) {
			continue;
		}
		b_result = true;

	} while (false);

	if (!b_result) {
		_push_message(n_step, "ERROR - run application.");
	}
	else {
		_push_message(n_step, "run application.");
	}

	return b_result;
}

bool cupdater::_updates_sub_thread_wait_plugout_bootloader(int& n_step)
{
#ifdef _WIN32
	ATLTRACE(L"_updates_sub_thread_wait_plugout_bootloader\n");
#endif

	bool b_result(false);
	cshare& sh(cshare::get_instance());

	++n_step;

	_mp::cwait w;
	int n_w = w.generate_new_event();

	bool b_wait(true);
	int n_timeout_mm_unit = 300; //300msec
	int n_total_timeout_unit = 17;

	_mp::type_set_usb_filter set_usb_filter;
	set_usb_filter.emplace(_mp::_elpusk::const_usb_vid, _mp::_elpusk::_lpu237::const_usb_pid, _mp::_elpusk::_lpu237::const_usb_inf_hid); //lpu237
	set_usb_filter.emplace(_mp::_elpusk::const_usb_vid, _mp::_elpusk::_lpu238::const_usb_pid, _mp::_elpusk::_lpu238::const_usb_inf_hid); //lpu238
	set_usb_filter.emplace(_mp::_elpusk::const_usb_vid, _mp::_elpusk::_lpu237::const_usb_pid, _mp::_elpusk::const_usb_pid_hidbl); //hidboot

	_mp::clibhid& mlibhid(_mp::clibhid::get_manual_instance());
	mlibhid.set_usb_filter(set_usb_filter);

	_mp::clibhid_dev_info::type_set::iterator it_rm;

	_push_message(n_step, "waits plugout bootloader.");

	do {
		mlibhid.update_dev_set_in_manual();
		auto set_r = mlibhid.get_removed_device_set();
		
		it_rm = _mp::clibhid_dev_info::find(set_r, sh.get_bootloader_path());
		if (it_rm != std::end(set_r)) {
			sh.set_bootloader_path(""); // clear bootloader path
			b_wait = false;
			b_result = true; // plug in 검출.
			continue;
		}

		if (!m_b_is_running) {
			break;// kill thread.
		}
		// more wait
		w.wait_for_one_at_time(n_timeout_mm_unit);
		--n_total_timeout_unit;
		if (n_total_timeout_unit <= 0) {
			break; //timeout : n_timeout_mm_unit * n_total_timeout_unit mm sec.
		}

	} while (b_wait);

	if (!b_result) {
		_push_message(n_step, "ERROR - detect plugout bootloader.");
	}
	return b_result;
}

std::pair<bool, _mp::clibhid_dev_info> cupdater::_updates_sub_thread_wait_plugin_lpu23x(int& n_step)
{
#ifdef _WIN32
	ATLTRACE(L"_updates_sub_thread_wait_plugin_lpu23x\n");
#endif

	bool b_result(false);
	_mp::clibhid_dev_info dev_plug_in;

	cshare& sh(cshare::get_instance());

	++n_step;

	_mp::cwait w;
	int n_w = w.generate_new_event();

	bool b_wait(true);
	int n_timeout_mm_unit = 300; //300msec
	int n_total_timeout_unit = 30;
	_mp::clibhid& mlibhid(_mp::clibhid::get_manual_instance());
	_mp::clibhid_dev_info::type_set::iterator it_in;

	_push_message(n_step, "waits plugin lpu23x.");

	do {
		mlibhid.update_dev_set_in_manual();
		auto set_i = mlibhid.get_inserted_device_set();

		it_in = _mp::clibhid_dev_info::find(set_i, _mp::_elpusk::const_usb_vid, _mp::_elpusk::_lpu237::const_usb_pid);
		if (it_in != std::end(set_i)) {
			dev_plug_in = *it_in;
			b_wait = false;
			b_result = true; // lpu237 plug in 검출.
			continue;
		}
		it_in = _mp::clibhid_dev_info::find(set_i, _mp::_elpusk::const_usb_vid, _mp::_elpusk::_lpu238::const_usb_pid);
		if (it_in != std::end(set_i)) {
			dev_plug_in = *it_in;
			b_wait = false;
			b_result = true; // lpu238 plug in 검출.
			continue;
		}

		if (!m_b_is_running) {
			break;// kill thread.
		}
		// more wait
		w.wait_for_one_at_time(n_timeout_mm_unit);
		--n_total_timeout_unit;
		if (n_total_timeout_unit <= 0) {
			break; //timeout : n_timeout_mm_unit * n_total_timeout_unit mm sec.
		}

	} while (b_wait);

	if (!b_result) {
		_push_message(n_step, "ERROR - detect plugin lpu23x.");
	}
	else {
		_push_message(n_step, "detected plugin lpu23x.");
	}
	return std::make_pair(b_result, dev_plug_in);
}

bool cupdater::_updates_sub_thread_recover_system_param(int& n_step, const _mp::clibhid_dev_info& dev_info_after_update)
{
#ifdef _WIN32
	ATLTRACE(L"_updates_sub_thread_recover_system_param\n");
#endif

	bool b_result(false);
	bool b_complete(false);

	cshare& sh(cshare::get_instance());
	std::string s_msg_success = "the system parameters has been recovered.";
	std::string s_msg_error = "ERROR - recover lpu23x system parameters.";

	_push_message(n_step, "recovering system parameters.");

	do {
		if (sh.get_start_from_bootloader()) {
			s_msg_success = "No recovery of system parameters is needed.";
			b_result = true;
			continue; // bootloader 에서 시작했으므로 system parameter 를 복구 불요.
		}
		
		// create & open clibhid_dev.
		_mp::clibhid_dev::type_ptr ptr_dev = std::make_shared<_mp::clibhid_dev>(dev_info_after_update, m_ptr_hid_api_briage.get());
		if (!ptr_dev->is_open()) {
			s_msg_error = "ERROR - open lpu23x for recover system parameters.";
			continue;
		}

		if (!sh.io_load_basic_sys_parameter(ptr_dev) ){
			s_msg_error = "ERROR - loading the updated lpu23x info.";
			continue;
		}
		bool b_first(true);

		do {
			++n_step;
			std::tie(b_result, b_complete) = sh.io_recover_all_variable_sys_parameter(b_first, ptr_dev);
			b_first = false;
			if (b_result) {
				_push_message(n_step, "recovering system parameters");
			}
			else {
				_push_message(n_step, "ERROR - recovering system parameters");
#ifdef _WIN32
				ATLTRACE(L"ERROR : io_recover_all_variable_sys_parameter().\n");
#endif
			}
		} while (!b_complete && m_b_is_running);

	} while (false);

	if (!b_result) {
		_push_message(n_step, s_msg_error);
	}
	else {
		_push_message(n_step, s_msg_success);
	}

	return b_result;
}

bool cupdater::_updates_sub_thread_change_interface_after_update(int& n_step, const _mp::clibhid_dev_info& dev_info_after_update)
{
#ifdef _WIN32
	ATLTRACE(L"_updates_sub_thread_change_interface_after_update\n");
#endif
	cshare& sh(cshare::get_instance());
	if (sh.get_lpu23x_interface_change_after_update() == cshare::Lpu237Interface::nc) {
		return true;//bypass
	}

	bool b_result(false);

	std::string s_new_inf = sh.get_string(sh.get_lpu23x_interface_change_after_update());

	std::string s_msg_success = "the interface is changed to " + s_new_inf + ".";
	std::string s_msg_error = "ERROR - change interface(" + s_new_inf + ").";

	_push_message(n_step, "changing interface(" + s_new_inf + ").");

	do {
		++n_step;

		// create & open clibhid_dev.
		_mp::clibhid_dev::type_ptr ptr_dev = std::make_shared<_mp::clibhid_dev>(dev_info_after_update, m_ptr_hid_api_briage.get());
		if (!ptr_dev->is_open()) {
			s_msg_error = "ERROR - open lpu23x for chaning interface.";
			continue;
		}

		if (!sh.io_load_basic_sys_parameter(ptr_dev)) {
			s_msg_error = "ERROR - loading the updated lpu23x info.";
			continue;
		}

		if (!sh.io_set_interface(ptr_dev)) {
			s_msg_error = "ERROR - change interface(" + s_new_inf + ").";
			continue;
		}

		b_result = true;

	} while (false);

	if (!b_result) {
		_push_message(n_step, s_msg_error);
	}
	else {
		_push_message(n_step, s_msg_success);
	}

	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	return b_result;
}

bool cupdater::_updates_sub_thread_set_iso_mode_after_update(int& n_step, const _mp::clibhid_dev_info& dev_info_after_update)
{
#ifdef _WIN32
	ATLTRACE(L"_updates_sub_thread_set_iso_mode_after_update\n");
#endif
	cshare& sh(cshare::get_instance());
	if (!sh.is_iso_mode_after_update()) {
		return true;//bypass
	}

	bool b_result(false);

	std::string s_msg_success = "mmd1100 mode have been changed to iso mode.";
	std::string s_msg_error = "ERROR - mmd1100 mode change to iso mode.";

	_push_message(n_step, "mmd1100 iso mode.");

	do {
		++n_step;

		// create & open clibhid_dev.
		_mp::clibhid_dev::type_ptr ptr_dev = std::make_shared<_mp::clibhid_dev>(dev_info_after_update, m_ptr_hid_api_briage.get());
		if (!ptr_dev->is_open()) {
			s_msg_error = "ERROR - open lpu23x for mmd1100 iso mode change.";
			continue;
		}

		if (!sh.io_load_basic_sys_parameter(ptr_dev)) {
			s_msg_error = "ERROR - loading the updated lpu23x info.";
			continue;
		}

		if (sh.get_target_device_mmd1100_is_iso_mode()) {
			s_msg_success = "already mmd1100 mode is iso mode.";
			b_result = true;
			continue;
		}

		if (!sh.io_set_mmd1100_to_iso_mode(ptr_dev)) {
			s_msg_error = "ERROR - mmd1100 mode change to iso mode.";
			continue;
		}

		b_result = true;

	} while (false);

	if (!b_result) {
		_push_message(n_step, s_msg_error);
	}
	else {
		_push_message(n_step, s_msg_success);
	}

	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	return b_result;
}
