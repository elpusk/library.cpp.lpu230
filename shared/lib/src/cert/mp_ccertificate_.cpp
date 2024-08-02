#include <websocket/mp_win_nt.h>

#ifdef _WIN32
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif // _MSC_VER
#include <windows.h>
#include <wincrypt.h>
#pragma comment(lib, "crypt32.lib") // Ensure the crypt32 library is linked

#endif


#include <sstream>
#include <iomanip> 

#include <mp_type.h>
#include <mp_cstring.h>
#include <mp_cfile.h>
#include <mp_cconvert.h>
#include <cert/mp_ccertificate_.h>

namespace _mp {
    bool ccertificate::create_store_new_ca_cert_to_system_and_generate_save_new_cert(
        const std::wstring& s_ca_cert_common_name
        , const std::wstring& s_cert_common_name
        , const std::wstring& s_cert_organization_name
        , const std::wstring& s_file_cert
        , const std::wstring& s_file_key
    )
    {
        bool b_result(false);

        if (ccertificate::get_log()) {
            ccertificate::get_log()->log_fmt(L"[I] - %ls | enter.\n", __WFUNCTION__);
            ccertificate::get_log()->trace(L"[I] - %ls | enter.\n", __WFUNCTION__);
        }

        do {
            /////////////////////////////////
            //Create CA key & certificate.
            type_ptr_EVP_PKEY ptr_ca_private_key = ccertificate::generate_2048_rsa_key();
            if (!ptr_ca_private_key) {
                if (ccertificate::get_log()) {
                    ccertificate::get_log()->log_fmt(L"[E] - %ls | generate_2048_rsa_key.\n", __WFUNCTION__);
                    ccertificate::get_log()->trace(L"[E] - %ls | generate_2048_rsa_key.\n", __WFUNCTION__);
                }
                continue;
            }
            type_ptr_X509 ptr_ca_certificate = ccertificate::generate_self_signed_x509_certificate(
                ptr_ca_private_key
                , s_ca_cert_common_name
                , 20
            );

            if (!ptr_ca_certificate) {
                if (ccertificate::get_log()) {
                    ccertificate::get_log()->log_fmt(L"[E] - %ls | generate_self_signed_x509_certificate.\n", __WFUNCTION__);
                    ccertificate::get_log()->trace(L"[E] - %ls | generate_self_signed_x509_certificate.\n", __WFUNCTION__);
                }
                continue;
            }
            //
            /////////////////////////////////
            //Create certificate with the created CA key & certificate.

            type_ptr_X509 ptr_saver_certificate = ccertificate::generate_x509_certificate(
                ptr_ca_private_key
                , ptr_ca_certificate
                , s_cert_common_name
                , s_cert_organization_name
                , 10
                , s_file_key
                , s_file_cert);
            if (!ptr_saver_certificate) {
                if (ccertificate::get_log()) {
                    ccertificate::get_log()->log_fmt(L"[E] - %ls | generate_x509_certificate.\n", __WFUNCTION__);
                    ccertificate::get_log()->trace(L"[E] - %ls | generate_x509_certificate.\n", __WFUNCTION__);
                }
                continue;
            }
#ifdef _WIN32
            if (!ccertificate::remove_certificate_from_system_root_ca(s_ca_cert_common_name, s_file_cert)) {
                if (ccertificate::get_log()) {
                    ccertificate::get_log()->log_fmt(L"[E] - %ls | remove_certificate_from_system_root_ca.\n", __WFUNCTION__);
                    ccertificate::get_log()->trace(L"[E] - %ls | remove_certificate_from_system_root_ca.\n", __WFUNCTION__);
                }
                continue;
            }
#endif
            if (!ccertificate::store_certificate_to_system_root_ca(ptr_ca_certificate, s_file_cert)) {
                if (ccertificate::get_log()) {
                    ccertificate::get_log()->log_fmt(L"[E] - %ls | store_certificate_to_system_root_ca.\n", __WFUNCTION__);
                    ccertificate::get_log()->trace(L"[E] - %ls | store_certificate_to_system_root_ca.\n", __WFUNCTION__);
                }
                continue;
            }

            b_result = true;
        } while (false);
        return b_result;
    }

