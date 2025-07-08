#pragma once

#include <string>

#include <Windows.h>
#include <Userenv.h>
#include <memory>

namespace cwarp
{
	class cevent
	{
	public:
		typedef	std::shared_ptr<cwarp::cevent >	type_ptr_cevent;
		typedef	std::shared_ptr<cwarp::cevent >	type_ptr;
	public:
		cevent() : m_h_event(INVALID_HANDLE_VALUE), m_b_manual(true)
		{
			create(m_b_manual, false, L"");
		}
		cevent(bool b_manual, bool b_initial, const std::wstring& s_name) : m_h_event(INVALID_HANDLE_VALUE), m_b_manual(b_manual)
		{
			create(b_manual, b_initial, s_name);
		}

		cevent(HANDLE h_event) : m_h_event(h_event), m_b_manual(true)
		{
		}

		~cevent()
		{
			if (m_h_event != NULL && m_h_event != INVALID_HANDLE_VALUE)
				CloseHandle(m_h_event);
		}

		bool create(bool b_manual, bool b_initial, const std::wstring& s_name)
		{
			bool b_result(false);
			do {
				if (!empty()) {
					b_result = true;//already created .......
					continue;
				}
				BOOL bManual(FALSE);
				BOOL bInitial(FALSE);

				m_b_manual = b_manual;
				if (b_manual)
					bManual = TRUE;
				if (b_initial)
					bInitial = TRUE;

				if (s_name.empty())
					m_h_event = CreateEvent(nullptr, bManual, bInitial, NULL);
				else
					m_h_event = CreateEvent(nullptr, bManual, bInitial, s_name.c_str());
				//
				if (empty())
					continue;
				b_result = true;
			} while (false);
			return b_result;
		}

		bool close()
		{
			bool b_result(false);
			do {
				if (empty())
					continue;
				if (!CloseHandle(m_h_event))
					continue;
				m_h_event = INVALID_HANDLE_VALUE;
				b_result = true;
			} while (false);
			return b_result;
		}
		HANDLE get_handle()
		{
			return m_h_event;
		}

		bool empty()
		{
			if (m_h_event != NULL && m_h_event != INVALID_HANDLE_VALUE)
				return false;
			else
				return true;
		}

		bool set()
		{
			bool b_result(false);
			do {
				if (empty())
					continue;
				if (!SetEvent(m_h_event))
					continue;
				b_result = true;
			} while (false);
			return b_result;
		}
		bool reset()
		{
			bool b_result(false);
			do {
				if (empty())
					continue;
				if (!ResetEvent(m_h_event))
					continue;
				//
				b_result = true;
			} while (false);
			return b_result;
		}

		bool wait(DWORD dw_time_out = INFINITE)
		{
			bool b_result(false);

			do {
				if (empty())
					continue;
				//
				DWORD dw_result = WaitForSingleObject(m_h_event, dw_time_out);
				if (dw_result != WAIT_OBJECT_0)
					continue;
				if (m_b_manual)
					ResetEvent(m_h_event);
				b_result = true;
			} while (false);
			return b_result;
		}
	private:
		HANDLE m_h_event;
		bool m_b_manual;
	private:
		cevent(const cevent&);
		cevent& operator=(const cevent&);
	};

	class coverlap
	{
	public:
		typedef	std::shared_ptr<coverlap> type_ptr;
	public:
		coverlap() : m_event(false, false, L"")
		{
			memset(&m_overlap, 0, sizeof(m_overlap));

			m_overlap.hEvent = m_event.get_handle();
		}

		HANDLE get_handle()
		{
			return m_overlap.hEvent;
		}
		cevent& get_event()
		{
			return m_event;
		}
		DWORD get_offset_low()
		{
			return m_overlap.Offset;
		}
		DWORD get_offset_high()
		{
			return m_overlap.OffsetHigh;
		}

		LARGE_INTEGER get_offset()
		{
			LARGE_INTEGER result;

			result.LowPart = m_overlap.Offset;
			result.HighPart = m_overlap.OffsetHigh;
			return result;
		}
		void set_offset(LARGE_INTEGER offset)
		{
			m_overlap.Offset = offset.LowPart;
			m_overlap.OffsetHigh = offset.HighPart;
		}
		void set_offset(DWORD dw_low, DWORD dw_high = 0)
		{
			m_overlap.Offset = dw_low;
			m_overlap.OffsetHigh = dw_high;
		}

