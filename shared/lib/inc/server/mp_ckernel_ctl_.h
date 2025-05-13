#pragma once

#include <mutex>
#include <memory>
#include <map>

#include <cdll_service.h>

#include <mp_type.h>
#include <mp_cqueue.h>

#include <server/mp_cio_packet.h>
#include <server/mp_cworker_ctl.h>
#include <server/mp_cmain_ctl_fn_.h>
#include <server/mp_cdev_ctl_fn_.h>


namespace _mp
{

	class ckernel_ctl : public _mp::cworker_ctl, _mp::cmain_ctl_fn
	{
	private:
		typedef	std::pair<size_t, cdll_service::type_ptr_cdll_service>	_type_pair_ref_cnt_ptr_dll;
		typedef	std::map<std::wstring, _type_pair_ref_cnt_ptr_dll>		_type_map_path_pair_ref_cnt_ptr_dll;

	public:
		typedef	std::weak_ptr< ckernel_ctl >	type_wptr;
		typedef	std::shared_ptr< ckernel_ctl >	type_ptr;


	public:
		ckernel_ctl(clog* p_log);
		virtual ~ckernel_ctl();

	protected://
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

		/**
		* executed by worker thread. when the return of _execute() is the allocated ptr(not complete),and none new request
		* @paramter request request reference
		* @return true -> complete(the current request ptr with error or success), false -> not complete(_continue() will be recalled at next time)
		*/
		virtual bool _continue(cio_packet::type_ptr& ptr_req_cur);

	private:
		bool _execute_kernel_operation(cio_packet& request, cio_packet& response);
		bool _execute_kernel(const cio_packet& request, cio_packet& response);//device dependent kernal operation

	private:
		std::mutex m_mutex_for_path;
		ckernel_ctl::_type_map_path_pair_ref_cnt_ptr_dll m_map_default_path_pair_ref_cnt_ptr_dll;
		ckernel_ctl::_type_map_path_pair_ref_cnt_ptr_dll m_map_3th_path_pair_ref_cnt_ptr_dll;
		type_set_wstring m_set_loaded_dll_path;// items are - default/xxx.dll or 3thpart/xxx.dll

		cdev_ctl_fn m_dev_ctl_fun;

	private://don't call these method.
		ckernel_ctl();
		ckernel_ctl(const ckernel_ctl&);
		ckernel_ctl& operator=(const ckernel_ctl&);
	};

}//_mp