#pragma once

#include <map>
#include <mutex>

#include <mp_casync_parameter_result.h>
#include <tg_lpu237_fw.h>

class cmap_user_cb
{
public:
	virtual ~cmap_user_cb();

	cmap_user_cb();

	/**
	* @brief add callback to map
	* @param n_result_index - result index of callback function
	* @param v_dev_id - device ID, the size of vector can be 0
	* @param p_fun - callback function
	* @param p_para - user parameter of callback function
	* @return item index of callback function, or -1 if error.
	*/
	long add_callback(
		int n_result_index
		, const _mp::type_v_buffer & v_dev_id
		, type_lpu237_fw_callback p_fun
		, void* p_para
		, HWND hWnd
		, UINT uMsg
		, const wchar_t* sRomFileName
		, unsigned long dwIndex
	);

	/**
	* @brief add callback to map
	* @param n_item_index - item index of callback function
	* @param n_new_result_index - new result index of callback function
	* @param v_new_dev_id - device ID, the size of vector can be 0
	* @param p_new_fun - new  callback function
	* @param p_new_para - new  user parameter of callback function
	* @return false if error, true if success.
	*/
	bool change_callback(
		long n_item_index
		, int n_new_result_index
		, const _mp::type_v_buffer& v_new_dev_id
		, type_lpu237_fw_callback p_new_fun
		, void* p_new_para
		, HWND h_new_Wnd
		, UINT u_new_Msg
		, const wchar_t* s_new_RomFileName
		, unsigned long dw_new_Index
	);

	/**
	* @brief change result index of callback function
	* @param n_item_index - item index of callback function
	* @param n_new_result_index - new result index of callback function
	*/
	bool change_result_index(
		long n_item_index
		, int n_new_result_index
	);

	bool remove_callback(long n_item_index);
	bool get_callback(
		long n_item_index
		, bool b_remove_after_get
		, int &n_result_index
		, _mp::type_v_buffer& v_dev_id
		, type_lpu237_fw_callback& p_fun
		, void*& p_para
		, HWND& h_new_Wnd
		, UINT& u_new_Msg
		, std::wstring& s_new_RomFileName
		, unsigned long& dw_new_Index
	);

private:
	bool _remove_callback(long n_item_index);

private:
	// 0- result index, 
	// 1 - device id vector, 
	// 2 - callback function, 
	// 3 - user parameter of callback function
	// 4 - callback window handle(only windows system)
	// 5 - callback window message(only windows system)
	// 6 - rom file path
	// 7 - firmware index in rom file.
	typedef std::tuple<
		int
		, _mp::type_v_buffer 
		, type_lpu237_fw_callback
		, void*
		, HWND
		, UINT
		,std::wstring
		, unsigned long
	> _type_tuple_item;

	// key - item index
	typedef std::map<long, cmap_user_cb::_type_tuple_item  > _type_map_cb;

private:
	std::mutex m_mutex;
	cmap_user_cb::_type_map_cb m_map_cb;
	long m_n_cur_item_index;

private:
	cmap_user_cb(const cmap_user_cb&) = delete;
	cmap_user_cb& operator=(const cmap_user_cb&) = delete;
};