		OVERLAPPED& get()
		{
			return m_overlap;
		}

		void reset()
		{
			m_event.reset();
			memset(&m_overlap, 0, sizeof(m_overlap));
			m_overlap.hEvent = m_event.get_handle();
		}
	private:
		OVERLAPPED m_overlap;
		cevent m_event;
	private:
		coverlap(const coverlap&);
		coverlap& operator=(const coverlap&);
	};

	///////////////////////////////////////////////////
	///////////////////////////////////////////////////
	class chandle
	{
	public:
		chandle() : m_h_handle(NULL) {}
		explicit chandle(HANDLE h_handle) : m_h_handle(h_handle) {}
		~chandle()
		{
			if (m_h_handle != NULL && m_h_handle != INVALID_HANDLE_VALUE)
				CloseHandle(m_h_handle);
		}

		HANDLE get_handle() const
		{
			return m_h_handle;
		}
		void set_handle(HANDLE h_handle)
		{
			m_h_handle = h_handle;
		}

		bool empty()
		{
			if (m_h_handle != NULL && m_h_handle != INVALID_HANDLE_VALUE)
				return false;
			else
				return true;
		}

	private:
		HANDLE m_h_handle;
	private://don't call these method
		chandle(const chandle&);
		chandle& operator=(const chandle&);
	};

	class cevent_log_handle {
	public:
		cevent_log_handle() : m_h_handle(NULL) {}
		explicit cevent_log_handle(HANDLE h_handle) : m_h_handle(h_handle) {}
		~cevent_log_handle()
		{
			if (m_h_handle)
				DeregisterEventSource(m_h_handle);
		}

		HANDLE get_handle() const
		{
			return m_h_handle;
		}

		void set_handle(HANDLE h_handle)
		{
			m_h_handle = h_handle;
		}

	private:
		HANDLE m_h_handle;
	private://don't call these method
		cevent_log_handle(const cevent_log_handle&);
		cevent_log_handle& operator=(const cevent_log_handle&);
	};

	class cregistry_key_handle
	{
	public:
		explicit cregistry_key_handle(const HKEY& h_key)
		{
			memcpy(&m_h_key, &h_key, sizeof(m_h_key));
		}

		~cregistry_key_handle()
		{
			RegCloseKey(m_h_key);
		}

		HKEY& get_handle()
		{
			return m_h_key;
		}


	private:
		HKEY m_h_key;
	private://don't call these method
		cregistry_key_handle();
		cregistry_key_handle(const cregistry_key_handle&);
		cregistry_key_handle& operator=(const cregistry_key_handle&);
	};

	class cenvironment_block
	{
	public:
		explicit cenvironment_block(LPVOID lp_env) : m_lp_env(lp_env) {}

		~cenvironment_block()
		{
			if (m_lp_env)
				DestroyEnvironmentBlock(m_lp_env);
		}

		LPVOID get_block()
		{
			return m_lp_env;
		}


	private:
		LPVOID m_lp_env;
	private://don't call these method
		cenvironment_block();
		cenvironment_block(const cenvironment_block&);
		cenvironment_block& operator=(const cenvironment_block&);
	};

#include <winsvc.h>

	class cservice_handle
	{
	public:
		explicit cservice_handle(SC_HANDLE h_handle) : m_h_handle(h_handle) {}
		~cservice_handle()
		{
			if (m_h_handle)
				CloseServiceHandle(m_h_handle);
		}

		SC_HANDLE get_handle() const
		{
			return m_h_handle;
		}

	private:
		SC_HANDLE m_h_handle;
	private://don't call these method
		cservice_handle();
		cservice_handle(const cservice_handle&);
		cservice_handle& operator=(const cservice_handle&);

	};

#include <WtsApi32.h>

	class cWTS_memory
	{
	public:
		explicit cWTS_memory(PWTS_SESSION_INFO p_session_info) : m_p_session_info(p_session_info) {}

		~cWTS_memory()
		{
			if (m_p_session_info)
				WTSFreeMemory(m_p_session_info);
		}

		PWTS_SESSION_INFO GetSessionInfo() const
		{
			return m_p_session_info;
		}


	private:
		PWTS_SESSION_INFO m_p_session_info;
	private://don't call these method
		cWTS_memory();
		cWTS_memory(const cWTS_memory&);
		cWTS_memory& operator=(const cWTS_memory&);
	};
}//the end of cwarp
