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
#include <mp_clog.h>
#include <mp_cwait.h>
#include <server/mp_cio_packet.h>
#include <server/mp_cbase_ctl_fn_.h>
#include <hid/mp_clibhid_dev.h>

namespace _mp {

	/**
	* @brief sub processing of _execute() & _continue()
	*/
	class cdev_ctl_fn : public cbase_ctl_fn
	{
	public:
		typedef	std::weak_ptr< cdev_ctl_fn >	type_wptr;
		typedef	std::shared_ptr< cdev_ctl_fn >	type_ptr;

		// 0 - request ptr, 1-event ptr, 2 - event index ,3 - response ptr
		typedef	std::tuple< cio_packet::type_ptr, cwait::type_ptr, int, cio_packet::type_ptr> type_tuple_full;

		// vector of tuple : 0 - request ptr, 1-event ptr, 2 - event index ,3 - response ptr
		typedef std::vector< cdev_ctl_fn::type_tuple_full > type_v_tuple_full;

		typedef std::shared_ptr< std::map<unsigned long, _mp::type_ptr_v_buffer > > type_ptr_map_ptr_rx;

		// 0 - processing result, 1 - processing complete, 2 - response packet ptr
		typedef std::tuple<bool, bool, cio_packet::type_ptr> type_result_event;

		// first - request ptr, second - response ptr
		typedef std::pair<cio_packet::type_ptr, cio_packet::type_ptr> type_pair_ptr_req_ptr_rsp;

		typedef std::shared_ptr< std::vector< cdev_ctl_fn::type_pair_ptr_req_ptr_rsp> > type_ptr_v_pair_ptr_req_ptr_rsp;

	private:
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

			private:
				typedef	std::deque <type_tuple_full> _type_q;

			public:
				_cq();
				_cq(unsigned long n_session_number);
				virtual ~_cq();

				/**
				* @brief push item.
				* 
				*	- copy request by deep copy.
				* 
				*	- create response
				* 
				*	- create notification event
				* 
				* @param ptr_request - pushed item ptr
				* 
				* @return 0 - request ptr, 1-event, 2 - event index ,3 - response ptr(created)
				*/
				type_tuple_full push_back(const cio_packet::type_ptr& ptr_request);


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
				type_tuple_full pop_front(bool b_remove);

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
			* @param request - the pushed request ptr.
			*
			*	first - request ptr.
			*
			*	second - this processing event object ptr.
			*
			*	third - the index of event object.
			*
			*	forth - response ptr
			*/
			cdev_ctl_fn::type_tuple_full qm_sync_push_back(const cio_packet::type_ptr& ptr_request);

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
			cdev_ctl_fn::type_tuple_full qm_sync_pop_front(bool b_remove);

			/**
			* @brief remove front of sync Queue
			*/
			void qm_sync_remove_front();

			/**
			* @brief check if or not empty read-request queue of all session.
			* @return true - empty. false - not empty
			*/
			bool qm_read_is_empty();

			/**
			* @brief push the new read-request to read-queue.
			* @param request - the pushed request ptr.( this request must be read-request)
			* @return true - the first read request of all session, false - else
			*
			*	if return is true, start_read() have to be executed.
			*/
			bool qm_read_push_back(const cio_packet::type_ptr& ptr_request);

			/**
			* @brief from read-queue. pop_front current request set of all session.
			* @param n_session_number - the session number  of read-q.
			*
			*	if n_session_number is _MP_TOOLS_INVALID_SESSION_NUMBER(0xFFFFFFFF), set front value of all session
			*
			* @param b_remove - true : remove front after pop-front item. false - not removed
			* @return vector of tuple(0 - request, 1-event, 2 - event index, 3 - response)
			*/
			std::vector<cdev_ctl_fn::type_tuple_full> qm_read_pop_front(unsigned long n_session_number, bool b_remove);

