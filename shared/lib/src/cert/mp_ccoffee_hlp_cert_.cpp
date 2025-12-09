#include <websocket/mp_win_nt.h>



#include <mp_cstring.h>
#include <mp_clog.h>

#include <cert/mp_ccoffee_hlp_cert_.h>
#include <cert/mp_ccertificate_.h>

#ifdef _WIN32
#include <aclapi.h>
#pragma comment (lib, "crypt32")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "libcrypto.lib")
#pragma comment(lib, "libssl.lib")
#pragma comment(lib, "advapi32.lib")
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

#ifdef _WIN32
	bool ccoffee_hlp_cert::set_file_permissions_to_system_and_admins_only(const std::wstring& s_abs_full_path_file, std::wstring& s_deb_msg)
	{
		bool b_result(false);
		DWORD dwRes = 0;
		PACL pNewDACL = NULL;
		PSECURITY_DESCRIPTOR pSD = NULL;

		// EXPLICIT_ACCESS 배열: SYSTEM과 Administrators를 위한 2개의 ACE
		EXPLICIT_ACCESS ea[2];
		ZeroMemory(&ea, sizeof(EXPLICIT_ACCESS) * 2);

		// 1. SYSTEM 계정: Full Control (FILE_ALL_ACCESS)
		ea[0].grfAccessPermissions = FILE_ALL_ACCESS;  // 또는 GENERIC_ALL
		ea[0].grfAccessMode = GRANT_ACCESS;
		ea[0].grfInheritance = NO_INHERITANCE;  // 상속 없음 (파일이니)
		ea[0].Trustee.TrusteeForm = TRUSTEE_IS_NAME;
		ea[0].Trustee.ptstrName = const_cast<wchar_t*>(L"NT AUTHORITY\\SYSTEM");

		// 2. Administrators 그룹: Full Control
		ea[1].grfAccessPermissions = FILE_ALL_ACCESS;
		ea[1].grfAccessMode = GRANT_ACCESS;
		ea[1].grfInheritance = NO_INHERITANCE;
		ea[1].Trustee.TrusteeForm = TRUSTEE_IS_NAME;
		ea[1].Trustee.ptstrName = const_cast<wchar_t*>(L"BUILTIN\\Administrators");

		do {
			// 새 ACL 생성 (기존 ACL 무시하고 새로 만듦: NULL 전달)
			dwRes = SetEntriesInAcl(2, ea, NULL, &pNewDACL);
			if (dwRes != ERROR_SUCCESS) {
				std::wstringstream ss;
				ss << L"SetEntriesInAcl Error: " << dwRes;
				s_deb_msg = ss.str();
				continue;
			}

			// 새 ACL을 파일의 DACL로 설정 (상속 차단: PROTECTED_DACL_SECURITY_INFORMATION 플래그)
			dwRes = SetNamedSecurityInfo((LPTSTR)s_abs_full_path_file.c_str(), SE_FILE_OBJECT,
				DACL_SECURITY_INFORMATION | PROTECTED_DACL_SECURITY_INFORMATION,
				NULL, NULL, pNewDACL, NULL);
			if (dwRes != ERROR_SUCCESS) {
				std::wstringstream ss;
				ss << L"SetNamedSecurityInfo Error: " << dwRes;
				s_deb_msg = ss.str();
				continue;
			}

			b_result = true;
		} while (false);

		if (pNewDACL) LocalFree(pNewDACL);
		if (pSD) LocalFree(pSD);  // 필요 시
		return b_result;
	}

	bool ccoffee_hlp_cert::expand_file_permissions_to_everyone(const std::wstring& s_abs_full_path_file, std::wstring& s_deb_msg)
	{
		// 위 SetFilePermissionsToSystemAndAdminsOnly와 유사하게, 기존 ACL 가져와 Everyone 추가
		bool b_result(false);
		DWORD dwRes = 0;
		PACL pOldDACL = NULL, pNewDACL = NULL;
		PSECURITY_DESCRIPTOR pSD = NULL;
		EXPLICIT_ACCESS ea;

		do {
			dwRes = GetNamedSecurityInfo((LPTSTR)s_abs_full_path_file.c_str(), SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, NULL, NULL, &pOldDACL, NULL, &pSD);
			if (dwRes != ERROR_SUCCESS) {
				std::wstringstream ss;
				ss << L"SetNamedSecurityInfo-1 Error: " << dwRes;
				s_deb_msg = ss.str();
				continue;
			}

			ZeroMemory(&ea, sizeof(EXPLICIT_ACCESS));
			ea.grfAccessPermissions = FILE_ALL_ACCESS;
			ea.grfAccessMode = GRANT_ACCESS;
			ea.grfInheritance = NO_INHERITANCE;
			ea.Trustee.TrusteeForm = TRUSTEE_IS_NAME;
			ea.Trustee.ptstrName = const_cast<wchar_t*>(L"Everyone");

			dwRes = SetEntriesInAcl(1, &ea, pOldDACL, &pNewDACL);
			if (dwRes != ERROR_SUCCESS) {
				std::wstringstream ss;
				ss << L"SetEntriesInAcl Error: " << dwRes;
				s_deb_msg = ss.str();
				continue;
			}

			dwRes = SetNamedSecurityInfo((LPTSTR)s_abs_full_path_file.c_str(), SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, NULL, NULL, pNewDACL, NULL);
			if (dwRes != ERROR_SUCCESS) {
				std::wstringstream ss;
				ss << L"SetNamedSecurityInfo-2 Error: " << dwRes;
				s_deb_msg = ss.str();
				continue;
			}

			b_result = true;
		} while (false);

		if (pSD) LocalFree(pSD);
		if (pNewDACL) LocalFree(pNewDACL);
		return b_result;
	}
#endif

}//the end of _mp