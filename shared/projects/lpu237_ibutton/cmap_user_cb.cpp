#include <websocket/mp_win_nt.h>

#include <cmap_user_cb.h>


cmap_user_cb::cmap_user_cb() : m_n_cur_item_index(0)
{
}

cmap_user_cb::~cmap_user_cb()
{
	m_map_cb.clear();
}

long cmap_user_cb::add_callback(int n_result_index, HANDLE h_dev,_mp::casync_parameter_result::type_callback p_fun, void* p_para)
{
	long n_item_index(-1);

	do {
		std::lock_guard<std::mutex> lock(m_mutex);
		auto it = m_map_cb.find(m_n_cur_item_index);
		if (it != m_map_cb.end()) {
			continue; // error
		}
		//add new callback
		m_map_cb[m_n_cur_item_index] = std::make_tuple(n_result_index,h_dev,p_fun, p_para);
		n_item_index = m_n_cur_item_index;
		++m_n_cur_item_index;
		if (m_n_cur_item_index < 0) {
			m_n_cur_item_index = 0; //reset index
		}
	} while (false);
	return n_item_index;
}

bool cmap_user_cb::change_callback(
	long n_item_index
	, int n_new_result_index
	, HANDLE h_new_dev
	, _mp::casync_parameter_result::type_callback p_new_fun
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
		std::get<1>(it->second) = h_new_dev;
		std::get<2>(it->second) = p_new_fun;
		std::get<3>(it->second) = p_new_para;
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


bool cmap_user_cb::get_callback(
	long n_item_index
	, bool b_remove_after_get
	, int& n_result_index
	, HANDLE& h_dev
	,_mp::casync_parameter_result::type_callback& p_fun
	, void*& p_para
)
{
	bool b_reslt(false);

	do {
		std::lock_guard<std::mutex> lock(m_mutex);
		auto it = m_map_cb.find(n_item_index);
		if (it == m_map_cb.end()) {
			continue;
		}

		n_result_index = std::get<0>(it->second);
		h_dev = std::get<1>(it->second);
		p_fun = std::get<2>(it->second);
		p_para = std::get<3>(it->second);
		if (b_remove_after_get) {
			m_map_cb.erase(it);
		}
		b_reslt = true;
	} while (false);
	return b_reslt;
}
