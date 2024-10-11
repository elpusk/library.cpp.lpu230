#pragma once

#include <memory>
#include <mutex>
#include <map>
#include <mutex>
#include <atomic>
#include <functional>

#include <mp_clog.h>
#include <mp_elpusk.h>
#include <mp_coperation.h>
#include <hid/mp_clibhid_dev.h>

namespace _mp{

    /**
    * this class job.
    * single tone type
    * 1. create & remove device.
    * 2. detect plug in/out device.
    */
    class clibhid
    {
    public:
        typedef std::shared_ptr< clibhid > type_ptr;

        typedef std::pair< clibhid_dev::type_ptr, clibhid_dev_info::type_ptr> pair_ptrs;
        typedef std::map< std::string, clibhid::pair_ptrs > type_map_ptrs;

        /**
        * callback funtion prototype
        * Don't use the member function in this callack function. If use, deadlock!! 
        * @parameter 1'st - inserted devices. 2'nd - removed devices. 3'rd - current device. 4'th - user parameter for callack
        */
        typedef	std::function<void(const clibhid_dev_info::type_set&, const clibhid_dev_info::type_set&, const clibhid_dev_info::type_set&, void*)>	type_callback_pluginout;

    private:
        enum {
            _const_dev_pluginout_check_interval_mmsec = 30
        };
    public:
        static clibhid& get_instance()
        {
            static clibhid obj;
            return obj;
        }

        ~clibhid()
        {
            m_map_ptrs.clear();

            if(m_b_ini){
                if (m_ptr_th_pluginout) {
                    m_b_run_th_pluginout = false;
                    if (m_ptr_th_pluginout->joinable()) {
                        m_ptr_th_pluginout->join();
                    }
                }
                hid_exit();
            }

            m_ptr_usb_lib.reset();
        }

        bool is_ini() const
        {
            return m_b_ini;
        }

        void set_callback_pluginout(clibhid::type_callback_pluginout cb, void *p_user)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_cb = cb;
            m_p_user = p_user;
        }

