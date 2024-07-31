#pragma once

#include <string>
#include <memory>
#include <vector>

#ifdef _WIN32
#include <winsock2.h>
#include <Ws2tcpip.h>
#endif //_WIN32

#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/x509_vfy.h>

#include <mp_clog.h>

namespace _mp {

class ccertificate
{
public:
	typedef	std::shared_ptr< EVP_PKEY >		type_ptr_EVP_PKEY;
	typedef	std::shared_ptr< X509 >			type_ptr_X509;
	typedef	std::shared_ptr< X509_REQ >		type_ptr_X509_REQ;
	typedef std::shared_ptr< X509_EXTENSION >	type_ptr_X509_EXTENSION;
	typedef std::shared_ptr< X509_STORE >	type_ptr_X509_STORE;

private:
	enum {
		_size_serial_numerber_randum_bits = 159
	};

public:
	static clog* get_log(clog* p_log = nullptr)
	{
		static clog* _p_log(nullptr);
		if (p_log)
			_p_log = p_log;
		return _p_log;
	}

	//Generates a self-signed x509 certificate. store to system
	//Generates the new certificate of "self-signed x509 certificate". 
	//certificate & private key file are saved to files.
	static bool create_store_new_ca_cert_to_system_and_generate_save_new_cert(
		const std::wstring& s_ca_cert_common_name
		, const std::wstring& s_cert_common_name
		, const std::wstring& s_cert_organization_name
		, const std::wstring& s_file_cert
		, const std::wstring& s_file_key
	);

	//Generates a 2048-bit RSA key.
	static type_ptr_EVP_PKEY generate_2048_rsa_key(const std::wstring& s_will_be_saved_key_file_by_pem = std::wstring());

	//Generates a self-signed x509 certificate.
	static type_ptr_X509 generate_self_signed_x509_certificate(
		const type_ptr_EVP_PKEY& ptr_key
		, const std::wstring & s_common_name
		, long n_validity_year=20
		, const std::wstring& s_self_signed_cert_pem_format_file = std::wstring()
	);

	//Generates a x509 certificate.
	static type_ptr_X509 generate_x509_certificate(
		const type_ptr_EVP_PKEY& ptr_ca_key
		, const type_ptr_X509& ptr_ca_cert
		, const std::wstring& s_common_name
		, const std::wstring& s_organization_name
		, long n_validity_year = 10
		, const std::wstring& s_cert_pem_format_key_file = std::wstring()
		, const std::wstring& s_cert_pem_format_file = std::wstring()
	);

	static bool store_certificate_to_system_root_ca(const type_ptr_X509& ptr_cert, const std::wstring& s_file_cert);

	static bool remove_certificate_from_system_root_ca(
		const std::wstring& s_ca_cert_common_name,
		const std::wstring& s_file_cert,
		bool b_not_found_is_true = true
	);

private:
	static std::string _rand_key_id();
	static bool _rand_serial(BIGNUM* b, ASN1_INTEGER* ai);
	static bool _cert_add_ext(X509* issuer, X509* subject, int nid,const char* value);
	
	static type_ptr_X509_REQ _generate_x509_certificate_signing_request(
		const type_ptr_EVP_PKEY& ptr_key
		, const std::wstring& s_common_name
		, const std::wstring& s_organization_name
		, const std::wstring& s_csr_file = std::wstring()
	);

	static type_ptr_X509 _generate_x509_certificate(
		const type_ptr_EVP_PKEY & ptr_ca_key
		, const type_ptr_X509 & ptr_ca_cert
		, const type_ptr_X509_REQ & ptr_csr
		, long n_validity_year = 10
		, const std::wstring& s_cert_pem_format_file = std::wstring()
	);//Generates a x509 certificate.

private://don't call these method.
	~ccertificate();
	ccertificate();
	ccertificate(const ccertificate&);
	ccertificate& operator=(const ccertificate&);
};

}//the end of _mp namespace