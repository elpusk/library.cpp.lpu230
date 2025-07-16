#pragma once
#include <i_device_of_client.h>

class csimple_of_client : public i_device_of_client
{
public:
	typedef std::shared_ptr< csimple_of_client >	type_ptr;
	//
public:
	virtual const std::wstring get_class_name();
	csimple_of_client();
	csimple_of_client(unsigned long n_client_index, const std::wstring& s_device_path);
	virtual ~csimple_of_client();

};

