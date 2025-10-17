
#pragma once
#include <mp_cversion.h>

#include <lpu230/info_sys.h>

/**
 * The function of this class is
 * to check the supported features in the current lpu237 based on the firmware version and system name.
 * 
 * all inline code.
 */
class cfun_lpu237
{
public:
    typedef _mp::cversion<unsigned char>    type_version;

public:    
    cfun_lpu237() : m_version(0,0,0,0)
    {}
    cfun_lpu237(const std::wstring & s_name,const type_version& version) : m_s_name(s_name), m_version(version)
    {}

    cfun_lpu237(const _mp::type_v_buffer& v_name, const type_version& version) : m_version(version)
    {
        for (auto item : v_name) {
            if(item == ' ' || item == 0) {
                break;
			}
			m_s_name.push_back(static_cast<wchar_t>(item));
        }//end for
    }

    cfun_lpu237(const std::wstring & s_name,unsigned char major, unsigned char minor, unsigned char revision, unsigned char build)
        : m_s_name(s_name),m_version(major, minor, revision, build)
    {}
    cfun_lpu237(const cfun_lpu237& other) : m_s_name(other.m_s_name),m_version(other.m_version)
    {}

    cfun_lpu237& operator=(const cfun_lpu237& other)
    {
        if (this != &other) {
            m_s_name = other.m_s_name;
            m_version = other.m_version;
        }
        return *this;
    }
    virtual ~cfun_lpu237() {}
    const type_version& get_version() const { return m_version; }
    void set_version(const type_version& version) { m_version = version; }
    void set_version(unsigned char major, unsigned char minor, unsigned char revision, unsigned char build)
    {
        m_version = type_version(major, minor, revision, build);
    }

    const std::wstring& get_name() const { return m_s_name; }
    void set_name(const std::wstring& s_name) { m_s_name = s_name; }
    void set_name(const char* ps_name,size_t n_name)
    {
        do{
            if (ps_name == NULL || n_name == 0)
                continue;
            if (n_name > SYSTEM_SIZE_NAME){
                continue;
            }
            m_s_name.clear();

			int n_offset = 0;

            while(ps_name[n_offset] !=NULL && ps_name[n_offset] !=' ' && n_name > 0) {
                m_s_name.push_back(static_cast<wchar_t>(ps_name[n_offset]));
                ++n_offset;
                n_name--;
            }
        }while(false);
    }

    bool empty() const
    {
        if( m_s_name.empty() ){
            return true;
        }
        if( m_version.empty() ){
            return true;
        }
        return false;
    }

    void reset()
    {
        m_s_name.clear();
        m_version.reset();
    }

    // supports functions

    /**
     * @brief support or not hidboot loader.abort
     * @return true - support. false - not support
     */
    bool is_support_hid_boot() const
    {
        bool b_support(true);
        do{
            std::wstring s_n;
            cfun_lpu237::type_version v;
            //
            s_n = L"callisto";
            v = cfun_lpu237::type_version(2,2,0,4);
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"ganymede";
            v = cfun_lpu237::type_version(4,2,1,4);
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"himalia";
            v = cfun_lpu237::type_version(1,0,0,0);
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"europa";
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"elara";
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            b_support = false;
        }while(false);
        return b_support;
    }

    bool is_support_keymap_download() const
    {
        bool b_support(true);
        do{
            std::wstring s_n;
            cfun_lpu237::type_version v;
            //
            s_n = L"callisto";
            v = cfun_lpu237::type_version(3,4,0,1);
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"ganymede";
            v = cfun_lpu237::type_version(4,2,1,4);
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"himalia";
            v = cfun_lpu237::type_version(1,0,0,0);
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"europa";
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"elara";
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            b_support = false;
        }while(false);
        return b_support;
    }

    bool is_support_device_uid() const
    {
        bool b_support(true);
        do{
            std::wstring s_n;
            cfun_lpu237::type_version v;
            //
            s_n = L"callisto";
            v = cfun_lpu237::type_version(3,7,0,4);
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"ganymede";
            v = cfun_lpu237::type_version(4,2,1,4);
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"himalia";
            v = cfun_lpu237::type_version(1,0,0,0);
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"europa";
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"elara";
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            b_support = false;
        }while(false);
        return b_support;
    }

    bool is_support_uart_prepostfix() const
    {
        bool b_support(true);
        do{
            std::wstring s_n;
            cfun_lpu237::type_version v;
            //
            s_n = L"callisto";
            v = cfun_lpu237::type_version(3,3,0,2);
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"ganymede";
            v = cfun_lpu237::type_version(4,2,1,4);
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"himalia";
            v = cfun_lpu237::type_version(1,0,0,0);
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"europa";
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"elara";
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            b_support = false;
        }while(false);
        return b_support;
    }

