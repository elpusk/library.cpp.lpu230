
#include <algorithm>

// Headers needed for sleeping.
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include <mp_cstring.h>
#include <mp_elpusk.h>
#include <hid/_vhid_info.h>
#include <hid/mp_clibhid_dev_info.h>

namespace _mp{
    type_set_wstring clibhid_dev_info::get_dev_path_by_wstring(const clibhid_dev_info::type_set& in)
    {
        type_set_wstring s;

        for (const _mp::clibhid_dev_info& item : in) {
            if (!item.m_vs_path.empty()) {
                s.insert(item.get_path_by_wstring());
            }
        }//end for
        return s;
    }
    type_set_string clibhid_dev_info::get_dev_path_by_string(const clibhid_dev_info::type_set& in)
    {
        type_set_string s;

        for (const _mp::clibhid_dev_info& item : in) {
            if (!item.m_vs_path.empty()) {
                s.insert(item.get_path_by_string());
            }
        }//end for
        return s;
    }

    size_t clibhid_dev_info::filter_dev_info_set(
        clibhid_dev_info::type_set& out,
        const clibhid_dev_info::type_set& in, 
        const type_tuple_usb_filter t_filter
    )
    {
        do {
            out.clear();

            std::for_each(std::begin(in), std::end(in), [&](const clibhid_dev_info &info) {
                int fvid, fpid, finf;
                int n_vid, n_pid, n_inf;
                std::tie(n_vid, n_pid, n_inf) = t_filter;

                do {
                    if (n_vid < 0) {
                        out.insert(info);//all 
                        continue;
                    }

                    fvid = info.get_vendor_id();
                    if (fvid != n_vid)
                        continue;

                    if (n_pid < 0) {
                        out.insert(info);//all pid & all inf
                        continue;
                    }
                    fpid = info.get_product_id();
                    if (fpid != n_pid)
                        continue;
                    if (n_inf < 0) {
                        out.insert(info);// all inf
                        continue;
                    }

                    finf = info.get_interface_number();
                    if (finf == n_inf)
                        out.insert(info);
                    //
                } while (false);
            });

        } while (false);
        return out.size();
    }
    size_t clibhid_dev_info::filter_dev_info_set(clibhid_dev_info::type_set& out, const clibhid_dev_info::type_set& in, const type_v_bm_dev& v_type_filter)
    {
        do {
            clibhid_dev_info::type_set in_temp(in);
            out.clear();

            std::for_each(std::begin(in_temp), std::end(in_temp), [&](const clibhid_dev_info& info) {

                bool b_composite(false);
                _mp::type_bm_dev t(_mp::type_bm_dev_unknown);

                std::tie(b_composite,t,std::ignore,std::ignore) = _vhid_info::is_path_compositive_type(info.get_path_by_string());
                if (std::find(v_type_filter.begin(), v_type_filter.end(), t) != std::end(v_type_filter)) {
                    //found matched type.
                    out.insert(info);
                }
                });

        } while (false);
        return out.size();
    }

    clibhid_dev_info::clibhid_dev_info() :
        m_w_vid(0), m_w_pid(0), m_n_interface(-1),
        m_n_size_in_report(0),
        m_n_size_out_report(0)
    {

    }
        
    clibhid_dev_info::clibhid_dev_info(
        const char *ps_path,
        unsigned short w_vid,
        unsigned short w_pid,
        int n_interface,
        const std::string & s_extra_path
    ) :
        m_w_vid(w_vid), m_w_pid(w_pid), m_n_interface(n_interface),
        m_n_size_in_report(0),
        m_n_size_out_report(0),
        m_s_extra_path(s_extra_path)
    {
        if(ps_path){
            m_vs_path.assign(ps_path, ps_path + strlen(ps_path));
        }

        //assign data of known device
        if (w_vid == _elpusk::const_usb_vid) {
            if (w_pid == _elpusk::_lpu237::const_usb_pid && n_interface == _elpusk::_lpu237::const_usb_inf_hid) {
                m_n_size_in_report = _elpusk::_lpu237::const_size_report_in_except_id;
                m_n_size_out_report = _elpusk::_lpu237::const_size_report_out_except_id;
            }
            else if (w_pid == _elpusk::_lpu238::const_usb_pid && n_interface == _elpusk::_lpu238::const_usb_inf_hid) {
                m_n_size_in_report = _elpusk::_lpu238::const_size_report_in_except_id;
                m_n_size_out_report = _elpusk::_lpu238::const_size_report_out_except_id;
            }
            else if (w_pid == _elpusk::const_usb_pid_hidbl) {
                m_n_size_in_report = _elpusk::const_size_hidbl_report_in_except_id;
                m_n_size_out_report = _elpusk::const_size_hidbl_report_out_except_id;
            }
        }
    }
        
