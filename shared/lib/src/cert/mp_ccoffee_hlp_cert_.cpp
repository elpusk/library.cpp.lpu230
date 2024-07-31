#include <websocket/mp_win_nt.h>



#include <mp_cstring.h>
#include <mp_clog.h>

#include <cert/mp_ccoffee_hlp_cert_.h>
#include <cert/mp_ccertificate_.h>

#ifdef _WIN32
#pragma comment (lib, "crypt32")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "libcrypto.lib")
#pragma comment(lib, "libssl.lib")
#endif

namespace _mp {


	bool ccoffee_hlp_cert::coffee_hlp_cert_create_and_store_cert_for_coffee_manager(
		const std::wstring& s_ca_cert_common_name
		, const std::wstring& s_server_cert_common_name
		, const std::wstring& s_server_cert_organization_name
		, const std::wstring& s_file_server_cert
		, const std::wstring& s_file_server_key
	)
	{
		bool b_result(false);

		do {
			if (s_ca_cert_common_name.empty())
				continue;
			if (s_server_cert_common_name.empty())
				continue;
			if (s_server_cert_organization_name.empty())
				continue;
			if (s_file_server_cert.empty())
				continue;
			if (s_file_server_key.empty())
				continue;

			if (!ccertificate::create_store_new_ca_cert_to_system_and_generate_save_new_cert(
				s_ca_cert_common_name
				, s_server_cert_common_name
				, s_server_cert_organization_name
				, s_file_server_cert
				, s_file_server_key
			))
				continue;

			b_result = true;
		} while (false);

		return b_result;

	}

	bool ccoffee_hlp_cert::coffee_hlp_cert_remove_cert_from_system_root_ca(
		const std::wstring& s_ca_cert_common_name,
		const std::wstring& s_file_server_cert
	)
	{
		bool b_result(false);

		do {
			if (s_ca_cert_common_name.empty())
				continue;

			if (!ccertificate::remove_certificate_from_system_root_ca(
				s_ca_cert_common_name,
				s_file_server_cert
			))
				continue;

			b_result = true;
		} while (false);

		return b_result;

	}


}//the end of _mp