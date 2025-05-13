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
#include <server/mp_cmain_ctl_fn_.h>


namespace _mp {
	class cmain_ctl : public _mp::cworker_ctl, cmain_ctl_fn
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
		* @paramter new request ptr
		* @paramter cur request ptr
		* @return the current request ptr :
		*
		*	if the stored pointer is a null pointer -> complete(with error or success),
		*
		*	else -> not complete( need more running by _continue() ).
		*/
		virtual cio_packet::type_ptr _execute(cio_packet::type_ptr& ptr_req_new, cio_packet::type_ptr& ptr_req_cur);

	private://don't call these methods
		cmain_ctl();
		cmain_ctl(const cmain_ctl&);
		cmain_ctl& operator=(const cmain_ctl&);

	};

}//the end of _mp namespace