    bool ccertificate::remove_certificate_from_system_root_ca(
        const std::wstring& s_ca_cert_common_name,
        const std::wstring& s_file_cert,
        bool b_not_found_is_true
    ) 
    {
        bool b_result = true;

#ifdef _WIN32
        unsigned long dw_result = 0;
        HCERTSTORE h_store = NULL;
        PCCERT_CONTEXT p_cert_context = NULL;

        if (ccertificate::get_log()) {
            ccertificate::get_log()->log_fmt(L"[I] - %ls | enter.\n", __WFUNCTION__);
            ccertificate::get_log()->trace(L"[I] - %ls | enter.\n", __WFUNCTION__);
        }

        do {
            h_store = CertOpenStore(
                CERT_STORE_PROV_SYSTEM,
                X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                0,
                CERT_SYSTEM_STORE_LOCAL_MACHINE,
                L"ROOT"
            );
            if (h_store == NULL) {
                if (ccertificate::get_log()) {
                    ccertificate::get_log()->log_fmt(L"[E] - %ls | CertOpenStore.\n", __WFUNCTION__);
                    ccertificate::get_log()->trace(L"[E] - %ls | CertOpenStore.\n", __WFUNCTION__);
                }
                b_result = false;
                break;
            }

            std::wstring s_common_name(s_ca_cert_common_name);
            char s_id[] = szOID_COMMON_NAME;
            CERT_RDN_ATTR subject_cn[1];
            subject_cn[0].pszObjId = s_id;
            subject_cn[0].dwValueType = CERT_RDN_ANY_TYPE;
            subject_cn[0].Value.cbData = s_common_name.length() * (unsigned long)sizeof(wchar_t);
            subject_cn[0].Value.pbData = (unsigned char*)s_common_name.c_str();

            CERT_RDN rdn;
            rdn.cRDNAttr = 1;
            rdn.rgRDNAttr = subject_cn;

            p_cert_context = CertFindCertificateInStore(
                h_store,
                X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                CERT_UNICODE_IS_RDN_ATTRS_FLAG,
                CERT_FIND_SUBJECT_ATTR,
                &rdn,
                NULL
            );
            if (p_cert_context == NULL) {
                dw_result = GetLastError();
                if (dw_result == CRYPT_E_NOT_FOUND && b_not_found_is_true) {
                    if (ccertificate::get_log()) {
                        ccertificate::get_log()->log_fmt(L"[W] - %ls | CertFindCertificateInStore error code = CRYPT_E_NOT_FOUND.\n", __WFUNCTION__);
                        ccertificate::get_log()->trace(L"[W] - %ls | CertFindCertificateInStore error code = CRYPT_E_NOT_FOUND.\n", __WFUNCTION__);
                    }
                    b_result = true;
                }
                else {
                    if (ccertificate::get_log()) {
                        ccertificate::get_log()->log_fmt(L"[E] - %ls | CertFindCertificateInStore error code = %u.\n", __WFUNCTION__, dw_result);
                        ccertificate::get_log()->trace(L"[E] - %ls | CertFindCertificateInStore error code = %u.\n", __WFUNCTION__, dw_result);
                    }
                    b_result = false;
                }
                break;
            }

            if (!CertDeleteCertificateFromStore(CertDuplicateCertificateContext(p_cert_context))) {
                if (ccertificate::get_log()) {
                    ccertificate::get_log()->log_fmt(L"[E] - %ls | CertDeleteCertificateFromStore.\n", __WFUNCTION__);
                    ccertificate::get_log()->trace(L"[E] - %ls | CertDeleteCertificateFromStore.\n", __WFUNCTION__);
                }
                b_result = false;
                break;
            }

            b_result = true;
        } while (false);

        if (p_cert_context) CertFreeCertificateContext(p_cert_context);
        if (h_store) CertCloseStore(h_store, 0);

#else
        X509* cert = NULL;
        FILE* fp = fopen("/etc/ssl/certs/ca-certificates.crt", "r");
        if (!fp) {
            perror("fopen");
            return false;
        }

        std::string s_ansi_of_ca_cert_common_name = cstring::get_mcsc_from_unicode(s_ca_cert_common_name);

        bool found = false;
        while ((cert = PEM_read_X509(fp, NULL, NULL, NULL)) != NULL) {
            X509_NAME* subj = X509_get_subject_name(cert);
            int idx = X509_NAME_get_index_by_NID(subj, NID_commonName, -1);
            if (idx >= 0) {
                X509_NAME_ENTRY* e = X509_NAME_get_entry(subj, idx);
                ASN1_STRING* d = X509_NAME_ENTRY_get_data(e);
                unsigned char* cn = ASN1_STRING_data(d);

                if (cn != nullptr && !s_ansi_of_ca_cert_common_name.empty()) {
                    if (s_ansi_of_ca_cert_common_name.compare(std::string((const char*)cn)) == 0) {
                        found = true;
                        break;
                    }
                }
            }
            X509_free(cert);
        }
        fclose(fp);

        if (!found) {
            if (b_not_found_is_true) {
                if (ccertificate::get_log()) {
                    ccertificate::get_log()->log_fmt(L"[W] - %ls | Certificate not found.\n", __FUNCTION__);
                    ccertificate::get_log()->trace(L"[W] - %ls | Certificate not found.\n", __FUNCTION__);
                }
                return true;
            }
            else {
                if (ccertificate::get_log()) {
                    ccertificate::get_log()->log_fmt(L"[E] - %ls | Certificate not found.\n", __FUNCTION__);
                    ccertificate::get_log()->trace(L"[E] - %ls | Certificate not found.\n", __FUNCTION__);
                }
                return false;
            }
        }

        std::wstring s_out_drive;
        std::wstring s_out_dir;
        std::wstring s_out_file_name;
        std::wstring s_out_ext;

        cfile::split_path(
            s_file_cert
            , s_out_drive
            , s_out_dir
            , s_out_file_name
            , s_out_ext
        );

        std::string cmd = "sudo rm -f /usr/local/share/ca-certificates/ca-" + cstring::get_mcsc_from_unicode(s_out_file_name) + ".crt";
        int ret = system(cmd.c_str());
        if (ret != 0) {
            if (ccertificate::get_log()) {
                ccertificate::get_log()->log_fmt(L"[E] - %ls | Failed to delete certificate.\n", __FUNCTION__);
                ccertificate::get_log()->trace(L"[E] - %ls | Failed to delete certificate.\n", __FUNCTION__);
            }
            return false;
        }

        ret = system("sudo update-ca-certificates");
        if (ret != 0) {
            if (ccertificate::get_log()) {
                ccertificate::get_log()->log_fmt(L"[E] - %ls | Failed to update CA certificates.\n", __FUNCTION__);
                ccertificate::get_log()->trace(L"[E] - %ls | Failed to update CA certificates.\n", __FUNCTION__);
            }
            return false;
        }

        b_result = true;
#endif

        return b_result;
    }

