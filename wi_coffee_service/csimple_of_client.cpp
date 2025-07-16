#include <websocket/mp_win_nt.h>

#include "csimple_of_client.h"


const std::wstring csimple_of_client::get_class_name()
{
	return std::wstring(L"csimple_of_client");
}

csimple_of_client::csimple_of_client()
	: i_device_of_client()
{

}
csimple_of_client::csimple_of_client(unsigned long n_client_index, const std::wstring& s_device_path)
	: i_device_of_client(n_client_index, s_device_path)
{

}

csimple_of_client::~csimple_of_client()
{

}
