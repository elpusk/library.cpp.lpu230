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
		* executed by worker thread.
		* processing request.
		* @paramter request request reference
		* @return true -> complete(with error or success), false -> not complete
		*/
		virtual bool _execute(cio_packet& request);

	private://don't call these methods
		cdev_ctl();
		cdev_ctl(const cdev_ctl&);
		cdev_ctl& operator=(const cdev_ctl&);

	};

}//the end of _mp namespace

