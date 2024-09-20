#pragma once

#include <memory>
#include <mutex>
#include <map>
#include <mutex>
#include <atomic>
#include <functional>

#if defined(_WIN32)
#include <windows.h>
#endif

#include <libusb.h>
#include <mp_clog.h>
#include <mp_cstring.h>

namespace _mp {

    class clibusb
    {
    public:
        typedef std::shared_ptr< clibusb > type_ptr;
        typedef std::weak_ptr< clibusb > type_wptr;

    public:
        class cdev{
        public:
            typedef std::shared_ptr< cdev > type_ptr;
        public:
            cdev(libusb_device* p_libusb_device, int n_interface_number) : 
                m_p_usb_handle(NULL),
                m_n_interface_number(n_interface_number)
            {
                bool b_result(false);
                do {
                    if (libusb_open(p_libusb_device, &m_p_usb_handle)) {
                        //libusb_open is return zero succcess
                        // here error
                        m_p_usb_handle = NULL;
                        continue;
                    }
                    if (libusb_claim_interface(m_p_usb_handle, n_interface_number)) {
                        continue; // error
                    }

                    b_result = true;
                } while (false);

                if (!b_result) {
                    if (m_p_usb_handle) {
                        libusb_close(m_p_usb_handle);
                    }
                    m_p_usb_handle = NULL;
                    m_n_interface_number = -1;
                }
            }
            cdev(libusb_device* p_libusb_device) :
                m_p_usb_handle(NULL),
                m_n_interface_number(-1)
            {
                bool b_result(false);
                do {
                    if (libusb_open(p_libusb_device, &m_p_usb_handle)) {
                        //libusb_open is return zero succcess
                        // here error
                        m_p_usb_handle = NULL;
                        continue;
                    }

                    b_result = true;
                } while (false);

                if (!b_result) {
                    if (m_p_usb_handle) {
                        libusb_close(m_p_usb_handle);
                    }
                    m_p_usb_handle = NULL;
                }
            }

            ~cdev()
            {
                if (m_p_usb_handle) {
                    if (m_n_interface_number >= 0) {
                        libusb_release_interface(m_p_usb_handle, m_n_interface_number);
                    }
                    libusb_close(m_p_usb_handle);
                }
            }

            bool is_open() const
            {
                if (m_p_usb_handle)
                    return true;
                else
                    return false;
            }

            bool clear_halt(unsigned char c_endpoint)
            {
                bool b_result(false);

                do {
                    if (m_p_usb_handle == NULL)
                        continue;
                    if (libusb_clear_halt(m_p_usb_handle, c_endpoint))
                        continue;
                    //
                    b_result = true;
                } while (false);

                return b_result;
            }
            bool reset_port()
            {
                bool b_result(false);

                do {
                    if (m_p_usb_handle == NULL)
                        continue;
                    if (libusb_reset_device(m_p_usb_handle))
                        continue;
                    //
                    b_result = true;
                } while (false);

                return b_result;
            }

        private:
            libusb_device_handle *m_p_usb_handle;
            int m_n_interface_number;

        private://don't call these methods
            cdev();
            cdev(const cdev&);
            cdev& operator=(const cdev&);
        };

    public:
        clibusb() : 
            m_p_ctx(NULL), m_pp_devs(NULL), m_n_dev(-1)
        {
            libusb_init(&m_p_ctx);
        }

        virtual ~clibusb()
        {
            if (m_pp_devs) {
                libusb_free_device_list(m_pp_devs, 1);
            }
            libusb_exit(m_p_ctx);
        }

        ssize_t update_device_list()
        {
            std::lock_guard<std::mutex>lock(m_mutex);

            if (m_pp_devs) {
                libusb_free_device_list(m_pp_devs, 1);
                m_pp_devs = NULL;
            }
            //
            m_n_dev = libusb_get_device_list(m_p_ctx, &m_pp_devs);
            return m_n_dev;
        }

