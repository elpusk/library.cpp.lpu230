#pragma once

#include <map>
#include <mutex>

#include <mp_casync_parameter_result.h>
#include <tg_lpu237_ibutton.h>

class cmap_user_cb
{
public:
	virtual ~cmap_user_cb();

	cmap_user_cb();

	/**
	* @brief add callback to map
	* @param n_result_index - result index of callback function
	* @param h_dev - device handle, can be INVALID_HANDLE_VALUE
	* @param p_fun - callback function
	* @param p_para - user parameter of callback function
	* @return item index of callback function, or -1 if error.
	*/
	long add_callback(int n_result_index, HANDLE h_dev, _mp::casync_parameter_result::type_callback p_fun, void* p_para);

	/**
	* @brief add callback to map
	* @param n_item_index - item index of callback function
	* @param n_new_result_index - new result index of callback function
	* @param h_new_dev - device handle, can be INVALID_HANDLE_VALUE
	* @param p_new_fun - new  callback function
	* @param p_new_para - new  user parameter of callback function
	* @return false if error, true if success.
	*/
	bool change_callback(long n_item_index,int n_new_result_index, HANDLE h_new_dev, _mp::casync_parameter_result::type_callback p_new_fun, void* p_new_para);

	/**
	* @brief change result index of callback function
	* @param n_item_index - item index of callback function
	* @param n_new_result_index - new result index of callback function
	*/
	bool change_result_index(long n_item_index, int n_new_result_index);

	bool remove_callback(long n_item_index);
	bool get_callback(
		long n_item_index
		, bool b_remove_after_get
		, int &n_result_index
		, HANDLE& h_dev
		,_mp::casync_parameter_result::type_callback& p_fun
		, void*& p_para
	);

private:
	bool _remove_callback(long n_item_index);

private:
	// 0- result index, 1 - device handle, 2 - callback function, 3 - user parameter of callback function
	typedef std::tuple<int, HANDLE ,_mp::casync_parameter_result::type_callback, void*> _type_tuple_item;

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

