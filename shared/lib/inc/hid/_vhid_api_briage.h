#pragma once

/**
* don't include in header file(.h file).
* include only body file.(.cpp file)
*/
#include <set>
#include <map>
#include <utility>
#include <memory>
#include <thread>
#include <mutex>
#include <atomic>
#include <functional>
#include <deque>

#include <hidapi.h>

#include <mp_type.h>
#include <mp_cwait.h>
#include <mp_cqueue.h>
#include <hid/_vhid_info.h>
#include <hid/_hid_api_briage.h>

/**
* the instance of this class must be created only one in the process.
*/
class _vhid_api_briage: public _hid_api_briage
{
public:
	typedef	std::shared_ptr<_vhid_api_briage>	type_ptr;

private:
	class _q_item {
	public:
		typedef	enum : int {
			cmd_undefined = -1,
			cmd_set_nonblocking = 1,
			cmd_get_report_descriptor,
			cmd_write,
			cmd_read_timeout,
			cmd_read
		}	type_cmd;
	public:
		typedef	std::shared_ptr<_vhid_api_briage::_q_item>	type_ptr;

	public:
		_q_item(int n_compositive_map_index);
		~_q_item();

		/**
		* set blocking mode.(hid_set_nonblocking())
		* @param
		* nonblock:int - 0 is blocking mode, 1 is nonblocking mode
		*/
		void setup_for_set_nonblocking(int nonblock);
		void setup_for_get_report_descriptor( unsigned char* buf, size_t buf_size);
		void setup_for_write(const unsigned char* data, size_t length, _hid_api_briage::type_next_io next);
		void setup_for_read(unsigned char* data, size_t length, size_t n_report);

		_vhid_api_briage::_q_item::type_cmd get_cmd() const;
		_mp::type_v_buffer get_tx() const;
		_hid_api_briage::type_next_io get_next_io_type() const;

		size_t get_rx_size() const;
		_vhid_api_briage::_q_item& set_rx(const _mp::type_v_buffer& v_rx);
		_vhid_api_briage::_q_item& append_rx(const _mp::type_v_buffer& v_rx,bool b_add_to_head = false);
		_vhid_api_briage::_q_item& set_result(int n_result);

		size_t get_user_rx_buffer_size() const;
		unsigned char* get_user_rx_buffer();

		int get_param_nonblock() const;

		int get_map_index() const;

		size_t get_size_report_in() const;
		int get_rx_timeout_mmsec() const;

		void set_start_time();
		double get_elapsed_usec_time();
		/**
		* reutrn first - result code, second rx data
		*/
		std::pair<int, _mp::type_v_buffer> get_result() const;

		bool waits();
		void set_event();
	private:
		_q_item::type_cmd m_cmd;
		_mp::type_v_buffer m_v_tx, m_v_rx;

		_hid_api_briage::type_next_io m_next;
		size_t m_n_in_report;
		int m_n_rx_timeout_mmsec;
		
		std::chrono::high_resolution_clock::time_point m_start_time;

		int m_n_result;
		unsigned char* m_ps_rx; //user memory for receving
		size_t m_n_rx; //buffer size  of m_ps_rx

		int m_n_nonblock;
		int m_n_milliseconds;

		_mp::cwait m_waiter;
		int m_n_event;
		int m_n_compositive_map_index; //for debugging

	private://don't call these methods
		_q_item();
	};

	class _q_worker {
	public:
		typedef	std::shared_ptr<_vhid_api_briage::_q_worker>	type_ptr;
	private:
		typedef std::deque<_vhid_api_briage::_q_item::type_ptr> _type_q;
		typedef std::shared_ptr<_vhid_api_briage::_q_worker::_type_q> _type_ptr_q;
		typedef std::map<int, _vhid_api_briage::_q_worker::_type_ptr_q> _type_map_ptr_q;//key-compositive index
	private:
		enum {
			_const_worker_interval_mmsec = 30
		};
		enum {
			_const_txrx_pair_tx_interval_mmsec = 1
		};

	public:
		_q_worker(int n_primitive_map_index, _vhid_api_briage *p_api_briage);
		~_q_worker();

		bool push(const _vhid_api_briage::_q_item::type_ptr &ptr_item);

	private:
		void _pop_all(int n_result, const _mp::type_v_buffer& v_rx = _mp::type_v_buffer(0));

		bool _try_pop(_q_worker::_type_map_ptr_q& map_ptr_q);
		

		void _worker(_vhid_api_briage* p_api_briage);

		/**
		* @brief this method will be called by _worker() on idle status
		* @return first - int result value, second rx data, third - true(pass), false(use result)
		*/
		std::tuple<int, _mp::type_v_buffer, bool> _worker_on_idle_status(
			const _vhid_api_briage::_q_worker::_type_map_ptr_q& cur_map_ptr_q,
			_hid_api_briage* p_api_briage
		);