    clibhid_dev_info::~clibhid_dev_info()
    {
    }
        
    const std::wstring clibhid_dev_info::get_path_by_wstring() const
    {
        return cstring::get_unicode_from_mcsc(get_path_by_string());
    }
    const std::string clibhid_dev_info::get_path_by_string() const
    {
        if( m_s_extra_path.empty() )
            return std::string(m_vs_path.begin(), m_vs_path.end());
        else {
            std::string s(m_vs_path.begin(), m_vs_path.end());
            s = s + m_s_extra_path;
            return s;
        }
    }

    unsigned short clibhid_dev_info::get_vendor_id() const
    {
        return m_w_vid;
    }

    unsigned short clibhid_dev_info::get_product_id() const
    {
        return m_w_pid;
    }

    int clibhid_dev_info::get_interface_number() const
    {
        return m_n_interface;
    }

    int clibhid_dev_info::get_size_in_report() const
    {
        return m_n_size_in_report;
    }
    int clibhid_dev_info::get_size_out_report() const
    {
        return m_n_size_out_report;
    }

    std::string clibhid_dev_info::get_extra_path_by_string() const
    {
        return m_s_extra_path;
    }
    std::wstring clibhid_dev_info::get_extra_path_by_wstring() const
    {
        return cstring::get_unicode_from_mcsc(get_extra_path_by_string());
    }

    bool clibhid_dev_info::operator<(const clibhid_dev_info& other) const
    {
        auto v_this(m_vs_path);
        v_this.insert(v_this.end(), m_s_extra_path.begin(), m_s_extra_path.end());

        auto v_other(other.m_vs_path);
        v_other.insert(v_other.end(), other.m_s_extra_path.begin(), other.m_s_extra_path.end());

        return std::lexicographical_compare(v_this.begin(), v_this.end(),
            v_other.begin(), v_other.end());

    }

    bool clibhid_dev_info::operator==(const clibhid_dev_info& other) const
    {
        bool b_equal(false);

        do {
            if (this->m_vs_path.size() != other.m_vs_path.size()) {
                continue;
            }
            if (this->m_s_extra_path.size() != other.m_s_extra_path.size()) {
                continue;
            }

            b_equal = true;

            for (auto i = 0; i < this->m_vs_path.size(); i++) {
                if (this->m_vs_path[i] != other.m_vs_path[i]) {
                    b_equal = false;
                    break;
                }
            }//end for

            if (b_equal && !m_s_extra_path.empty()) {
                for (auto i = 0; i < this->m_s_extra_path.size(); i++) {
                    if (this->m_s_extra_path[i] != other.m_s_extra_path[i]) {
                        b_equal = false;
                        break;
                    }
                }//end for
            }

        } while (false);
        return b_equal;
    }

    clibhid_dev_info& clibhid_dev_info::operator=(const clibhid_dev_info& src)
    {
        if (this == &src)  // prevent self-assignment
            return *this;
        //
        this->m_w_vid = src.m_w_vid;
        this->m_w_pid = src.m_w_pid;
        this->m_n_interface = src.m_n_interface;

        this->m_n_size_in_report = src.m_n_size_in_report;
        this->m_n_size_out_report = src.m_n_size_out_report;
        this->m_vs_path = src.m_vs_path;
        this->m_s_extra_path = src.m_s_extra_path;
        return *this;
    }


}//the end of namespace _mp
