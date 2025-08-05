#pragma once

#include <functional>
#include <memory>
#include <map>
#include <mutex>
#include <thread>
#include <vector>
#include <deque>

#include <mp_type.h>
#include <mp_cqueue.h>
#include <mp_cio_packet.h>
#include <server/mp_cworker_ctl.h>
#include <server/mp_cdev_ctl_fn_.h>

#include <hid/mp_clibhid_dev.h>

namespace _mp {
	class cdev_ctl : public cworker_ctl
	{
	public:
		typedef	std::weak_ptr< cdev_ctl >		type_wptr;
		typedef	std::shared_ptr< cdev_ctl >	type_ptr;

	public:
		virtual ~cdev_ctl();
		cdev_ctl(clog* p_log, long long ll_worker_sleep_interval_mmsec);

	protected:

		/**
		* @brief executed by worker thread. processing request.
		* 
		* this function will be called when the new request is poped in request queue.
		* 
		* if return is complete, the result of processing send to client.
		* 
		* @param request- new request ptr
		* 
		* @param request- current request ptr
		* 
		* @return the current request ptr :
		*
		*	if the stored pointer is a null pointer -> complete(with error or success),
		*
		*	else -> not complete( need more running by _continue() ).
		*/
		virtual cio_packet::type_ptr _execute(cio_packet::type_ptr& ptr_req_new, const cio_packet::type_ptr& ptr_req_cur);

		/**
		* @brief executed by worker thread.
		* 
		*	this function will be called when the return of _execute() is the allocated ptr(not complete),and none new request.
		* 
		*	if the completed request is exsited in read or sync Q, get all completed response packet.
		* 
		* @param request - request ptr
		* @return true -> complete(the current request ptr with error or success), false -> not complete(_continue() will be recalled at next time)
		*/
		virtual bool _continue(cio_packet::type_ptr& ptr_req_cur);

	private:
		cdev_ctl_fn m_fun;

	private://don't call these methods
		cdev_ctl();
		cdev_ctl(const cdev_ctl&);
		cdev_ctl& operator=(const cdev_ctl&);

	};

}//the end of _mp namespace

