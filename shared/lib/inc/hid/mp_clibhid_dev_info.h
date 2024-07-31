#pragma once

#include <memory>
#include <list>
#include <set>
#include <string>
#include <vector>
#include <utility>

#include <hidapi.h>

// Headers needed for sleeping.
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

// Fallback/example
#ifndef HID_API_MAKE_VERSION
#define HID_API_MAKE_VERSION(mj, mn, p) (((mj) << 24) | ((mn) << 8) | (p))
#endif
#ifndef HID_API_VERSION
#define HID_API_VERSION HID_API_MAKE_VERSION(HID_API_VERSION_MAJOR, HID_API_VERSION_MINOR, HID_API_VERSION_PATCH)
#endif

//
// Sample using platform-specific headers
#if defined(__APPLE__) && HID_API_VERSION >= HID_API_MAKE_VERSION(0, 12, 0)
#include <hidapi_darwin.h>
#endif

#if defined(_WIN32) && HID_API_VERSION >= HID_API_MAKE_VERSION(0, 12, 0)
#include <hidapi_winapi.h>
#endif

#if defined(USING_HIDAPI_LIBUSB) && HID_API_VERSION >= HID_API_MAKE_VERSION(0, 12, 0)
#include <hidapi_libusb.h>
#endif

#include <mp_type.h>
#include <mp_elpusk.h>

namespace _mp{

    class clibhid_dev_info
    {
    public:
        typedef std::shared_ptr<clibhid_dev_info> type_ptr;
        typedef std::set<clibhid_dev_info> type_set;

        static type_set_wstring get_dev_path_by_wstring(const clibhid_dev_info::type_set& in)
        {
            type_set_wstring s;

            for (const _mp::clibhid_dev_info& item : in) {
                if (item.m_dev_info.path) {
                    s.insert(item.get_path_by_wstring());
                }
            }//end for
            return s;
        }
        static type_set_string get_dev_path_by_string(const clibhid_dev_info::type_set& in)
        {
            type_set_string s;

            for (const _mp::clibhid_dev_info& item : in) {
                if (item.m_dev_info.path) {
                    s.insert(item.get_path_by_string());
                }
            }//end for
            return s;
        }

        static clibhid_dev_info::type_set get_dev_info_set(struct hid_device_info *p_dev_info)
        {
            clibhid_dev_info::type_set dev_info_set;
            while (p_dev_info)
            {
                clibhid_dev_info::type_ptr dev_info = std::make_shared<clibhid_dev_info>(p_dev_info);
                dev_info_set.insert(*dev_info);
                p_dev_info = p_dev_info->next;
            }
            return dev_info_set;
        }

