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
#include <server/mp_cio_packet.h>
#include <server/mp_cworker_ctl.h>
#include <server/mp_cdev_ctl_fn_.h>

#include <hid/mp_clibhid_dev.h>

namespace _mp {
	class cdev_ctl : public cworker_ctl, cdev_ctl_fn
	{
	public:
		typedef	std::weak_ptr< cdev_ctl >		type_wptr;
		typedef	std::shared_ptr< cdev_ctl >	type_ptr;

	public:
		virtual ~cdev_ctl();
		cdev_ctl(clog* p_log);

	protected:

		/**
		* @brief executed by worker thread. processing request.
		* 
		* this function will be called when the new request is poped in request queue.
		* 
		* if return is complete, the result of processing send to client.
		* 
		* @param request-request reference
		* @return true -> complete(with error or success), false -> not complete
		*/
		virtual bool _execute(cio_packet& request);

		/**
		* @brief executed by worker thread.
		* 
		* this function will be called when _execute return false(not complete),and none new request.
		* 
		* @param request-request reference
		* @return true -> complete(with error or success), false -> not complete(_continue() will be recalled at next time)
		*/
		virtual bool _continue(cio_packet& request);

		/**
		* @brief send all completed response to client.
		* @return pair type 
		*	first - the number of completed response.
		*	second - the number of sent response packet.
		*/
		std::pair<int,int> _send_all_complete_response();

	private://don't call these methods
		cdev_ctl();
		cdev_ctl(const cdev_ctl&);
		cdev_ctl& operator=(const cdev_ctl&);

	};

}//the end of _mp namespace