    bool is_support_vcom_port() const
    {
        bool b_support(true);
        do{
            std::wstring s_n;
            cfun_lpu237::type_version v;
            //
            s_n = L"europa";
            v = cfun_lpu237::type_version(1,0,0,0);
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"elara";
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            b_support = false;
        }while(false);
        return b_support;
    }   

    /////////////////////////////////////////////
    // is_suuport_msr_x() serise functions
    ////////////////////////////////////////////

    bool is_suport_msr_magtek() const
    {
        bool b_support(true);

        do{
            std::wstring s_n;
            cfun_lpu237::type_version v;
            //
            s_n = L"callisto";
            v = cfun_lpu237::type_version(1,0,0,0);
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"ganymede";
            v = cfun_lpu237::type_version(5,18,0,0);
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            b_support = false;
        }while(false);
        return b_support;
    }

    bool is_suport_msr_mmd1100() const
    {
        bool b_support(true);

        do{
            std::wstring s_n;
            cfun_lpu237::type_version v,v1;
            //
            s_n = L"ganymede";
            v = cfun_lpu237::type_version(5,0,0,0);
            v1 = cfun_lpu237::type_version(5,12,0,0);
            if(m_s_name.compare(s_n) == 0 && m_version >= v && m_version <= v1){
                continue;
            }
            //
            s_n = L"europa";
            v = cfun_lpu237::type_version(1,0,0,0);
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            b_support = false;
        }while(false);
        return b_support;
    }

    bool is_suport_msr_mh1902t() const
    {
        bool b_support(true);

        do{
            std::wstring s_n;
            cfun_lpu237::type_version v;
            //
            s_n = L"himalia";
            v = cfun_lpu237::type_version(1,0,0,0);
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"elara";
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            b_support = false;
        }while(false);
        return b_support;
    }

    bool is_suport_msr_mmd1100_reset() const
    {
        bool b_support(true);

        do{
            std::wstring s_n;
            cfun_lpu237::type_version v;
            //
            s_n = L"ganymede";
            v = cfun_lpu237::type_version(5,16,0,0);
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"europa";
            v = cfun_lpu237::type_version(1,0,0,0);
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            b_support = false;
        }while(false);
        return b_support;
    }

    bool is_suport_msr_mmd1100_iso_mode() const
    {
        bool b_support(true);

        do{
            std::wstring s_n;
            cfun_lpu237::type_version v;
            //
            s_n = L"ganymede";
            v = cfun_lpu237::type_version(5,22,0,1);
            if(m_s_name.compare(s_n) == 0 && m_version == v){
                continue;
            }
            //
            b_support = false;
        }while(false);
        return b_support;
    }

    bool is_support_msr_hid_interface() const
    {
        bool b_support(true);
        do{
            std::wstring s_n;
            cfun_lpu237::type_version v;
            //
            s_n = L"callisto";
            v = cfun_lpu237::type_version(3,9,0,0);
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"ganymede";
            v = cfun_lpu237::type_version(4,5,0,0);
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"himalia";
            v = cfun_lpu237::type_version(1,0,0,0);
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"europa";
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"elara";
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            b_support = false;
        }while(false);
        return b_support;
    }    

    bool is_support_msr_keyboard_interface() const
    {
        bool b_support(true);
        do{
            std::wstring s_n;
            cfun_lpu237::type_version v;
            //
            s_n = L"callisto";
            v = cfun_lpu237::type_version(1,0,0,0);
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"ganymede";
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"himalia";
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            b_support = false;
        }while(false);
        return b_support;
    }

    bool is_support_msr_iso2_max_size_38() const
    {
        bool b_support(true);
        do{
            std::wstring s_n;
            cfun_lpu237::type_version v;
            //
            s_n = L"callisto";
            v = cfun_lpu237::type_version(3,10,0,0);
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"ganymede";
            v = cfun_lpu237::type_version(5,2,0,0);
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"himalia";
            v = cfun_lpu237::type_version(1,0,0,0);
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"europa";
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"elara";
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            b_support = false;
        }while(false);
        return b_support;
    } 
    
    bool is_support_msr_global_tag() const
    {
        bool b_support(true);
        do{
            std::wstring s_n;
            cfun_lpu237::type_version v;
            //
            s_n = L"callisto";
            v = cfun_lpu237::type_version(3,11,0,0);
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"ganymede";
            v = cfun_lpu237::type_version(5,3,0,0);
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"himalia";
            v = cfun_lpu237::type_version(1,0,0,0);
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"europa";
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"elara";
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            b_support = false;
        }while(false);
        return b_support;
    }    

