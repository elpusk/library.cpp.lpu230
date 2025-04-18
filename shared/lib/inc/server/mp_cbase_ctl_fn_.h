#pragma once
#include <memory>

#include <mp_type.h>
#include <mp_clog.h>
#include <server/mp_cio_packet.h>

namespace _mp {
	class cbase_ctl_fn
	{
	public:
		typedef	std::weak_ptr< cbase_ctl_fn >	type_wptr;
		typedef	std::shared_ptr< cbase_ctl_fn >	type_ptr;

	protected:
		enum : int {
			_const_max_n_cnt_open = 255 // max m_n_cnt_open value in shared mode.
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
		* @brief get device path from client parameter on open request.( used in _execute_open_sync() )
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
		* @brief open with lower library layer. .( used in _execute_open_sync() )
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
