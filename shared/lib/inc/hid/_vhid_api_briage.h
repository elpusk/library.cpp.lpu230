#pragma once

/**
* don't include in header file(.h file).
* include only body file.(.cpp file)
*/
#include <set>
#include <map>
#include <utility>
#include <memory>

#include <hidapi.h>

#include <hid/_hid_api_briage.h>

class _vhid_api_briage: public _hid_api_briage
{
public:
	typedef	std::shared_ptr<_vhid_api_briage>	type_ptr;

protected:
	typedef std::map<int, hid_device*> _type_map_hid_dev;
public:
	static _vhid_api_briage::type_ptr& get_instance(bool b_remove)
	{
		static _vhid_api_briage::type_ptr ptr(new _vhid_api_briage());
		if (b_remove) {
			ptr.reset();
		}
		return ptr;
	}

	virtual ~_vhid_api_briage();

	/**
	* get connected information of devices.
	* return  each item's 1'st - device apth, 2'nd - usb vendor id, 3'th - usb product id, 4'th - usb interface number
	*/
	virtual std::set< std::tuple<std::string, unsigned short, unsigned short, int> >hid_enumerate();

	/**
	* open device with path. (hid_open_path())
	* return the index of map(m_map_hid_dev), this value must be greater then equal 0.
	*/
	virtual int api_open_path(const char* path);


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
	virtual void api_close(int n_map_index);

	/**
	* (hid_close())
	*/
	virtual int api_get_report_descriptor(int n_map_index, unsigned char* buf, size_t buf_size);

	/**
	* (hid_write())
	*/
	virtual int api_write(int n_map_index, const unsigned char* data, size_t length);

	/**
	* (hid_read_timeout())
	*/
	virtual int api_read_timeout(int n_map_index, unsigned char* data, size_t length, int milliseconds);

	/**
	* (hid_read())
	*/
	virtual int api_read(int n_map_index, unsigned char* data, size_t length);

	/**
	* (hid_error())
	*/
	virtual const wchar_t* api_error(int n_map_index);
protected:
	_vhid_api_briage();


private://don't call these methods.
	_vhid_api_briage(const _vhid_api_briage&);
	_vhid_api_briage& operator=(const _vhid_api_briage&);
};