    bool is_support_msr_global_tag_send_condition() const
    {
        return is_support_msr_global_tag();
    }

    bool is_support_msr_combination() const
    {
        bool b_support(true);
        do{
            std::wstring s_n;
            cfun_lpu237::type_version v;
            //
            s_n = L"callisto";
            v = cfun_lpu237::type_version(3,21,0,0);
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"ganymede";
            v = cfun_lpu237::type_version(5,13,0,0);
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"himalia";
            v = cfun_lpu237::type_version(1,0,0,0);
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"europa";
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"elara";
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            b_support = false;
        }while(false);
        return b_support;
    }

    bool is_support_msr_success_indication_condition() const
    {
        return is_support_msr_combination();
    }

    bool is_support_msr_remove_dupricated_iso2() const
    {
        bool b_support(true);
        do{
            std::wstring s_n;
            cfun_lpu237::type_version v;
            //
            s_n = L"callisto";
            v = cfun_lpu237::type_version(3,21,0,0);
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"ganymede";
            v = cfun_lpu237::type_version(5,14,0,0);
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"himalia";
            v = cfun_lpu237::type_version(1,0,0,0);
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"europa";
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"elara";
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            b_support = false;
        }while(false);
        return b_support;
    }

    bool is_support_msr_woori_security_company_passbook() const
    {
        return is_support_msr_remove_dupricated_iso2();
    }

    bool is_support_msr_read_direction() const
    {
        bool b_support(true);
        do{
            std::wstring s_n;
            cfun_lpu237::type_version v;
            //
            s_n = L"callisto";
            v = cfun_lpu237::type_version(3,21,0,0);
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"ganymede";
            v = cfun_lpu237::type_version(5,17,0,0);
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"himalia";
            v = cfun_lpu237::type_version(1,0,0,0);
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"europa";
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"elara";
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            b_support = false;
        }while(false);
        return b_support;
    }

    bool is_support_msr_encryption() const
    {
        bool b_support(true);
        do{
            std::wstring s_n;
            cfun_lpu237::type_version v;
            //
            s_n = L"himalia";
            v = cfun_lpu237::type_version(2,0,0,0);
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            b_support = false;
        }while(false);
        return b_support;
    }

    bool is_support_msr_stx_etx_lrc_only_is_error() const
    {
        bool b_support(true);
        do{
            std::wstring s_n;
            cfun_lpu237::type_version v;
            //
            s_n = L"callisto";
            v = cfun_lpu237::type_version(3,25,0,0);
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"ganymede";
            v = cfun_lpu237::type_version(5,25,0,0);
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"himalia";
            v = cfun_lpu237::type_version(2,5,0,0);
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"europa";
            v = cfun_lpu237::type_version(1,3,0,0);
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            b_support = false;
        }while(false);
        return b_support;
    }

    bool is_support_ibutton() const
    {
        bool b_support(true);
        do{
            std::wstring s_n;
            cfun_lpu237::type_version v;
            //
            s_n = L"callisto";
            v = cfun_lpu237::type_version(3,3,0,2);
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"ganymede";
            v = cfun_lpu237::type_version(4,2,1,4);
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"himalia";
            v = cfun_lpu237::type_version(1,0,0,0);
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"europa";
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"elara";
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            b_support = false;
        }while(false);
        return b_support;
    }

    bool is_support_ibutton_only_device_command() const
    {
        cfun_lpu237::type_version v = cfun_lpu237::type_version(3, 6, 0, 4);
        if (v < m_version) {
            return true;
        }
        return false;
	}

    bool is_support_ibutton_in_kb_interface_prepost_tag() const
    {
        return is_support_ibutton();
    }

    bool is_support_ibutton_in_kb_interface_remove_code_zero_16times() const
    {
        bool b_support(true);
        do{
            std::wstring s_n;
            cfun_lpu237::type_version v;
            //
            s_n = L"callisto";
            v = cfun_lpu237::type_version(3,4,0,1);
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"ganymede";
            v = cfun_lpu237::type_version(4,2,1,4);
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"himalia";
            v = cfun_lpu237::type_version(1,0,0,0);
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"europa";
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"elara";
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            b_support = false;
        }while(false);
        return b_support;
    }    

