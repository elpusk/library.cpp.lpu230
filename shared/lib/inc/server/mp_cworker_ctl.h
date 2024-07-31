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
#include <server/mp_cio_packet.h>


namespace _mp {

	/**
	* base class of view worker
	* need the defintion of
	* push_back_request() function.
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
			m_w_device_index = 0;
			m_n_session = _MP_TOOLS_INVALID_SESSION_NUMBER;
		}

		unsigned short get_device_index() const
		{
			return m_w_device_index;
		}

		std::wstring get_dev_path() const
		{
			return m_s_dev_path;
		}

		unsigned long get_connected_session() const
		{
			return m_n_session;
		}

		void set_connected_session(unsigned long n_session)
		{
			m_n_session = n_session;
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
		* executed by worker thread.
		* processing request.
		* @paramter request request reference
		* @return true -> complete(with error or success), false -> not complete 
		*/
		virtual bool _execute(cio_packet& request) 
		{
			return false;
		}

		/**
		* executed by worker thread. when _execute return false(not complete),and none new request
		* @paramter request request reference
		* @return true -> complete(with error or success), false -> not complete(_continue() will be recalled at next time)
		*/
		virtual bool _continue(cio_packet& request)
		{
			return true;
		}

	protected:
		unsigned long m_n_session;
		unsigned short m_w_device_index;
		std::wstring m_s_dev_path;


	private://don't call these methods
		cworker_ctl();
		cworker_ctl(const cworker_ctl&);
		cworker_ctl& operator=(const cworker_ctl&);

	};

}//the end of _mp namespace

