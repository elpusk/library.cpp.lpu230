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
    public: //constructor & destructor      
        clibhid_dev_info();
        
        clibhid_dev_info(const char* ps_path, unsigned short w_vid, unsigned short w_pid, int n_interface);
        
        virtual ~clibhid_dev_info();
        
        const std::wstring get_path_by_wstring() const;
        const std::string get_path_by_string() const;

        unsigned short get_vendor_id() const;

        unsigned short get_product_id() const;

        int get_interface_number() const;

        int get_size_in_report() const;
        int get_size_out_report() const;

        bool operator<(const clibhid_dev_info& other) const;

        bool operator==(const clibhid_dev_info& other) const;

        clibhid_dev_info& operator=(const clibhid_dev_info& src);

    protected:
        unsigned short m_w_vid, m_w_pid;
        int m_n_interface;

        int m_n_size_in_report, m_n_size_out_report;//the size is less then equal zero. used not  report 
        //
        _mp::type_v_s_buffer m_vs_path;
    };

}//the end of namespace _mp