			/**
			* @brief from read-queue. set response of current request set of all session.
			* 
			*	and set result event. if a request have recover flag, the reponse of request is NOT set,
			*	
			*	and removed recover flag.
			* 
			* @param qi - processing result
			*
			* @return vector of tuple(0 - request, 1-event, 2 - event index, 3 - response)
			*/
			std::vector<cdev_ctl_fn::type_tuple_full> qm_read_set_response_front(cqitem_dev& qi);

			
			/**
			* @brief from read-queue. set the current request of all session to recover.
			* @param n_session_number - the session number of read-q.
			* @return size_t - the number of request set to recover.
			*/
			size_t qm_read_set_request_front_to_recover_of_another_session(unsigned long n_session_number);
			
			
			/**
			* @brief from read-queue. pop front and remove that processing is complete of all read queue.
			* @return vector of tuple(0 - request, 1-event, 2 - event index, 3 - response). event must be set!
			*/
			std::vector<cdev_ctl_fn::type_tuple_full> qm_read_pop_front_remove_complete_of_all();

			/**
			* @brief from read-queue. pop front and remove that processing is complete of all read queue.
			* @return vector ptr of pair( request ptr, response ptr )
			*/
			std::shared_ptr< std::vector< std::pair<cio_packet::type_ptr,cio_packet::type_ptr>> > qm_read_pop_front_remove_complete_of_all_by_rsp_ptr();

			/**
			* @brief remove front of read Queue of the session
			* @pararm n_session_number - session number.
			*/
			void qm_read_remove_front(unsigned long n_session_number);

			/**
			* @brief remove front of read Queue of all session
			*/
			void qm_read_remove_front_all();

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


	private:
		/**
		* callback of clibhid_dev.start_read on exclusive mode
		* @param first cqitem_dev reference
		* @param second user data
		* @return true -> complete, false -> read more
		*/
		static bool _cb_dev_read_on_exclusive(cqitem_dev& qi, void* p_user);

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

	private:
		/**
		* @brief get the combinatin state of another session.
		* 
		*	must be called on protection by mutex.
		* 
		* @param n_session - session number.
		* @return pair type
		*
		*	first - true : another session state exsit. false : none  another session state
		*
		*	second - the generated combinatin state.  if anther session state is none, st_not.
		*
		*	Generation rule: The largest number (ASY) among the states in each section is used as the combination state.
		*/
		std::pair<bool, cbase_ctl_fn::_cstate::type_state> _get_state_another_(unsigned long n_session);

		/**
		* @brief get the current state of the given session.
		*
		*	must be called on protection by mutex.
		*
		* @param n_session - session numner,
		* @return
		*	first - true(There is a session), false(none session)
		*
		*	second - the current state of the given session.
		*
		*	third - the combination state of all session.
		*/
		std::tuple<bool, cbase_ctl_fn::_cstate::type_state, cbase_ctl_fn::_cstate::type_state> _get_state_cur_(unsigned long n_session) const;

		/**
		* @brief ini status. reset map( session to state )
		*
		*	must be called on protection by mutex.
		*
		*	1. change mode.
		*
		*	2. combination state set to  st_not(default)
		*
		*	3. remove all state map.
		*
		* @param b_shared_mode - true(do by shared mode), false(ini by exclusive mode)
		*/
		void _reset_(bool b_shared_mode);

	public:


		/**
		* @brief process request. transaction state with the given event.
		*
		* @param ptr_req_new[in] - new request ptr ( must be allocated ! )
		* 
		* @param ptr_req_cur[in] - currnet request ptr
		* 
		* @return result type - this ptr must be allocated in the body of this function.
		*/
		cbase_ctl_fn::cresult::type_ptr process_event
		(
			const cio_packet::type_ptr & ptr_req_new,
			const cio_packet::type_ptr& ptr_req_cur
		);

		/**
		* @brief get all completed response for sending to clients.
		* 
		* @param pair_ptr_req_ptr_rsp_additional[in] : If the element is not already in the returned vector, add it to the end of the vector.
		* 
		* @return std::shared_ptr<std::vector<cio_packet::type_ptr>> - the vector ptr of pair( req ptr,rsp ptr ).
		*
		*	The order of vector is the order in which it is sent.
		* 
		*	If the returned vector is allocated, it have to a item.( size() must be greater then 0)
		* 
		*	If the sync item is exsited on vector, it is posited on the last position.( asyn-s-first, sync-last)
		* 
		*	Requests that are not pushed, such as open and close, are not included in the return vector except the given parameter.
		*/
		cdev_ctl_fn::type_ptr_v_pair_ptr_req_ptr_rsp get_all_complete_response
		(
			const cdev_ctl_fn::type_pair_ptr_req_ptr_rsp & pair_ptr_req_ptr_rsp_additional
			= std::make_pair(cio_packet::type_ptr(), cio_packet::type_ptr())
		);

