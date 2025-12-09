#pragma once

#include <string>
#include <vector>

namespace _mp {
	class ccoffee_hlp_cert
	{
	public:

		static bool coffee_hlp_cert_create_and_store_cert_for_coffee_manager(
			const std::wstring& s_ca_cert_common_name
			, const std::wstring& s_server_cert_common_name
			, const std::wstring& s_server_cert_organization_name
			, const std::wstring& s_file_server_cert
			, const std::wstring& s_file_server_key
		);

		static bool coffee_hlp_cert_remove_cert_from_system_root_ca(
			const std::wstring& s_ca_cert_common_name,
			const std::wstring& s_file_server_cert
		);

#ifdef _WIN32
		static bool set_file_permissions_to_system_and_admins_only(const std::wstring& s_abs_full_path_file, std::wstring& s_deb_msg);
		static bool expand_file_permissions_to_everyone(const std::wstring& s_abs_full_path_file, std::wstring& s_deb_msg);
#endif
		~ccoffee_hlp_cert()
		{
		}

	private:
		ccoffee_hlp_cert()
		{
		}

	private://don't call these methods.
		ccoffee_hlp_cert(const ccoffee_hlp_cert&);
		ccoffee_hlp_cert& operator=(const ccoffee_hlp_cert&);

	};

}//the end of _mp