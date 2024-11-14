#pragma once

#include <memory>
#include <mutex>
#include <map>
#include <mutex>
#include <atomic>
#include <functional>

#include <mp_clog.h>
#include <mp_cstring.h>

/**
* 경고 - libusb.h 는 안에서 Windows.h 를 include 하고, interface 가 정의되어 있으면, #undef 하므로
* Windows 에서 사용하는 모든 interface type 에 대해 에러라 발생한다.
* 따라서 편하게 #include <libusb.h> 이 후에 어떤것도 include 하지 말자.
*/
#include <libusb.h>

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
            enum {
                _const_default_mmsec_timeout_tx = 500
            };
        public:
            cdev(libusb_device* p_libusb_device, int n_interface_number) : 
                m_p_usb_handle(NULL),
                m_n_interface_number(n_interface_number),
                m_active_endpt_address_out(0xFF),
                m_active_endpt_address_in(0xFF),
                m_b_active_endpt_out_bulk_transfer(true),
                m_b_active_endpt_in_bulk_transfer(true)
            {
                bool b_result(false);
                do {
                    if (!_get_default_endpoint_address(p_libusb_device)) {
                        continue;
                    }
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
                m_n_interface_number(-1),
                m_active_endpt_address_out(0xFF),
                m_active_endpt_address_in(0xFF),
                m_b_active_endpt_out_bulk_transfer(true),
                m_b_active_endpt_in_bulk_transfer(true)
            {
                bool b_result(false);
                do {
                    if (!_get_default_endpoint_address(p_libusb_device)) {
                        continue;
                    }
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

                    for (auto item : m_map_rx) {
                        if (item.first) {
                            libusb_free_transfer(item.first);
                        }
                    }//end for

                    if (m_n_interface_number >= 0) {
                        libusb_release_interface(m_p_usb_handle, m_n_interface_number);
                    }
                    libusb_close(m_p_usb_handle);
                }
            }

            bool is_open()
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                if (m_p_usb_handle)
                    return true;
                else
                    return false;
            }

            void set_active_endpoint_out(unsigned char c_endpoint,bool b_bulk_transfer)
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                m_active_endpt_address_out = c_endpoint;
                m_b_active_endpt_out_bulk_transfer = b_bulk_transfer;
            }
            void set_active_endpoint_in(unsigned char c_endpoint, bool b_bulk_transfer)
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                m_active_endpt_address_in = c_endpoint;
                m_b_active_endpt_in_bulk_transfer = b_bulk_transfer;
            }
            unsigned char get_active_endpoint_out()
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                return m_active_endpt_address_out;
            }
            unsigned char get_active_endpoint_in()
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                return m_active_endpt_address_in;
            }

            bool tx(const _mp::type_v_buffer& v_tx)
            {
                bool b_result(false);

                do {
                    std::lock_guard<std::mutex> lock(m_mutex);

                    int n_result(0);
                    int n_tx(0);

                    if (!v_tx.empty()) {
                        _mp::type_v_buffer v(v_tx);
                        if (m_b_active_endpt_out_bulk_transfer) {
                            n_result = libusb_bulk_transfer(m_p_usb_handle, m_active_endpt_address_out, &v[0], v.size(), &n_tx, cdev::_const_default_mmsec_timeout_tx);
                        }
                        else {
                            n_result = libusb_interrupt_transfer(m_p_usb_handle, m_active_endpt_address_out, &v[0], v.size(), &n_tx, cdev::_const_default_mmsec_timeout_tx);
                        }

                        if (n_result != LIBUSB_SUCCESS) {
                            continue;
                        }
                        if (n_tx != (int)v.size()) {
                            continue;
                        }
                    }
                    //
                    b_result = true;
                } while (false);
                return b_result;

            }

            bool start_rx(int n_timeout)
            {
                bool b_result(false);
                int n_result(0);
                struct libusb_transfer* in_transfer(nullptr);

                do {
                    std::lock_guard<std::mutex> lock(m_mutex);

                    struct libusb_transfer* in_transfer = libusb_alloc_transfer(0);
                    if (!in_transfer) {
                        continue;
                    }

                    _mp::type_ptr_v_buffer ptr_v(std::make_shared<_mp::type_v_buffer>(64, 0));
                    if (m_b_active_endpt_in_bulk_transfer) {
                        libusb_fill_bulk_transfer(in_transfer, m_p_usb_handle, m_active_endpt_address_in, &(*ptr_v)[0], ptr_v->size(), cdev::_transfer_callback, this, n_timeout);
                    }
                    else {
                        libusb_fill_interrupt_transfer(in_transfer, m_p_usb_handle, m_active_endpt_address_in, &(*ptr_v)[0], ptr_v->size(), cdev::_transfer_callback, this, n_timeout);
                    }
                    //
                    n_result = libusb_submit_transfer(in_transfer);
                    if (n_result < 0) {
                        continue;
                    }

                    m_map_rx.emplace(in_transfer, ptr_v);
                    b_result = true;
                }while(false);

                if (!b_result && in_transfer!=nullptr) {
                    libusb_free_transfer(in_transfer);
                }
                return b_result;
            }

            void looper_event(libusb_context* ctx)
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                int n_result = libusb_handle_events(ctx);
                if (n_result < 0) {
                    //printf("libusb_handle_events failed\n");
                }
            }
            bool clear_halt(unsigned char c_endpoint)
            {
                bool b_result(false);

                do {
                    std::lock_guard<std::mutex> lock(m_mutex);
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
                    std::lock_guard<std::mutex> lock(m_mutex);
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

            /**
            * this callback will be called by looper_event().
            * therefore this callback is guarded by m_mutex
            */
            static void LIBUSB_CALL _transfer_callback(struct libusb_transfer* transfer)
            {
                if (transfer->status == LIBUSB_TRANSFER_COMPLETED) {
                   /*printf("Transfer completed: %d bytes transferred\n", transfer->actual_length);
                    if (transfer->endpoint == ENDPOINT_IN) {
                        printf("Data received: ");
                        for (int i = 0; i < transfer->actual_length; i++) {
                            printf("%02x ", transfer->buffer[i]);
                        }
                        printf("\n");
                    }
                    */
                }
                else {
                    //printf("Transfer failed: %d\n", transfer->status);
                }

                // free transfer
                libusb_free_transfer(transfer);
                //
                clibusb::cdev* p_obj = (clibusb::cdev*)transfer->user_data;
                if (p_obj) {
                    p_obj->m_map_rx.erase(transfer);
                }
            }

            bool _get_default_endpoint_address(libusb_device* p_libusb_device)
            {
                bool b_result(false);
                int n_result(0);
                unsigned char c_default_endpt_in(0xFF);
                unsigned char c_default_endpt_out(0xFF);

                struct libusb_device_descriptor desc {0,};
                struct libusb_config_descriptor* p_conf_desc = NULL;

                do {
                    if (!p_libusb_device) {
                        continue;
                    }
                    //

                    n_result = libusb_get_device_descriptor(p_libusb_device, &desc);
                    if (n_result < 0) {
                        continue;
                    }

                    n_result = libusb_get_active_config_descriptor(p_libusb_device, &p_conf_desc);
                    if (n_result < 0) {
                        libusb_get_config_descriptor(p_libusb_device, 0, &p_conf_desc);
                    }
                    if (!p_conf_desc) {
                        continue;
                    }

                    for (int i = 0; i < p_conf_desc->bNumInterfaces; i++) {
                        const struct libusb_interface* intf = &p_conf_desc->interface[i];
                        for (int j = 0; j < intf->num_altsetting; j++) {
                            const struct libusb_interface_descriptor* intf_desc = &intf->altsetting[j];

                            for (int k = 0; k < intf_desc->bNumEndpoints; j++) {
                                const struct libusb_endpoint_descriptor* endpt_desc = &intf_desc->endpoint[k];

                                if (endpt_desc->bEndpointAddress & 0x80) {
                                    //in endpoint
                                    if (c_default_endpt_in == 0xFF) {
                                        //not yet setting
                                        m_active_endpt_address_in = c_default_endpt_in = endpt_desc->bEndpointAddress;

                                        if ((endpt_desc->bmAttributes & 0x03) == 0x02) {
                                            m_b_active_endpt_in_bulk_transfer = true;
                                        }
                                        else {
                                            m_b_active_endpt_in_bulk_transfer = false;//interrupt 0x03, control 0x00, isochronous 0x01
                                        }
                                    }
                                }
                                else {
                                    //out endpoint
                                    if (c_default_endpt_out == 0xFF) {
                                        //not yet setting
                                        m_active_endpt_address_out =  c_default_endpt_out = endpt_desc->bEndpointAddress;

                                        if ((endpt_desc->bmAttributes & 0x03) == 0x02) {
                                            m_b_active_endpt_out_bulk_transfer = true;
                                        }
                                        else {
                                            m_b_active_endpt_out_bulk_transfer = false;//interrupt 0x03, control 0x00, isochronous 0x01
                                        }
                                    }
                                }
                            }//end for k

                        }//end for j
                    }//end for i
                    
                    //
                    b_result = true;
                } while (false);

                if (p_conf_desc) {
                    libusb_free_config_descriptor(p_conf_desc);
                }
                return b_result;
            }

        private:
            std::mutex m_mutex;
            libusb_device_handle *m_p_usb_handle;//guard by m_mutex
            int m_n_interface_number;
            unsigned char m_active_endpt_address_out;//0x0x , guard by m_mutex
            unsigned char m_active_endpt_address_in;//0x8x , guard by m_mutex

            bool m_b_active_endpt_out_bulk_transfer;//guard by m_mutex, false ->interrupt transfer
            bool m_b_active_endpt_in_bulk_transfer;//guard by m_mutex, false ->interrupt transfer

            std::map<struct libusb_transfer*, _mp::type_ptr_v_buffer> m_map_rx;

        private://don't call these methods
            cdev();
            cdev(const cdev&);
            cdev& operator=(const cdev&);
        };

    public:
        clibusb() : 
            m_b_ini(false),
            m_p_ctx(NULL), m_pp_devs(NULL), m_n_dev(-1),
            m_b_registered_hotpluginout(false), m_cb_handle_pluginout(0)
        {
            if (libusb_init_context(&m_p_ctx, NULL, 0) == LIBUSB_SUCCESS) {
                m_b_ini = true;
            }
            else {
                return;
            }

            //register hot plugin out
            /*
            do {
                if (!libusb_has_capability(LIBUSB_CAP_HAS_HOTPLUG)) {
                    continue;
                }
                int rc = libusb_hotplug_register_callback(
                    m_p_ctx,
                    LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED | LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT,
                    LIBUSB_HOTPLUG_ENUMERATE,
                    LIBUSB_HOTPLUG_MATCH_ANY,
                    LIBUSB_HOTPLUG_MATCH_ANY,
                    LIBUSB_HOTPLUG_MATCH_ANY,
                    clibusb::_cb_hotpluginout,
                    this,
                    &m_cb_handle_pluginout
                    );
                if (rc != LIBUSB_SUCCESS) {
                    continue;
                }

                m_b_registered_hotpluginout = true;
            } while (false);
            */
        }

        virtual ~clibusb()
        {
            if (!m_b_ini) {
                return;
            }
            if (m_b_registered_hotpluginout) {
                libusb_hotplug_deregister_callback(m_p_ctx, m_cb_handle_pluginout);
            }

            if (m_pp_devs) {
                libusb_free_device_list(m_pp_devs, 1);
            }
            libusb_exit(m_p_ctx);
        }

        bool is_ini() const
        {
            return m_b_ini;
        }

        libusb_context* get_context()
        {
            return m_p_ctx;
        }
        /**
        * update m_pp_devs
        */
        ssize_t update_device_list()
        {
            //libusb_handle_events_completed(m_p_ctx, NULL);

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
                std::lock_guard<std::mutex>lock(m_mutex);

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

                //open device temporarily
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
        static int LIBUSB_CALL _cb_hotpluginout(
            libusb_context* ctx, 
            libusb_device* device,
            libusb_hotplug_event event, 
            void* user_data
        ) {
            clibusb* p_libusb = (clibusb*)user_data;

            struct libusb_device_descriptor desc;
            libusb_get_device_descriptor(device, &desc);

            if (event == LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED) {
                _mp::clog::get_instance().log_fmt_in_debug_mode(L"[D] _cb_hotpluginout : USB connected: VendorID=%04x, ProductID=%04x\n", desc.idVendor, desc.idProduct);
            }
            else if (event == LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT) {
                _mp::clog::get_instance().log_fmt_in_debug_mode(L"[D] _cb_hotpluginout : USB removed: VendorID=%04x, ProductID=%04x\n", desc.idVendor, desc.idProduct);
            }
            return 0;
        }

    private:
        std::mutex m_mutex;

        bool m_b_ini;

        libusb_context* m_p_ctx;
        libusb_device** m_pp_devs;
        ssize_t m_n_dev;
        bool m_b_registered_hotpluginout;
        libusb_hotplug_callback_handle m_cb_handle_pluginout;

    private://don't call these functions.
        clibusb(const clibusb&);
        clibusb& operator= (const clibusb&);

    };


}