        libusb_device* get_device(const std::string& s_device_path_by_hidapi)
        {
            libusb_device* p_dev(NULL);

            do {
                if (m_n_dev <= 0) {
                    _mp::clog::get_instance().log_fmt_in_debug_mode(L"[D] get_device : m_n_dev <= 0.\n");
                    continue;
                }

                libusb_device* found_dev = nullptr;

                for (ssize_t i = 0; i < m_n_dev; ++i) {
                    libusb_device* dev = m_pp_devs[i];
                    struct libusb_device_descriptor desc;
                    if (libusb_get_device_descriptor(dev, &desc) < 0) {
                        //std::cerr << "Failed to get device descriptor!" << std::endl;
                        _mp::clog::get_instance().log_fmt_in_debug_mode(L"[D] get_device : Failed to get device descriptor.\n");
                        continue;
                    }

#ifdef _WIN32
                    char device_path_libusb[256];
                    libusb_get_string_descriptor_ascii(libusb_open_device_with_vid_pid(m_p_ctx, desc.idVendor, desc.idProduct), desc.iProduct, (unsigned char*)device_path_libusb, sizeof(device_path_libusb));

                    if (s_device_path_by_hidapi == std::string(device_path_libusb)) {
                        found_dev = dev;
                        break;
                    }
#else
                    uint8_t bus_num = libusb_get_bus_number(dev);
                    uint8_t dev_addr = libusb_get_device_address(dev);

                    std::string dev_path = "/dev/bus/usb/" + std::to_string(bus_num) + "/" + std::to_string(dev_addr);
                    if (s_device_path_by_hidapi == dev_path) {
                        found_dev = dev;
                        break;
                    }
#endif
                }//end for

                if (!found_dev) {
                    _mp::clog::get_instance().log_fmt_in_debug_mode(L"[D] get_device : No matching USB device found.\n");
                    //No matching USB device found!"
                    continue;
                }

                p_dev = found_dev;
            } while (false);

            return p_dev;
        }

        libusb_device* get_device(const std::wstring& ws_device_path_by_hidapi)
        {
            libusb_device* p_dev(NULL);

            std::string s_device_path_by_hidapi = cstring::get_mcsc_from_unicode(ws_device_path_by_hidapi);

            return get_device(s_device_path_by_hidapi);
        }

        bool clear_halt(const std::string& s_device_path_by_hidapi, int n_interface_number, unsigned char c_endpoint)
        {
            bool b_result(false);

            do{
                libusb_device* p_d = get_device(s_device_path_by_hidapi);
                if (p_d == NULL) {
                    continue;
                }
                clibusb::cdev::type_ptr ptr(new clibusb::cdev(p_d, n_interface_number));
                if (!ptr) {
                    continue;
                }
                b_result = ptr->clear_halt(c_endpoint);
            } while (false);
            return b_result;
        }

        bool reset_port(const std::string& s_device_path_by_hidapi, int n_interface_number)
        {
            bool b_result(false);

            do {
                libusb_device* p_d = get_device(s_device_path_by_hidapi);
                if (p_d == NULL) {
                    continue;
                }
                clibusb::cdev::type_ptr ptr(new clibusb::cdev(p_d, n_interface_number));
                if (!ptr) {
                    continue;
                }
                b_result = ptr->reset_port();
            } while (false);
            return b_result;
        }
        
        bool clear_halt_and_reset_port(const std::string& s_device_path_by_hidapi, int n_interface_number, unsigned char c_endpoint)
        {
            bool b_result(false);

            do {
                libusb_device* p_d = get_device(s_device_path_by_hidapi);
                if (p_d == NULL) {
                    continue;
                }
                clibusb::cdev::type_ptr ptr(new clibusb::cdev(p_d, n_interface_number));
                if (!ptr) {
                    continue;
                }
                if (ptr->clear_halt(c_endpoint)) {
                    b_result = true;
                    continue;
                }
                b_result = ptr->reset_port();
            } while (false);
            return b_result;
        }

    private:
        std::mutex m_mutex;

        libusb_context* m_p_ctx;
        libusb_device** m_pp_devs;
        ssize_t m_n_dev;

    private://don't call these functions.
        clibusb(const clibusb&);
        clibusb& operator= (const clibusb&);

    };


}