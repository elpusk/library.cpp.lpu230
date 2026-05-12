#pragma once

#include <ct_type.h>

#include <ct_operation.h>
#include <ct_warp.h>
#include <ct_file.h>
#include <ct_version.h>

#include <memory>
#include <string>
#include <ctime>

#include <Windows.h>
#include <Softpub.h>
#include <wincrypt.h>
#include <wintrust.h>

#include <process.h>
#include <TlHelp32.h>
#include <WtsApi32.h>
#include <UserEnv.h>

#pragma comment(lib, "wintrust")

#pragma comment(lib, "Userenv.lib")
#pragma comment(lib, "Wtsapi32.lib")

namespace _ns_tools
{
	class ct_system
	{
	public:
		/**
		* return first - operation success(true) or fail
		* return second - version data 
		*/
		static std::pair< bool,ct_version<unsigned short> > get_version_from_resource_table(HMODULE h_module)
		{
			ct_version<unsigned short> ver;
			bool b_result(false);

			do{
				if(!h_module){
					continue;
				}
				HRSRC hResource = FindResource(h_module, MAKEINTRESOURCE(VS_VERSION_INFO), RT_VERSION);
				if(!hResource){
					continue;
				}
				HGLOBAL hResourceData = LoadResource(h_module, hResource);
				if(!hResourceData){
					continue;
				}
				LPVOID lpResource = LockResource(hResourceData);
				if(!lpResource){
					continue;
				}

				VS_FIXEDFILEINFO* fileInfo(NULL);
				unsigned int n_fileInfo(0);
				if (!VerQueryValue(lpResource, L"\\", (LPVOID*)&fileInfo, &n_fileInfo)) {
					FreeResource(lpResource);
					continue;
				}
				unsigned short w_maj = HIWORD(fileInfo->dwFileVersionMS);
				unsigned short w_min = LOWORD(fileInfo->dwFileVersionMS);
				unsigned short w_fix = HIWORD(fileInfo->dwFileVersionLS);
				unsigned short w_build = LOWORD(fileInfo->dwFileVersionLS);

				ver = ct_version<unsigned short>(w_maj,w_min,w_fix,w_build);
				FreeResource(lpResource);

			}while(false);

			return std::make_pair(b_result,ver);
		} 

		static bool is_windows_vista_or_greater()
		{
			return ct_system::is_windows_version_or_greater(HIBYTE(_WIN32_WINNT_VISTA), LOBYTE(_WIN32_WINNT_VISTA), 0);
		}

		static bool is_windows_version_or_greater(WORD wMajorVersion, WORD wMinorVersion, WORD wServicePackMajor)
		{
			OSVERSIONINFOEXW osvi = {sizeof(osvi), 0, 0, 0, 0, {0}, 0, 0};
			DWORDLONG const dwlConditionMask = VerSetConditionMask(
				VerSetConditionMask(
					VerSetConditionMask(0, VER_MAJORVERSION, VER_GREATER_EQUAL),
					VER_MINORVERSION, VER_GREATER_EQUAL),
				VER_SERVICEPACKMAJOR, VER_GREATER_EQUAL);

			osvi.dwMajorVersion = wMajorVersion;
			osvi.dwMinorVersion = wMinorVersion;
			osvi.wServicePackMajor = wServicePackMajor;

			if (VerifyVersionInfoW(&osvi, VER_MAJORVERSION | VER_MINORVERSION | VER_SERVICEPACKMAJOR, dwlConditionMask))
				return true;
			else
				return false;
		}

		static bool is_exist_named_event(const std::wstring &s_name)
		{
			bool b_result(false);

			do
			{
				if (s_name.empty())
					continue;
				HANDLE h_manager_instance = OpenEvent(
					SYNCHRONIZE | EVENT_MODIFY_STATE, FALSE, s_name.c_str());
				if (h_manager_instance == NULL)
					continue;
				//
				CloseHandle(h_manager_instance);
				b_result = true;
			} while (false);
			return b_result;
		}