    bool ccertificate::store_certificate_to_system_root_ca(const std::shared_ptr<X509>& ptr_cert, const std::wstring& s_file_cert)
    {
        bool b_result = false;

#ifdef _WIN32
        unsigned long dw_result = 0;
        HCERTSTORE h_store = NULL;
        unsigned char* s_cert = NULL;

        do {
            if (!ptr_cert) continue;

            h_store = CertOpenStore(
                CERT_STORE_PROV_SYSTEM,
                X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                0,
                CERT_SYSTEM_STORE_LOCAL_MACHINE,
                L"ROOT"
            );

            if (h_store == NULL) {
                if (ccertificate::get_log()) {
                    ccertificate::get_log()->trace(L"[E] - %s - %s - CertOpenStore().\n", __WFILE__, __WFUNCTION__);
                }
                continue;
            }

            int n_cert = i2d_X509(ptr_cert.get(), &s_cert);
            if (n_cert <= 0) {
                if (ccertificate::get_log()) {
                    ccertificate::get_log()->trace(L"[E] - %s - %s - i2d_X509().\n", __WFILE__, __WFUNCTION__);
                }
                continue;
            }

            if (!CertAddEncodedCertificateToStore(
                h_store,
                X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                s_cert,
                n_cert,
                CERT_STORE_ADD_REPLACE_EXISTING,
                NULL
            )) {
                dw_result = GetLastError();
                if (ccertificate::get_log()) {
                    ccertificate::get_log()->trace(L"[E] - %s - %s - CertAddEncodedCertificateToStore() - code - %u.\n", __WFILE__, __WFUNCTION__, dw_result);
                }
                continue;
            }

            b_result = true;
        } while (false);

        if (s_cert) OPENSSL_free(s_cert);
        if (h_store) CertCloseStore(h_store, 0);

#else
     do {
        if (!ptr_cert) {
            if (ccertificate::get_log()) {
                ccertificate::get_log()->trace(L"[E] - %s - %s - Invalid certificate.\n", __WFILE__, __WFUNCTION__);
            }
            continue;
        }

        std::wstring s_out_drive;
        std::wstring s_out_dir;
        std::wstring s_out_file_name;
        std::wstring s_out_ext;

        cfile::split_path(
            s_file_cert
            , s_out_drive
            , s_out_dir
            , s_out_file_name
            , s_out_ext
        );

        std::string cert_path = "/usr/local/share/ca-certificates/";
        cert_path = cert_path + "ca-";//linux only for root ca certificate prefix
        cert_path = cert_path + cstring::get_mcsc_from_unicode(s_out_file_name);
        cert_path = cert_path + ".";
        cert_path = cert_path + cstring::get_mcsc_from_unicode(s_out_ext);

        FILE* fp = fopen(cert_path.c_str(), "w");
        if (!fp) {
            perror("fopen");
            continue;
        }

        if (PEM_write_X509(fp, ptr_cert.get()) == 0) {
            if (ccertificate::get_log()) {
                ccertificate::get_log()->trace(L"[E] - %s - %s - PEM_write_X509().\n", __WFILE__, __WFUNCTION__);
            }
            fclose(fp);
            continue;
        }

        fclose(fp);

        int ret = system("sudo update-ca-certificates");
        if (ret != 0) {
            if (ccertificate::get_log()) {
                ccertificate::get_log()->trace(L"[E] - %s - %s - update CA certificates.\n", __WFILE__, __WFUNCTION__);
            }
            continue;
        }

        b_result = true;
    }while (false);
#endif

    return b_result;
}

