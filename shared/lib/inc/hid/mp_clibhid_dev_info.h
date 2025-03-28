#pragma once

#include <memory>
#include <list>
#include <set>
#include <string>
#include <vector>
#include <utility>

#include <mp_type.h>


namespace _mp{

    class clibhid_dev_info
    {
    public:
        typedef std::shared_ptr<clibhid_dev_info> type_ptr;
        typedef std::set<clibhid_dev_info> type_set;

        static type_set_wstring get_dev_path_by_wstring(const clibhid_dev_info::type_set& in);
        static type_set_string get_dev_path_by_string(const clibhid_dev_info::type_set& in);

        static size_t filter_dev_info_set(
            clibhid_dev_info::type_set& out,
            const clibhid_dev_info::type_set& in,
            const type_tuple_usb_filter t_filter
        );

        static size_t filter_dev_info_set(
            clibhid_dev_info::type_set& out,
            const clibhid_dev_info::type_set& in,
            const type_v_bm_dev& v_type_filter
        );
    public: //constructor & destructor      
        clibhid_dev_info();
        
        /**
        * this construct for lpu237. if device isn't lpu237, s_extra_path is std::string().
        */
        clibhid_dev_info(
            const char* ps_path,
            unsigned short w_vid,
            unsigned short w_pid,
            int n_interface,
            const std::string &s_extra_path
        );

        /**
        * this construct for generic device.
        */
        clibhid_dev_info(
            const char* ps_path,
            unsigned short w_vid,
            unsigned short w_pid,
            int n_interface
        );

        
        virtual ~clibhid_dev_info();

        bool is_support_shared_open() const;
        
		type_bm_dev get_type() const;

        const std::wstring get_path_by_wstring() const;
        const std::string get_path_by_string() const;

        unsigned short get_vendor_id() const;

        unsigned short get_product_id() const;

        int get_interface_number() const;

        int get_size_in_report() const;
        int get_size_out_report() const;

        std::string get_extra_path_by_string() const;
        std::wstring get_extra_path_by_wstring() const;

        bool operator<(const clibhid_dev_info& other) const;

        bool operator==(const clibhid_dev_info& other) const;

        clibhid_dev_info& operator=(const clibhid_dev_info& src);

    protected:
        unsigned short m_w_vid, m_w_pid;
        int m_n_interface;

        int m_n_size_in_report, m_n_size_out_report;//the size is less then equal zero. used not  report 
        //
        _mp::type_v_s_buffer m_vs_path;
        std::string m_s_extra_path;//for virtual device path

        bool m_b_support_shared; // lpu237 ibutton supprts shared open and shared rx of ibutton key.
    };

}//the end of namespace _mp
