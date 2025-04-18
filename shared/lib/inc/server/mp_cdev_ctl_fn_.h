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

		// 0 - request ptr, 1-event, 2 - event index ,3 - response ptr
		typedef	std::tuple< cio_packet::type_ptr, cwait::type_ptr, int, cio_packet::type_ptr> type_tuple_full;

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

			private:
				typedef	std::deque <type_tuple_full> _type_q;

			public:
				_cq();
				_cq(unsigned long n_session_number);
				virtual ~_cq();

				/**
				* @brief push item
				* @param req - pushed item
				* @return 0 - request ptr, 1-event, 2 - event index ,3 - response ptr
				*/
				type_tuple_full push_back(const cio_packet& req);


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
			cdev_ctl_fn::type_tuple_full sync_push_back(const cio_packet& request);

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
			cdev_ctl_fn::type_tuple_full sync_pop_front(bool b_remove);

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
			std::vector<cdev_ctl_fn::type_tuple_full> read_pop_front(unsigned long n_session_number, bool b_remove);

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
			std::vector<cdev_ctl_fn::type_tuple_full> read_set_response_front(cqitem_dev& qi, bool b_also_cancel = false);

			/**
			* @brief from read-queue. pop front and remove that processing is complete of all read queue.
			* @return vector of tuple(0 - request, 1-event, 2 - event index, 3 - response). event must be set!
			*/
			std::vector<cdev_ctl_fn::type_tuple_full> read_pop_front_remove_complete_of_all();

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

		/**
		* innner class for status transaction control
		*
		*/
		class _cstatus_mgmt {

		public:
			class _cstate {
			public:
				typedef	std::shared_ptr< _cstatus_mgmt::_cstate> type_ptr;
				typedef std::map<unsigned long, _cstate::type_ptr> type_map_ptr_state;

				// event definition
				typedef enum : int {
					ev_none = -1, // none event
					ev_open = 0, //open event
					ev_close, //close event
					ev_sync, //sync event
					ev_asy, //async event
					ev_rx, // rx ent
					ev_total // the number of defined events
				}type_evt;

				// state definition
				typedef enum : int {
					st_undefined = -1, //undefined state
					st_not = 0, //not open
					st_idl, //open but none request
					st_asy, //waiting async response
					st_total // the number of defined state
				}type_state;

			public:
				//the result of event processing
				//0 - true(state is changed. get<1>==get<2>), 1- new state, 2- old state
				typedef	std::tuple<bool, _cstate::type_state, _cstate::type_state > type_result_evt;

			public:

				/**
				* @brief get comination state in map except selected session.
				* @param map_ptr_state_cur - state map
				* @param n_selected_session_number - selected session number
				* @return pair type
				*
				*	first - true : another session state exsit. false : none  another session state
				*
				*	second - the generated combinatin state.  if anther session state is none, st_not.
				*
				*	Generation rule: The largest number (ASY) among the states in each section is used as the combination state.
				*/
				static std::pair<bool, cdev_ctl_fn::_cstatus_mgmt::_cstate::type_state> get_combination_state_in_map_except_selected_session
				(
					const _cstatus_mgmt::_cstate::type_map_ptr_state& map_ptr_state_cur,
					unsigned long n_selected_session_number
				);

				/**
				* @brief get event type from request action.
				* @param act - request action
				* @return event type
				*/
				static _cstate::type_evt get_event_from_request_action(cio_packet::type_act act);

				/**
				* @brief get mask pattern from _cstate::type_state
				* @param st - input _cstate::type_state( st_not ~ st_asy )
				* @return mask pattern
				*	mask pattern = 0x1 << st;
				*
				*	0 - error
				*/
				static int get_mask_from_state(_cstate::type_state st);

				/**
				* @brief get state from mask
				* @param n_mask - 0x1, 0x2 or 0x04
				* @return _cstate::type_state( st_not ~ st_asy )
				*	(n_mask,return) -> (0x1,st_not), (0x2,st_idl) or (0x4,st_asy)
				*
				*	st_undefined - error
				*/
				static _cstate::type_state get_state_from_mask(int n_mask);
			public:
				_cstate();
				~_cstate();

				_cstate(const _cstatus_mgmt::_cstate& src);

				_cstate& operator=(const _cstatus_mgmt::_cstate& src);

				/**
				* @brief initialize
				*/
				void reset();

				/**
				* @brief get the current state
				* @return the current state
				*/
				_cstate::type_state get() const;

				/**
				* @brief get the last event
				* @return the last event
				*/
				_cstate::type_evt get_last_event() const;

				/**
				* @brief set state by the given event.
				* @param evt - the triggered event.
				* @return 0 - true(state is changed. get<1>==get<2>), 1- new state, 2- old state
				*/
				_cstate::type_result_evt set(_cstate::type_evt evt);
			private:
				static const int _m_trans_table_base[_cstatus_mgmt::_cstate::st_total][_cstatus_mgmt::_cstate::ev_total];

			private:
				_cstate::type_state m_st_cur; //the current state.
				_cstate::type_evt m_ev_last; // the last event
			};

			class _cresult {
			public:
				_cresult();

				_cresult(const _cresult& src);

				_cresult& operator=(const _cresult& src);

				_cresult(const cio_packet& request);

				void reset();

				/**
				* @brief get request packet.
				* @return the request packet.
				*/
				const cio_packet::type_ptr& get_req() const;

				/**
				* @brief get session number of the current processing.
				* @return the session number of the current processing.
				*/
				unsigned long get_session_number() const;


				/**
				* @brief get event of the current processing.
				* @return the current processing event. Be calculated by request packet(m_ptr_req)
				*/
				_cstate::type_evt get_cur_event() const;

				/**
				* @brief set request packet. and create default response in function body.
				*
				* @param request - request packet
				* @return created default(set error response) response ptr
				*/
				cio_packet::type_ptr process_set_req_packet(const cio_packet& request);

				/**
				* @brief get the result of processing. 
				* @return pair< bool, bool > type
				* 
				*	first - true( processing result is success), false( processing result is error)
				* 
				*	second - true( processing is complete), false( processing is not complete)
				*/
				type_pair_bool_result_bool_complete process_get_result() const;

				/**
				* @brief set ptr_rsp member with error response.
				* 
				*	set member m_ptr_rsp.
				* 
				*	set m_b_process_result is false.
				* 
				*	set m_b_process_complete is true.
				* 
				*	set m_b_process_exist_session is false. if  n_reason is error_reason_session.
				* 
				*	set m_s_dev_path.
				* 
				* @param n_reason - the reason of error.
				* @param s_dev_path - the device path
				*/
				void process_set_rsp_with_error_complete
				(
					cio_packet::type_error_reason n_reason,
					const std::wstring & s_dev_path = std::wstring()
				);

				/**
				* @brief set ptr_rsp member with success response.

				*	set member m_ptr_rsp.
				*
				*	set m_b_process_result is true.
				*
				*	set m_b_process_complete is true.
				*
				*	set m_s_dev_path.
				* 
				* @param v_rx - the received data
				* @param s_dev_path - the device path
				* 
				*/
				void process_set_rsp_with_succss_complete
				(
					const _mp::type_v_buffer & v_rx,
					const std::wstring& s_dev_path = std::wstring()
				);

				/**
				* @brief set ptr_rsp member with success response.

				*	set member m_ptr_rsp.
				*
				*	set m_b_process_result is true.
				*
				*	set m_b_process_complete is true.
				*
				*	set m_s_dev_path.
				* 
				* @param s_data - the received data
				* @param s_dev_path - the device path
				*/
				void process_set_rsp_with_succss_complete
				(
					const std::wstring& s_data,
					const std::wstring& s_dev_path = std::wstring()
				);

				/**
				* @brief set ptr_rsp member with success and need more processing.
				* 
				*	no need to send to client.
				* 
				* @param s_dev_path - the device path
				*/
				void process_set_rsp_with_succss_ing
				(
					const std::wstring& s_dev_path = std::wstring()
				);

				/**
				* @brief check if or not changed state of the selected session.
				* @return true - changed, false - not changed
				*/
				bool is_changed_state_of_selected_session() const;

				/**
				* @brief check if or not changed state of all session.
				* @return true - changed, false - not changed
				*/
				bool is_changed_combination_state_of_all_session() const;

				/**
				* @brief set the selected session state.
				* @param st_new - new state of the selected session by processing.
				* @param st_old - old state of the selected session before processing.
				*/
				void set_selected_session_state(_cstatus_mgmt::_cstate::type_state st_new, _cstatus_mgmt::_cstate::type_state st_old);

				/**
				* @brief set the selected session state.
				* @param st_result - states of the selected session by processing.
				*/
				void set_selected_session_state(_cstatus_mgmt::_cstate::type_result_evt st_result);

				/**
				* @brief set combination state.
				* @param st_new - new combination state.
				* @param st_old - old combination state.
				*/
				void set_combination_state(_cstatus_mgmt::_cstate::type_state st_new, _cstatus_mgmt::_cstate::type_state st_old);

				/**
				* @brief get the combination state of all session.
				* @return pair type
				* 
				*	first - new combination state.
				* 
				*	second - old combination state.
				* 
				*/
				std::pair< _cstatus_mgmt::_cstate::type_state, _cstatus_mgmt::_cstate::type_state> get_combination_state() const;

			private:
				////////////////////////////////////////////////////
				// for state section members.
				// old state of the selected session before processing.
				_cstatus_mgmt::_cstate::type_state m_st_session_old;

				// new state of the selected session by processing.
				_cstatus_mgmt::_cstate::type_state m_st_session_new;

				// old combination state of all session before processing.
				_cstatus_mgmt::_cstate::type_state m_st_combination_old;

				// new combination state of all session by processing.
				_cstatus_mgmt::_cstate::type_state m_st_combination_new;

				/////////////////////////////////////////////////////
				// request processing section members
				// there is session on map.
				bool m_b_process_exist_session;

				// the result of processing
				bool m_b_process_result;

				// true - complete transaction.
				bool m_b_process_complete;

				cio_packet::type_ptr m_ptr_req; // request packet

				cio_packet::type_ptr m_ptr_rsp; // response packet

				std::wstring m_s_dev_path; // the opened device path in open req.
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

		public:
			_cstatus_mgmt();
			_cstatus_mgmt(bool b_shared_mode);
			~_cstatus_mgmt();

			/**
			* @brief ini status. reset map( session to state )
			* @param b_shared_mode - true(do by shared mode), false(ini by exclusive mode)
			*/
			void reset(bool b_shared_mode);

			/**
			* @brief get the current state of the given session.
			* @param n_session - session numner, 
			* @return 
			*	first - true(There is a session), false(none session)
			* 
			*	second - the current state of the given session.
			* 
			*	third - the combination state of all session.
			*/
			std::tuple<bool,_cstatus_mgmt::_cstate::type_state, _cstatus_mgmt::_cstate::type_state> get_cur_state(unsigned long n_session) const;

			/**
			* @brief get the current combination state of all session.
			* @return the current combination state of all session.
			*/
			_cstatus_mgmt::_cstate::type_state get_combination_cur_state() const;

			/**
			* @brief process request. transaction state with the given event.
			*	
			* @param request[in] - request
			* @return _cresult - event pricessing total info.
			* 
			*	_cresult.ptr_rsp is must be created in this function. 
			* 
			*	if _cresult.b_process_complete is true, this function set a data which is sent to client. 
			*/
			_cstatus_mgmt::_cstatus_mgmt::_cresult process_event(const cio_packet& request);

			size_t get_the_number_of_session() const;

			bool is_shared_mode() const;

		private:
			/**
			* @brief process transaction state by the given event.
			*
			* 	thi function have to be called in process_event() on exclusive mode.
			*
			* @param request[in] - request
			* @return _cresult - event pricessing total info.
			*/
			_cstatus_mgmt::_cresult _process_event_on_exclusive_mode(const cio_packet& request);

			/**
			* @brief process st_not statse by the given event.
			*
			* 	thi function have to be called in _process_event_on_exclusive_mode().
			*
			* @param _cresult[in/out] - result of processing.
			*	
			*	ptr_req member of _cresult class must be set before be calling.
			*/
			void _process_exclusive_st_not(_cstatus_mgmt::_cresult& result);

			/**
			* @brief process st_idl statse by the given event.
			*
			* 	thi function have to be called in _process_event_on_exclusive_mode().
			*
			* @param _cresult[in/out] - result of processing.
			*
			*	ptr_req member of _cresult class must be set before be calling.
			*/
			void _process_exclusive_st_idl(_cstatus_mgmt::_cresult& result);

			/**
			* @brief process st_asy statse by the given event.
			*
			* 	thi function have to be called in _process_event_on_exclusive_mode().
			*
			* @param _cresult[in/out] - result of processing.
			*
			*	ptr_req member of _cresult class must be set before be calling.
			*/
			void _process_exclusive_st_asy(_cstatus_mgmt::_cresult& result);

			/**
			* @brief transaction state by the given event.
			*
			* 	thi function have to be called in process_event() on shared mode.
			*
			* @param request[in] - request
			* @return _cresult - event pricessing total info.
			*/
			_cstatus_mgmt::_cresult _process_event_on_shared_mode(const cio_packet& request);

			/**
			* @brief transit state and result by a request event.
			* @param [in/out]result - result class instance. DON't touch result.b_processing_complete member in this function. 
			*/
			void _transit_state_by_processing_result(_cstatus_mgmt::_cresult &result);

			/**
			* @brief callback of self cancel on close request
			* @param first cqitem_dev reference
			* @param second user data
			* @return true -> complete, false -> read more
			*/
			static bool _cb_dev_for_self_cancel(cqitem_dev& qi, void* p_user);

			/**
			* @brief close with lower library layer. ( used in _execute_close() )
			* @param [in]p_log - log class instance pointer
			* @param [in/out]result
			* @return true : close success.
			*
			*	false : error. error info must be set to response.
			*/
			bool _close_by_close_req(clog* p_log, _cstatus_mgmt::_cresult& result);


			/**
			* @brief start sync pattern request transaction. and wait for complete.
			* @param [in/out]result
			* @return true : success transaction. false : error.	
			*/
			bool _start_and_complete_by_sync_req(_cstatus_mgmt::_cresult& result);

			/**
			* @brief start async pattern request transaction. and not wait for complete.
			* @param [in]p_log - log class instance pointer
			* @param [in/out]result
			* @return true : success starting-transaction. false : error.
			*/
			bool _start_by_async_req(clog* p_log, _cstatus_mgmt::_cresult& result);


			/**
			* @brief execute write request
			* @param [in]p_log - log class instance pointer
			* @param [in]request - write request packet.
			* @return _cresult : result class.
			*/
			_cstatus_mgmt::_cresult _execute_write(clog* p_log, const cio_packet& request);

			/**
			* @brief execute read request
			* @param [in]p_log - log class instance pointer
			* @param [in]request - read request packet.
			* @return _cresult : result class.
			*/
			_cstatus_mgmt::_cresult _execute_read(clog* p_log, const cio_packet& request);

			/**
			* @brief execute transmit request
			* @param [in]p_log - log class instance pointer
			* @param [in]request - transmit request packet.
			* @return _cresult : result class.
			*/
			_cstatus_mgmt::_cresult _execute_transmit(clog* p_log, const cio_packet& request);

			/**
			* @brief execute cancel request
			* @param [in]p_log - log class instance pointer
			* @param [in]request - cancel request packet.
			* @return _cresult : result class.
			*/
			_cstatus_mgmt::_cresult _execute_cancel(clog* p_log, const cio_packet& request);

		private: //state operation

			/**
			* @brief get the current state of the session.
			* @param n_session - session number.
			* @return pair type
			* 
			*	first - true : session state exsit. false : none  session state
			*	
			*	second - session sstate, if session state is none, st_not.
			*/
			std::pair<bool, _cstatus_mgmt::_cstate::type_state> _get_state(unsigned long n_session);

			/**
			* @brief get the combinatin state of another session.
			* @param n_session - session number.
			* @return pair type
			*
			*	first - true : another session state exsit. false : none  another session state
			*
			*	second - the generated combinatin state.  if anther session state is none, st_not.
			* 
			*	Generation rule: The largest number (ASY) among the states in each section is used as the combination state.
			*/
			std::pair<bool, _cstatus_mgmt::_cstate::type_state> _get_state_another(unsigned long n_session);
		private:
			bool m_b_cur_shared_mode;// the current open mode : true - shared mode, false - exclusive mode(default)

			// key is session number, value  the current status ptr.
			_cstatus_mgmt::_cstate::type_map_ptr_state m_map_ptr_state_cur; 

			// current combination state of all session
			_cstatus_mgmt::_cstate::type_state m_st_combi;

			// device path
			std::wstring m_s_dev_path;

			cdev_ctl_fn::_cq_mgmt m_mgmt_q; 
		};
	protected:

	public:
		cdev_ctl_fn(clog* p_log);
		virtual ~cdev_ctl_fn();
	protected:
		
		type_pair_bool_result_bool_complete _execute_close_sync(
			clog* p_log,
			const cio_packet& request,
			cio_packet& response,
			const std::wstring & s_dev_path
		);


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
		* @bref get device path of cdev_ctl object.
		* @return device path
		*/
		const std::wstring _get_device_path() const;

	protected:
		std::mutex m_mutex_for_open;
		int m_n_cnt_open;//open counter
		bool m_b_shared_mode;// current is shared open mode.
		std::wstring m_s_dev_path;// current device apth


		cdev_ctl_fn::_cstatus_mgmt m_mgmt_state; // state manager

	private://don't call these methods
		cdev_ctl_fn();
		cdev_ctl_fn(const cdev_ctl_fn&);
		cdev_ctl_fn& operator=(const cdev_ctl_fn&);

	};

}//the end of _mp namespace

