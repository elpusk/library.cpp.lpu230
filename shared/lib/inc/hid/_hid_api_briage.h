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

#include <hidapi.h>

#include <usb/mp_clibusb.h>

/**
* this class method must be protected by sync-object
* sinlgle-tone type
* supports primitve hid type device. 
*/
class _hid_api_briage
{
public:
	// For construting one transaction, the next request type.
	typedef	enum : int{
		// the current transaction is terminated in the current phase(request).
		next_io_none = -1, 

		// the current transaction isn't terminated in the current phase(request).
		// the next phase is writing request.
		next_io_write = -2, 
		
		// the current transaction isn't terminated in the current phase(request).
		// the next phase is reaing request.
		next_io_read = -3
	}type_next_io;
public:
	typedef	std::shared_ptr<_hid_api_briage>	type_ptr;

protected:

	/**
	* second lock(exclusive using after opening device)
	*/
	typedef	std::pair< hid_device*, bool > _type_pair_hid_dev_lock;
	typedef std::map<int, _hid_api_briage::_type_pair_hid_dev_lock> _type_map_hid_dev;

private:
	typedef std::map<hid_device*, bool> _type_map_lpu237_disable_ibutton;
public:

	static std::mutex& get_mutex_for_hidapi()
	{
		static std::mutex mutex_hidapi; //each hidapi function must be guarded by this mutex. all instance of this class must be shared this mutex.

		return mutex_hidapi;
	}

	virtual ~_hid_api_briage();

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
	virtual int api_write(int n_primitive_map_index, const unsigned char* data, size_t length, _hid_api_briage::type_next_io next);

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
protected:
	_hid_api_briage();

private:
	bool _lpu237_ibutton_enable(hid_device* p_dev,bool b_enable);

protected:
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
	_hid_api_briage::_type_map_lpu237_disable_ibutton m_map_lpu237_disable_ibutton;
	
private://don't call these methods.
	_hid_api_briage(const _hid_api_briage&);
	_hid_api_briage& operator=(const _hid_api_briage&);
};