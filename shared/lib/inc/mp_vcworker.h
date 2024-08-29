#pragma once

#include <functional>
#include <memory>
#include <map>
#include <mutex>
#include <thread>
#include <vector>

#include <mp_type.h>
#include <mp_cqueue.h>
#include <mp_vcpacket.h>
#include <mp_clog.h>

namespace _mp {

	/**
	* virtual class
	* need the defintion of
	* push_back_request() function.
	* _execute() function
	* _continue(T& request) = 0;
	*/
	template<typename T>
	class vcworker
	{
    protected:
        enum {
            _const_worker_sleep_interval_mmsec = 30
        };

	public:
		typedef	std::weak_ptr< vcworker >		type_wptr;
		typedef	std::shared_ptr< vcworker >	type_ptr;

    protected:
		typedef cqueue< std::shared_ptr<T> > _type_q;

	public:

		virtual ~vcworker()
		{
			if (m_ptr_th_worker) {
				m_b_run_th_worker = false;
				if (m_ptr_th_worker->joinable()) {
					m_ptr_th_worker->join();
				}
			}
		}
		vcworker(clog *p_log) : m_b_run_th_worker(true), m_p_log(p_log)
		{
			m_ptr_th_worker = std::shared_ptr<std::thread>(new std::thread(vcworker::_worker, std::ref(*this)));
		}

		void push_back_request(std::shared_ptr<T>& ptr)
		{
			m_q.push(ptr);
		}

    protected:

		/**
		* executed by worker thread.
		* processing request.
		* @paramter request request reference
		* @return true -> complete(with error or success), false -> not complete 
		*/
		virtual bool _execute(T& request) = 0;

		/**
		* executed by worker thread. when _execute return false(not complete),and none new request
		* @paramter request request reference
		* @return true -> complete(with error or success), false -> not complete(_continue() will be recalled at next time) 
		*/
		virtual bool _continue(T& request) = 0;

	protected:
		static void _worker(vcworker& obj)
        {
			std::shared_ptr<T> ptr_request;
			bool b_completet(true);

			while (obj.m_b_run_th_worker) {
				do {
					if (obj.m_q.try_pop(ptr_request)) {
						if (!ptr_request) {
							continue;//idle
						}
						//
						b_completet = obj._execute(*ptr_request);
						if (b_completet) {
							ptr_request.reset();
						}
						continue;
					}

					if (b_completet) {
						continue;//idle
					}
					b_completet = obj._continue(*ptr_request);
					if (b_completet) {
						ptr_request.reset();
					}

				}while (false);
                std::this_thread::sleep_for(std::chrono::milliseconds(vcworker::_const_worker_sleep_interval_mmsec));
            }//end while
        }
		//
	protected:
		std::shared_ptr<std::thread> m_ptr_th_worker;
		std::atomic<bool> m_b_run_th_worker;
        vcworker::_type_q m_q;

		clog* m_p_log;

	private://don't call these methods
		vcworker();
		vcworker(const vcworker&);
		vcworker& operator=(const vcworker&);

	};

}//the end of _mp namespace