    bool is_support_ibutton_hid_interface() const
    {
        bool b_support(true);
        do{
            std::wstring s_n;
            cfun_lpu237::type_version v;
            //
            s_n = L"callisto";
            v = cfun_lpu237::type_version(3,12,0,0);
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"ganymede";
            v = cfun_lpu237::type_version(5,4,0,0);
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"himalia";
            v = cfun_lpu237::type_version(1,0,0,0);
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"europa";
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"elara";
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            b_support = false;
        }while(false);
        return b_support;
    }    

    bool is_support_ibutton_in_kb_interface_remove_code_f12() const
    {
        bool b_support(true);
        do{
            std::wstring s_n;
            cfun_lpu237::type_version v;
            //
            s_n = L"callisto";
            v = cfun_lpu237::type_version(3,13,0,0);
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"ganymede";
            v = cfun_lpu237::type_version(5,5,0,0);
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"himalia";
            v = cfun_lpu237::type_version(1,0,0,0);
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"europa";
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"elara";
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            b_support = false;
        }while(false);
        return b_support;
    }    

    bool is_support_ibutton_in_kb_interface_remove_code_none() const
    {
        bool b_support(true);
        do{
            std::wstring s_n;
            cfun_lpu237::type_version v;
            //
            s_n = L"callisto";
            v = cfun_lpu237::type_version(3,14,0,0);
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"ganymede";
            v = cfun_lpu237::type_version(5,6,0,0);
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"himalia";
            v = cfun_lpu237::type_version(1,0,0,0);
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"europa";
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"elara";
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            b_support = false;
        }while(false);
        return b_support;
    }    

    bool is_support_ibutton_in_kb_interface_remove_code_zero_7times() const
    {
        bool b_support(true);
        do{
            std::wstring s_n;
            cfun_lpu237::type_version v;
            //
            s_n = L"callisto";
            v = cfun_lpu237::type_version(3,16,0,0);
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"ganymede";
            v = cfun_lpu237::type_version(5,8,0,0);
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"himalia";
            v = cfun_lpu237::type_version(1,0,0,0);
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"europa";
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"elara";
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            b_support = false;
        }while(false);
        return b_support;
    }    

    bool is_support_ibutton_codestick() const
    {
        bool b_support(true);
        do{
            std::wstring s_n;
            cfun_lpu237::type_version v;
            //
            s_n = L"callisto";
            v = cfun_lpu237::type_version(3,17,0,0);
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"ganymede";
            v = cfun_lpu237::type_version(5,9,0,0);
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"himalia";
            v = cfun_lpu237::type_version(1,0,0,0);
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"europa";
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"elara";
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            b_support = false;
        }while(false);
        return b_support;
    }        

    bool is_support_ibutton_in_kb_interface_remove_code_user_define() const
    {
        bool b_support(true);
        do{
            std::wstring s_n;
            cfun_lpu237::type_version v;
            //
            s_n = L"callisto";
            v = cfun_lpu237::type_version(3,22,0,0);
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"ganymede";
            v = cfun_lpu237::type_version(5,21,0,0);
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"himalia";
            v = cfun_lpu237::type_version(1,0,0,0);
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"europa";
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"elara";
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            b_support = false;
        }while(false);
        return b_support;
    }

    bool is_support_ibutton_in_kb_interface_remove_prepost_tag() const
    {
        return is_support_ibutton_in_kb_interface_remove_code_user_define();
    }

    bool is_support_ibutton_in_kb_interface_code_range() const
    {
        bool b_support(true);
        do{
            std::wstring s_n;
            cfun_lpu237::type_version v;
            //
            s_n = L"callisto";
            v = cfun_lpu237::type_version(3,23,0,0);
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"ganymede";
            v = cfun_lpu237::type_version(5,22,0,0);
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"himalia";
            v = cfun_lpu237::type_version(1,0,0,0);
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"europa";
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"elara";
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            b_support = false;
        }while(false);
        return b_support;
    }    

    bool is_support_ibutton_read_start_stop() const
    {
        bool b_support(true);
        do{
            std::wstring s_n;
            cfun_lpu237::type_version v;
            //
            s_n = L"callisto";
            v = cfun_lpu237::type_version(3,24,0,0);
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"ganymede";
            v = cfun_lpu237::type_version(5,23,0,0);
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"himalia";
            v = cfun_lpu237::type_version(2,4,0,0);
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            s_n = L"europa";
            v = cfun_lpu237::type_version(1,2,0,0);
            if(m_s_name.compare(s_n) == 0 && m_version >= v){
                continue;
            }
            //
            b_support = false;
        }while(false);
        return b_support;
    }      

private:
    cfun_lpu237::type_version m_version; // system version
    std::wstring m_s_name; // system name, space is not allowed, max size is SYSTEM_SIZE_NAME
};