    bool ccertificate::_cert_add_ext(X509* issuer, X509* subject, int nid, const char* value)
    {
        bool b_result(false);
        X509_EXTENSION* p_ex;
        X509V3_CTX ctx;

        do {
            //No configuration database
            X509V3_set_ctx_nodb(&ctx);
            // Set issuer and subject certificates in the context
            X509V3_set_ctx(&ctx, issuer, subject, NULL, NULL, 0);
            p_ex = X509V3_EXT_conf_nid(NULL, &ctx, nid, value);
            if (!p_ex)
                continue;
            X509_add_ext(subject, p_ex, -1);
            X509_EXTENSION_free(p_ex);

            b_result = true;
        } while (false);
        return b_result;
    }

    bool ccertificate::_rand_serial(BIGNUM* b, ASN1_INTEGER* ai)
    {
        bool b_result(false);

        do {
            BIGNUM* btmp;
            btmp = b == NULL ? BN_new() : b;
            if (btmp == NULL)
                continue;
            if (!BN_rand(btmp, _size_serial_numerber_randum_bits, BN_RAND_TOP_ANY, BN_RAND_BOTTOM_ANY)) {
                if (btmp != b)
                    BN_free(btmp);
                continue;
            }
            if (ai && !BN_to_ASN1_INTEGER(btmp, ai)) {
                if (btmp != b)
                    BN_free(btmp);
                continue;
            }
            //
            b_result = true;
        } while (false);
        return b_result;
    }

    std::string ccertificate::_rand_key_id()
    {
        std::vector<unsigned char> v_key_id(0);
        std::string s_key_id;

        do {
            std::shared_ptr<BIGNUM> ptr_btmp = std::shared_ptr<BIGNUM>(BN_new(), BN_free);
            if (!ptr_btmp)
                continue;
            if (!BN_rand(ptr_btmp.get(), _size_serial_numerber_randum_bits, BN_RAND_TOP_ANY, BN_RAND_BOTTOM_ANY))
                continue;
            ASN1_INTEGER* p_asn1_int = BN_to_ASN1_INTEGER(ptr_btmp.get(), nullptr);
            if (p_asn1_int == nullptr)
                continue;
            v_key_id.resize(p_asn1_int->length);
            std::copy(&p_asn1_int->data[0], &p_asn1_int->data[p_asn1_int->length], std::begin(v_key_id));

            cconvert::hex_string_from_binary(s_key_id, v_key_id);
        } while (false);
        return s_key_id;

    }

    //Generates a 2048-bit RSA key.
    ccertificate::type_ptr_EVP_PKEY ccertificate::generate_2048_rsa_key(const std::wstring& s_will_be_saved_key_file_by_pem)
    {
        type_ptr_EVP_PKEY ptr_key;

        if (ccertificate::get_log()) {
            ccertificate::get_log()->log_fmt(L"[I] - %ls | enter.\n", __WFUNCTION__);
            ccertificate::get_log()->trace(L"[I] - %ls | enter.\n", __WFUNCTION__);
        }

        do {
            std::shared_ptr<BIGNUM> ptr_bne = std::shared_ptr<BIGNUM>(BN_new(), BN_free);
            if (!ptr_bne) {
                continue;
            }
            if (BN_set_word(ptr_bne.get(), RSA_F4) != 1) {
                continue;
            }
            RSA* p_rsa_key = RSA_new();
            if (p_rsa_key == nullptr) {
                continue;
            }

            // Generate the RSA key and assign it to p_rsa_key. 
            if (RSA_generate_key_ex(p_rsa_key, 2048, ptr_bne.get(), NULL) != 1) {
                RSA_free(p_rsa_key);
                continue;
            }

            //Allocate memory for the EVP_PKEY structure.
            ptr_key = type_ptr_EVP_PKEY(EVP_PKEY_new(), EVP_PKEY_free);
            if (!ptr_key) {
                RSA_free(p_rsa_key);
                continue;
            }

            //The RSA structure will be automatically freed when the EVP_PKEY structure is freed.
            if (!EVP_PKEY_assign_RSA(ptr_key.get(), p_rsa_key)) {
                ptr_key.reset();
                RSA_free(p_rsa_key);
                continue;
            }

            if (s_will_be_saved_key_file_by_pem.empty())
                continue;
            //private key will be saved to file.
            std::string s_key_file = cstring::get_mcsc_from_unicode(s_will_be_saved_key_file_by_pem);

#ifdef _WIN32
            errno_t file_err;
            FILE* p_key_file;
            file_err = fopen_s(&p_key_file, s_key_file.c_str(), "wb");//pem format. file name *.key
            if (file_err != 0) {
                //error
                if (ccertificate::get_log()) {
                    ccertificate::get_log()->log_fmt(L"[E] - %ls | fopen_s(%d) | %ls.\n", __WFUNCTION__, file_err, s_will_be_saved_key_file_by_pem.c_str());
                    ccertificate::get_log()->trace(L"[E] - %ls | fopen_s(%d) | %ls.\n", __WFUNCTION__, file_err, s_will_be_saved_key_file_by_pem.c_str());
                }
                continue;
            }
#else
            FILE* p_key_file = fopen(s_key_file.c_str(), "wb");
            if (p_key_file == nullptr) {
                // Handle error
                perror("fopen");
                if (ccertificate::get_log()) {
                    ccertificate::get_log()->log_fmt(L"[E] - %ls | fopen_s() | %ls.\n", __WFUNCTION__, s_will_be_saved_key_file_by_pem.c_str());
                    ccertificate::get_log()->trace(L"[E] - %ls | fopen_s() | %ls.\n", __WFUNCTION__, s_will_be_saved_key_file_by_pem.c_str());
                }
                continue;
            }
#endif

            // Write the key to disk.
            bool b_return = PEM_write_PrivateKey(p_key_file, ptr_key.get(), NULL, NULL, 0, NULL, NULL);
            if (!b_return) {
                if (ccertificate::get_log()) {
                    ccertificate::get_log()->log_fmt(L"[E] - %ls | PEM_write_PrivateKey.\n", __WFUNCTION__);
                    ccertificate::get_log()->trace(L"[E] - %ls | PEM_write_PrivateKey.\n", __WFUNCTION__);
                }
            }

            if (p_key_file)
                fclose(p_key_file);

        } while (false);

        return ptr_key;
    }

