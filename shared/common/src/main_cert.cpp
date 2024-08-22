#include <main.h>

#include <iostream>

#include <mp_type.h>
#include <mp_clog.h>
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
		n_result = cdef_const::exit_error_create_install_cert;
	}
	return n_result;
}
