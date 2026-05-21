#include <websocket/mp_win_nt.h>

#include <cmap_user_cb.h>


cmap_user_cb::cmap_user_cb() :
	m_n_cur_item_index(0)
{
}

cmap_user_cb::~cmap_user_cb()
{
	m_map_cb.clear();
}

std::tuple<long, _mp::cwait::type_ptr, std::shared_ptr<std::mutex>> cmap_user_cb::add_callback(
	int n_result_index
	, const _mp::type_v_buffer& v_dev_id
	, type_lpu237_tools_callback p_fun
	, void* p_para
	, size_t n_total_phase /*= 0*/
)
{
	long n_item_index(-1);
	_mp::cwait::type_ptr ptr_evt;
	std::shared_ptr<std::mutex> ptr_m;

	do {
		std::lock_guard<std::mutex> lock(m_mutex);
		auto it = m_map_cb.find(m_n_cur_item_index);
		if (it != m_map_cb.end()) {
			continue; // error
		}
		//add new callback
		ptr_evt = std::make_shared<_mp::cwait>();
		ptr_evt->generate_new_event(); // event 는 한개만 사용 할 것이어서, 이벤트 인덱스 번호 저장 불요.

		ptr_m = std::make_shared<std::mutex>();
		m_map_cb[m_n_cur_item_index] = std::make_tuple(
			n_result_index
			, v_dev_id
			, p_fun
			, nullptr
			, nullptr
			, p_para
			, ptr_evt
			, LPU237_TOOLS_RESULT_ERROR
			, ptr_m
			, n_total_phase
		);

		n_item_index = m_n_cur_item_index;
		++m_n_cur_item_index;
		if (m_n_cur_item_index < 0) {
			m_n_cur_item_index = 0; //reset index
		}
	} while (false);
	return std::make_tuple(n_item_index, ptr_evt, ptr_m);
}

std::tuple<long, _mp::cwait::type_ptr, std::shared_ptr<std::mutex>> cmap_user_cb::add_callback(
	int n_result_index
	, const _mp::type_v_buffer& v_dev_id
	, type_lpu237_tools_callback_get_parameter p_fun
	, void* p_para
	, size_t n_total_phase /*= 0*/
)
{
	long n_item_index(-1);
	_mp::cwait::type_ptr ptr_evt;
	std::shared_ptr<std::mutex> ptr_m;

	do {
		std::lock_guard<std::mutex> lock(m_mutex);
		auto it = m_map_cb.find(m_n_cur_item_index);
		if (it != m_map_cb.end()) {
			continue; // error
		}
		//add new callback
		ptr_evt = std::make_shared<_mp::cwait>();
		ptr_evt->generate_new_event(); // event 는 한개만 사용 할 것이어서, 이벤트 인덱스 번호 저장 불요.

		ptr_m = std::make_shared<std::mutex>();
		m_map_cb[m_n_cur_item_index] = std::make_tuple(
			n_result_index
			, v_dev_id
			, nullptr
			, p_fun
			, nullptr
			, p_para
			, ptr_evt
			, LPU237_TOOLS_RESULT_ERROR
			, ptr_m
			, n_total_phase
		);

		n_item_index = m_n_cur_item_index;
		++m_n_cur_item_index;
		if (m_n_cur_item_index < 0) {
			m_n_cur_item_index = 0; //reset index
		}
	} while (false);
	return std::make_tuple(n_item_index, ptr_evt, ptr_m);
}

std::tuple<long, _mp::cwait::type_ptr, std::shared_ptr<std::mutex>> cmap_user_cb::add_callback(
	int n_result_index
	, const _mp::type_v_buffer& v_dev_id
	, type_lpu237_tools_callback_set_parameter p_fun
	, void* p_para
	, size_t n_total_phase /*= 0*/
)
{
	long n_item_index(-1);
	_mp::cwait::type_ptr ptr_evt;
	std::shared_ptr<std::mutex> ptr_m;

	do {
		std::lock_guard<std::mutex> lock(m_mutex);
		auto it = m_map_cb.find(m_n_cur_item_index);
		if (it != m_map_cb.end()) {
			continue; // error
		}
		//add new callback
		ptr_evt = std::make_shared<_mp::cwait>();
		ptr_evt->generate_new_event(); // event 는 한개만 사용 할 것이어서, 이벤트 인덱스 번호 저장 불요.

		ptr_m = std::make_shared<std::mutex>();
		m_map_cb[m_n_cur_item_index] = std::make_tuple(
			n_result_index
			, v_dev_id
			, nullptr
			, nullptr
			, p_fun
			, p_para
			, ptr_evt
			, LPU237_TOOLS_RESULT_ERROR
			, ptr_m
			, n_total_phase
		);

		n_item_index = m_n_cur_item_index;
		++m_n_cur_item_index;
		if (m_n_cur_item_index < 0) {
			m_n_cur_item_index = 0; //reset index
		}
	} while (false);
	return std::make_tuple(n_item_index, ptr_evt, ptr_m);
}


