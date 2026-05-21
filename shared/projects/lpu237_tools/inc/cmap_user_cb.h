#pragma once

#include <map>
#include <mutex>

#include <mp_casync_parameter_result.h>
#include <tg_lpu237_tools.h>

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
	* @param n_total_phase - total phase of transaction. 대부분 이 함수 실행시에 몰라서 0으로 설정.
	* @return get<0> - item index of callback function, or -1 if error.
	*
	*	get<1> - for sync prrocessing, complete event(success or error)
	*
	*	get<2> - std::shared_ptr<std::mutex> for sync between add_callback() & get_callback().
	*/
	std::tuple<long, _mp::cwait::type_ptr, std::shared_ptr<std::mutex>> add_callback(
		int n_result_index
		, const _mp::type_v_buffer& v_dev_id
		, type_lpu237_tools_callback p_fun
		, void* p_para
		, size_t n_total_phase = 0
	);
	std::tuple<long, _mp::cwait::type_ptr, std::shared_ptr<std::mutex>> add_callback(
		int n_result_index
		, const _mp::type_v_buffer& v_dev_id
		, type_lpu237_tools_callback_get_parameter p_fun
		, void* p_para
		, size_t n_total_phase = 0
	);
	std::tuple<long, _mp::cwait::type_ptr, std::shared_ptr<std::mutex>> add_callback(
		int n_result_index
		, const _mp::type_v_buffer& v_dev_id
		, type_lpu237_tools_callback_set_parameter p_fun
		, void* p_para
		, size_t n_total_phase = 0
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
		, type_lpu237_tools_callback p_new_fun
		, type_lpu237_tools_callback_get_parameter p_fun_get
		, type_lpu237_tools_callback_set_parameter p_fun_set
		, void* p_new_para
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

	/**
	* @brief change result index of callback function
	* @param n_item_index - item index of callback function
	* @param n_new_result_index - new result index of callback function
	* @param n_new_total_phase - new total phase of transaction. 대부분 aync 시작 후, return 값으로 알아서 이때 설정.
	*/
	bool change_result_index(
		long n_item_index
		, int n_new_result_index
		, size_t n_new_total_phase
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
		, int& n_result_index
		, _mp::type_v_buffer& v_dev_id
		, size_t& n_total_phase
		, type_lpu237_tools_callback& p_fun
		, type_lpu237_tools_callback_get_parameter& p_fun_get
		, type_lpu237_tools_callback_set_parameter& p_fun_set
		, void*& p_para
	);

	/**
	* @brief 비동기 방식으로 실행 중인, transaction 을 중지 시킨다.
	* 
	*	callback이 호출 되면, callback 함수에서는 이 함수에 의해 생성된 ptr wait 객체가 있는지 확인하고
	* 
	*	ptr wait 가 있으면, 현재 실행한 phase 가 성공이고, 다음 phase 가 남아 있어도, 
	*
	*	cancel 하고,  ptr wait 을 set 해주어서, 이 함수로 호출하고, cancel 이 되기를 기다리는 쓰레드의 기다림을 종료 시켜야 한다.
	* 
	* @param _mp::cwait::type_ptr 가 empty 이면 에러, 그렇지 않으면, 이 returuned ptr 을 가지고, cancel 완료 될때 까지 기다림.
	*/
	_mp::cwait::type_ptr start_cancel();

	/**
	* @brief start_cancel() 가 실행 되고 아직 cancel_done() 이 호출된 적이 없는가 검사
	* @return 
	* 
	*	true - start_cancel() 가 실행되고, cancel_done() 이 호출된 적 없음.
	* 
	*	false - start_cancel() 실행된적이 없거나, 실행되고, cancel_done() 이 호출이 호출됨.
	*/
	bool is_cancel_requested();

	/**
	* @brief start_cancel() 에 의해 시작된 cancel 를 callback 에서 이 함수를 불러 
	* 
	*	wait event set 해서 기다림 종료 함.
	* 
	* @return true - success cancel, false - no need cancel(not requested cancelation).
	*/
	bool cancel_done();


private:
	bool _remove_callback(long n_item_index);

private:

	// 0- result index, 
	// 1 - device id vector, 
	// 2 - callback function, 
	// 3 - get all parameter callback function, 
	// 4 - set all parameter callback function, 
	// 5 - user parameter of callback function( 2 ~ 4 )
	// 6 - for sync, complete event
	// 7 - for sync, processing result
	// 8 - add_callback() 후, result indx 가 업데이트 전에, 다른 쓰레드에서 get_callback() 이 불려져서, 잘못된 result index 얻는 갓을 방지 하기 위한 동기화 object
	// 9 - total phase number in this transaction.
	typedef std::tuple<
		int
		, _mp::type_v_buffer
		, type_lpu237_tools_callback
		, type_lpu237_tools_callback_get_parameter
		, type_lpu237_tools_callback_set_parameter
		, void*
		, _mp::cwait::type_ptr
		, unsigned long
		, std::shared_ptr<std::mutex>
		, size_t
	> _type_tuple_item;

	// key - item index
	typedef std::map<long, cmap_user_cb::_type_tuple_item  > _type_map_cb;

private:
	std::mutex m_mutex;
	cmap_user_cb::_type_map_cb m_map_cb;
	long m_n_cur_item_index;

	_mp::cwait::type_ptr m_ptr_wait_cancel;

private:
	cmap_user_cb(const cmap_user_cb&) = delete;
	cmap_user_cb& operator=(const cmap_user_cb&) = delete;
};