		/**
		* @brief get the set front value of all session on read queue.
		* @return the vector of req ptr of front all read queue.
		*/
		std::vector<cio_packet::type_ptr> get_front_of_read_queue();

		/**
		* @brief the job of this functions.
		* 
		*	this function must be call at occuring rx-event in processing a async-request.
		*
		*	1. transit state of all completed async req.
		*
		*	2. if need to change mode, this function will change the mode.(shared/exclusive)
		*
		*	3. this function is designed for state transition by aync function. 
		*
		* @param [in] v_tuple_full - vector of tuple : 0 - request ptr, 1-event ptr, 2 - event index ,3 - response ptr
		*
		*/
		void transit_state_for_only_all_async_by_rx_event(const cdev_ctl_fn::type_v_tuple_full & v_tuple_full);

		/**
		* @brief the job of this functions.
		*
		*	1. transit state by a request event.
		*
		*	2. if need to change mode, this function will change the mode.(shared/exclusive)
		*
		*	3. add new session to map in open request if the condition is satisfied with creating it.
		*
		*	4. remove a session from map in close request if the condition is satisfied with removing it.
		*
		* @param [in/out]result - result class instance. DON't touch result.b_processing_complete member in this function.
		*
		* @param [in]b_user_shared_mode_on_open_request - true : open request is shared mode. false : exclusive mode.
		*
		*	this paramer is used only in open request.
		*/
		void transit_state_by_processing_result(cbase_ctl_fn::cresult& result, bool b_user_shared_mode_on_open_request = false);

	private:
		/**
		* @brief process transaction state by the given event.
		*
		* 	this function have to be called in process_event() on exclusive mode.
		*
		* @param ptr_req_new[in] - new request ptr
		* @param ptr_req_cur[in] - current request ptr
		* @return cresult::type_ptr - event pricessing total info ptr.
		*/
		cbase_ctl_fn::cresult::type_ptr _process_event_on_exclusive_mode
		(
			const cio_packet::type_ptr& ptr_req_new,
			const cio_packet::type_ptr& ptr_req_cur
		);

		/**
		* @brief process st_not statse by the given event.
		*
		* 	this function have to be called in _process_event_on_exclusive_mode().
		*
		* @param cresult[in/out] - result of processing.
		*
		*	ptr_req member of cresult class must be set before be calling.
		*/
		void _process_exclusive_st_not(cbase_ctl_fn::cresult& result);

		/**
		* @brief process st_idl statse by the given event.
		*
		* 	this function have to be called in _process_event_on_exclusive_mode().
		*
		* @param cresult[in/out] - result of processing.
		*
		*	ptr_req member of cresult class must be set before be calling.
		*/
		void _process_exclusive_st_idl(cbase_ctl_fn::cresult& result);

		/**
		* @brief process st_asy statse by the given event.
		*
		* 	this function have to be called in _process_event_on_exclusive_mode().
		*
		* @param cresult[in/out] - result of processing.
		*
		*	ptr_req member of cresult class must be set before be calling.
		*/
		void _process_exclusive_st_asy(cbase_ctl_fn::cresult& result);

		/**
		* @brief transaction state by the given event.
		*
		* 	this function have to be called in process_event() on shared mode.
		*
		* @param ptr_req_new[in] - new request ptr
		* @param ptr_req_cur[in] - current request ptr
		* @return cresult::type_ptr - event processing total info ptr.
		*/
		cbase_ctl_fn::cresult::type_ptr _process_event_on_shared_mode
		(
			const cio_packet::type_ptr& ptr_req_new,
			const cio_packet::type_ptr& ptr_req_cur
		);