    //Generates a self-signed x509 certificate.
    ccertificate::type_ptr_X509 ccertificate::generate_self_signed_x509_certificate(
        const type_ptr_EVP_PKEY& ptr_key
        , const std::wstring& s_common_name
        , long n_validity_year/*=20*/
        , const std::wstring& s_self_signed_cert_pem_format_file /*= std::wstring()*/
    )
    {
        type_ptr_X509 ptr_x509;

        if (ccertificate::get_log()) {
            ccertificate::get_log()->log_fmt(L"[I] - %ls | enter.\n", __WFUNCTION__);
            ccertificate::get_log()->trace(L"[I] - %ls | enter.\n", __WFUNCTION__);
        }

        do {
            if (s_common_name.empty())
                continue;
            if (n_validity_year <= 0)
                continue;
            if (n_validity_year > 20)
                continue;
            //
            ptr_x509 = type_ptr_X509(X509_new(), X509_free); //Allocate memory for the X509 structure.
            if (!ptr_x509)
                continue;
            //
            X509_set_version(ptr_x509.get(), 2);//set version(V3)
            // Set the serial number by randum.
            if (!ccertificate::_rand_serial(NULL, X509_get_serialNumber(ptr_x509.get()))) {
                ptr_x509.reset();
                continue;
            }

            // This certificate is valid from now until exactly one year from now.
            X509_gmtime_adj(X509_get_notBefore(ptr_x509.get()), 0);
            X509_gmtime_adj(X509_get_notAfter(ptr_x509.get()), n_validity_year * 365 * 24 * 60 * 60);//vaild range unit sec 31536000 = 365 * 24 * 60 * 60 = 1 year

            // Set the public key for our certificate.
            X509_set_pubkey(ptr_x509.get(), ptr_key.get());

            // We want to copy the subject name to the issuer name.
            X509_NAME* p_name = X509_get_subject_name(ptr_x509.get());

            // Set the country code and common name.
            std::string s_cn = cstring::get_mcsc_from_unicode(s_common_name);

            X509_NAME_add_entry_by_txt(p_name, "C", MBSTRING_ASC, (const unsigned char*)"KR", -1, -1, 0);
            X509_NAME_add_entry_by_txt(p_name, "CN", MBSTRING_ASC, (const unsigned char*)s_cn.c_str(), -1, -1, 0);

            // Now set the issuer name.
            X509_set_issuer_name(ptr_x509.get(), p_name);

            // add x509v3 extensions as specified
            std::string s_key_id(ccertificate::_rand_key_id());
            if (s_key_id.empty()) {
                ptr_x509.reset();
                continue;
            }
            // subject key identifier (SKID)
            if (!ccertificate::_cert_add_ext(ptr_x509.get(), ptr_x509.get(), NID_subject_key_identifier, "hash")) {
                ptr_x509.reset();
                continue;
            }
            //authority key identifier (AKID)
            if (!ccertificate::_cert_add_ext(ptr_x509.get(), ptr_x509.get(), NID_authority_key_identifier, "keyid:always")) {
                ptr_x509.reset();
                continue;
            }

            /*The pathlen parameter indicates the maximum number of CAs that can appear
                below this one in a chain.So if you have a CA with a pathlen of zero it can
                only be used to sign end user certificates and not further CAs.
                */
            if (!ccertificate::_cert_add_ext(ptr_x509.get(), ptr_x509.get(), NID_basic_constraints, "critical,CA:TRUE,pathlen:0")) {
                ptr_x509.reset();
                continue;
            }

            // Actually sign the certificate with our key.
            if (!X509_sign(ptr_x509.get(), ptr_key.get(), EVP_sha256())) {
                ptr_x509.reset();
                continue;
            }

            //save as file
            if (s_self_signed_cert_pem_format_file.empty()) {
                continue;
            }
            
            std::string s_cert_pem_file = cstring::get_mcsc_from_unicode(s_self_signed_cert_pem_format_file);

#ifdef _WIN32
            errno_t file_err;
            FILE* p_x509_file;
            file_err = fopen_s(&p_x509_file, s_cert_pem_file.c_str(), "wb");//pem format. file name *.key
            if (file_err != 0) {
                //error
                if (ccertificate::get_log()) {
                    ccertificate::get_log()->log_fmt(L"[E] - %ls | fopen_s(%d) | %ls.\n", __WFUNCTION__, file_err, s_self_signed_cert_pem_format_file.c_str());
                    ccertificate::get_log()->trace(L"[E] - %ls | fopen_s(%d) | %ls.\n", __WFUNCTION__, file_err, s_self_signed_cert_pem_format_file.c_str());
                }
                continue;
            }
#else
            FILE* p_x509_file = fopen(s_cert_pem_file.c_str(), "wb");
            if (p_x509_file == nullptr) {
                // Handle error
                perror("fopen");
                if (ccertificate::get_log()) {
                    ccertificate::get_log()->log_fmt(L"[E] - %ls | fopen_s() | %ls.\n", __WFUNCTION__, s_self_signed_cert_pem_format_file.c_str());
                    ccertificate::get_log()->trace(L"[E] - %ls | fopen_s() | %ls.\n", __WFUNCTION__, s_self_signed_cert_pem_format_file.c_str());
                }
                continue;
            }
#endif

            // Write the certificate to disk.
            bool b_return = PEM_write_X509(p_x509_file, ptr_x509.get());
            if (!b_return) {
                if (ccertificate::get_log()) {
                    ccertificate::get_log()->log_fmt(L"[E] - %ls | PEM_write_X509.\n", __WFUNCTION__);
                    ccertificate::get_log()->trace(L"[E] - %ls | PEM_write_X509.\n", __WFUNCTION__);
                }
            }

            if (p_x509_file)
                fclose(p_x509_file);

        } while (false);

        return ptr_x509;
    }

