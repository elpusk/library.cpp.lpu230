#include <hid/_vhid_api_briage.h>


/**
* member function bodies
*/
_vhid_api_briage::_vhid_api_briage() : _hid_api_briage()
{
}

_vhid_api_briage::~_vhid_api_briage()
{
}

std::set< std::tuple<std::string, unsigned short, unsigned short, int> > _vhid_api_briage::hid_enumerate()
{
    return _hid_api_briage::hid_enumerate();
}

/**
* return the index of map(m_map_hid_dev)
*/
int _vhid_api_briage::api_open_path(const char* path)
{
    return _hid_api_briage::api_open_path(path);
}
int _vhid_api_briage::api_set_nonblocking(int n_map_index, int nonblock)
{
    return _hid_api_briage::api_set_nonblocking(n_map_index, nonblock);
}

void _vhid_api_briage::api_close(int n_map_index)
{
    return _hid_api_briage::api_close(n_map_index);
}

int _vhid_api_briage::api_get_report_descriptor(int n_map_index, unsigned char* buf, size_t buf_size)
{
    return _hid_api_briage::api_get_report_descriptor(n_map_index, buf, buf_size);
}

int _vhid_api_briage::api_write(int n_map_index, const unsigned char* data, size_t length)
{
    return _hid_api_briage::api_write(n_map_index, data, length);
}

int _vhid_api_briage::api_read_timeout(int n_map_index, unsigned char* data, size_t length, int milliseconds)
{
    return _hid_api_briage::api_read_timeout(n_map_index, data, length, milliseconds);
}

int _vhid_api_briage::api_read(int n_map_index, unsigned char* data, size_t length)
{
    return _hid_api_briage::api_read(n_map_index, data, length);
}

const wchar_t* _vhid_api_briage::api_error(int n_map_index)
{
    return _hid_api_briage::api_error(n_map_index);
}