		/**
		* @brief process the selected session state is st_not state
		*	and another session combination state is st_not by the given event.
		*
		* 	this function have to be called in _process_event_on_shared_mode().
		*
		* @param cresult[in/out] - result of processing.
		*
		*	ptr_req member of cresult class must be set before be calling.
		*/
		void _process_shared_selected_session_st_not_another_session_st_not(cbase_ctl_fn::cresult& result);

		/**
		* @brief process the selected session state is st_not state
		*	and another session combination state is st_idl by the given event.
		*
		* 	this function have to be called in _process_event_on_shared_mode().
		*
		* @param cresult[in/out] - result of processing.
		*
		*	ptr_req member of cresult class must be set before be calling.
		*/
		void _process_shared_selected_session_st_not_another_session_st_idl(cbase_ctl_fn::cresult& result);

		/**
		* @brief process the selected session state is st_not state
		*	and another session combination state is st_asy by the given event.
		*
		* 	this function have to be called in _process_event_on_shared_mode().
		*
		* @param cresult[in/out] - new req result of processing.
		* 
		* @param ptr_req_cur[in] - current req ptr
		*
		*	ptr_req member of cresult class must be set before be calling.
		*/
		void _process_shared_selected_session_st_not_another_session_st_asy
		(
			cbase_ctl_fn::cresult& result_new,
			const cio_packet::type_ptr& ptr_req_cur
		);

		/**
		* @brief process the selected session state is st_idl state
		*	and another session combination state is st_not by the given event.
		*
		* 	this function have to be called in _process_event_on_shared_mode().
		*
		* @param cresult[in/out] - result of processing.
		*
		*	ptr_req member of cresult class must be set before be calling.
		*/
		void _process_shared_selected_session_st_idl_another_session_st_not(cbase_ctl_fn::cresult& result);

		/**
		* @brief process the selected session state is st_idl state
		*	and another session combination state is st_idl by the given event.
		*
		* 	this function have to be called in _process_event_on_shared_mode().
		*
		* @param cresult[in/out] - result of processing.
		*
		*	ptr_req member of cresult class must be set before be calling.
		*/
		void _process_shared_selected_session_st_idl_another_session_st_idl(cbase_ctl_fn::cresult& result);

		/**
		* @brief process the selected session state is st_idl state
		*	and another session combination state is st_asy by the given event.
		*
		* 	this function have to be called in _process_event_on_shared_mode().
		*
		* @param cresult[in/out] - result of processing.
		* @param ptr_req_cur[in] - current req ptr.
		* 
		*	ptr_req member of cresult class must be set before be calling.
		*/
		void _process_shared_selected_session_st_idl_another_session_st_asy
		(
			cbase_ctl_fn::cresult& result,
			const cio_packet::type_ptr& ptr_req_cur
		);

		/**
		* @brief process the selected session state is st_asy state
		*	and another session combination state is st_not by the given event.
		*
		* 	this function have to be called in _process_event_on_shared_mode().
		*
		* @param cresult[in/out] - result of processing.
		* @param ptr_req_cur[in] - current req ptr
		*
		*	ptr_req member of cresult class must be set before be calling.
		*/
		void _process_shared_selected_session_st_asy_another_session_st_not
		(
			cbase_ctl_fn::cresult& result,
			const cio_packet::type_ptr& ptr_req_cur
		);

		/**
		* @brief process the selected session state is st_asy state
		*	and another session combination state is st_idl by the given event.
		*
		* 	this function have to be called in _process_event_on_shared_mode().
		*
		* @param cresult[in/out] - result of processing.
		* @param ptr_req_cur[in] current req ptr.
		*
		*	ptr_req member of cresult class must be set before be calling.
		*/
		void _process_shared_selected_session_st_asy_another_session_st_idl
		(
			cbase_ctl_fn::cresult& result,
			const cio_packet::type_ptr& ptr_req_cur
		);

		/**
		* @brief process the selected session state is st_asy state
		*	and another session combination state is st_asy by the given event.
		*
		* 	this function have to be called in _process_event_on_shared_mode().
		*
		* @param cresult[in/out] - result of processing.
		* @param ptr_req_cur[in] - current req ptr.
		*
		*	ptr_req member of cresult class must be set before be calling.
		*/
		void _process_shared_selected_session_st_asy_another_session_st_asy
		(
			cbase_ctl_fn::cresult& result,
			const cio_packet::type_ptr& ptr_req_cur
		);

