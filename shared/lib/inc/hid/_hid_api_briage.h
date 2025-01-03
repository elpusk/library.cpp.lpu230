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
	typedef	enum : int{
		next_io_none = -1,
		next_io_write = -2,
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
public:

	/**
	* ger reference of this class instance.
	* param b_remove : true(remove instance from memory, for deleting sync resource ), false(normal use case)
	* return this class instance
	*/
	/*
	static _hid_api_briage::type_ptr& get_instance(bool b_remove)
	{
		static _hid_api_briage::type_ptr ptr(new _hid_api_briage());
		if (b_remove) {
			ptr.reset();
		}
		return ptr;
	}
	*/
	static std::mutex& get_mutex_for_hidapi()
	{
		static std::mutex mutex_hidapi; //each hidapi function must be guarded by this mutex.

		return mutex_hidapi;
	}

	virtual ~_hid_api_briage();

	bool is_ini() const;

	/**
	* get connected information of devices.
	* return  each item's 1'st - device path, 2'nd - usb vendor id, 3'th - usb product id, 4'th - usb interface number, 5'th - extra data
	*/
	virtual std::set< std::tuple<std::string, unsigned short, unsigned short, int, std::string> >hid_enumerate();


	/**
	* @brief check is open or not
	* @param path - primitive path
	* @return first true - open, false not open or error, second - the opened device of the index of map. third - true(exclusive open), false(shared open or not open )
	*/
	virtual std::tuple<bool,int,bool> is_open(const char* path) const;


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
	* (hid_close())
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
protected:
	_hid_api_briage();

protected:
	std::wstring m_s_class_name;
	bool m_b_ini;
	_mp::clibusb::type_ptr m_ptr_usb_lib;

	// key is primity type key only
	// this value must be const_map_index_min~const_map_index_max, Multiples of const_map_index_inc_unit.
	_type_map_hid_dev m_map_hid_dev;

	// -1 or the last generated map index key of m_map_hid_dev
	int m_n_map_index;
	
private://don't call these methods.
	_hid_api_briage(const _hid_api_briage&);
	_hid_api_briage& operator=(const _hid_api_briage&);
};