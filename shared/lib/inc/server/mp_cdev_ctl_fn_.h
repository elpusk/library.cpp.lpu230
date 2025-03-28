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
		* inner queue of request and so on.......
		*/
		class _cq {
		public:
			typedef	std::shared_ptr<_cq>	type_ptr;

			// 0 - request, 1-event, 2 - event index.
			//typedef	std::tuple< cio_packet::type_ptr, cwait::type_ptr, int> type_tuple;

			// 0 - request, 1-event, 2 - event index ,3 - response ptr
			typedef	std::tuple< cio_packet::type_ptr, cwait::type_ptr, int, cio_packet::type_ptr> type_tuple_full;

		private:
			typedef	std::deque <_cq::type_tuple_full> _type_q;

		public:
			_cq( unsigned long n_session_number);
			virtual ~_cq();

			/**
			* @brief push item
			* @param req - pushed item
			* @return first - event of pushed item, second - the index of event. third - response ptr
			* 
			*	if first is false, fail push item.
			*/
			std::tuple<cwait::type_ptr, int, cio_packet::type_ptr> push_back(const cio_packet& req);


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
			_cq::type_tuple_full pop_front( bool b_remove );

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
			* @brief save item for recovering
			* @return 
			*	true - save ok.
			* 
			*	false - already the saved item.
			*/
			bool save_for_recover(const _cq::type_tuple_full &item);

			/**
			* @brief save item for recovering
			* @return
			*	true - save ok.
			* 
			*	false - already the saved item.
			*/
			bool save_for_recover(const cio_packet::type_ptr & ptr_req, const cwait::type_ptr& ptr_evt, int n_event, const cio_packet::type_ptr& ptr_rsp);

			/**
			* @brief recover from saved item to front of queue
			* @return
			*	true - recover  ok.
			* 
			*	false - none the save item
			*/
			bool recover_to_front();

		private:
			unsigned long m_n_session_number; //only set on the constrcture

			//std::mutex m_mutex;
			_cq::_type_q m_q;
			_cq::type_tuple_full m_must_be_recoverd_read_item; // this member must be recovered after processing a sync request.

		private://don;t call these methods
			_cq();
		};

	protected:
		//typedef	std::map<unsigned long, std::shared_ptr<std::deque<cio_packet::type_ptr>>>	_type_map_ptr_q;
		typedef	std::map<unsigned long, cdev_ctl_fn::_cq::type_ptr>	_type_map_ptr_q;

	public:
		virtual ~cdev_ctl_fn();
	protected:
		cdev_ctl_fn();

		type_pair_bool_result_bool_complete _execute_open_sync(clog *p_log, const cio_packet& request, cio_packet& response);
		type_pair_bool_result_bool_complete _execute_close_sync(
			clog* p_log,
			const cio_packet& request,
			cio_packet& response,
			unsigned long n_conected_session_number, const std::wstring & s_dev_path
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
			unsigned long n_conected_session_number, const std::wstring& s_dev_path
			);
		type_pair_bool_result_bool_complete _execute_write(
			clog* p_log,
			const cio_packet& request,
			cio_packet& response,
			unsigned long n_conected_session_number, const std::wstring& s_dev_path
		);
		type_pair_bool_result_bool_complete _execute_transmit(
			clog* p_log,
			const cio_packet& request,
			cio_packet& response,
			unsigned long n_conected_session_number, const std::wstring& s_dev_path
		);
		type_pair_bool_result_bool_complete _execute_cancel(
			clog* p_log,
			const cio_packet& request,
			cio_packet& response,
			unsigned long n_conected_session_number, const std::wstring& s_dev_path
		);

		type_pair_bool_result_bool_complete _execute_aync(
			clog* p_log,
			const cio_packet& request,
			cio_packet& response,
			unsigned long n_conected_session_number, const std::wstring& s_dev_path
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
		bool _set_response(cio_packet& out_response, cqitem_dev& in_qi, cio_packet& in_request);
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
		* @brief for async response, push the request on it's session queue, 
		* only use on cio_packet::act_dev_transmit, cio_packet::act_dev_cancel and cio_packet::act_dev_write
		* 
		* @param req - pushed item
		* 
		* @param b_save_for_recover - true : before pushing new item, save the front item of all session queue for recovering. 
		* 
		*	If device isn't shared open, b_save_for_recover is ignored
		* 
		* @return first - event, second event index(0 or positive value), third response ptr
		*/
		std::tuple<cwait::type_ptr, int, cio_packet::type_ptr> _push_back_request_for_response(const cio_packet& req, bool b_save_for_recover);

		/**
		* @brief for async response, push the request on it's session queue and check read request is running
		* only use on cio_packet::act_dev_read
		*
		* @param req - pushed item
		*
		* @return true - read is running already. false - not running read-request
		*/
		bool _push_back_request_for_response_check_running_read_request(const cio_packet& req);

		/**
		* @brief pop current request, for  async response
		* @param n_session_number - session number of poped queue.
		* @param b_remove - true : remove item after pop.
		* @return tuple(0 - request, 1-event, 2 - event index.)
		*/
		cdev_ctl_fn::_cq::type_tuple_full _pop_front_cur_reqeust_for_response(unsigned long n_session_number,bool b_remove);

		/**
		* @brief if front item type is act, pop current request set of all session. 
		* @param act : search type
		* @param b_remove - true : remove item after pop.
		* @return list of tuple(0 - request, 1-event, 2 - event index.)
		*/
		std::list<cdev_ctl_fn::_cq::type_tuple_full> _pop_front_cur_reqeust_for_response_as_act(cio_packet::type_act act,bool b_remove);

		/**
		* @brief if front item type is act, pop current request set of all session.
		* @param act : search type
		* @param qi - processing result inforamtion
		* @return vector of tuple(0 - request, 1-event, 2 - event index, 3 - response)
		*/
		std::vector<cdev_ctl_fn::_cq::type_tuple_full> _pop_front_and_remove_cur_reqeust_for_response_as_act(
			cio_packet::type_act act, 
			cqitem_dev& qi
		);

		/**
		* @brief reomve front item of request queue of response. (option : recover a item)
		* @param n_session_number - session number of poped queue.
		* @param b_recover_on_all_q - Do recover operation of all queue
		* @return std::set<std::pair<unsigned long,cio_packet::type_act>> , set of recoverd session numbers and act code.
		*/
		std::set<std::pair<unsigned long, cio_packet::type_act>> _remove_front_for_response(unsigned long n_session_number, bool b_recover_on_all_q);


		/**
		* @brief check if or not the current request is read.
		* 
		*	this function must be call before executing  _push_back_request_for_response().
		* 
		* @return 
		*	true - now running read-request.
		*	false - NO.
		*/
		bool is_running_read_request();

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
		* callback of clibhid_dev.start_write. start_write_read.( sync style)
		* @param first cqitem_dev reference
		* @param second user data
		* @return true -> complete, false -> read more
		*/
		static bool _cb_dev_for_sync_req(cqitem_dev& qi, void* p_user);

		/**
		* callback of clibhid_dev.start_cancel
		* @param first cqitem_dev reference
		* @param second user data
		* @return true -> complete, false -> read more
		*/
		static bool _cb_dev_cancel(cqitem_dev& qi, void* p_user);

	protected:
		std::mutex m_mutex_for_open;
		int m_n_cnt_open;//open counter
		bool m_b_shared_mode;// current is shared open mode.
		std::wstring m_s_dev_path;// current device apth

		std::mutex m_mutex_for_map;
		// key is session number, value queue ptr of cio_packet ptr.
		cdev_ctl_fn::_type_map_ptr_q m_map_ptr_q_ptr_cur_req;

	private://don't call these methods
		cdev_ctl_fn(const cdev_ctl_fn&);
		cdev_ctl_fn& operator=(const cdev_ctl_fn&);

	};

}//the end of _mp namespace