		static bool is_exist_named_mutex(const std::wstring &s_name)
		{
			bool b_result(false);

			do
			{
				HANDLE h_mutex = OpenMutex(SYNCHRONIZE, FALSE, s_name.c_str());
				if (h_mutex == NULL)
				{
					unsigned long dw_result = GetLastError();
					if (dw_result == ERROR_FILE_NOT_FOUND)
					{
						//Not found event
						b_result = false;
					}
					else
					{ //opening event error
						b_result = false;
					}
					continue;
				}

				//
				CloseHandle(h_mutex); //decrease reference counter
				b_result = true;
			} while (false);

			return b_result;
		}

		static bool set_named_event(const std::wstring &s_name, bool b_set = true)
		{
			bool b_result(false);

			do
			{
				if (s_name.empty())
					continue;
				HANDLE h_manager_instance = OpenEvent(
					SYNCHRONIZE | EVENT_MODIFY_STATE, FALSE, s_name.c_str());
				if (h_manager_instance == NULL)
					continue;
				//
				if (b_set)
					SetEvent(h_manager_instance);
				else
					ResetEvent(h_manager_instance);
				//
				CloseHandle(h_manager_instance);
				b_result = true;
			} while (false);
			return b_result;
		}

		static HANDLE create_one_app_mutex(const std::wstring &s_mutex_name)
		{
			HANDLE h_mutex(nullptr);

			do
			{
				if (s_mutex_name.empty())
					continue;
				h_mutex = CreateMutex(NULL, FALSE, s_mutex_name.c_str());
				switch (GetLastError())
				{
				case ERROR_SUCCESS:
					// Mutex created successfully. There is no instances running
					break;
				case ERROR_ALREADY_EXISTS:
					// Mutex already exists so there is a running instance of our app.
					h_mutex = NULL;
					break;
				default:
					// Failed to create mutex by unkniown reason
					break;
				} //end switch

			} while (false);
			return h_mutex;
		}

		static HANDLE create_mutex(bool b_inheritHandle, bool b_initialOwner, const std::wstring &s_mutex_name = std::wstring())
		{
			HANDLE h_new_mutex = NULL;
			BOOL bInitialOwner(FALSE);
			BOOL bInheritHandle(FALSE);

			if (b_inheritHandle)
				bInheritHandle = TRUE;
			if (b_initialOwner)
				bInitialOwner = TRUE;
			//
			do
			{
				if (s_mutex_name.empty())
				{
					h_new_mutex = ::CreateMutex(NULL, bInitialOwner, NULL);
					continue;
				}

				//the first open-try
				h_new_mutex = ::OpenMutex(MUTEX_ALL_ACCESS, bInheritHandle, s_mutex_name.c_str());
				if (h_new_mutex == NULL)
				{
					if (GetLastError() == ERROR_FILE_NOT_FOUND)
					{
						//the second create-try
						h_new_mutex = ::CreateMutex(NULL, bInitialOwner, s_mutex_name.c_str());
					}
					continue;
				}

				/*
				::DuplicateHandle(
					GetCurrentProcess(),
					h_new_mutex,
					GetCurrentProcess(),
					&hMutexDup,
					0,
					FALSE,
					DUPLICATE_SAME_ACCESS);
				*/

			} while (false);

			return h_new_mutex;
		}

		static HANDLE create_event_for_ipc(bool b_manual, bool b_initial_state, const std::wstring &s_event_name)
		{
			HANDLE h_event(NULL);

			do
			{
				if (s_event_name.empty())
					continue;

				SECURITY_DESCRIPTOR sd = {0};
				::InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
				::SetSecurityDescriptorDacl(&sd, TRUE, 0, FALSE);
				SECURITY_ATTRIBUTES sa = {0};
				sa.nLength = sizeof(SECURITY_ATTRIBUTES);
				sa.lpSecurityDescriptor = &sd;

				BOOL bManual(FALSE);
				BOOL bInitialState(FALSE);
				if (b_manual)
					bManual = TRUE;
				if (b_initial_state)
					bInitialState = TRUE;

				h_event = CreateEvent(&sa, bManual, bInitialState, s_event_name.c_str());
				if (h_event == NULL)
				{
					continue;
				}
			} while (false);
			return h_event;
		}