bool cmap_user_cb::change_callback(
	long n_item_index
	, int n_new_result_index
	, const _mp::type_v_buffer& v_new_dev_id
	, type_lpu237_tools_callback p_new_fun
	, type_lpu237_tools_callback_get_parameter p_fun_get
	, type_lpu237_tools_callback_set_parameter p_fun_set
	, void* p_new_para
)
{
	bool b_result(false);

	do {

		std::lock_guard<std::mutex> lock(m_mutex);
		auto it = m_map_cb.find(n_item_index);
		if (it == m_map_cb.end()) {
			continue; // not found
		}
		//change result index
		std::get<0>(it->second) = n_new_result_index;
		std::get<1>(it->second) = v_new_dev_id;
		std::get<2>(it->second) = p_new_fun;
		std::get<3>(it->second) = p_fun_get;
		std::get<4>(it->second) = p_fun_set;
		std::get<5>(it->second) = p_new_para;
		b_result = true;
	} while (false);
	return b_result;
}

bool cmap_user_cb::change_result_index(long n_item_index, int n_new_result_index)
{
	bool b_result(false);

	do {

		std::lock_guard<std::mutex> lock(m_mutex);
		auto it = m_map_cb.find(n_item_index);
		if (it == m_map_cb.end()) {
			continue; // not found
		}
		//change result index
		std::get<0>(it->second) = n_new_result_index;
		b_result = true;
	} while (false);
	return b_result;
}

bool cmap_user_cb::change_result_index(
	long n_item_index
	, int n_new_result_index
	, size_t n_new_total_phase
)
{
	bool b_result(false);

	do {

		std::lock_guard<std::mutex> lock(m_mutex);
		auto it = m_map_cb.find(n_item_index);
		if (it == m_map_cb.end()) {
			continue; // not found
		}
		//change result index
		std::get<0>(it->second) = n_new_result_index;

		// change total phase
		std::get<9>(it->second) = n_new_total_phase;
		b_result = true;
	} while (false);
	return b_result;
}

bool cmap_user_cb::remove_callback(long n_item_index)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	return _remove_callback(n_item_index);
}

bool cmap_user_cb::_remove_callback(long n_item_index)
{
	bool b_reslt(false);

	do {
		auto it = m_map_cb.find(n_item_index);
		if (it == m_map_cb.end()) {
			continue;
		}
		m_map_cb.erase(it);
		b_reslt = true;
	} while (false);
	return b_reslt;
}


std::tuple<bool, _mp::cwait::type_ptr, std::shared_ptr<std::mutex>> cmap_user_cb::get_callback(
	long n_item_index
	, bool b_remove_after_get
	, int& n_result_index
	, _mp::type_v_buffer& v_dev_id
	, size_t& n_total_phase
	, type_lpu237_tools_callback& p_fun
	, type_lpu237_tools_callback_get_parameter& p_fun_get
	, type_lpu237_tools_callback_set_parameter& p_fun_set
	, void*& p_para
)
{
	bool b_reslt(false);
	_mp::cwait::type_ptr ptr_evt;
	std::shared_ptr<std::mutex> ptr_m;

	do {
		std::lock_guard<std::mutex> lock(m_mutex);
		auto it = m_map_cb.find(n_item_index);
		if (it == m_map_cb.end()) {
			continue;
		}

		n_result_index = std::get<0>(it->second);
		v_dev_id = std::get<1>(it->second);
		p_fun = std::get<2>(it->second);
		p_fun_get = std::get<3>(it->second);
		p_fun_set = std::get<4>(it->second);
		p_para = std::get<5>(it->second);
		ptr_evt = std::get<6>(it->second);
		ptr_m = std::get<8>(it->second);

		n_total_phase = std::get<9>(it->second);

		if (b_remove_after_get) {
			m_map_cb.erase(it); // 여기서 지워도 ptr_evt 은 shared_ptr 이므로 제거 되지 않는다.
		}
		b_reslt = true;
	} while (false);
	return std::make_tuple(b_reslt, ptr_evt, ptr_m);
}

_mp::cwait::type_ptr cmap_user_cb::start_cancel()
{
	std::lock_guard<std::mutex> lock(m_mutex);

	if (!m_ptr_wait_cancel) {
		m_ptr_wait_cancel = std::make_shared<_mp::cwait>();
		m_ptr_wait_cancel->generate_new_event(); // event 는 한개만 사용 할 것이어서, 이벤트 인덱스 번호 저장 불요.
	}
	return m_ptr_wait_cancel;
}

bool cmap_user_cb::is_cancel_requested()
{
	bool b_cancel_is_requested_but_not_yet_canceled(false);
	std::lock_guard<std::mutex> lock(m_mutex);
	if (m_ptr_wait_cancel) {
		b_cancel_is_requested_but_not_yet_canceled = true;
	}
	return b_cancel_is_requested_but_not_yet_canceled;
}

bool cmap_user_cb::cancel_done()
{
	bool b_cancel(false);

	std::lock_guard<std::mutex> lock(m_mutex);
	if (m_ptr_wait_cancel) {
		m_ptr_wait_cancel->set();
		m_ptr_wait_cancel.reset(); // 이기서 shared_ptr release 해도, cancel 를 기다리는 shared_ptr 이 메모리 유지 하므로 OK.
		b_cancel = true;
	}

	return b_cancel;
}
