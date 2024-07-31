#include <main.h>

#include <iostream>

#include <mp_type.h>
#include <cert/mp_ccoffee_hlp_cert_.h>

/**
 * remove certificate.
 */
int main_remove_cert(const _mp::type_set_wstring& set_parameters)
{
#ifdef _WIN32
	std::wstring s_server_certificate_file = L"C:\\job\\library.cpp.lpu230\\shared\\data_for_debug\\cert\\coffee_server.crt";
	std::wstring s_server_private_key_file = L"C:\\job\\library.cpp.lpu230\\shared\\data_for_debug\\cert\\coffee_server.key";
#else
	std::wstring s_server_certificate_file = L"/home/tester/projects/LiElpuskHidDaemon/bin/x64/Debug/coffee_server.crt";
	std::wstring s_server_private_key_file = L"/home/tester/projects/LiElpuskHidDaemon/bin/x64/Debug/coffee_server.key";
#endif

	if ([=]()->bool {
		bool b_result(false);

		do {
			std::wstring s_ca_cert_common_name(L"coffee-server-root-ca");

			std::wstring s_server_cert_common_name(L"coffee_server.local");
			std::wstring s_server_cert_organization_name(L"coffee-server-ca");

			std::wstring s_file_server_cert(s_server_certificate_file);
			std::wstring s_file_server_key(s_server_private_key_file);

			if (!_mp::ccoffee_hlp_cert::coffee_hlp_cert_remove_cert_from_system_root_ca(
				s_ca_cert_common_name,
				s_file_server_cert
			)) {

				continue;
			}

			b_result = true;
		} while (false);
		return b_result;
		}()
			) {
		wprintf(L"[I] %ls | remove certificates.\n", __WFUNCTION__);
		std::wcout << L"success : remove certificates." << std::endl;
	}
	else {
		wprintf(L"[E] %ls | remove certificates.\n", __WFUNCTION__);
		std::wcout << L"fail : remove certificates." << std::endl;
	}

	return 0;
}