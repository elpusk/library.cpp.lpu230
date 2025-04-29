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
		* @paramter request request reference
		* @return true -> complete(with error or success), false -> not complete 
		*/
		virtual bool _execute(cio_packet::type_ptr& ptr_request);

	private://don't call these methods
		cmain_ctl();
		cmain_ctl(const cmain_ctl&);
		cmain_ctl& operator=(const cmain_ctl&);

	};

}//the end of _mp namespace

