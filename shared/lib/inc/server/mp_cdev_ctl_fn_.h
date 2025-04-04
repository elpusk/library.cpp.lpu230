#pragma once

#include <functional>
#include <memory>
#include <map>
#include <mutex>
#include <thread>
#include <vector>
#include <deque>
#include <string>
#include <list>

#include <mp_type.h>
#include <mp_cqueue.h>
#include <mp_clog.h>
#include <mp_cwait.h>
#include <server/mp_cio_packet.h>
#include <hid/mp_clibhid_dev.h>

namespace _mp {
	class cdev_ctl_fn
	{
	protected:
		enum : int {
			_const_max_n_cnt_open = 255 // max m_n_cnt_open value in shared mode.
		};
	public:
		typedef	std::weak_ptr< cdev_ctl_fn >		type_wptr;
		typedef	std::shared_ptr< cdev_ctl_fn >	type_ptr;

	protected:

		/**
		* inner queue manager of request and so on.......
		* Use the sync object. this each member function must be protected from multi threading circumstance
		*/
		class _cq_mgmt {
		public:
			typedef	std::shared_ptr< cdev_ctl_fn::_cq_mgmt >	type_ptr;
		public:
			/**
			* inner queue of request and so on.......
			* Dont't use the sync object. It have to use in using this queue.
			*/
			class _cq {
			public:
				typedef	std::shared_ptr<_cq>	type_ptr;

				// 0 - request ptr, 1-event, 2 - event index ,3 - response ptr
				typedef	std::tuple< cio_packet::type_ptr, cwait::type_ptr, int, cio_packet::type_ptr> type_tuple_full;

			private:
				typedef	std::deque <_cq::type_tuple_full> _type_q;

			public:
				_cq();
				_cq(unsigned long n_session_number);
				virtual ~_cq();

				/**
				* @brief push item
				* @param req - pushed item
				* @return 0 - request ptr, 1-event, 2 - event index ,3 - response ptr
				*/
				_cq::type_tuple_full push_back(const cio_packet& req);


				/**
				* @brief pop front. option - remove the poped item.
				*
				* @param b_remove - true : remove after pop item.
				*
				* @return tuple type
				*
				*	first - request ptr.
				*
				*	second - this processing event object ptr.
				*
				*	third - the index of event object.
				*
				*	forth - response ptr
				*/
				_cq::type_tuple_full pop_front(bool b_remove);

				/**
				* @brief remove front
				*/
				void remove_front();

				/**
				* @brief check empty of request queue.
				*
				* @return true - empty. false - not empty.
				*/
				bool empty();

				/**
				* @brief remove all pushed item and recover-item
				*/
				void clear();
			private:
				unsigned long m_n_session_number; //only set on the constrcture

				_cq::_type_q m_q;

			private://don;t call these methods
			};
		private:
			//typedef	std::map<unsigned long, std::shared_ptr<std::deque<cio_packet::type_ptr>>>	_type_map_ptr_q;
			typedef	std::map<unsigned long, _cq_mgmt::_cq::type_ptr>	_type_map_ptr_q;

		public:
			/**
			* @brief push the new sync-request to sync-queue.(act_dev_transmit, act_dev_cancel, act_dev_write )
			* @param request - the pushed request.
			*
			*	first - request ptr.
			*
			*	second - this processing event object ptr.
			*
			*	third - the index of event object.
			*
			*	forth - response ptr
			*/
			cdev_ctl_fn::_cq_mgmt::_cq::type_tuple_full sync_push_back(const cio_packet& request);

			/**
			* @brief pop front. option - remove the poped item.
			* @param b_remove - true : remove after pop item.
			* @return tuple type
			*
			*	first - request ptr.
			*
			*	second - this processing event object ptr.
			*
			*	third - the index of event object.
			*
			*	forth - response ptr
			*/
			cdev_ctl_fn::_cq_mgmt::_cq::type_tuple_full sync_pop_front(bool b_remove);

			/**
			* @brief remove front of sync Queue
			*/
			void sync_remove_front();

			/**
			* @brief check if or not empty read-request queue of all session.
			* @return true - empty. false - not empty
			*/
			bool read_is_empty();

			/**
			* @brief push the new read-request to read-queue.
			* @param request - the pushed request.( this request must be read-request)
			* @return true - the first read request of all session, false - else
			* 
			*	if return is true, start_read() have to be executed.
			*/
			bool read_push_back(const cio_packet& request);

			/**
			* @brief from read-queue. pop_front current request set of all session.
			* @param n_session_number - the session number  of read-q.
			*
			*	if n_session_number is _MP_TOOLS_INVALID_SESSION_NUMBER(0xFFFFFFFF), set front value of all session
			* 
			* @param b_remove - true : remove front after pop-front item. false - not removed
			* @return vector of tuple(0 - request, 1-event, 2 - event index, 3 - response)
			*/
			std::vector<_cq_mgmt::_cq::type_tuple_full> read_pop_front(unsigned long n_session_number,bool b_remove);

			/**
			* @brief from read-queue. set response of current request set of all session.
			*  and set result event.
			* @param qi - processing result
			* @param b_also_cancel is
			* 
			*	true : result info set to all session front.
			* 
			*	false  : if qi.get_request_type() == req_cancel, result info set to the session front of qi.get_session_number()
			* 
			* @return vector of tuple(0 - request, 1-event, 2 - event index, 3 - response)
			*/
			std::vector<_cq_mgmt::_cq::type_tuple_full> read_set_response_front(cqitem_dev& qi,bool b_also_cancel = false);

