#pragma once

#include <functional>
#include <memory>
#include <map>
#include <mutex>
#include <thread>
#include <vector>

#include <mp_type.h>
#include <mp_cqueue.h>
#include <mp_clog.h>
#include <server/mp_cio_packet.h>

namespace _mp {
	class cmain_ctl_fn
	{

	public:
		typedef	std::weak_ptr< cmain_ctl_fn >		type_wptr;
		typedef	std::shared_ptr< cmain_ctl_fn >	type_ptr;

	public:

		virtual ~cmain_ctl_fn();
	protected:
		cmain_ctl_fn();

		//
		bool _execute_general_error_response(clog* p_log,cio_packet& request, cio_packet& response, cio_packet::type_error_reason n_reason /*= cio_packet::error_reason_none*/);

		bool _execute_mgmt_get_echo(clog* p_log, cio_packet& request, cio_packet& response);
		
		bool _execute_mgmt_get_device_list(clog* p_log, const type_list_wstring& list_wstring_filter, cio_packet& request, cio_packet& response);
		
		bool _execute_mgmt_ctl_show(clog* p_log, cio_packet& request, cio_packet& response);
		
		bool _execute_file_operation(clog* p_log, cio_packet& request, cio_packet& response);

		bool _execute_advance_operation(clog* p_log, cio_packet& request, cio_packet& response, cio_packet& response_for_the_other_session);

	private://don't call these methods
		cmain_ctl_fn(const cmain_ctl_fn&);
		cmain_ctl_fn& operator=(const cmain_ctl_fn&);

	};

}//the end of _mp namespace

