
#include <algorithm>

// Headers needed for sleeping.
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include <mp_cstring.h>
#include <mp_elpusk.h>
#include <mp_coperation.h>
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
			clibhid_dev_info::type_set in_temp(in);
            out.clear();

            std::for_each(std::begin(in_temp), std::end(in_temp), [&](const clibhid_dev_info &info) {
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

                _mp::type_bm_dev t(_mp::type_bm_dev_unknown);

                std::tie(std::ignore,t,std::ignore,std::ignore) = _vhid_info::is_path_compositive_type(info.get_path_by_string());
                if (std::find(v_type_filter.begin(), v_type_filter.end(), t) != std::end(v_type_filter)) {
                    //found matched type.
                    out.insert(info);
                }
                });

        } while (false);
        return out.size();
    }

    size_t clibhid_dev_info::filter_dev_info_set(clibhid_dev_info::type_set& out, const clibhid_dev_info::type_set& in, const type_set_wstring& set_wstring_filter)
    {
        do {
            clibhid_dev_info::type_set in_temp(in);

            if (set_wstring_filter.empty()) {
				out = in_temp;//all
                continue;
            }

            out.clear();

			// _mp::type_set_wstring type filter 를 _mp::type_set_usb_filter type filer로 변환.
			_mp::type_set_usb_filter set_usb_filter;
            for(auto& s_item : set_wstring_filter){
                int n_vid = -1, n_pid = -1, n_inf = -1;

                size_t nfound = s_item.find(L"vid_");
                if (nfound == std::wstring::npos) {
                    continue;
                }
                nfound += 4;
                std::wstring sVal = s_item.substr(nfound, 4);
                if (!sVal.empty()) {
                    n_vid = std::wcstol(sVal.c_str(), NULL, 16);
                }
                else {
                    continue;
                }

                //pid
                nfound = s_item.find(L"pid_");
                if (nfound == std::wstring::npos) {
					set_usb_filter.insert(std::make_tuple(n_vid, n_pid, n_inf));//vid only filter
                    continue;
                }
                nfound += 4;
                sVal = s_item.substr(nfound, 4);
                if (!sVal.empty()) {
                    n_pid = std::wcstol(sVal.c_str(), NULL, 16);
                }
                else {
                    set_usb_filter.insert(std::make_tuple(n_vid, n_pid, n_inf));//vid only filter
					continue;
                }

                //inf
                nfound = s_item.find(L"mi_");
                if (nfound == std::wstring::npos) {
					set_usb_filter.insert(std::make_tuple(n_vid, n_pid, n_inf));//vid & pid filter
                    continue;
                }
                nfound += 3;
                sVal = s_item.substr(nfound, 2);
                if (!sVal.empty()) {
                    n_inf = std::wcstol(sVal.c_str(), NULL, 16);
                }
                set_usb_filter.insert(std::make_tuple(n_vid, n_pid, n_inf));//vid & pid & inf or none inf filter

            }//end for

            if(set_usb_filter.empty()){
                out = in_temp;//all
                continue;
			}

            for( auto & t_item : set_usb_filter){
                clibhid_dev_info::type_set st_filtered;
                clibhid_dev_info::filter_dev_info_set(st_filtered, in_temp, t_item);
                out.insert(st_filtered.begin(), st_filtered.end());
			}//end for

        } while (false);
        return out.size();
    }

    clibhid_dev_info::type_set::iterator clibhid_dev_info::find(clibhid_dev_info::type_set& in, const std::wstring& ws_path)
    {
        std::string s_path = _mp::cstring::get_mcsc_from_unicode(ws_path);
        return clibhid_dev_info::find(in,s_path);
    }

    clibhid_dev_info::type_set::iterator clibhid_dev_info::find(clibhid_dev_info::type_set& in, const std::string& s_path)
    {
        clibhid_dev_info::type_set::iterator it = std::begin(in);

        for (; it != std::end(in); ++it) {
            if (it->get_path_by_string() == s_path) {
                break; //found ok
            }
        }//end for
        return it;
    }

    clibhid_dev_info::type_set::iterator clibhid_dev_info::find(clibhid_dev_info::type_set& in, int n_vid, int n_pid)
    {
        clibhid_dev_info::type_set::iterator it = std::begin(in);

        for (; it != std::end(in); ++it) {
            if (it->get_vendor_id() == (unsigned short)n_vid && it->get_product_id() == (unsigned short)n_pid) {
                break; //found ok
            }
        }//end for
        return it;
    }


    clibhid_dev_info::clibhid_dev_info() :
        m_w_vid(0), m_w_pid(0), m_n_interface(-1),
        m_n_size_in_report(0),
        m_n_size_out_report(0),
        m_b_support_shared(false)
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
        m_s_extra_path(s_extra_path),
        m_b_support_shared(false)
    {
        if(ps_path){
            m_vs_path.assign(ps_path, ps_path + strlen(ps_path));
            std::string s_p(ps_path);
            std::wstring sw_p = _mp::cstring::get_unicode_from_mcsc(s_p);
            m_s_pis = _mp::coperation::get_usb_pis_from_path(sw_p);
        }

        //assign data of known device
        if (w_vid == _elpusk::const_usb_vid) {
            if (w_pid == _elpusk::_lpu237::const_usb_pid && n_interface == _elpusk::_lpu237::const_usb_inf_hid) {
                m_n_size_in_report = _elpusk::_lpu237::const_size_report_in_except_id;
                m_n_size_out_report = _elpusk::_lpu237::const_size_report_out_except_id;
                //
                m_b_support_shared = _vhid_info::is_support_shared_open(m_s_extra_path);
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
        
    clibhid_dev_info::clibhid_dev_info(const char* ps_path, unsigned short w_vid, unsigned short w_pid, int n_interface) :
        m_w_vid(w_vid), m_w_pid(w_pid), m_n_interface(n_interface),
        m_n_size_in_report(0),
        m_n_size_out_report(0),
        m_b_support_shared(false)
    {
        if (ps_path) {
            m_vs_path.assign(ps_path, ps_path + strlen(ps_path));
            std::string s_p(ps_path);
            std::wstring sw_p = _mp::cstring::get_unicode_from_mcsc(s_p);
            m_s_pis = _mp::coperation::get_usb_pis_from_path(sw_p);
        }
    }

    clibhid_dev_info::clibhid_dev_info(const clibhid_dev_info& src)
    {
        *this = src;
    }

    clibhid_dev_info::~clibhid_dev_info()
    {
    }

    bool clibhid_dev_info::is_support_shared_open() const
    {
        return m_b_support_shared;
    }

    type_bm_dev clibhid_dev_info::get_type() const
    {
        static const _vhid_info::type_set_path_type _set_extra_paths_lpu237(_vhid_info::get_extra_paths(
            _mp::_elpusk::const_usb_vid,
            _mp::_elpusk::_lpu237::const_usb_pid,
            _mp::_elpusk::_lpu237::const_usb_inf_hid
        ));
        static const _vhid_info::type_set_path_type _set_extra_paths_lpu238(_vhid_info::get_extra_paths(
            _mp::_elpusk::const_usb_vid,
            _mp::_elpusk::_lpu238::const_usb_pid,
            _mp::_elpusk::_lpu238::const_usb_inf_hid
        ));

        type_bm_dev t(_mp::type_bm_dev_unknown);

        do {
			if (m_vs_path.empty()) {
				continue;
			}
            t = _mp::type_bm_dev_hid;

			if (m_s_extra_path.empty()) {
				continue;
			}

            bool b_found(false);
            //check lpu237
			for (auto item : _set_extra_paths_lpu237) {
				if (m_s_extra_path.size() < std::get<0>(item).size()) {
					continue;
				}
				if (std::get<0>(item).size() == 0) {
					continue;
				}
				if (m_s_extra_path.compare(m_s_extra_path.size() - std::get<0>(item).size(), std::get<0>(item).size(), std::get<0>(item)) == 0) {
					t = std::get<1>(item);
                    b_found = true;
					break;
				}
			}//end for
            if (b_found) {
                continue;
            }
            //check lpu238
            for (auto item : _set_extra_paths_lpu238) {
                if (m_s_extra_path.size() < std::get<0>(item).size()) {
                    continue;
                }
                if (std::get<0>(item).size() == 0) {
                    continue;
                }
                if (m_s_extra_path.compare(m_s_extra_path.size() - std::get<0>(item).size(), std::get<0>(item).size(), std::get<0>(item)) == 0) {
                    t = std::get<1>(item);
                    b_found = true;
                    break;
                }
            }//end for

        } while (false);
        return t;
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

    std::wstring clibhid_dev_info::get_port_id_string() const
    {
        return m_s_pis;
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

        this->m_s_pis = src.m_s_pis;
        this->m_b_support_shared = src.m_b_support_shared;
        return *this;
    }

    bool clibhid_dev_info::is_valid() const
    {
        if (m_w_vid == 0 && m_w_pid == 0) {
			return false;
        }
        else {
			return true;
        }

    }


}//the end of namespace _mp