			/**
			* @brief from read-queue. pop front and remove that processing is complete of all read queue.
			* @return vector of tuple(0 - request, 1-event, 2 - event index, 3 - response). event must be set!
			*/
			std::vector<_cq_mgmt::_cq::type_tuple_full> read_pop_front_remove_complete_of_all();

			/**
			* @brief remove front of read Queue of the session
			* @pararm n_session_number - session number.
			*/
			void read_remove_front(unsigned long n_session_number);

			/**
			* @brief remove front of read Queue of all session
			*/
			void read_remove_front_all();

		private:
			std::mutex m_mutex;

			// key is session number, value queue ptr of cio_packet ptr. ONLY read reqeust.
			// protected by m_mutex.
			_cq_mgmt::_type_map_ptr_q m_map_ptr_q_ptr_cur_req_read;

			// request that executed start_read()
			cio_packet::type_ptr m_ptr_cur_req_read;//

			// for sync io, the current info(tuple), 0-ptr req, 1-ptr_event, 2-event number, 3-ptr_rsp
			// protected by m_mutex.
			_cq_mgmt::_cq m_q_sync_req_evt_rsp;

		};
	protected:

	public:
		virtual ~cdev_ctl_fn();
	protected:
		cdev_ctl_fn();

		type_pair_bool_result_bool_complete _execute_open_sync(clog *p_log, const cio_packet& request, cio_packet& response);
		type_pair_bool_result_bool_complete _execute_close_sync(
			clog* p_log,
			const cio_packet& request,
			cio_packet& response,
			const std::wstring & s_dev_path
		);

		/**
		* @brief close device( decrease open conuter )
		* @return always result is true, and complete is true.
		*/
		type_pair_bool_result_bool_complete _execute_close(clog* p_log, const cio_packet& request, cio_packet& response);

		type_pair_bool_result_bool_complete _execute_read(
			clog* p_log,
			const cio_packet& request,
			cio_packet& response,
			const std::wstring& s_dev_path
			);
		type_pair_bool_result_bool_complete _execute_write(
			clog* p_log,
			const cio_packet& request,
			cio_packet& response,
			const std::wstring& s_dev_path
		);
		type_pair_bool_result_bool_complete _execute_transmit(
			clog* p_log,
			const cio_packet& request,
			cio_packet& response,
			const std::wstring& s_dev_path
		);
		type_pair_bool_result_bool_complete _execute_cancel(
			clog* p_log,
			const cio_packet& request,
			cio_packet& response,
			const std::wstring& s_dev_path
		);

		type_pair_bool_result_bool_complete _execute_aync(
			clog* p_log,
			const cio_packet& request,
			cio_packet& response,
			const std::wstring& s_dev_path
		);

		
		bool _execute_general_error_response(clog* p_log, cio_packet& request, cio_packet& response, cio_packet::type_error_reason n_reason = cio_packet::error_reason_none);

		/**
		* @brief set response.
		* @param out_response - output response
		* @param in_qi - processing result info
		* @param in_request - the request of this processing. 
		* @return
		*	true - response setting complete with success or error
		*	
		*	false - response cannot be set.(not yet processing complete)
		*/
		static bool _set_response(cio_packet& out_response, cqitem_dev& in_qi, cio_packet& in_request);

		/**
		* @brief set response.
		* @param out_response - output response
		* @param result - result code.
		* @param s_result - result description string.
		* @param in_request - the request of this processing.
		* @return
		*	true - response setting complete with success or error
		*
		*	false - response cannot be set.(not yet processing complete)
		*/
		static void _set_response_result_only(cio_packet& out_response, cqitem_dev::type_result result,const std::wstring & s_result, cio_packet& in_request);

		/**
		* @brief check openable condition
		* @param b_is_support_by_shared_open_device - if or not a device is supporting the shared open.
		* @param b_is_request_shared_open_by_user - open request need a shared mode.
		* @return true - the given condition can try open device.
		* 
		*	false - the given condition can't try open device.
		*/
		bool _is_openable(bool b_is_support_by_shared_open_device, bool b_is_request_shared_open_by_user) const;

		/**
		* @bref get device path of cdev_ctl object.
		* @return device path
		*/
		const std::wstring _get_device_path() const;


	protected://callback of dev control

		/**
		* callback of clibhid_dev.start_read on exclusive mode
		* @param first cqitem_dev reference
		* @param second user data
		* @return true -> complete, false -> read more
		*/
		static bool _cb_dev_read_on_exclusive(cqitem_dev& qi, void*p_user);

		/**
		* callback of clibhid_dev.start_read on shared mode
		* @param first cqitem_dev reference
		* @param second user data
		* @return true -> complete, false -> read more
		*/
		static bool _cb_dev_read_on_shared(cqitem_dev& qi, void* p_user);

		/**
		* callback of clibhid_dev.start_write. start_write_read. start_cancel.( sync style)
		* @param first cqitem_dev reference
		* @param second user data
		* @return true -> complete, false -> read more
		*/
		static bool _cb_dev_for_sync_req(cqitem_dev& qi, void* p_user);

	protected:
		std::mutex m_mutex_for_open;
		int m_n_cnt_open;//open counter
		bool m_b_shared_mode;// current is shared open mode.
		std::wstring m_s_dev_path;// current device apth

		cdev_ctl_fn::_cq_mgmt m_mgmt_q;

	private://don't call these methods
		cdev_ctl_fn(const cdev_ctl_fn&);
		cdev_ctl_fn& operator=(const cdev_ctl_fn&);

	};

}//the end of _mp namespace