        clibhid_dev_info::type_set get_cur_device_set()
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_set_cur_dev_info;
        }

        clibhid_dev::type_wptr get_device(const clibhid_dev_info & dev_info)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            return _get_device(dev_info);
        }
        clibhid_dev::type_wptr get_device(const std::string& s_path)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            return _get_device(s_path);
        }
        clibhid_dev::type_wptr get_device(const std::wstring& sw_path)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            return _get_device(sw_path);
        }

    private:
        clibhid() : m_b_ini(false), m_b_run_th_pluginout(false), m_p_user(NULL), m_cb(nullptr)
        {
            //setup usb filter
            m_set_usb_filter.emplace(_elpusk::const_usb_vid, _elpusk::_lpu237::const_usb_pid, _elpusk::_lpu237::const_usb_inf_hid);
            m_set_usb_filter.emplace(_elpusk::const_usb_vid, _elpusk::_lpu238::const_usb_pid, _elpusk::_lpu238::const_usb_inf_hid);
            m_set_usb_filter.emplace(_elpusk::const_usb_vid, _elpusk::const_usb_pid_hidbl, _elpusk::const_usb_inf_hidbl);

            m_ptr_usb_lib = std::make_shared<clibusb>();

            m_ptr_mutex_hidapi = std::make_shared<std::mutex>();

            if (hid_init() == 0) {
                m_b_ini = true;

#if defined(__APPLE__) && HID_API_VERSION >= HID_API_MAKE_VERSION(0, 12, 0)
                // To work properly needs to be called before hid_open/hid_open_path after hid_init.
                // Best/recommended option - call it right after hid_init.
                hid_darwin_set_open_exclusive(0);
#endif
                m_set_cur_dev_info = _get_device_set();

                for (const clibhid_dev_info& item : m_set_cur_dev_info) {
                    auto ptr_dev = std::make_shared<clibhid_dev>(item, m_ptr_mutex_hidapi);
                    if (!ptr_dev) {
                        continue;
                    }
                    if (!ptr_dev->is_open()) {
                        continue;
                    }
                    auto ptr_info = std::make_shared< clibhid_dev_info>(item);
                    m_map_ptrs.emplace(item.get_path_by_string(), std::make_pair(ptr_dev, ptr_info));
                }//end for

                //
                m_b_run_th_pluginout = true;
                m_ptr_th_pluginout = std::shared_ptr<std::thread>(new std::thread(clibhid::_worker_pluginout, std::ref(*this)));
            }
        
        }

        clibhid_dev::type_wptr _get_device(const std::string& s_path)
        {
            clibhid_dev::type_wptr wptr;
            do {
                auto it = m_map_ptrs.find(s_path);
                if (it != m_map_ptrs.end()) {
                    wptr = it->second.first;
                }
            } while (false);
            return wptr;
        }

        clibhid_dev::type_wptr _get_device(const clibhid_dev_info& dev_info)
        {
            return _get_device(dev_info.get_path_by_string());
        }

        clibhid_dev::type_wptr _get_device(const std::wstring & sw_path)
        {
            std::string s_path(cstring::get_mcsc_from_unicode(sw_path));
            return _get_device(s_path);
        }

    private:

        /**
        * detect device plug in & out
        */
        static void _worker_pluginout(clibhid& lib)
        {
            clibhid_dev_info::type_set set_dev;

            while (lib.m_b_run_th_pluginout) {
                do {
                    
                    std::lock_guard<std::mutex> lock(lib.m_mutex);
                    if (lib._update_dev_set()) {
                        //device list is changed
                        lib._update_device_map();

                        if (lib.m_cb) {//call callback.. for dev_ctl
                            lib.m_cb(lib.m_set_inserted_dev_info, lib.m_set_removed_dev_info, lib.m_set_cur_dev_info, lib.m_p_user);
                        }
                    }
                    
                } while (false);

                std::this_thread::sleep_for(std::chrono::milliseconds(clibhid::_const_dev_pluginout_check_interval_mmsec));
            }//end while

            std::wstringstream ss;
            ss << std::this_thread::get_id();
            std::wstring s_id = ss.str();
            _mp::clog::get_instance().log_fmt(L"[I] exit : %ls : id = %ls.\n", __WFUNCTION__, s_id.c_str());

        }

        /**
        * ONLY for _worker_pluginout.
        * update m_set_removed_dev_info, m_set_inserted_dev_info and m_set_cur_dev_info.
        */
        bool _update_dev_set()
        {
            bool b_changed(false);
            clibhid_dev_info::type_set set_dev;

            do {
                set_dev = _get_device_set();

                //set_dev = m_set_cur_dev_info;//for debugging
                //m_ptr_usb_lib->update_device_list();//for debugging
                //
                if (set_dev.size() == m_set_cur_dev_info.size()) {
                    if (set_dev.empty()) {
                        //no connected device
                        m_set_removed_dev_info.clear();	m_set_inserted_dev_info.clear();
                        continue;
                    }
                    if (set_dev==m_set_cur_dev_info) {
                        //no changned
                        m_set_removed_dev_info.clear();	m_set_inserted_dev_info.clear();
                        continue;
                    }
                }

                m_set_removed_dev_info = coperation::subtract<clibhid_dev_info>(m_set_cur_dev_info, set_dev);
                m_set_inserted_dev_info = coperation::subtract<clibhid_dev_info>(set_dev, m_set_cur_dev_info);
                m_set_cur_dev_info = set_dev;

                for (auto item : m_set_removed_dev_info) {
                    clog::get_instance().trace(L"[I] - %ls - removed : %ls.\n", __WFUNCTION__, item.get_path_by_wstring().c_str());
                    clog::get_instance().log_fmt(L"[I] - %ls - removed : %ls.\n", __WFUNCTION__, item.get_path_by_wstring().c_str());
                }

                for (auto item : m_set_inserted_dev_info) {
                    clog::get_instance().trace(L"[I] - %ls - inserted : %ls.\n", __WFUNCTION__, item.get_path_by_wstring().c_str());
                    clog::get_instance().log_fmt(L"[I] - %ls - inserted : %ls.\n", __WFUNCTION__, item.get_path_by_wstring().c_str());
                }

                if (m_set_cur_dev_info.empty()) {
                    clog::get_instance().trace(L"[I] - current : none.\n");
                }
                else {
                    for (auto item : m_set_cur_dev_info) {
                        clog::get_instance().trace(L"[I] - %ls - current : %ls.\n", __WFUNCTION__, item.get_path_by_wstring().c_str());
                    }
                }

                //
                b_changed = true;
            } while (false);
            return b_changed;
        }

        /**
          Max length of the result: "000-000.000.000.000.000.000.000:000.000" (39 chars).
          64 is used for simplicity/alignment.
        */
        static void _hidapi_get_path(char (*result)[64], libusb_device* dev, int config_number, int interface_number)
        {
            char* str = *result;

            /* Note that USB3 port count limit is 7; use 8 here for alignment */
            uint8_t port_numbers[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
            int num_ports = libusb_get_port_numbers(dev, port_numbers, 8);

            if (num_ports > 0) {
                int n = snprintf(str, sizeof("000-000"), "%u-%u", libusb_get_bus_number(dev), port_numbers[0]);
                for (uint8_t i = 1; i < num_ports; i++) {
                    n += snprintf(&str[n], sizeof(".000"), ".%u", port_numbers[i]);
                }
                n += snprintf(&str[n], sizeof(":000.000"), ":%u.%u", (uint8_t)config_number, (uint8_t)interface_number);
                str[n] = '\0';
            }
            else {
                /* Likely impossible, but check: USB3.0 specs limit number of ports to 7 and buffer size here is 8 */
                if (num_ports == LIBUSB_ERROR_OVERFLOW) {
                    //LOG("make_path() failed. buffer overflow error\n");
                }
                else {
                    //LOG("make_path() failed. unknown error\n");
                }
                str[0] = '\0';
            }
        }

        //modified make_path of hidapi
        static char* _hidapi_make_path(libusb_device* dev, int config_number, int interface_number)
        {
            char str[64];
            _hidapi_get_path(&str, dev, config_number, interface_number);
#ifdef _WIN32
            return _strdup(str);
#else
            return strdup(str);
#endif
            
        }

        //modified create_device_info_for_device of hidapi
        static struct hid_device_info* _hidapi_create_device_info_for_device(libusb_device* device, struct libusb_device_descriptor* desc, int config_number, int interface_num)
        {
            struct hid_device_info* cur_dev = (struct hid_device_info*)calloc(1, sizeof(struct hid_device_info));
            if (cur_dev == NULL) {
                return NULL;
            }

            /* VID/PID */
            cur_dev->vendor_id = desc->idVendor;
            cur_dev->product_id = desc->idProduct;

            cur_dev->release_number = desc->bcdDevice;

            cur_dev->interface_number = interface_num;

            cur_dev->bus_type = HID_API_BUS_USB;

            cur_dev->path = _hidapi_make_path(device, config_number, interface_num);

            return cur_dev;
        }

        struct hid_device_info* _hid_enumerate(libusb_context* usb_context,unsigned short w_vid, unsigned short w_pid)
        {
            libusb_device** devs;
            libusb_device* dev;
            libusb_device_handle* handle = NULL;
            ssize_t num_devs;
            int i = 0;

            struct hid_device_info* root = NULL; /* return object */
            struct hid_device_info* cur_dev = NULL;

            if (hid_init() < 0)
                return NULL;

            num_devs = libusb_get_device_list(usb_context, &devs);
            if (num_devs < 0)
                return NULL;
            while ((dev = devs[i++]) != NULL) {
                struct libusb_device_descriptor desc;
                struct libusb_config_descriptor* conf_desc = NULL;
                int j, k;

                int res = libusb_get_device_descriptor(dev, &desc);
                if (res < 0)
                    continue;

                unsigned short dev_vid = desc.idVendor;
                unsigned short dev_pid = desc.idProduct;

                if ((w_vid != 0x0 && w_vid != dev_vid) ||
                    (w_pid != 0x0 && w_pid != dev_pid)) {
                    continue;
                }

                res = libusb_get_active_config_descriptor(dev, &conf_desc);
                if (res < 0)
                    libusb_get_config_descriptor(dev, 0, &conf_desc);
                if (conf_desc) {
                    for (j = 0; j < conf_desc->bNumInterfaces; j++) {
                        const struct libusb_interface* intf = &conf_desc->interface[j];
                        for (k = 0; k < intf->num_altsetting; k++) {
                            const struct libusb_interface_descriptor* intf_desc;
                            intf_desc = &intf->altsetting[k];

                            if (intf_desc->bInterfaceClass == LIBUSB_CLASS_HID) {
                                struct hid_device_info* tmp;
                                //
                                tmp = _hidapi_create_device_info_for_device(dev, &desc, conf_desc->bConfigurationValue, intf_desc->bInterfaceNumber);

                                if (tmp) {
                                    if (cur_dev) {
                                        cur_dev->next = tmp;
                                    }
                                    else {
                                        root = tmp;
                                    }
                                    cur_dev = tmp;
                                }
                                break;
                            }
                        } /* altsettings */
                    } /* interfaces */
                    libusb_free_config_descriptor(conf_desc);
                }
            }

            libusb_free_device_list(devs, 1);

            return root;
        }

        void  _hid_free_enumeration(struct hid_device_info* devs)
        {
            struct hid_device_info* d = devs;
            while (d) {
                struct hid_device_info* next = d->next;
                free(d->path);
                free(d->serial_number);
                free(d->manufacturer_string);
                free(d->product_string);
                free(d);
                d = next;
            }
        }

        clibhid_dev_info::type_set _get_device_set()
        {
            clibhid_dev_info::type_set st;

            struct hid_device_info* devs = NULL;

            do {
                std::lock_guard<std::mutex> lock(*m_ptr_mutex_hidapi);
                devs = _hid_enumerate(m_ptr_usb_lib->get_context(),0x0, 0x0);

                if (devs) {
                    st = clibhid_dev_info::get_dev_info_set(devs);
                    _hid_free_enumeration(devs);
                }
            } while (false);

            //m_ptr_usb_lib->update_device_list();
            //TODO. you must assign st.

            // checks the device critial error 
            clibhid_dev_info::type_set st_must_be_removed;
            for (auto item : m_map_ptrs) {
                if (!item.second.first) {
                    continue;
                }
                if (!item.second.first->is_detect_replugin()) {
                    continue;
                }
                //critical error .. remove device forcelly
                if (item.second.second) {
                    st_must_be_removed.insert(*item.second.second);
                }
            }//end for

            clibhid_dev_info::type_set st_after_removed = coperation::subtract<clibhid_dev_info>(st, st_must_be_removed);
            //
            if (!m_set_usb_filter.empty()) {
                //filtering.. 
                clibhid_dev_info::type_set set_union_found;
                std::for_each(std::begin(m_set_usb_filter), std::end(m_set_usb_filter), [&](const type_tuple_usb_filter& filter) {
                    clibhid_dev_info::type_set set_found;
                    if (clibhid_dev_info::filter_dev_info_set(set_found, st_after_removed, filter) > 0) {
                        set_union_found.insert(std::begin(set_found), std::end(set_found));
                    }
                    });

                st_after_removed.swap(set_union_found);
            }

            return st_after_removed;
        }

        /**
        * for _worker_pluginout
        * create & remove  device instance of map.
        */
        void _update_device_map()
        {
            do {
                //remove
                for (auto item : m_set_removed_dev_info) {
                    m_map_ptrs.erase(item.get_path_by_string());
                }

                //insert
                for (const clibhid_dev_info & item : m_set_inserted_dev_info) {
                    auto ptr_dev = std::make_shared<clibhid_dev>(item, m_ptr_mutex_hidapi);
                    if (!ptr_dev) {
                        continue;
                    }
                    if (!ptr_dev->is_open()) {
                        continue;
                    }
                    auto ptr_info = std::make_shared< clibhid_dev_info>(item);
                    m_map_ptrs.emplace(item.get_path_by_string(), std::make_pair(ptr_dev, ptr_info));
                }//end for
            } while (false);
        }

    private:
        clibusb::type_ptr m_ptr_usb_lib;

        void* m_p_user;
        clibhid::type_callback_pluginout m_cb;

        std::shared_ptr<std::mutex> m_ptr_mutex_hidapi;//mutex for hidapi library
        bool m_b_ini;
        std::mutex m_mutex;
        clibhid::type_map_ptrs m_map_ptrs;

        std::shared_ptr<std::thread> m_ptr_th_pluginout;
        std::atomic<bool> m_b_run_th_pluginout;

        type_set_usb_filter m_set_usb_filter;

        clibhid_dev_info::type_set m_set_cur_dev_info;//this member will be changed after _update_dev_set ().
        clibhid_dev_info::type_set m_set_inserted_dev_info;//this member will be changed after _update_dev_set ().
        clibhid_dev_info::type_set m_set_removed_dev_info;//this member will be changed after _update_dev_set ().

    private://don't call these functions.
        
        clibhid(const clibhid&);
        clibhid& operator= (const clibhid&);
    };
   
    
}