    ccertificate::type_ptr_X509 ccertificate::generate_x509_certificate(
        const type_ptr_EVP_PKEY& ptr_ca_key
        , const type_ptr_X509& ptr_ca_cert
        , const std::wstring& s_common_name
        , const std::wstring& s_organization_name
        , long n_validity_year/*= 10*/
        , const std::wstring& s_cert_pem_format_key_file /*= std::wstring()*/
        , const std::wstring& s_cert_pem_format_file /*= std::wstring()*/
    )
    {
        type_ptr_X509 ptr_saver_certificate;

        if (ccertificate::get_log()) {
            ccertificate::get_log()->log_fmt(L"[I] - %ls | enter.\n", __WFUNCTION__);
            ccertificate::get_log()->trace(L"[I] - %ls | enter.\n", __WFUNCTION__);
        }

        do {
            if (!ptr_ca_key)
                continue;
            if (!ptr_ca_cert)
                continue;
            //
            type_ptr_EVP_PKEY ptr_server_private_key = ccertificate::generate_2048_rsa_key(s_cert_pem_format_key_file);
            if (!ptr_server_private_key) {
                if (ccertificate::get_log()) {
                    ccertificate::get_log()->log_fmt(L"[E] - %ls | generate_2048_rsa_key.\n", __WFUNCTION__);
                    ccertificate::get_log()->trace(L"[E] - %ls | generate_2048_rsa_key.\n", __WFUNCTION__);
                }
                continue;
            }
            type_ptr_X509_REQ ptr_csr = ccertificate::_generate_x509_certificate_signing_request(
                ptr_server_private_key
                , s_common_name
                , s_organization_name
            );
            if (!ptr_csr) {
                if (ccertificate::get_log()) {
                    ccertificate::get_log()->log_fmt(L"[E] - %ls | _generate_x509_certificate_signing_request.\n", __WFUNCTION__);
                    ccertificate::get_log()->trace(L"[E] - %ls | _generate_x509_certificate_signing_request.\n", __WFUNCTION__);
                }
                continue;
            }
            ptr_saver_certificate = ccertificate::_generate_x509_certificate(ptr_ca_key, ptr_ca_cert, ptr_csr, n_validity_year, s_cert_pem_format_file);
            if (!ptr_saver_certificate) {
                if (ccertificate::get_log()) {
                    ccertificate::get_log()->log_fmt(L"[E] - %ls | _generate_x509_certificate.\n", __WFUNCTION__);
                    ccertificate::get_log()->trace(L"[E] - %ls | _generate_x509_certificate.\n", __WFUNCTION__);
                }
                continue;
            }
        } while (false);
        return ptr_saver_certificate;
    }

