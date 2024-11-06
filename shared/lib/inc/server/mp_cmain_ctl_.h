#pragma once

#include <functional>
#include <memory>
#include <map>
#include <mutex>
#include <thread>
#include <vector>

#include <mp_type.h>
#include <mp_cqueue.h>
#include <server/mp_cio_packet.h>
#include <server/mp_cworker_ctl.h>


namespace _mp {
	class cmain_ctl : public _mp::cworker_ctl
	{

	public:
		typedef	std::weak_ptr< cmain_ctl >		type_wptr;
		typedef	std::shared_ptr< cmain_ctl >	type_ptr;

	public:

		virtual ~cmain_ctl();
		cmain_ctl(clog* p_log);

    protected:

		/**
		* executed by worker thread.
		* processing request.
		* @paramter request request reference
		* @return true -> complete(with error or success), false -> not complete 
		*/
		virtual bool _execute(cio_packet& request);

	private:
		bool _execute_general_error_response(cio_packet& request, cio_packet& response, cio_packet::type_error_reason n_reason /*= cio_packet::error_reason_none*/);

		bool _execute_mgmt_get_echo(cio_packet& request, cio_packet& response);
		
		bool _execute_mgmt_get_device_list(const type_list_wstring& list_wstring_filter, cio_packet& request, cio_packet& response);
		
		bool _execute_mgmt_ctl_show(cio_packet& request, cio_packet& response);
		/*
		bool _execute_file_operation(cio_packet& request, cio_packet& response);

		bool _execute_advance_operation(cio_packet& request, cio_packet& response, cio_packet& response_for_the_other_session);

		bool _execute_kernel_operation(cio_packet& request, cio_packet& response);
		*/
		//
	private://don't call these methods
		cmain_ctl();
		cmain_ctl(const cmain_ctl&);
		cmain_ctl& operator=(const cmain_ctl&);

	};

}//the end of _mp namespace