		static bool get_environment_variable_of_this_process(std::wstring &s_value, const std::wstring &s_key)
		{
			bool b_result(false);

			do
			{
				_ns_tools::type_v_buffer v_value(4096, 0);
				DWORD dw_result = GetEnvironmentVariable(s_key.c_str(), (wchar_t *)&v_value[0], (DWORD)v_value.size() / sizeof(wchar_t));
				if (dw_result == 0)
				{
					/*
					DWORD dw_error = GetLastError();
					if (dw_error == ERROR_ENVVAR_NOT_FOUND) {

					}
					*/
					continue;
				}
				if ((v_value.size() / sizeof(wchar_t)) < dw_result)
				{
					v_value.resize(dw_result * sizeof(wchar_t), 0);
					v_value.assign(v_value.size(), 0);
					dw_result = GetEnvironmentVariable(s_key.c_str(), (wchar_t *)&v_value[0], (DWORD)v_value.size() / sizeof(wchar_t));
					if (dw_result == 0)
						continue;
				}

				s_value = (wchar_t *)&v_value[0];
				// getting ok
				b_result = true;
			} while (false);
			return b_result;
		}

		static _ns_tools::type_v_buffer get_current_date_by_yymmdd_format()
		{
			_ns_tools::type_v_buffer v_value(6, 0);

			time_t rawtime;
			struct tm timeinfo = {
				0,
			};

			time(&rawtime);
			localtime_s(&timeinfo, &rawtime);
			//
			char s_buffer[20] = {
				0,
			};
			sprintf_s(s_buffer, 20, "%04d%02d%02d", timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday);
			std::copy(&s_buffer[2], &s_buffer[2+v_value.size()], std::begin(v_value));
			return v_value;
		}
		static _ns_tools::type_v_buffer get_current_time_by_hhmmss_format()
		{
			_ns_tools::type_v_buffer v_value(6, 0);

			time_t rawtime;
			struct tm timeinfo = {
				0,
			};

			time(&rawtime);
			localtime_s(&timeinfo, &rawtime);
			//
			char s_buffer[20] = {
				0,
			};
			sprintf_s(s_buffer, 20, "%02d%02d%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
			std::copy(&s_buffer[0], &s_buffer[v_value.size()], std::begin(v_value));
			return v_value;
		}

		static bool verify_embedded_signature(const std::wstring &s_src_file)
		{
			bool b_result(false);
			bool b_need_release(false);

				WINTRUST_FILE_INFO file_data{0,};

			GUID WVTPolicyGUID = WINTRUST_ACTION_GENERIC_VERIFY_V2;
			WINTRUST_DATA win_trust_data{0,};

			do{
				if (s_src_file.empty())
					continue;

				LONG l_status(0);
				DWORD dw_last_error(0);

				// Initialize the WINTRUST_FILE_INFO structure.
				file_data.cbStruct = sizeof(WINTRUST_FILE_INFO);
				file_data.pcwszFilePath = s_src_file.c_str();
				file_data.hFile = NULL;
				file_data.pgKnownSubject = NULL;

				/*
				WVTPolicyGUID specifies the policy to apply on the file
				WINTRUST_ACTION_GENERIC_VERIFY_V2 policy checks:
				
				1) The certificate used to sign the file chains up to a root 
				certificate located in the trusted root certificate store. This 
				implies that the identity of the publisher has been verified by 
				a certification authority.
				
				2) In cases where user interface is displayed (which this example
				does not do), WinVerifyTrust will check for whether the  
				end entity certificate is stored in the trusted publisher store,  
				implying that the user trusts content from this publisher.
				
				3) The end entity certificate has sufficient permission to sign 
				code, as indicated by the presence of a code signing EKU or no 
				EKU.
				*/
				win_trust_data.cbStruct = sizeof(win_trust_data);
				win_trust_data.pPolicyCallbackData = NULL;// Use default code signing EKU.
				win_trust_data.pSIPClientData = NULL;// No data to pass to SIP.
				win_trust_data.dwUIChoice = WTD_UI_NONE;// Disable WVT UI.
				win_trust_data.fdwRevocationChecks = WTD_REVOKE_NONE;// No revocation checking.
				win_trust_data.dwUnionChoice = WTD_CHOICE_FILE;// Verify an embedded signature on a file.
				win_trust_data.dwStateAction = WTD_STATEACTION_VERIFY;// Verify action.
				win_trust_data.hWVTStateData = NULL;// Verification sets this value.
				win_trust_data.pwszURLReference = NULL;// Not used.

				// This is not applicable if there is no UI because it changes
				// the UI to accommodate running applications instead of
				// installing applications.
				win_trust_data.dwUIContext = 0;
				win_trust_data.pFile = &file_data;// Set pFile.

				b_need_release = true;
				// WinVerifyTrust verifies signatures as specified by the GUID and Wintrust_Data.
				l_status = WinVerifyTrust(NULL, &WVTPolicyGUID, &win_trust_data);

				switch (l_status)
				{
				case ERROR_SUCCESS://success
					/*
					Signed file:
						- Hash that represents the subject is trusted.
						- Trusted publisher without any verification errors.
						- UI was disabled in dwUIChoice. No publisher or 
							time stamp chain errors.

						- UI was enabled in dwUIChoice and the user clicked 
							"Yes" when asked to install and run the signed 
							subject.
					*/
					break;
				case TRUST_E_NOSIGNATURE:
					// The file was not signed or had a signature
					// that was not valid.

					// Get the reason for no signature.
					dw_last_error = GetLastError();
					if (TRUST_E_NOSIGNATURE == dw_last_error ||
						TRUST_E_SUBJECT_FORM_UNKNOWN == dw_last_error ||
						TRUST_E_PROVIDER_UNKNOWN == dw_last_error){
						continue;// The file was not signed.
					}
					else{
						// The signature was not valid or there was an error opening the file.
						continue;
					}
					continue;
				case TRUST_E_EXPLICIT_DISTRUST:
					// The hash that represents the subject or the publisher is not allowed by the admin or user.
					continue;
				case TRUST_E_SUBJECT_NOT_TRUSTED:
					// The user clicked "No" when asked to install and run.
					continue;
				case CRYPT_E_SECURITY_SETTINGS:
					/*
					The hash that represents the subject or the publisher 
					was not explicitly trusted by the admin and the 
					admin policy has disabled user trust. No signature, 
					publisher or time stamp errors.
					*/
					continue;
				default:
					// The UI was disabled in dwUIChoice or the admin policy has disabled user trust.
					// l_status contains the publisher or time stamp chain error.
					continue;
				}//end switch

				b_result = true;
			} while (false);

			if(b_need_release){
				// Any hWVTStateData must be released by a call with close.
				win_trust_data.dwStateAction = WTD_STATEACTION_CLOSE;
				WinVerifyTrust(NULL,&WVTPolicyGUID,&win_trust_data);

			}
			return b_result;
			////////////////
		}

		/**
		 * @brief execute exe file.
		 * 
		 * @param dw_session_id 
		 * @param s_exe_file_path - exe file full path
		 * @param s_command_line - command line
		 * @param s_working_directory
		 * @param b_user_or_session0 - true wininit.exe or winlogon.exe, false - current module exe
		 * @param out_proc_info - out executed process information
		 * @param out_error_return_functions - out, function name that return a error.
		 * @return true executed success( out_error_return_functions can have a funtion name.)
		 * @return false 
		 */
		static bool execute_process(
			unsigned long dw_session_id,
			const std::wstring & s_exe_file_path,
			const std::wstring & s_command_line,
			const std::wstring & s_working_directory,
			bool b_user_or_session0,
			PROCESS_INFORMATION & out_proc_info,
			_ns_tools::type_v_wstring & out_error_return_functions)
		{
			bool b_result(false);
			DWORD dw_result(0);

			do {
				PROCESS_INFORMATION pi{0,};
				STARTUPINFO si{0,};
				BOOL bResult = FALSE;
				DWORD dw_pid(0);
				DWORD dwCreationFlags(0);

				out_error_return_functions.clear();

				// Find the winlogon process
				PROCESSENTRY32 procEntry;
				HANDLE h_snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
				if (h_snap == INVALID_HANDLE_VALUE) {
					out_error_return_functions.push_back(L"CreateToolhelp32Snapshot");
					continue;
				}

				_ns_tools::ct_warp::chandle handle_snap(h_snap);
				procEntry.dwSize = sizeof(PROCESSENTRY32);

				if (!Process32First(handle_snap.get_handle(), &procEntry)) {
					out_error_return_functions.push_back(L"Process32First");
					continue;
				}

				std::wstring s_base_exe;

				if (b_user_or_session0) {
					if (dw_session_id == 0) {
						s_base_exe = L"wininit.exe";
					}
					else {
						s_base_exe = L"winlogon.exe";
					}
				}
				else {
					std::wstring s_file_path;
					_ns_tools::type_v_ws_buffer s_path(_MAX_PATH + 1, 0);
					if (!::GetModuleFileName(NULL, &s_path[0], (unsigned long)(s_path.size() - 1))) {
						out_error_return_functions.push_back(L"GetModuleFileName");
						continue;
					}
					//
					s_file_path = (WCHAR*)&s_path[0];

					std::wstring s_out_drive, s_out_dir, s_out_file_name, s_out_ext;

					_ns_tools::ct_file::split_path(
						s_file_path
						, s_out_drive
						, s_out_dir
						, s_out_file_name
						, s_out_ext
					);

					s_base_exe = s_out_file_name + s_out_ext;
				}


				do {
					if (s_base_exe.compare(procEntry.szExeFile) == 0) {
						// We found a winlogon process...make sure it's running in the console session
						DWORD dw_win_log_on_session_id = 0;

						//ProcessIdToSessionId - Callers must hold the PROCESS_QUERY_INFORMATION(need admin)
						if (!ProcessIdToSessionId(procEntry.th32ProcessID, &dw_win_log_on_session_id)) {
							out_error_return_functions.push_back(L"ProcessIdToSessionId");
							break;//error exit
						}
						if (dw_win_log_on_session_id == dw_session_id){
							dw_pid = procEntry.th32ProcessID;
							break;//success exit
						}
					}
				} while (Process32Next(h_snap, &procEntry));
				if (dw_pid == 0) {
					continue;
				}

				static wchar_t s_desktop[] = L"winsta0\\default";

				dwCreationFlags = NORMAL_PRIORITY_CLASS | CREATE_NEW_CONSOLE;
				ZeroMemory(&si, sizeof(STARTUPINFO));
				si.cb = sizeof(STARTUPINFO);
				si.lpDesktop = s_desktop;
				ZeroMemory(&pi, sizeof(pi));
				TOKEN_PRIVILEGES tp;
				LUID luid;
				LPVOID p_env = NULL;
				HANDLE h_user_token_dup(NULL);
				_ns_tools::ct_warp::chandle user_token_dup_handle;
				_ns_tools::type_v_ws_buffer v_exe_file(_MAX_PATH);
				_ns_tools::type_v_ws_buffer v_command_line(_MAX_PATH);
				const wchar_t* lpCurrentDirectory(NULL);

				if (dw_session_id != 0) {
					
					_ns_tools::ct_warp::chandle process_handle(::OpenProcess(MAXIMUM_ALLOWED, FALSE, dw_pid));
					HANDLE h_ptoken(NULL);

					if (!::OpenProcessToken(//OpenProcess - The process must have the PROCESS_QUERY_INFORMATION access permission(need admin)
						process_handle.get_handle(),
						TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY | TOKEN_DUPLICATE | TOKEN_ASSIGN_PRIMARY | TOKEN_ADJUST_SESSIONID | TOKEN_READ | TOKEN_WRITE,
						&h_ptoken)
						)
					{
						out_error_return_functions.push_back(L"OpenProcessToken");
					}
					_ns_tools::ct_warp::chandle ptoken_handle(h_ptoken);
					if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid))
					{
						out_error_return_functions.push_back(L"LookupPrivilegeValue");
					}
					tp.PrivilegeCount = 1;
					tp.Privileges[0].Luid = luid;
					tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;


					if (!::DuplicateTokenEx(///////.......
						ptoken_handle.get_handle(),
						MAXIMUM_ALLOWED,
						NULL,
						SecurityIdentification,
						TokenPrimary,
						&h_user_token_dup
					)) {
						out_error_return_functions.push_back(L"DuplicateTokenEx");
					}
					else {
						user_token_dup_handle.set_handle(h_user_token_dup);
					}

					//Adjust Token privilege
					::SetTokenInformation(
						user_token_dup_handle.get_handle(),
						TokenSessionId,
						reinterpret_cast<void*>(dw_session_id),
						sizeof(DWORD)
					);

					if (!AdjustTokenPrivileges(///////.......
						user_token_dup_handle.get_handle(),
						FALSE,
						&tp,
						sizeof(TOKEN_PRIVILEGES),
						(PTOKEN_PRIVILEGES)NULL,
						NULL)
						) {
							out_error_return_functions.push_back(L"AdjustTokenPrivileges");
					}

					if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)
					{
						out_error_return_functions.push_back(L"AdjustTokenPrivileges:ERROR_NOT_ALL_ASSIGNED");
					}
				}
				s_exe_file_path.copy(&v_exe_file[0], s_exe_file_path.size() + 1);
				
