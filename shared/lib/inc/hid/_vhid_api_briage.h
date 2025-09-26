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
#include <list>

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
		void setup_for_write(const unsigned char* data, size_t length, _mp::type_next_io next);
		void setup_for_read(unsigned char* data, size_t length, size_t n_report);

		_vhid_api_briage::_q_item::type_cmd get_cmd() const;
		std::wstring get_cmd_by_string() const;
		_mp::type_v_buffer get_tx() const;
		_mp::type_next_io get_next_io_type() const;

		size_t get_rx_size() const;
		_vhid_api_briage::_q_item& set_rx(const _mp::type_ptr_v_buffer& ptr_v_rx);
		_vhid_api_briage::_q_item& set_rx(const _mp::type_v_buffer& v_rx);
		_vhid_api_briage::_q_item& append_rx(const _mp::type_v_buffer& v_rx,bool b_add_to_head = false);
		_vhid_api_briage::_q_item& set_result(int n_result,const std::wstring & s_debug_msg = std::wstring());

		size_t get_user_rx_buffer_size() const;
		unsigned char* get_user_rx_buffer() const;

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

		_mp::type_next_io m_next;
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
		int m_n_compositive_map_index;

	private://don't call these methods
		_q_item();
	};


	/**
	* container of _q_item.
	* queue style basically.
	* 
	*/
	class _q_container {
	public:

		// list status of request
		typedef	enum :int {
			st_transaction,
			st_not_yet_transaction,
			st_cancel
		} type_req_list_status;

	public:
		typedef std::deque<_vhid_api_briage::_q_item::type_ptr> type_q;
		typedef std::shared_ptr<_vhid_api_briage::_q_container::type_q> type_ptr_q;
		typedef std::map<int, _vhid_api_briage::_q_container::type_ptr_q> type_map_ptr_q;//key-compositive index

		//
		typedef std::list< _q_item::type_ptr > type_list;
		typedef	std::shared_ptr< _q_container::type_list > type_ptr_list;
		typedef	std::pair<_q_container::type_req_list_status, _q_container::type_ptr_list > type_pair_st_ptr_list;
		typedef	std::deque< _q_container::type_pair_st_ptr_list >	type_q_pair;


	public:
		_q_container();

		~_q_container();

		/**
		* @brief push requst to worker queue.
		*
		* @param ptr_item - request(q item)
		*
		* @return true:success push, false:error
		*
		*/
		bool push( const _vhid_api_briage::_q_item::type_ptr& ptr_item);

		void pop_all(int n_result, const _mp::type_v_buffer& v_rx = _mp::type_v_buffer(0));

		/**
		* @brief get the requestes queue of complete transaction in all type(primitive & compositive)
		*
		* @param map_ptr_q(out) - the requestes queue map of complete transaction
		*
		* @return
		*	first - true : map_ptr_q isn't empty. false : map_ptr_q is empty.
		*
		*	second - true : cancel list, false : normal list
		*/
		std::pair<bool, bool> try_pop(_q_container::type_ptr_list& ptr_list);

	private:
		void _dump_map();

	private:
		// mutex of m_q_pair map
		mutable std::mutex m_mutex;

		/**
		* worker queue
		* each item of queue is the pair of status of list and list of request.
		* 
		* the status of list can be below
		*	_q_container::st_transaction, - Each request in the list either constitutes a transaction on its own (each request has a different map index value), 
		*		or all requests constitute a transaction as one (each request has the same map index value).
		*	_q_container::st_not_yet_transaction - The map index value of each request in the list is the same, 
		*		but it does not yet constitute a transaction. (More requests are needed.)
		*	_q_container::st_cancel - If the map index values of each request in the list are the same,
		*		the requests in the entire list are canceled. Otherwise,
		*		only the requests in the list whose map index is the same as the cancel request are canceled.
		* 
		*/
		_q_container::type_q_pair m_q_pair;

		//item is int -> primitive or compositive map index. older value has the high priority
		// for 
		std::deque<int> m_q_map_index;

	};

	class _q_worker {
	public:
		typedef	std::shared_ptr<_vhid_api_briage::_q_worker>	type_ptr;
	private:
		typedef	std::pair<int, _mp::type_ptr_v_buffer> _type_pair_result_ptr_rx;
		typedef std::deque< _q_worker::_type_pair_result_ptr_rx > _type_q_result;

	private:
		enum {
			_const_one_packet_of_in_report_retry_counter = 300  // times of _q_worker::_const_txrx_pair_rx_interval_mmsec
		};

	public:
		_q_worker(int n_primitive_map_index, _vhid_api_briage* p_api_briage,bool b_remove_all_zero_in_report);
		~_q_worker();
		
		/**
		* @brief push requst to worker queue.
		* 
		* @param ptr_item - request(q item)
		* 
		* @return true:success push, false:error
		* 
		*/
		bool push(const _vhid_api_briage::_q_item::type_ptr &ptr_item);

	private:
		void _pop_all(int n_result, const _mp::type_v_buffer& v_rx = _mp::type_v_buffer(0));

		/**
		* @brief get the requestes queue of complete transaction in all type(primitive & compositive)
		* 
		* @param map_ptr_q(out) - the requestes queue map of complete transaction
		* 
		* @return
		*	first - true : map_ptr_q isn't empty. false : map_ptr_q is empty.
		* 
		*	second - true : cancel list, false : normal list
		*/
		std::pair<bool,bool> _try_pop(_q_container::type_ptr_list& ptr_list);
		

		void _worker(_vhid_api_briage* p_api_briage);

		/**
		* @brief process list of requestes. with nottification of result.
		* 
		* @param ptr_list - list of requestes
		* 
		* @return 
		*	first - true: success, false : error
		*	
		*	second - the received ddata.
		*/
		std::pair<bool, _mp::type_v_buffer> _process(
			_q_container::type_ptr_list& ptr_list,
			_hid_api_briage* p_api_briage
		);

		/**
		* @brief cancel list of requestes. with nottification of result.
		*
		* @param ptr_list - list of requestes
		*
		* @return
		*	true: success, false : error
		*/
		bool _process_cancel(
			_q_container::type_ptr_list& ptr_list,
			_hid_api_briage* p_api_briage
		);

		/**
		* @brief executes request.
		*
		* @param ptr_q_item - in/out request.
		*
		* @param p_api_briage - pointer of _hid_api_briage instance.
		* 
		* @param ptr_v_rx[in,out] - receved data, this shared_ptr must be allocated to a new memory( if none rx, allocated as _mp:type_v_buffer(0) ) 
		* 
		* @param s_debug_msg - string for debugging.
		* 
		* @return
		*	first - true : this phase is complete process with success or error, false : not yet complete.
		* 
		*	second - int : result code, -1: error, 0: success, else success & the number of rx data
		* 
		*/
		std::tuple<bool, int> _process_req(
			const _vhid_api_briage::_q_item::type_ptr& ptr_q_item,
			_hid_api_briage* p_api_briage,
			_mp::type_ptr_v_buffer& v_ptr_rx,
			const std::wstring& s_debug_msg = std::wstring(L"")
			);

		/**
		* @brief notify the result of request to waiting worker.
		* 
		* this function have to be called only that the poped request-list is single or only rx requests.
		* 
		* @param ptr_req - request
		* 
		* @param n_result - result code
		* 
		* @param ptr_v_rx - the received data from device.
		* 
		* @return true(result is notified), false(the notification is passed.)
		*/
		bool _notify_in_single_or_multi_rx_requests(
			_q_item::type_ptr &ptr_req,
			int n_result,
			const _mp::type_ptr_v_buffer & ptr_v_rx
		);

		/**
		* @brief save result code and the received data to MSR reponse buffer Q(m_q_result_msr) or ibutton buffer Q(m_q_result_ibutton)
		* 
		* @param n_result - result code
		* 
		* @param ptr_v_rx - the received data 
		* 
		* @param ptr_req - current request. if this is empty, the current request is expected read_request. 
		* 
		*/
		void _save_rx_to_msr_or_ibutton_buffer_in_single_or_multi_rx_requests(
			int n_result,
			const _mp::type_ptr_v_buffer& ptr_v_rx,
			const _q_item::type_ptr& ptr_req= _q_item::type_ptr()
		);

		/**
		* @brief recevive data by _hid_api_briage::api_read. 
		* 
		* @param v_rx[in/out] - v_rx.size() must be eqaul to in-report size, the receved data will be save to this buffer,
		* 
		* @param p_api_briage - pointer of _hid_api_briage instance.
		*
		* @return return value of _hid_api_briage::api_read() in zero or negative.
		* 
		*	if _hid_api_briage::api_read() return partial value, this function concates the received all data to v_rx by the size of v_rx. 
		*
		*/
		int _rx(_mp::type_v_buffer& v_rx, _hid_api_briage* p_api_briage);

	private:
		std::shared_ptr<std::thread> m_ptr_worker; // inner worker thread
		std::atomic<bool> m_b_run_worker;//self killing flag of m_ptr_worker.

		_vhid_api_briage::_q_container m_q;

		// received msr data queue.
		// this queue must be reset on txrx request.
		_q_worker::_type_q_result m_q_result_msr; 

		// received ibutton data queue 
		// this queue must be reset on txrx request.
		_q_worker::_type_q_result m_q_result_ibutton;

		// primitive map index. setting on constructure.
		const int m_n_primitive_map_index;

		bool m_b_remove_all_zero_in_report; // default false, all zeros value report is ignored

	private://don't call these methods
		_q_worker();
	};