    ccertificate::type_ptr_X509_REQ ccertificate::_generate_x509_certificate_signing_request(
        const type_ptr_EVP_PKEY& ptr_key
        , const std::wstring& s_common_name
        , const std::wstring& s_organization_name
        , const std::wstring& s_csr_file /*= std::wstring()*/
    )
    {
        type_ptr_X509_REQ ptr_x509_req;

        do {
            if (s_common_name.empty())
                continue;
            if (s_organization_name.empty())
                continue;
            int n_version = 2;//(V3)
            // set version of x509 req
            ptr_x509_req = type_ptr_X509_REQ(X509_REQ_new(), X509_REQ_free);
            if (!ptr_x509_req)
                continue;
            if (X509_REQ_set_version(ptr_x509_req.get(), n_version) != 1) {
                ptr_x509_req.reset();
                continue;
            }

            // set subject of x509 req
            std::string s_cn = cstring::get_mcsc_from_unicode(s_common_name);
            std::string s_organization = cstring::get_mcsc_from_unicode(s_organization_name);

            X509_NAME* p_x509_name = X509_REQ_get_subject_name(ptr_x509_req.get());

            if (X509_NAME_add_entry_by_txt(p_x509_name, "C", MBSTRING_ASC, (const unsigned char*)"KR", -1, -1, 0) != 1) {
                ptr_x509_req.reset();
                continue;
            }
            if (X509_NAME_add_entry_by_txt(p_x509_name, "L", MBSTRING_ASC, (const unsigned char*)"Seoul", -1, -1, 0) != 1) {
                ptr_x509_req.reset();
                continue;
            }
            if (X509_NAME_add_entry_by_txt(p_x509_name, "O", MBSTRING_ASC, (const unsigned char*)s_organization.c_str(), -1, -1, 0) != 1) {
                ptr_x509_req.reset();
                continue;
            }
            if (X509_NAME_add_entry_by_txt(p_x509_name, "CN", MBSTRING_ASC, (const unsigned char*)s_cn.c_str(), -1, -1, 0) != 1) {
                ptr_x509_req.reset();
                continue;
            }

            // set public key of x509 req
            if (X509_REQ_set_pubkey(ptr_x509_req.get(), ptr_key.get()) != 1) {
                ptr_x509_req.reset();
                continue;
            }

            // set sign key of x509 req// return x509_req->signature->length
            if (X509_REQ_sign(ptr_x509_req.get(), ptr_key.get(), EVP_sha256()) <= 0) {
                ptr_x509_req.reset();
                continue;
            }

            if (s_csr_file.empty())
                continue;
            //csr save as file
            std::string s_csr = cstring::get_mcsc_from_unicode(s_csr_file);

            // Open the PEM file for writing the certificate to disk.
#ifdef _WIN32
            errno_t file_err;
            FILE* p_x509_req_file;
            file_err = fopen_s(&p_x509_req_file, s_csr.c_str(), "wb");//pem format. file name *.key
            if (file_err != 0) {
                //error
                if (ccertificate::get_log()) {
                    ccertificate::get_log()->log_fmt(L"[E] - %ls | fopen_s(%d) | %ls.\n", __WFUNCTION__, file_err, s_csr_file.c_str());
                    ccertificate::get_log()->trace(L"[E] - %ls | fopen_s(%d) | %ls.\n", __WFUNCTION__, file_err, s_csr_file.c_str());
                }
                continue;
            }
#else
            FILE* p_x509_req_file = fopen(s_csr.c_str(), "wb");
            if (p_x509_req_file == nullptr) {
                // Handle error
                perror("fopen");
                if (ccertificate::get_log()) {
                    ccertificate::get_log()->log_fmt(L"[E] - %ls | fopen_s() | %ls.\n", __WFUNCTION__, s_csr_file.c_str());
                    ccertificate::get_log()->trace(L"[E] - %ls | fopen_s() | %ls.\n", __WFUNCTION__, s_csr_file.c_str());
                }
                continue;
            }
#endif

            // Write the certificate to disk.
            bool b_return = PEM_write_X509_REQ(p_x509_req_file, ptr_x509_req.get());
            if (!b_return) {
                if (ccertificate::get_log()) {
                    ccertificate::get_log()->log_fmt(L"[E] - %ls | PEM_write_X509_REQ.\n", __WFUNCTION__);
                    ccertificate::get_log()->trace(L"[E] - %ls | PEM_write_X509_REQ.\n", __WFUNCTION__);
                }
            }
            if (p_x509_req_file)
                fclose(p_x509_req_file);

        } while (false);

        return ptr_x509_req;
    }

