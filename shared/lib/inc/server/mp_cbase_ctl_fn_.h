#pragma once
#include <memory>

#include <mp_type.h>
#include <mp_clog.h>
#include <mp_cqitem_dev.h>
#include <server/mp_cio_packet.h>

namespace _mp {
	class cbase_ctl_fn
	{
	public:
		typedef	std::weak_ptr< cbase_ctl_fn >	type_wptr;
		typedef	std::shared_ptr< cbase_ctl_fn >	type_ptr;

	protected:
		enum : int {
			_const_max_n_cnt_open = 255 // max open counter value in shared mode.
		};

	protected:
		class _cstate {
		public:
			typedef	std::shared_ptr<_cstate> type_ptr;
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
				st_idl = 1, //open but none request
				st_asy = 2, //waiting async response
				st_total // the number of defined state
			}type_state;

			// the concated state of selected session and another session
			// cal method - the selected session state * 100 +  another session state.
			typedef enum : int {
				st_snot_anot = (int)(st_not *100+ st_not), // st_not + st_not
				st_snot_aidl = (int)(st_not * 100 + st_idl), // st_not + st_idl
				st_snot_aasy = (int)(st_not * 100 + st_asy), // st_not + st_asy

				st_sidl_anot = (int)(st_idl * 100 + st_not), // st_idl + st_not
				st_sidl_aidl = (int)(st_idl * 100 + st_idl), // st_idl + st_idl
				st_sidl_aasy = (int)(st_idl * 100 + st_asy), // st_idl + st_asy

				st_sasy_anot = (int)(st_asy * 100 + st_not), // st_asy + st_not
				st_sasy_aidl = (int)(st_asy * 100 + st_idl), // st_asy + st_idl
				st_sasy_aasy = (int)(st_asy * 100 + st_asy), // st_asy + st_asy
			}type_state_sel_another;

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
			static std::pair<bool, cbase_ctl_fn::_cstate::type_state> get_combination_state_in_map_except_selected_session
			(
				const cbase_ctl_fn::_cstate::type_map_ptr_state& map_ptr_state_cur,
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

			_cstate(const cbase_ctl_fn::_cstate& src);

			_cstate& operator=(const cbase_ctl_fn::_cstate& src);

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
			static const int _m_trans_table_base[cbase_ctl_fn::_cstate::st_total][cbase_ctl_fn::_cstate::ev_total];

		private:
			_cstate::type_state m_st_cur; //the current state.
			_cstate::type_evt m_ev_last; // the last event
		};

	public:
		class cresult {
		public:
			typedef	std::shared_ptr<cbase_ctl_fn::cresult>	type_ptr;

		public:

			/**
			* @brief default contructure
			* 
			*	ptr_req and ptr_rsp is "not" allocated in this function.
			*/
			cresult();

			cresult(const cresult& src);

			cresult& operator=(const cresult& src);

			/**
			* @brief constructor. 
			*	only deep copy request packet and isn't create response.
			*/
			cresult(const cio_packet& request);

			/**
			* @brief constructor.
			*	only increases ptr_request reference counter and isn't create response.
			*/
			cresult(const cio_packet::type_ptr& ptr_request);

			/**
			* @brief reset state & result.
			* 
			*	set state to default.
			* 
			*	set result to default.
			* 
			*	clear device path.
			* 
			*	not be changed ptr_req and ptr_rsp.
			* 
			*/
			void reset();

			/**
			* @brief get request packet.
			* @return the request packet.
			*/
			const cio_packet::type_ptr& get_req() const;

			/**
			* @brief get rsp packet.
			* @return the response packet ptr.
			*/
			const cio_packet::type_ptr& get_rsp() const;

			/**
			* @brief get session number of the current processing.
			* @return the session number of the current processing.
			*/
			unsigned long get_session_number() const;

			std::wstring get_dev_path() const;

			/**
			* @brief get the received data.
			* @param [in/out]v_data - the received data
			* @return the size of the received data.
			*/
			size_t get_rx(type_v_buffer& v_data) const;

			/**
			* @brief get event of the current processing.
			* @return the current processing event. Be calculated by request packet(m_ptr_req)
			*/
			_cstate::type_evt get_cur_event() const;

			/**
			* @brief set request packet. and create default response in function body.
			*
			* @param request - request packet
			*	
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
			*	set member m_ptr_rsp.( if m_ptr_rsp is null, create new response packet)
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
			void after_processing_set_rsp_with_error_complete
			(
				cio_packet::type_error_reason n_reason,
				const std::wstring& s_dev_path = std::wstring()
			);

			/**
			* @brief set ptr_rsp member with success response.
			*
			*	set member m_ptr_rsp.( if m_ptr_rsp is null, create new response packet)
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
			void after_processing_set_rsp_with_succss_complete
			(
				const _mp::type_v_buffer& v_rx,
				const std::wstring& s_dev_path = std::wstring()
			);

			/**
			* @brief set ptr_rsp member with success response.
			*
			*	set member m_ptr_rsp.( if m_ptr_rsp is null, create new response packet)
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
			void after_processing_set_rsp_with_succss_complete
			(
				const std::wstring& s_data,
				const std::wstring& s_dev_path = std::wstring()
			);

			/**
			* @brief set ptr_rsp member with success response.
			*
			*	set member m_ptr_rsp.( if m_ptr_rsp is null, create new response packet)
			*
			*	set m_b_process_result is true.
			*
			*	set m_b_process_complete is true.
			*
			*	set m_s_dev_path.
			*
			* @param ptr_reponse - alredy created response packet.
			* @param s_dev_path - the device path
			*/
			void after_processing_set_rsp_with_succss_complete
			(
				const cio_packet::type_ptr& ptr_reponse,
				const std::wstring& s_dev_path = std::wstring()
			);