				wchar_t *p_ws_command_line(NULL);
				if(!s_command_line.empty()){
					s_command_line.copy(&v_command_line[0], s_command_line.size() + 1);
					p_ws_command_line = &v_command_line[0];
				}

				if (!s_working_directory.empty()) {
					lpCurrentDirectory = s_working_directory.c_str();
				}

				if (::CreateEnvironmentBlock(&p_env, user_token_dup_handle.get_handle(), TRUE))
				{
					dwCreationFlags |= CREATE_UNICODE_ENVIRONMENT;
				}
				else {
					p_env = NULL;
					out_error_return_functions.push_back(L"CreateEnvironmentBlock");
				}

				_ns_tools::ct_warp::cenvironment_block env_block(p_env);

				// Launch the process in the client's logon session. or session0
				if (!::CreateProcessAsUser(
					user_token_dup_handle.get_handle(),            // client's access token
					&v_exe_file[0],              // file to execute
					p_ws_command_line,     // command line
					NULL,              // pointer to process SECURITY_ATTRIBUTES
					NULL,              // pointer to thread SECURITY_ATTRIBUTES
					FALSE,             // handles are not inheritable
					dwCreationFlags,  // creation flags
					env_block.get_block(),              // pointer to new environment block 
					lpCurrentDirectory,              // name of current directory 
					&si,               // pointer to STARTUPINFO structure
					&out_proc_info		// receives information about new process
				)) {
					dw_result = GetLastError();
					out_error_return_functions.push_back(std::wstring(L"CreateProcessAsUser:")+std::to_wstring(dw_result));
					continue;
				}
				b_result = true;

			} while (false);

			return b_result;
		}
	public:
		~ct_system() {}

	private: //don't call these methods
		ct_system();
		ct_system(const ct_system &);
		ct_system &operator=(const ct_system &);
	};
}