    //Generates a x509 certificate.
    ccertificate::type_ptr_X509 ccertificate::_generate_x509_certificate(
        const type_ptr_EVP_PKEY& ptr_ca_key
        , const type_ptr_X509& ptr_ca_cert
        , const type_ptr_X509_REQ& ptr_csr
        , long n_validity_year/*=10*/
        , const std::wstring& s_cert_pem_format_file /*= std::wstring()*/
    )
    {
        type_ptr_X509 ptr_x509;

        do {
            if (!ptr_ca_key)
                continue;
            if (!ptr_ca_cert)
                continue;
            if (!ptr_csr)
                continue;
            if (n_validity_year <= 0)
                continue;
            if (n_validity_year > 20)
                continue;
            //
            EVP_PKEY* p_key(nullptr);
            p_key = X509_REQ_get_pubkey(ptr_csr.get());
            //verify signature on the request
            if (X509_REQ_verify(ptr_csr.get(), p_key) != 1)
                continue;

            ptr_x509 = type_ptr_X509(X509_new(), X509_free); //Allocate memory for the X509 structure.
            if (!ptr_x509) {
                ptr_x509.reset();
                continue;
            }
            //
            X509_set_version(ptr_x509.get(), 2);//set version. v3

            // Set the serial number by randum.
            if (!ccertificate::_rand_serial(NULL, X509_get_serialNumber(ptr_x509.get()))) {
                ptr_x509.reset();
                continue;
            }


            // This certificate is valid from now until exactly one year from now.
            X509_gmtime_adj(X509_get_notBefore(ptr_x509.get()), 0);
            X509_gmtime_adj(X509_get_notAfter(ptr_x509.get()), n_validity_year * 365 * 24 * 60 * 60);//vaild range unit sec 31536000 = 365 * 24 * 60 * 60 = 1 year

            // We want to copy the subject name to the issuer name.
            X509_NAME* p_name = X509_REQ_get_subject_name(ptr_csr.get());
            X509_set_subject_name(ptr_x509.get(), p_name);

            p_name = X509_get_subject_name(ptr_ca_cert.get());
            X509_set_issuer_name(ptr_x509.get(), p_name);

            // Set the public key for our certificate.
            X509_set_pubkey(ptr_x509.get(), p_key);

            // add x509v3 extensions as specified 
            if (!ccertificate::_cert_add_ext(ptr_ca_cert.get(), ptr_x509.get(), NID_authority_key_identifier, "keyid:always")) {
                ptr_x509.reset();
                continue;
            }
            if (!ccertificate::_cert_add_ext(ptr_ca_cert.get(), ptr_x509.get(), NID_basic_constraints, "CA:FALSE")) {
                ptr_x509.reset();
                continue;
            }
            if (!ccertificate::_cert_add_ext(ptr_ca_cert.get(), ptr_x509.get(), NID_key_usage, "nonRepudiation, digitalSignature, keyEncipherment, dataEncipherment")) {
                ptr_x509.reset();
                continue;
            }

            if (!ccertificate::_cert_add_ext(ptr_ca_cert.get(), ptr_x509.get(), NID_subject_alt_name, "DNS:localhost,IP:127.0.0.1")) {
                ptr_x509.reset();
                continue;
            }

            // Actually sign the certificate with our key.
            if (!X509_sign(ptr_x509.get(), ptr_ca_key.get(), EVP_sha256())) {
                ptr_x509.reset();
                continue;
            }

            //save as file
            if (s_cert_pem_format_file.empty())
                continue;
            std::string s_cert_pem_format = cstring::get_mcsc_from_unicode(s_cert_pem_format_file);

            // Open the PEM file for writing the certificate to disk.
#ifdef _WIN32
            errno_t file_err;
            FILE* p_x509_file;
            file_err = fopen_s(&p_x509_file, s_cert_pem_format.c_str(), "wb");//pem format. file name *.key
            if (file_err != 0) {
                //error
                if (ccertificate::get_log()) {
                    ccertificate::get_log()->log_fmt(L"[E] - %ls | fopen_s(%d) | %ls.\n", __WFUNCTION__, file_err, s_cert_pem_format_file.c_str());
                    ccertificate::get_log()->trace(L"[E] - %ls | fopen_s(%d) | %ls.\n", __WFUNCTION__, file_err, s_cert_pem_format_file.c_str());
                }
                continue;
            }
#else
            FILE* p_x509_file = fopen(s_cert_pem_format.c_str(), "wb");
            if (p_x509_file == nullptr) {
                // Handle error
                perror("fopen");
                if (ccertificate::get_log()) {
                    ccertificate::get_log()->log_fmt(L"[E] - %ls | fopen_s() | %ls.\n", __WFUNCTION__, s_cert_pem_format_file.c_str());
                    ccertificate::get_log()->trace(L"[E] - %ls | fopen_s() | %ls.\n", __WFUNCTION__, s_cert_pem_format_file.c_str());
                }
                continue;
            }
#endif

            // Write the certificate to disk.
            bool b_return = PEM_write_X509(p_x509_file, ptr_x509.get());
            if (!b_return) {
                if (ccertificate::get_log()) {
                    ccertificate::get_log()->log_fmt(L"[E] - %ls | PEM_write_X509.\n", __WFUNCTION__);
                    ccertificate::get_log()->trace(L"[E] - %ls | PEM_write_X509.\n", __WFUNCTION__);
                }
            }
            if (p_x509_file)
                fclose(p_x509_file);

        } while (false);

        return ptr_x509;
    }
}//the end of _mp namespace