private:
	//key is primitive map index.
	//value is pair of vhid_info ptr and worker ptr
	typedef std::map<int, std::pair<_vhid_info::type_ptr, _q_worker::type_ptr> > _type_map_ptr_vhid_info_ptr_worker;

public:
	_vhid_api_briage();
	_vhid_api_briage(_mp::clog* p_clog);
	_vhid_api_briage(_mp::clog* p_clog, bool b_remove_all_zero_in_report);

	virtual ~_vhid_api_briage();

	/**
	* @brief get connected information of devices.
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
	* @return compositive or primitive type map index
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
	* @brief close device. 
	*
	* @param n_map_index int - primitive or compositive type map index(m_map_hid_dev)
	*/
	virtual void api_close(int n_map_index);

	/**
	* @brief set blocking mode.
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
	* @brief get report descriptor.
	*
	* @param n_map_index int - primitive or compositive map index.
	*
	* @param buf:unsigned char* - the buffer of returned report descriptor.
	*
	* @param buf_size : size_t - the size of buf(unit byte)
	*
	* @return -1 : error, else : the size of saved data(on buf).
	*/
	virtual int api_get_report_descriptor(int n_map_index, unsigned char* buf, size_t buf_size);

	/**
	* @brief send data to device.
	*
	* @param n_map_index int - primitive or compositive map index.
	*
	* @param data:const unsigned char* - the buffer of tx data.
	*
	* @param length : size_t - the size of data(unit byte)
	*
	* @param next : type_next_io - dummy in this class
	*
	* @return -1 : error, else : the size of sent data(on buf).
	*/
	virtual int api_write(int n_map_index, const unsigned char* data, size_t length, _mp::type_next_io next);

	/**
	* @brief receive data from device.(hid_read()).
	* 
	* This function reads data in in-report size units.
	*
	* @param n_map_index int - primitive or compositive map index.
	*
	* @param data: unsigned char* - the buffer of rx data.
	*
	* @param length : size_t - the size of data buffer(unit byte)
	*
	* @param n_report : int - the size of in report data.
	*
	* @return -1 : error, else : the size of received data(on data).
	*/
	virtual int api_read(int n_map_index, unsigned char* data, size_t length, size_t n_report);

	/**
	* @brief get last error string(hid_error())
	*
	* @param n_map_index int - primitive or compositive map index.
	*
	*
	* @return 0 : error, else : unicode string buffer(error description)
	*
	*	Don't free return memory.
	*/
	virtual const wchar_t* api_error(int n_map_index);

private:
	std::tuple<bool, int, bool> _is_open(const char* path) const;

	bool m_b_remove_all_zero_in_report; // default false, all zeros value report is ignored
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