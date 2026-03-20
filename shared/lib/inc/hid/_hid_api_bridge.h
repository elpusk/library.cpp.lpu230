#pragma once

/**
* don't include in header file(.h file).
* include only body file.(.cpp file)
*/
#include <set>
#include <map>
#include <utility>
#include <memory>
#include <mutex>
#include <atomic>
#include <optional>
#include <map>
#include <tuple>
#include <thread>
#include <mutex>

#include <hidapi.h>

#include <mp_clog.h>
#include <usb/mp_clibusb.h>

/**
* this class method must be protected by sync-object
* sinlgle-tone type
* supports primitve hid type device. 
*/
class _hid_api_bridge
{
public:
	enum {
		const_default_hid_read_interval_mmsec = 1
	};
	enum {
		const_default_hid_write_interval_mmsec = 1
	};

protected:
	enum {
		const_default_req_q_check_interval_mmsec_of_child = 1 // for child class, the child will use worker thread.
	};

public:
	typedef	std::shared_ptr<_hid_api_bridge>	type_ptr;

protected:

	struct _hid_report_size_entry {
		uint8_t id = 0;
		int     in_bytes = 0;   // IN  report 1개의 byte 크기 (Report ID 포함)
		int     out_bytes = 0;   // OUT report 1개의 byte 크기 (Report ID 포함)
	};

	// key = Report ID  (0 = Report ID item 자체가 없는 장비)
	typedef std::map<uint8_t, _hid_api_bridge::_hid_report_size_entry> _type_hid_report_size_map;
	typedef std::shared_ptr<_hid_api_bridge::_type_hid_report_size_map> _type_ptr_hid_report_size_map;


	/**
	* get<0> hid_device pointer
	* get<1> true - exclusive using after opening device.
	* get<2> device path
	* get<3> shared_ptr of report size map (Report ID 별 IN/OUT report size 정보)
	* get<4> mutex for hid_device pointer (each hid_device pointer must be guarded by this mutex. the instance of hid_device pointer must be shared this mutex.)
	*/
	typedef	std::tuple< hid_device*, bool,std::string, _hid_api_bridge::_type_ptr_hid_report_size_map, std::shared_ptr<std::mutex> > _type_tuple_hid_dev_lock_path;
	typedef std::map<int, _hid_api_bridge::_type_tuple_hid_dev_lock_path> _type_map_hid_dev;

private:
	typedef std::map<hid_device*, bool> _type_map_lpu237_disable_ibutton;

protected:
	class _crx_worker{
	public:
		typedef std::shared_ptr<_crx_worker>	type_ptr;
	private:
		enum {
			_const_rx_sleep_time_msec = 2
		};

		typedef std::tuple<bool, _mp::type_ptr_v_buffer, std::wstring>	_type_tuple_result_rx; // get<0> - true is success, false is fail. get<1> - rx data buffer.
		typedef std::deque<_crx_worker::_type_tuple_result_rx>	_type_q_rx;

	public:
		_crx_worker(_hid_api_bridge::_type_tuple_hid_dev_lock_path & tuple_hid_dev);
		virtual ~_crx_worker();
		void start();
		void stop();
		/**
		* @brief try pop one item from queue and remove it.
		* @return true - success, false - fail (queue is empty)
		*/
		bool q_try_pop(_crx_worker::_type_tuple_result_rx& tuple_result_rx);

		/**
		* @brief clear all items in queue.
		*/
		void q_clear();
	private:
		void _q_push( bool b_result_rx, const _mp::type_v_buffer & v_in_report = _mp::type_v_buffer(), const std::wstring & s_debug_msg = std::wstring());

		void _worker();
	private:
		_hid_api_bridge::_type_tuple_hid_dev_lock_path & m_tuple_hid_dev;
		std::atomic_bool m_b_run;
		std::shared_ptr<std::thread> m_ptr_thread;
		_crx_worker::_type_q_rx m_q_rx; // m_thread 에서 계속 데이터를 pump 해서 저장.
	private:
		_crx_worker() = delete;
		_crx_worker(const _crx_worker&) = delete;
		_crx_worker& operator=(const _crx_worker&) = delete;
	};

public:

	static std::mutex& get_mutex_for_hidapi()
	{
		static std::mutex mutex_hidapi; //each hidapi function must be guarded by this mutex. all instance of this class must be shared this mutex.

		return mutex_hidapi;
	}

	virtual _mp::clog* get_clog() const
	{
		return m_p_clog;
	}

	virtual ~_hid_api_bridge();

	bool is_ini() const;

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
	* @param path - primitive path
	* 
	* @return
	*	first true - open, false not open or error
	* 
	*	second - the opened device of the index of map. 
	* 
	*	third - true(exclusive open), false(shared open or not open )
	*/
	virtual std::tuple<bool,int,bool> is_open(const char* path) const;


