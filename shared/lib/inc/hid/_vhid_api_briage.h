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
		typedef std::map<int, _vhid_api_briage::_q_worker::_type_ptr_q> _type_map_ptr_q;//key-compositive
	private:
		enum {
			_const_worker_interval_mmsec = 30
		};
		enum {
			_const_txrx_pair_tx_interval_mmsec = 3
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
	typedef std::map<int, std::pair<_vhid_info::type_ptr, _q_worker::type_ptr> > _type_map_ptr_vhid_info_ptr_worker;

public:
	_vhid_api_briage();

	virtual ~_vhid_api_briage();

	/**
	* get connected information of devices.
	* return  each item's 1'st - device apth, 2'nd - usb vendor id, 3'th - usb product id, 4'th - usb interface number, 5'th - extra data
	*/
	virtual std::set< std::tuple<std::string, unsigned short, unsigned short, int, std::string> >hid_enumerate();

	/**
	* @brief check is open or not
	* @param path - primitive or composite path
	* @return first true - open, false not open or error, second - the opened device of the index of map. third - true(exclusive open), false(shared open or not open )
	*/
	virtual std::tuple<bool, int, bool> is_open(const char* path) const;

	/**
	* open device with path. (hid_open_path())
	* return the index of map(m_map_hid_dev), this value must be const_map_index_min~const_map_index_max, Multiples of const_map_index_inc_unit.
	* -1(error)
	*/
	virtual int api_open_path(const char* path);

	/**
	* (hid_close())
	*/
	virtual void api_close(int n_map_index);

	/**
	* set blocking mode.(hid_set_nonblocking())
	* @param
	* n_map_index:int - the index of map(m_map_hid_dev)
	* nonblock:int - 0 is blocking mode, 1 is nonblocking mode
	*/
	virtual int api_set_nonblocking(int n_map_index, int nonblock);


	/**
	* (hid_get_report_descriptor())
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
	bool _is_open(int n_map_index);

	/**
	* @brief create primitive map item.
	* @retutrn primitive map index
	*/
	int _create_new_map_primitive_item(const std::string& s_primitive_path, _mp::type_bm_dev composite_or_primitive_type);

	/**
	* @brief create composite map item.
	* @retutrn composite m primitive or _vhid_info::const_map_index_invalid by b_return_index_is_composite_index parameter.
	*/
	int _create_new_map_compositive_item(int n_primitive_index, _mp::type_bm_dev composite_or_primitive_type, bool b_return_index_is_composite_index);

	/**
	* @brief increase composite type open counter!
	* @return composite map index. or _vhid_info::const_map_index_invalid(error)
	*/
	int _open_composite(int n_primitive_index, _mp::type_bm_dev composite_type, bool b_shared_open);

private:
	//compoitive type and primitive type map key index
	// a key is equal to the key of m_map_hid_dev
	_type_map_ptr_vhid_info_ptr_worker m_map_ptr_hid_info_ptr_worker;

private://don't call these methods.
	_vhid_api_briage(const _vhid_api_briage&);
	_vhid_api_briage& operator=(const _vhid_api_briage&);
};