			/**
			* @brief set ptr_rsp member with success and need more processing.
			*
			*	no need to send to client.
			* 
			*	not access ptr_rsp
			*
			* @param s_dev_path - the device path
			*/
			void after_starting_process_set_rsp_with_succss_ing
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
			void set_selected_session_state(cbase_ctl_fn::_cstate::type_state st_new, cbase_ctl_fn::_cstate::type_state st_old);

			/**
			* @brief set the selected session state.
			* @param st_result - states of the selected session by processing.
			*/
			void set_selected_session_state(cbase_ctl_fn::_cstate::type_result_evt st_result);

			/**
			* @brief set combination state.
			* @param st_new - new combination state.
			* @param st_old - old combination state.
			*/
			void set_combination_state(cbase_ctl_fn::_cstate::type_state st_new, cbase_ctl_fn::_cstate::type_state st_old);

			/**
			* @brief get the combination state of all session.
			* @return pair type
			*
			*	first - new combination state.
			*
			*	second - old combination state.
			*
			*/
			std::pair< cbase_ctl_fn::_cstate::type_state, cbase_ctl_fn::_cstate::type_state> get_combination_state() const;

		private:
			////////////////////////////////////////////////////
			// for state section members.
			// old state of the selected session before processing.
			cbase_ctl_fn::_cstate::type_state m_st_session_old;

			// new state of the selected session by processing.
			cbase_ctl_fn::_cstate::type_state m_st_session_new;

			// old combination state of all session before processing.
			cbase_ctl_fn::_cstate::type_state m_st_combination_old;

			// new combination state of all session by processing.
			cbase_ctl_fn::_cstate::type_state m_st_combination_new;

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

	public:
		cbase_ctl_fn(clog* p_log);
		virtual ~cbase_ctl_fn();

	protected:
		/**
		* @brief generate response packet with error setting.
		* @param request - request packet.
		* @param n_reason - the reason of error.
		* @return ptr of the generated response packet. 
		*/
		cio_packet::type_ptr _generate_error_response
		(
			const cio_packet& request,
			cio_packet::type_error_reason n_reason = cio_packet::error_reason_none
		);

		/**
		* @brief get device path from client parameter on open request.( used in _proc_event_open() )
		* @param [in] ptr_request - the ptr of request packet.
		* @return
		*	first : path from client parameter.
		*
		*	second : if or not shared mode by user( true - need shared mode open : user request )
		*
		*	if  first is emtpy, error. error info is set to response.
		*/
		static std::pair<std::wstring, bool> _get_device_path_from_req(const cio_packet::type_ptr& ptr_request);

		/**
		* @brief open with lower library layer. .( used in _proc_event_open() )
		* @param [in] s_dev_path - device path for open.
		* @param [in] b_open_by_shared_mode - if or not open by shared mode. (true - shared mode by user request)
		* @param [in] n_open_cnt - open counter.( except this open request )
		* @return true : open success.
		*
		*	false : error. error info must be set to response.
		*/
		static bool _open_device_by_req
		(
			const std::wstring & s_dev_path,
			bool b_open_by_shared_mode,
			size_t n_open_cnt
		);

	protected:
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
		static void _set_response_result_only(cio_packet& out_response, cqitem_dev::type_result result, const std::wstring& s_result, cio_packet& in_request);

		/**
		* @brief check openable condition
		* @param b_is_support_by_shared_open_device - if or not a device is supporting the shared open.
		* @param b_is_request_shared_open_by_user - open request need a shared mode.
		* @param n_open_cnt - The number of times the current device is open
		* @return true - the given condition can try open device.
		*
		*	false - the given condition can't try open device.
		*/
		static bool _is_openable(bool b_is_support_by_shared_open_device, bool b_is_request_shared_open_by_user, size_t n_open_cnt);

		/**
		* @brief In any function!
		*
		*	display ATL start-function debugging message in windows system.
		*
		* @param request - request info
		* @param s_fun_name - the name of fucntion.
		*/
		static void _debug_win_enter_x(const cio_packet& request, const std::wstring& s_fun_name);

		/**
		* @brief In any function!
		*
		*	display ATL exit-function debugging message in windows system.
		*
		* @param request - request info
		* @param s_fun_name - the name of fucntion.
		*/
		static void _debug_win_leave_x(const cio_packet& request, const std::wstring& s_fun_name);

	protected:
		clog* m_p_ctl_fun_log;

	private://don't call these methods
		cbase_ctl_fn();
	};


}//the end of _mp namespace
