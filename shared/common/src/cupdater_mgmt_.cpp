#include <websocket/mp_win_nt.h>
#include <server/mp_cserver_.h>
#include <cupdater_mgmt_.h>

#if defined(_WIN32)
#include <atltrace.h>
#endif

cupdater_mgmt& cupdater_mgmt::get_instance()
{
	static cupdater_mgmt obj;
	return obj;
}

bool cupdater_mgmt::run_update(unsigned long n_session)
{
	bool b_result(false);

	do {
		auto it = m_map.find(n_session);
		if (it == std::end(m_map)) {
			continue;
		}
		cupdater_param::type_ptr ptr_boot_param = it->second;
		if (!ptr_boot_param) {
			continue;
		}

		b_result = ptr_boot_param->start_update();

	} while (false);

	return b_result;
}

bool cupdater_mgmt::notify_to_server(const _mp::ccoffee_pipe::type_tuple_notify_info& nf)
{
	bool b_result(false);

	do {
		bool b_complete(false); // update .complete with error or success
		int n_vid(0), n_pid(0);
		unsigned long n_session(0);
		int n_step_cur(0), n_step_max(0);
		bool b_step_result(false);
		std::wstring s_result_info;//result info or error reason

		std::tie(n_vid, n_pid, std::ignore, n_session, n_step_cur, n_step_max, b_step_result, s_result_info) = nf;

		auto it = m_map.find(n_session);
		if (it == std::end(m_map)) {
			continue;
		}

		auto ptr_param = it->second;
		if (!ptr_param) {
			continue;
		}

		_mp::cio_packet::type_ptr ptr_rsp(ptr_param->get_rsp_packet_before_setting());
		if (!ptr_rsp) {
			continue;
		}
		_mp::cio_packet response(*ptr_rsp);
		if (b_step_result) {
			response.set_data_by_utf8(L"success", false);
			if (n_step_cur >= n_step_max) {
				b_complete = true;// complete with success
			}
		}
		else {
			b_complete = true; //complete with error
			response.set_data_by_utf8(L"error", false);
		}
		
		if (n_step_max > 0 && n_step_cur > 0) {
			// dregon
			// 이 코드는 coffee manager 1'st ed 에서 개발된 js library 호환성을 위한 코드.
			// tools_lpu237_control.js
			// 1'st 에서는 n_step_cur 는 0 ~ n_step_max-1 까지으 범위를 갖음.
			--n_step_cur;
		}
		response.set_data_by_utf8(std::to_wstring(n_step_cur), true);
		response.set_data_by_utf8(std::to_wstring(n_step_max), true);
		response.set_data_by_utf8(s_result_info, true);
		//
		_mp::type_v_buffer v_rsp(0);
		response.get_packet_by_json_format(v_rsp);
		b_result = _mp::cserver::get_instance().send_data_to_client_by_ip4(v_rsp, response.get_session_number());

#if defined(_WIN32)
		if (b_complete) {
			if (b_result) {
				ATLTRACE(L"[I][_EXE] complete - update with success.\n");
			}
			else {
				ATLTRACE(L"[I][_EXE] complete - update with error.\n");
			}
		}
#endif

	} while (false);
	return b_result;
}
cupdater_mgmt::cupdater_mgmt()
{
}

cupdater_mgmt::~cupdater_mgmt()
{
	m_map.clear();
}

cupdater_param::type_ptr cupdater_mgmt::get(unsigned long n_session_number) const
{
	cupdater_param::type_ptr ptr;

	auto it = m_map.find(n_session_number);
	if (it != std::end(m_map)) {
		ptr = it->second;
	}
	return ptr;
}

std::pair<bool, cupdater_param::type_ptr> cupdater_mgmt::insert(unsigned long n_session_number)
{
	bool b_inserted(false);
	cupdater_param::type_ptr ptr;
	auto it = m_map.find(n_session_number);
	if (it == std::end(m_map)) {
		// not found, insert
		ptr = std::make_shared<cupdater_param>(n_session_number);
		m_map.insert(std::make_pair(n_session_number, ptr));
		b_inserted = true;
	}
	else {
		// found
		ptr = it->second;
	}
	return std::make_pair(b_inserted, ptr);
}

bool cupdater_mgmt::erase(unsigned long n_session_number)
{
	auto it = m_map.find(n_session_number);
	if (it != std::end(m_map)) {
		m_map.erase(it);
		return true;
	}
	return false;
}

void cupdater_mgmt::clear()
{
	m_map.clear();
}