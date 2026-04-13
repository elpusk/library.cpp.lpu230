#pragma once

#include <map>
#include <mutex>

#include <mp_casync_parameter_result.h>
#include <tg_lpu237_fw.h>

class cmap_user_cb
{
public:
	// by lpu230_fw_api_UM_KOR_V5.0.pdf
	// 정상 처리의 경우 LPU237 은 LPU237_FW_WPARAM_FOUND_BL 한 번
	// , LPU237_FW_WPARAM_SECTOR_ERASE 한 번
	// , LPU237_FW_WPARAM_SECTOR_WRITE 여섯번 - 문서에서 잘못됨. 실제 테스트 결과, LPC1343 의 경우 LPU237_FW_WPARAM_SECTOR_WRITE 는 7번 전달됨. MH1902T의 경우 N(가변) 번 전달됨.
	// , LPU237_FW_WPARAM_COMPLETE 한 번이 전달 된다.
	// lpu230_fw_api_UM_KOR_V5.0.pdf 에 있는 위 문장은 LPC1343 마이컴만을 기준으로 한 것으로.
	// 업데이트 프로그램이 lpu230_update 로 독립해서 완전히 변경되었음.
	// 따라서 위 문자에 따라 프로그래밍한 application 은 문제가 생길 수 있다. 
	// 호환성을 위해 메시지 카운터가 필요하다고 판단하여, 추가함.

	// std::get<0> - LPU237_FW_WPARAM_SECTOR_ERASE wparam counter
	// std::get<1> - LPU237_FW_WPARAM_SECTOR_WRITE wparam counter
	// std::get<2> - LPU237_FW_WPARAM_COMPLETE wparam counter
	typedef std::tuple<	int, int, int> type_tuple_msg_counter;

public:
	virtual ~cmap_user_cb();

	cmap_user_cb();

	/**
	* @brief add callback to map
	* @param n_result_index - result index of callback function
	* @param v_dev_id - device ID, the size of vector can be 0
	* @param p_fun - callback function
	* @param p_para - user parameter of callback function
	* @return get<0> - item index of callback function, or -1 if error.
	* 
	*	get<1> - for sync prrocessing, complete event(success or error)
	* 
	*	get<2> - std::shared_ptr<std::mutex> for sync between add_callback() & get_callback().
	*/
	std::tuple<long,_mp::cwait::type_ptr, std::shared_ptr<std::mutex>> add_callback(
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

	/**
	* @brief get callback to map
	* @return get<0> - true : found item.
	*
	*	get<1> - for sync prrocessing, complete event(success or error)
	* 
	*	get<2> - std::shared_ptr<std::mutex> for sync between add_callback() & get_callback().
	*/
	std::tuple<bool, _mp::cwait::type_ptr, std::shared_ptr<std::mutex>> get_callback(
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

	void set_msg_counter(
		long n_item_index
		, int n_sector_erase_cnt
		, int n_sector_write_cnt
		, int n_complete_cnt
	);

	/**
	* @brief get message counter.
	* @param n_item_index - item index of callback function
	* @return tuple of (LPU237_FW_WPARAM_SECTOR_ERASE wparam counter, LPU237_FW_WPARAM_SECTOR_WRITE wparam counter, LPU237_FW_WPARAM_COMPLETE wparam counter)
	*/
	cmap_user_cb::type_tuple_msg_counter get_msg_counter(long n_item_index);

	/**
	* @brief increase message counter by given increment values.
	* @param n_item_index - item index of callback function
	* @param n_sector_erase_inc - increment value for LPU237_FW_WPARAM_SECTOR_ERASE wparam counter canbe a negative value.
	* @param n_sector_write_inc - increment value for LPU237_FW_WPARAM_SECTOR_WRITE wparam counter canbe a negative value.
	* @param n_complete_inc - increment value for LPU237_FW_WPARAM_COMPLETE wparam counter canbe a negative value.
	*/
	void inc_msg_counter(
		long n_item_index
		, int n_sector_erase_inc
		, int n_sector_write_inc
		, int n_complete_inc
	);
	
	void set_mode(int n_mode);

	int get_mode() const;

	/**
	* @brief get a sync processing result.
	* @param n_item_index - item index of callback function
	* @return LPU237_FW_RESULT_SUCCESS, LPU237_FW_RESULT_ERROR, LPU237_FW_RESULT_CANCEL, LPU237_FW_RESULT_TIMEOUT or LPU237_FW_RESULT_NO_MSR
	*/
	unsigned long get_sync_result(long n_item_index);

	/**
	* @brief set a sync processing result.
	* @param n_item_index - item index of callback function
	* @param dw_result - LPU237_FW_RESULT_SUCCESS, LPU237_FW_RESULT_ERROR, LPU237_FW_RESULT_CANCEL, LPU237_FW_RESULT_TIMEOUT or LPU237_FW_RESULT_NO_MSR
	*/
	void set_sync_result(long n_item_index, unsigned long dw_result);

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
	// 8 - for sync, complete event
	// 9 - for sync, processing result
	// 10 - add_callback() 후, result indx 가 업데이트 전에, 다른 쓰레드에서 get_callback() 이 불려져서, 잘못된 result index 얻는 갓을 방지 하기 위한 동기화 object
	typedef std::tuple<
		int
		, _mp::type_v_buffer 
		, type_lpu237_fw_callback
		, void*
		, HWND
		, UINT
		,std::wstring
		, unsigned long
		, _mp::cwait::type_ptr
		, unsigned long
		, std::shared_ptr<std::mutex>
	> _type_tuple_item;

	// key - item index
	typedef std::map<long, cmap_user_cb::_type_tuple_item  > _type_map_cb;

	// key - item index
	typedef std::map<long, cmap_user_cb::type_tuple_msg_counter  > _type_map_msg_cnt;

private:
	std::mutex m_mutex;
	cmap_user_cb::_type_map_cb m_map_cb;
	cmap_user_cb::_type_map_msg_cnt m_map_msg_cnt;
	long m_n_cur_item_index;
	int m_n_mode; 

private:
	cmap_user_cb(const cmap_user_cb&) = delete;
	cmap_user_cb& operator=(const cmap_user_cb&) = delete;
};

