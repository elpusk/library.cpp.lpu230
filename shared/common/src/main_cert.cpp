#include <main.h>

#include <iostream>
#include <filesystem>

#include <mp_type.h>
#include <mp_clog.h>
#include <mp_cstring.h>
#include <cert/mp_ccoffee_hlp_cert_.h>
/**
 * generate & install certificate 
 */
int main_cert(const _mp::type_set_wstring &set_parameters)
{
	int n_result(EXIT_FAILURE);
	std::wstring s_server_certificate_file = cdef_const::get_certificate_file();
	std::wstring s_server_private_key_file = cdef_const::get_private_key_file();

	if ([&]()->bool {
		bool b_result(false);

		do {
			std::wstring s_ca_cert_common_name(L"coffee-server-root-ca");

			std::wstring s_server_cert_common_name(L"coffee_server.local");
			std::wstring s_server_cert_organization_name(L"coffee-server-ca");

			std::wstring s_file_server_cert(s_server_certificate_file);
			std::wstring s_file_server_key(s_server_private_key_file);

			if (!_mp::ccoffee_hlp_cert::coffee_hlp_cert_create_and_store_cert_for_coffee_manager(
				s_ca_cert_common_name
				, s_server_cert_common_name
				, s_server_cert_organization_name
				, s_file_server_cert
				, s_file_server_key

			)) {

				continue;
			}

#ifdef _WIN32
			std::wstring s_debug_msg;
			if (!_mp::ccoffee_hlp_cert::set_file_permissions_to_system_and_admins_only(s_file_server_key, s_debug_msg)) {
				_mp::clog::get_instance().trace(L"[E] - %ls - %ls - set pk file permissions to system and admins only : %ls.\n", __WFILE__, __WFUNCTION__, s_debug_msg.c_str());
				continue;
			}
			if(!_mp::ccoffee_hlp_cert::set_file_permissions_to_system_and_admins_only(s_file_server_cert, s_debug_msg)) {
				_mp::clog::get_instance().trace(L"[E] - %ls - %ls - set cert file permissions to system and admins only : %ls.\n", __WFILE__, __WFUNCTION__, s_debug_msg.c_str());
				continue;
			}
#else
			//linux
			std::wstring ws_debug_msg;
			std::string s_debug_msg;

			std::string s_pk_file = _mp::cstring::get_mcsc_from_unicode(s_file_server_key);
			std::string s_cert_file = _mp::cstring::get_mcsc_from_unicode(s_file_server_cert);

			std::string s_dir = std::filesystem::path(s_pk_file).parent_path().string();

			// Private Key 파일 먼저 설정 (가장 중요: 0600 - root만 읽기/쓰기)
			if (!_mp::ccoffee_hlp_cert::set_file_permissions_with_own_root_on_linux(s_pk_file,0600, s_debug_msg)) {
				ws_debug_msg = _mp::cstring::get_unicode_from_mcsc(s_debug_msg);
				_mp::clog::get_instance().trace(L"[E] - %ls - %ls - set pk file permissions : %ls.\n", __WFILE__, __WFUNCTION__, ws_debug_msg.c_str());
				continue;
			}

			// 인증서 파일 설정 (공개라서 0644 - 모든 사용자 읽기 가능)
			if (!_mp::ccoffee_hlp_cert::set_file_permissions_with_own_root_on_linux(s_cert_file, 0644, s_debug_msg)) {
				ws_debug_msg = _mp::cstring::get_unicode_from_mcsc(s_debug_msg);
				_mp::clog::get_instance().trace(L"[E] - %ls - %ls - set cert file permissions : %ls.\n", __WFILE__, __WFUNCTION__, ws_debug_msg.c_str());
				continue;
			}

			// 인증서와 private key 있는 directory (0700)
			if (!_mp::ccoffee_hlp_cert::set_file_permissions_with_own_root_on_linux(s_dir, 0700, s_debug_msg)) {
				ws_debug_msg = _mp::cstring::get_unicode_from_mcsc(s_debug_msg);
				_mp::clog::get_instance().trace(L"[E] - %ls - %ls - set cert file diectory permissions : %ls.\n", __WFILE__, __WFUNCTION__, ws_debug_msg.c_str());
				continue;
			}

#endif
			b_result = true;
		} while (false);
		return b_result;
		}()
			) {
		_mp::clog::get_instance().trace(L"[I] - %ls - %ls - config certificates.\n", __WFILE__, __WFUNCTION__);
		n_result = EXIT_SUCCESS;
	}
	else {
		_mp::clog::get_instance().trace(L"[E] - %ls - %ls - config certificates.\n", __WFILE__, __WFUNCTION__);
		n_result = _mp::exit_error_create_install_cert;
	}
	return n_result;
}