		/**
		* @brief process open event.
		*
		*	If the crrent state is openabled, this functon will be called in _process_x().
		*
		*	Before calling this function, the request must be set in the result class.
		*
		*	1. process get-parameters
		*
		*	2. process low level lib.(optional)
		*
		*	3. set result.
		*
		*	4. transit state by the result.
		*
		* @param [in/out]result - result class instance.
		*
		* @param [in]b_need_low_level - true : need to call low level function. false : not need to call low level function.
		*
		* @return true : success, false : error.
		*/
		bool _proc_event_open(cbase_ctl_fn::cresult& result, bool b_need_low_level);

		/**
		* @brief start sync pattern request transaction. and wait for complete.
		*
		*	This function does "not" remove the request from the sync pattern request queue.
		*
		*	The callback function used in this function only sets the result for the response of the first request in the sync pattern request queue
		*
		*	and triggers an event, but it does "not" remove the request from the sync pattern request queue.
		*
		* @param [in] s_dev_path - device path.
		* @param [in] ptr_req - request packet.
		* @return cio_packet::type_ptr : ptr response packet , if( !return )-> error.
		*/
		cio_packet::type_ptr _start_and_complete_by_sync_req
		(
			const std::wstring& s_dev_path,
			const cio_packet::type_ptr& ptr_req
		);

		/**
		* @brief start cancel(sync,self generated) pattern request transaction. and wait for complete.
		*
		*	This function does "not" remove the request from the sync pattern request queue.
		*
		*	The callback function used in this function only sets the result for the response of the first request in the sync pattern request queue
		*
		*	and triggers an event, but it does "not" remove the request from the sync pattern request queue.
		*
		* @param [in] s_dev_path - device path.
		* @param [in] ptr_req - request packet.
		* @return true: success, false: error.
		*/
		bool _start_and_complete_by_cancel_self_req
		(
			const std::wstring& s_dev_path,
			const cio_packet::type_ptr& ptr_req
		);


		/**
		* @brief start async pattern request transaction. and not wait for complete.
		*
		*	This function does "not" remove the request from the async pattern request queue.
		*
		*	The callback function used in this function only sets the result for the response of the first request in the async pattern request queue
		*
		*	,but it does "not" remove the request from the sync pattern request queue or "not" tiggres an event.
		*
		* @param [in] s_dev_path - device path.
		* @param [in] ptr_req - request packet.
		* @return true : success starting-transaction. false : error.
		*/
		bool _start_by_async_req
		(
			const std::wstring& s_dev_path,
			const cio_packet::type_ptr& ptr_req
		);

		/**
		* @brief if a state is changed in transit_state_by_processing_result(),
		* 
		*	logging and trace state information. this function will be used for debugging.
		* 
		*	but it is included at release build time also. 
		*/
		void _logging_if_state_is_changed();

	public:
		cdev_ctl_fn(clog* p_log);
		virtual ~cdev_ctl_fn();

	protected:
		std::mutex m_mutex_for_state;
		bool m_b_cur_shared_mode;// the current open mode : true - shared mode, false - exclusive mode(default)
		bool m_b_cur_shared_mode_old_for_debug; // For runtime debugging.

		// key is session number, value  the current status ptr.
		cbase_ctl_fn::_cstate::type_map_ptr_state m_map_ptr_state_cur;
		cbase_ctl_fn::_cstate::type_map_ptr_state m_map_ptr_state_cur_old_for_debug; // For runtime debugging.

		// current combination state of all session
		cbase_ctl_fn::_cstate::type_state m_st_combi;
		cbase_ctl_fn::_cstate::type_state m_st_combi_old_for_debug; // For runtime debugging.

		// device path
		std::wstring m_s_dev_path;

		// manager of request and response Q
		cdev_ctl_fn::_cq_mgmt m_mgmt_q;

	private://don't call these methods
		cdev_ctl_fn();
		cdev_ctl_fn(const cdev_ctl_fn&);
		cdev_ctl_fn& operator=(const cdev_ctl_fn&);

	};

}//the end of _mp namespace