		void _process_req_and_erase(
			_vhid_api_briage::_q_worker::_type_map_ptr_q& cur_map_ptr_q,
			_hid_api_briage* p_api_briage,
			_vhid_api_briage::_q_item::type_cmd cmd);

		int _receive_for_txrx_pair(_hid_api_briage* p_api_briage, unsigned char* ps_rx, size_t n_rx, size_t n_size_report_in);

	private:
		std::shared_ptr<std::thread> m_ptr_worker;
		std::atomic<bool> m_b_run_worker;

		std::mutex m_mutex;
		_q_worker::_type_map_ptr_q m_map_ptr_q;
		std::deque<int> m_q_map_index; //int -> compositive map index. older value has the high priority
		//

		int m_n_primitive_map_index;

	private://don't call these methods
		_q_worker();
	};

private:
	//key is primitive map index.
	//value is pair of vhid_info ptr and worker ptr
	typedef std::map<int, std::pair<_vhid_info::type_ptr, _q_worker::type_ptr> > _type_map_ptr_vhid_info_ptr_worker;

	// key is compositive map index
	typedef std::map<int, _vhid_info::type_ptr > _type_compositive_map_ptr_vhid_info;

	// key is primitive map index
	typedef std::map<int, _q_worker::type_ptr > _type_primitive_map_ptr_primitive_worker;

public:
	_vhid_api_briage();

	virtual ~_vhid_api_briage();

	/**
	* @brief get connected information of devices.
	* 
	* @param none
	* 
	* @return  each item's 
	*
	*	1'st - std::string, device path, 
	* 
	*	2'nd - unsigned short, usb vendor id,
	* 
	*	3'th - unsigned short, usb product id, 
	* 
	*	4'th - int, usb interface number, 
	* 
	*	5'th - std::string, extra data
	*/
	virtual std::set< std::tuple<std::string, unsigned short, unsigned short, int, std::string> >hid_enumerate();

	/**
	* @brief check is open or not
	*
	* @param path - primitive or composite path
	*
	* @return tuple type
	*
	*   first true - open, false not open or error.
	*
	*   second - the opened device of the index of map.(primitive index)
	*
	*   third - true(exclusive open), false(shared open or not open )
	*/
	virtual std::tuple<bool, int, bool> is_open(const char* path) const;

	/**
	* @brief open device with path. (hid_open_path())
	* 
	* A primitive type device and all compositive types 
	* that share the primitive type device must be able to open simultaneously,
	* regardless of shared open support. 
	* Additionally, if shared open support is provided, 
	* it must be possible to open it more than twice simultaneously.
	* 
	* @param path - primitive or composite path
	* 
	* @return compositive type map index
	* 
	*	this value must be const_map_index_min~const_map_index_max, Multiples of const_map_index_inc_unit.
	* 
	*	this index cannot be index of map directly. but you can get map index(primitive index) from this value.
	* 
	*	Use _vhid_info::get_primitive_map_index_from_compositive_map_index() for getting map index(primitive index) from this value.
	* 
	*	error : -1
	*/
	virtual int api_open_path(const char* path);

	/**
	* @brief close (hid_close())
	* 
	* @param n_map_index - primitive or compositive map index.
	*/
	virtual void api_close(int n_map_index);

	/**
	* @brief set blocking mode.(hid_set_nonblocking())
	* 
	* @param n_map_index:int - primitive or compositive map index.
	* 
	* @param nonblock:int - 0 is blocking mode, 1 is nonblocking mode
	* 
	* @return int 
	*	0 - success
	* 
	*	-1 - error
	*/
	virtual int api_set_nonblocking(int n_map_index, int nonblock);


	/**
	* @brief get report descriptor (hid_get_report_descriptor())
	*/
	virtual int api_get_report_descriptor(int n_map_index, unsigned char* buf, size_t buf_size);

	/**
	* (hid_write())
	*/
	virtual int api_write(int n_map_index, const unsigned char* data, size_t length, _hid_api_briage::type_next_io next);

	/**
	* (hid_read())
	*/
	virtual int api_read(int n_map_index, unsigned char* data, size_t length, size_t n_report);

	/**
	* (hid_error())
	*/
	virtual const wchar_t* api_error(int n_map_index);

private:
	std::tuple<bool, int, bool> _is_open(const char* path) const;

private:
	// key is primitive map index.
	// value is pair of vhid_info ptr and worker ptr
	_type_map_ptr_vhid_info_ptr_worker m_map_ptr_hid_info_ptr_worker;

	// new part
	mutable std::mutex m_mutex_for_map;

private://don't call these methods.
	_vhid_api_briage(const _vhid_api_briage&);
	_vhid_api_briage& operator=(const _vhid_api_briage&);
};