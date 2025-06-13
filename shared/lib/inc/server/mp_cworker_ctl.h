#pragma once

#include <functional>
#include <memory>
#include <map>
#include <mutex>
#include <thread>
#include <vector>

#include <mp_type.h>
#include <mp_cstring.h>
#include <mp_cqueue.h>
#include <mp_clog.h>
#include <mp_vcworker.h>
#include <mp_cio_packet.h>


namespace _mp {

	/**
	* base class of view worker
	* need the defintion of
	* _execute() function
	* _continue() function
	*/
	class cworker_ctl : public vcworker<cio_packet>
	{

	public:
		typedef	std::weak_ptr< cworker_ctl >		type_wptr;
		typedef	std::shared_ptr< cworker_ctl >	type_ptr;

	public:

		virtual ~cworker_ctl()
		{
		}
		cworker_ctl(clog *p_log) : vcworker(p_log)
		{
			m_w_device_index = _MP_TOOLS_INVALID_DEVICE_INDEX;
		}

		unsigned short get_device_index() const
		{
			return m_w_device_index;
		}

		std::wstring get_dev_path() const
		{
			return m_s_dev_path;
		}

		void set_dev_path(const std::wstring & sw_path )
		{
			m_s_dev_path = sw_path;
		}
		void set_dev_path(const std::string& s_path)
		{
			m_s_dev_path = cstring::get_unicode_from_mcsc(s_path);
		}

		void set_device_index(unsigned short dw_dev_index)
		{
			m_w_device_index = dw_dev_index;
		}

    protected:

		/**
		* @brief executed by worker thread. processing request.
		* 
		* @paramter new request ptr
		* 
		* @paramter cur request ptr
		* 
		* @return the current request ptr :
		*
		*	if the stored pointer is a null pointer -> complete(with error or success),
		*
		*	else -> not complete( need more running by _continue() ).
		*/
		virtual cio_packet::type_ptr _execute(cio_packet::type_ptr& ptr_req_new, const cio_packet::type_ptr& ptr_req_cur)
		{
			return cio_packet::type_ptr();
		}

		/**
		* @brief executed by worker thread. when the return of _execute() is allocated.(not complete),and none new request
		* 
		* @paramter request ptr
		* 
		* @return
		*	
		*	true -> complete(the current request ptr with error or success), 
		* 
		*	false -> not complete(_continue() will be recalled at next time)
		*/
		virtual bool _continue(cio_packet::type_ptr& ptr_req_cur)
		{
			return true;
		}

	protected:
		unsigned short m_w_device_index;
		std::wstring m_s_dev_path;


	private://don't call these methods
		cworker_ctl();
		cworker_ctl(const cworker_ctl&);
		cworker_ctl& operator=(const cworker_ctl&);

	};

}//the end of _mp namespace

