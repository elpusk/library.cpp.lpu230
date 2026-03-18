#include <websocket/mp_win_nt.h>

#include <cmap_user_cb.h>


cmap_user_cb::cmap_user_cb() : m_n_cur_item_index(0)
{
}

cmap_user_cb::~cmap_user_cb()
{
	m_map_cb.clear();
}

long cmap_user_cb::add_callback(
	int n_result_index
	, const _mp::type_v_buffer& v_dev_id
	, type_lpu237_fw_callback p_fun
	, void* p_para
	, HWND hWnd
	, UINT uMsg
	, const wchar_t* sRomFileName
	, unsigned long dwIndex
)
{
	long n_item_index(-1);

	do {
		std::lock_guard<std::mutex> lock(m_mutex);
		auto it = m_map_cb.find(m_n_cur_item_index);
		if (it != m_map_cb.end()) {
			continue; // error
		}
		//add new callback
		m_map_cb[m_n_cur_item_index] = std::make_tuple(
			n_result_index
			, v_dev_id
			,p_fun
			, p_para
			, hWnd
			, uMsg
			, std::wstring(sRomFileName)
			, dwIndex
		);
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
	, const _mp::type_v_buffer& v_new_dev_id
	, type_lpu237_fw_callback p_new_fun
	, void* p_new_para
	, HWND h_new_Wnd
	, UINT u_new_Msg
	, const wchar_t* s_new_RomFileName
	, unsigned long dw_new_Index
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
		std::get<3>(it->second) = p_new_para;
		std::get<4>(it->second) = h_new_Wnd;
		std::get<5>(it->second) = u_new_Msg;
		std::get<6>(it->second) = std::wstring(s_new_RomFileName);
		std::get<7>(it->second) = dw_new_Index;
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
	, _mp::type_v_buffer& v_dev_id
	, type_lpu237_fw_callback& p_fun
	, void*& p_para
	, HWND& h_new_Wnd
	, UINT& u_new_Msg
	, std::wstring& s_new_RomFileName
	, unsigned long& dw_new_Index
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
		v_dev_id = std::get<1>(it->second);
		p_fun = std::get<2>(it->second);
		p_para = std::get<3>(it->second);
		h_new_Wnd = std::get<4>(it->second);
		u_new_Msg = std::get<5>(it->second);
		s_new_RomFileName = std::get<6>(it->second);
		dw_new_Index = std::get<7>(it->second);
		if (b_remove_after_get) {
			m_map_cb.erase(it);
		}
		b_reslt = true;
	} while (false);
	return b_reslt;
}
