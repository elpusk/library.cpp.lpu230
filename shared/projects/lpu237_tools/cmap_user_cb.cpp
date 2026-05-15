#include <websocket/mp_win_nt.h>

#include <cmap_user_cb.h>


cmap_user_cb::cmap_user_cb() :
	m_n_cur_item_index(0)
{
}

cmap_user_cb::~cmap_user_cb()
{
	m_map_cb.clear();
	m_map_msg_cnt.clear();
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
		m_map_msg_cnt[m_n_cur_item_index] = std::make_tuple(0, 0, 0); // initialize message counter

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
		m_map_msg_cnt[m_n_cur_item_index] = std::make_tuple(0, 0, 0); // initialize message counter

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
		m_map_msg_cnt[m_n_cur_item_index] = std::make_tuple(0, 0, 0); // initialize message counter

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

unsigned long cmap_user_cb::get_sync_result(long n_item_index)
{
	unsigned long n_result(LPU237_TOOLS_RESULT_ERROR);
	do {
		std::lock_guard<std::mutex> lock(m_mutex);
		auto it = m_map_cb.find(n_item_index);
		if (it == m_map_cb.end()) {
			continue; // not found
		}
		n_result = std::get<7>(it->second);
	} while (false);
	return n_result;
}

void cmap_user_cb::set_sync_result(long n_item_index, unsigned long dw_result)
{
	do {
		std::lock_guard<std::mutex> lock(m_mutex);
		auto it = m_map_cb.find(n_item_index);
		if (it == m_map_cb.end()) {
			continue; // not found
		}
		std::get<7>(it->second) = dw_result;
	} while (false);
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
		auto it_cnt = m_map_msg_cnt.find(n_item_index);
		if (it_cnt != m_map_msg_cnt.end()) {
			m_map_msg_cnt.erase(it_cnt);
		}
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

void cmap_user_cb::set_msg_counter(long n_item_index, int n_sector_erase_cnt, int n_sector_write_cnt, int n_complete_cnt)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	auto it = m_map_msg_cnt.find(n_item_index);
	if (it != m_map_msg_cnt.end()) {
		std::get<0>(it->second) = n_sector_erase_cnt;
		std::get<1>(it->second) = n_sector_write_cnt;
		std::get<2>(it->second) = n_complete_cnt;
	}
}

cmap_user_cb::type_tuple_msg_counter cmap_user_cb::get_msg_counter(long n_item_index)
{
	cmap_user_cb::type_tuple_msg_counter cnt(-1, -1, -1);
	std::lock_guard<std::mutex> lock(m_mutex);
	auto it = m_map_msg_cnt.find(n_item_index);
	if (it != m_map_msg_cnt.end()) {
		cnt = it->second;
	}
	return cnt;
}

void cmap_user_cb::inc_msg_counter(long n_item_index, int n_sector_erase_inc, int n_sector_write_inc, int n_complete_inc)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	auto it = m_map_msg_cnt.find(n_item_index);
	if (it != m_map_msg_cnt.end()) {
		std::get<0>(it->second) += n_sector_erase_inc;
		std::get<1>(it->second) += n_sector_write_inc;
		std::get<2>(it->second) += n_complete_inc;
	}
}