        static size_t filter_dev_info_set(
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
    public: //constructor & destructor      
        clibhid_dev_info() : m_dev_info({0,}), m_n_size_in_report(0), m_n_size_out_report(0)
        {

        }

        clibhid_dev_info(struct hid_device_info *p_dev_info) : m_dev_info({0,}), m_n_size_in_report(0), m_n_size_out_report(0)
        {
            if(p_dev_info){
                m_dev_info = *p_dev_info;
                if(p_dev_info->path){
                    m_vs_path.assign(p_dev_info->path, p_dev_info->path + strlen(p_dev_info->path));
                }

                if(p_dev_info->serial_number){
                    m_vws_serial_number.assign(p_dev_info->serial_number, p_dev_info->serial_number + wcslen(p_dev_info->serial_number));
                }

                if(p_dev_info->manufacturer_string){
                    m_vws_manufacturer_string.assign(p_dev_info->manufacturer_string, p_dev_info->manufacturer_string + wcslen(p_dev_info->manufacturer_string));
                }

                if(p_dev_info->product_string){
                    m_vws_product_string.assign(p_dev_info->product_string, p_dev_info->product_string + wcslen(p_dev_info->product_string));
                }
                //assign data of known device
                if (p_dev_info->vendor_id == _elpusk::const_usb_vid) {
                    if (p_dev_info->product_id == _elpusk::_lpu237::const_usb_pid && p_dev_info->interface_number == _elpusk::_lpu237::const_usb_inf_hid) {
                        m_n_size_in_report = _elpusk::_lpu237::const_size_report_in_except_id;
                        m_n_size_out_report = _elpusk::_lpu237::const_size_report_out_except_id;
                    }
                    else if (p_dev_info->product_id == _elpusk::_lpu238::const_usb_pid && p_dev_info->interface_number == _elpusk::_lpu238::const_usb_inf_hid) {
                        m_n_size_in_report = _elpusk::_lpu238::const_size_report_in_except_id;
                        m_n_size_out_report = _elpusk::_lpu238::const_size_report_out_except_id;
                    }
                    else if (p_dev_info->product_id == _elpusk::const_usb_pid_hidbl) {
                        m_n_size_in_report = _elpusk::const_size_hidbl_report_in_except_id;
                        m_n_size_out_report = _elpusk::const_size_hidbl_report_out_except_id;
                    }
                }
            }
        }

        virtual ~clibhid_dev_info()
        {
        }
        
        const std::wstring get_path_by_wstring() const
        {
            return cstring::get_unicode_from_mcsc(get_path_by_string());
        }
        const std::string get_path_by_string() const
        {
            return std::string(m_vs_path.begin(), m_vs_path.end());
        }

        const std::wstring get_serial_number() const
        {
            return std::wstring(m_vws_serial_number.begin(), m_vws_serial_number.end());
        }

        const std::wstring get_manufacturer_string() const
        {
            return std::wstring(m_vws_manufacturer_string.begin(), m_vws_manufacturer_string.end());
        }

        unsigned short get_vendor_id() const
        {
            return m_dev_info.vendor_id;
        }

        unsigned short get_product_id() const
        {
            return m_dev_info.product_id;
        }

        unsigned short get_release_number() const
        {
            return m_dev_info.release_number;
        }

        int get_interface_number() const
        {
            return m_dev_info.interface_number;
        }

        hid_bus_type get_bus_type() const
        {
            return m_dev_info.bus_type;
        }

        std::wstring get_bus_type_by_string() const
        {
            std::wstring s;
            switch(m_dev_info.bus_type){
                case HID_API_BUS_USB: s = L"USB"; break;
                case HID_API_BUS_BLUETOOTH: s = L"Bluetooth"; break;
                case HID_API_BUS_I2C: s = L"I2C"; break;
                case HID_API_BUS_SPI: s = L"SPI"; break;
                case HID_API_BUS_UNKNOWN:
                default: s = L"Unknown"; break;
            }
            return s;
        }

        int get_size_in_report() const
        {
            return m_n_size_in_report;
        }
        int get_size_out_report() const
        {
            return m_n_size_out_report;
        }

        bool operator<(const clibhid_dev_info& other) const 
        {
            return std::lexicographical_compare(m_vs_path.begin(), m_vs_path.end(),
                other.m_vs_path.begin(), other.m_vs_path.end());
        }

        bool operator==(const clibhid_dev_info& other) const
        {
            bool b_equal(false);

            do {
                if (this->m_vs_path.size() != other.m_vs_path.size()) {
                    continue;
                }

                b_equal = true;

                for (auto i = 0; i < this->m_vs_path.size(); i++) {
                    if (this->m_vs_path[i] != other.m_vs_path[i]) {
                        b_equal = false;
                        break;
                    }
                }//end for

            } while (false);
            return b_equal;
        }
    protected:
        struct hid_device_info m_dev_info;
        int m_n_size_in_report, m_n_size_out_report;//the size is less then equal zero. used not  report 
        //
        _mp::type_v_s_buffer m_vs_path;
        _mp::type_v_ws_buffer m_vws_serial_number;
        _mp::type_v_ws_buffer m_vws_manufacturer_string;
        _mp::type_v_ws_buffer m_vws_product_string;
    };

}//the end of namespace _mp