	/**
	* @brief open device
	*
	*   In windows, open with FILE_SHARE_READ|FILE_SHARE_WRITE flag
	*
	*   In linux, open by libusb_open()
	*
	* @param path : string - primitive device path
	* 
	* @return the primitive type map index of map(m_map_hid_dev)
	* 
	*	-1 : error( including already open status )
	*/
	virtual int api_open_path(const char* path);

	/**
	* @brief close device. internally, hid_close() will be called.
	* 
	* @param n_primitive_map_index int - primitive type map index(m_map_hid_dev)
	*/
	virtual void api_close(int n_primitive_map_index);


	/**
	* @brief set blocking mode.(hid_set_nonblocking())
	* 
	* @param n_primitive_map_index int - primitive type map index(m_map_hid_dev)
	* 
	* @param nonblock:int - 0 is blocking mode, 1 is nonblocking mode
	* 
	* @return 0 : success, -1 : error 
	*/
	virtual int api_set_nonblocking(int n_primitive_map_index, int nonblock);

	/**
	* @brief get report descriptor.(hid_get_report_descriptor())
	*
	* @param n_primitive_map_index int - primitive type map index(m_map_hid_dev)
	*
	* @param buf:unsigned char* - the buffer of returned report descriptor.
	* 
	* @param buf_size : size_t - the size of buf(unit byte) 
	*
	* @return -1 : error, else : the size of saved data(on buf).
	*/
	virtual int api_get_report_descriptor(int n_primitive_map_index, unsigned char* buf, size_t buf_size);

	/**
	* @brief send data to device.(hid_write())
	*
	* @param n_primitive_map_index int - primitive type map index(m_map_hid_dev)
	*
	* @param data:const unsigned char* - the buffer of tx data.
	*
	* @param length : size_t - the size of data(unit byte)
	* 
	* @param next : type_next_io - dummy in this class
	*
	* @return -1 : error, else : the size of sent data(on buf).
	*/
	virtual int api_write(int n_primitive_map_index, const unsigned char* data, size_t length, _mp::type_next_io next);

	/**
	* @brief receive data from device.(hid_read())
	*
	* @param n_primitive_map_index int - primitive type map index(m_map_hid_dev)
	*
	* @param data: unsigned char* - the buffer of rx data.
	*
	* @param length : size_t - the size of data buffer(unit byte)
	*
	* @param n_report : int - the size of in report data.
	*
	* @return -1 : error, else : the size of received data(on data).
	*/
	virtual int api_read(int n_primitive_map_index, unsigned char* data, size_t length, size_t n_report);

	/**
	* @brief get last error string(hid_error())
	*
	* @param n_primitive_map_index int - primitive type map index(m_map_hid_dev)
	*
	*
	* @return 0 : error, else : unicode string buffer(error description)
	* 
	*	Don't free return memory.
	*/
	virtual const wchar_t* api_error(int n_primitive_map_index);

	virtual _hid_api_bridge& set_req_q_check_interval_in_child(long long n_interval_mmsec);
	virtual _hid_api_bridge& set_hid_write_interval_in_child(long long n_interval_mmsec);
	virtual _hid_api_bridge& set_hid_read_interval_in_child(long long n_interval_mmsec);

	virtual long long get_req_q_check_interval_in_child() const;
	virtual long long get_hid_write_interval_in_child() const;
	virtual long long get_hid_read_interval_in_child() const;

protected:
	_hid_api_bridge();
	_hid_api_bridge(_mp::clog* p_clog);

private:
	bool _lpu237_ibutton_enable(hid_device* p_dev,bool b_enable);

	/**
	* @brief parse hid report size by report descriptor.
	* @param desc : the buffer of report descriptor. the output of hid_get_report_descriptor()
	* @param desc_len : the size of desc(unit byte)
	*/
	std::optional<_hid_api_bridge::_type_hid_report_size_map> _parse_hid_report_size(const uint8_t* desc, int desc_len);
protected:
	_mp::clog *m_p_clog;

	std::atomic_llong m_atll_req_q_check_interval_mmsec;
	std::atomic_llong m_atll_hid_write_interval_mmsec;
	std::atomic_llong m_atll_hid_read_interval_mmsec;

	std::wstring m_s_class_name;
	bool m_b_ini;
	_mp::clibusb::type_ptr m_ptr_usb_lib;

	// key is primity type key only
	// this value must be const_map_index_min~const_map_index_max, Multiples of const_map_index_inc_unit.
	_type_map_hid_dev m_map_hid_dev;

	// -1 or the last generated map index key of m_map_hid_dev
	int m_n_map_index;

private:
	// lpu237 & lpu238 device status of i-button disable request processing result.
	// warning: this is dregon
	_hid_api_bridge::_type_map_lpu237_disable_ibutton m_map_lpu237_disable_ibutton;
	
private://don't call these methods.
	_hid_api_bridge(const _hid_api_bridge&);
	_hid_api_bridge& operator=(const _hid_api_bridge&);
};