#pragma once

#include <functional>
#include <memory>
#include <map>
#include <mutex>
#include <thread>
#include <vector>
#include <atomic>

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
            _const_worker_sleep_interval_mmsec = 3
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

		vcworker(clog* p_log, long long ll_worker_sleep_interval_mmsec) :
			m_b_run_th_worker(true)
			, m_p_log(p_log)
			, m_atll_worker_sleep_interval_mmsec(ll_worker_sleep_interval_mmsec)
		{
			if(ll_worker_sleep_interval_mmsec <= 0) {
				m_atll_worker_sleep_interval_mmsec.store( vcworker::_const_worker_sleep_interval_mmsec, std::memory_order_relaxed);
			}

			m_ptr_th_worker = std::shared_ptr<std::thread>(new std::thread(vcworker::_worker, std::ref(*this)));
		}

		void push_back_request(std::shared_ptr<T>& ptr)
		{
			m_q.push(ptr);
		}

		void set_worker_sleep_interval(long long ll_worker_sleep_interval_mmsec)
		{
			m_atll_worker_sleep_interval_mmsec.store(ll_worker_sleep_interval_mmsec, std::memory_order_relaxed);
		}

    protected:

		/**
		* @brief executed processing request by worker thread.
		*	
		*	if a request is executing, this function shall cancel th running request.
		* 
		*	if the return is true, the allocated memeory of request will be freed.
		*
		* @parameter the new request ptr. this parameter must be allocated.
		* 
		* @parameter the current request ptr.
		* 
		* @return the current request ptr : 
		* 
		*	if the stored pointer is a null pointer -> complete(with error or success), 
		*
		*	else -> not complete( need more running by _continue() ). 
		*/
		virtual std::shared_ptr<T> _execute(std::shared_ptr<T>& ptr_req_new, const std::shared_ptr<T>& ptr_req_cur) = 0;

		/**
		* @brief executed by worker thread. when the return of _execute() is the allocated ptr(not complete),and none new request
		*	if the return is true, the allocated memeory of request will be freed.
		* 
		* @paramter the current request ptr. this parameter must be allocated.
		* 
		* @return true -> complete(the current request ptr with error or success), false -> not complete(_continue() will be recalled at next time) 
		*/
		virtual bool _continue(std::shared_ptr<T>& ptr_req_cur) = 0;

	protected:
		static void _worker(vcworker& obj)
        {
			std::shared_ptr<T> ptr_req_cur, ptr_req_new;
			bool b_completet(true);
			long long ll_worker_sleep_interval_mmsec(obj.m_atll_worker_sleep_interval_mmsec.load(std::memory_order_relaxed));
			int n_reload_count(0);

			while (obj.m_b_run_th_worker) {
				do {
					if (!obj.m_q.try_pop(ptr_req_new)) {
						ptr_req_new.reset();
					}

					if (ptr_req_new) {
						if (ptr_req_cur) {
							// case : ptr_req_new & ptr_req_cur
							ptr_req_cur = obj._execute(ptr_req_new, ptr_req_cur);
							ptr_req_new.reset();
						}
						else {
							// case : ptr_req_new & !ptr_req_cur
							ptr_req_cur = obj._execute(ptr_req_new, ptr_req_cur);
							ptr_req_new.reset();
						}
					}
					else {
						if (ptr_req_cur) {
							// case : !ptr_req_new & ptr_req_cur
							b_completet = obj._continue(ptr_req_cur);
							if (b_completet) {
								ptr_req_cur.reset();
							}
						}
						else {
							// case : !ptr_req_new & !ptr_req_cur
							b_completet = true;
							//idle
						}
					}

				}while (false);

				++n_reload_count;
				if (n_reload_count >= (2000/ ll_worker_sleep_interval_mmsec)) {
					n_reload_count = 0;
					auto ll = obj.m_atll_worker_sleep_interval_mmsec.load(std::memory_order_relaxed);
					if (ll_worker_sleep_interval_mmsec != ll) {
						ll_worker_sleep_interval_mmsec = ll;
					}
				}
                std::this_thread::sleep_for(std::chrono::milliseconds(ll_worker_sleep_interval_mmsec));
            }//end while
        }
		//
	protected:
		std::shared_ptr<std::thread> m_ptr_th_worker;
		std::atomic<bool> m_b_run_th_worker;
        vcworker::_type_q m_q;

		clog* m_p_log;
		std::atomic_llong m_atll_worker_sleep_interval_mmsec;

	private://don't call these methods
		vcworker();
		vcworker(const vcworker&);
		vcworker& operator=(const vcworker&);

	};

}//the end of _mp namespace

