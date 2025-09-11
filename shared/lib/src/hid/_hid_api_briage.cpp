#include <iostream>
#include <sstream>

#include <mp_cstring.h>
#include <mp_elpusk.h>
#include <hid/_hid_api_briage.h>
#include <hid/_vhid_info.h>

#ifdef _WIN32

extern "C"
{
#include <hidsdi.h>
}

#include <cfgmgr32.h>
#include <setupapi.h>

#ifdef _DEBUG
//#undef __THIS_FILE_ONLY__
#define __THIS_FILE_ONLY__
#undef __VIRTUAL_IBUTTON_DATA__

#include <atltrace.h>
#endif

#pragma comment(lib, "hid.lib")
#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "Cfgmgr32.lib")
#endif //_WIN32

/**
* local function prototype
*/
#ifdef _WIN32
//////////////////////////////////////////////////////////
// for windows

static struct hid_device_info* _hid_internal_get_device_info(const wchar_t* path);

static struct hid_device_info* _hid_enumerate(libusb_context* usb_context, unsigned short vendor_id, unsigned short product_id);
#else
//////////////////////////////////////////////////////////
// for linux
/**
  Max length of the result: "000-000.000.000.000.000.000.000:000.000" (39 chars).
  64 is used for simplicity/alignment.
*/
static  void _hidapi_get_path(char (*result)[64], libusb_device* dev, int config_number, int interface_number);

//modified make_path of hidapi
static char* _hidapi_make_path(libusb_device* dev, int config_number, int interface_number);

//modified create_device_info_for_device of hidapi
static struct hid_device_info* _hidapi_create_device_info_for_device(libusb_device* device, struct libusb_device_descriptor* desc, int config_number, int interface_num);

static struct hid_device_info* _hid_enumerate(libusb_context* usb_context, unsigned short w_vid, unsigned short w_pid);
#endif //_WIN32
static void  _hid_free_enumeration(struct hid_device_info* devs);

/**
* local function body
*/
#ifdef _WIN32
//////////////////////////////////////////////////////////
// for windows

struct hid_device_info* _hid_internal_get_device_info(const wchar_t* path)
{
    struct hid_device_info* dev = NULL; /* return object */

    unsigned short w_v(0), w_p(0);
    int n_v(0), n_p(0), n_inf(0);

    std::wstring s_low_path(path);
    _mp::cstring::to_lower(s_low_path);

    n_v = _mp::coperation::get_usb_vid_from_path(s_low_path);
    if (n_v < 0) {
        return NULL;
    }
    n_p = _mp::coperation::get_usb_pid_from_path(s_low_path);
    if (n_p < 0) {
        return NULL;
    }
    w_v = (unsigned short)n_v;
    w_p = (unsigned short)n_p;

    n_inf = _mp::coperation::get_usb_inf_from_path(s_low_path);

    /* Create the record. */
    dev = (struct hid_device_info*)calloc(1, sizeof(struct hid_device_info));

    if (dev == NULL) {
        return NULL;
    }

    /* Fill out the record */
    dev->next = NULL;

    std::string spath = _mp::cstring::get_mcsc_from_unicode(std::wstring(path));
    dev->path = (char*)calloc(spath.size() + 1, sizeof(char));
    memset(dev->path, 0, spath.size() + 1);
    memcpy(dev->path, spath.c_str(), spath.size());
    dev->interface_number = n_inf;

    /* VID/PID */
    dev->vendor_id = w_v;
    dev->product_id = w_p;

    /* detect bus type before reading string descriptors */
    dev->bus_type = HID_API_BUS_USB;

    return dev;
}

struct hid_device_info* _hid_enumerate(libusb_context* usb_context, unsigned short vendor_id, unsigned short product_id)
{
    struct hid_device_info* root = NULL; /* return object */
    struct hid_device_info* cur_dev = NULL;
    GUID interface_class_guid;
    CONFIGRET cr = CR_SUCCESS;
    std::vector<wchar_t> v_device_interface_list;
    DWORD len;

    if (hid_init() < 0) {
        /* register_global_error: global error is reset by hid_init */
        return NULL;
    }

    /* Retrieve HID Interface Class GUID */
    HidD_GetHidGuid(&interface_class_guid);

    /* Get the list of all device interfaces belonging to the HID class. */
    /* Retry in case of list was changed between calls to
      CM_Get_Device_Interface_List_SizeW and CM_Get_Device_Interface_ListW */
    do {
        cr = CM_Get_Device_Interface_List_SizeW(&len, &interface_class_guid, NULL, CM_GET_DEVICE_INTERFACE_LIST_PRESENT);
        if (cr != CR_SUCCESS) {
            _mp::clog::get_instance().log_fmt(L"[E] %ls : Failed to get size of HID device interface list.\n", __WFUNCTION__);
            break;
        }

        v_device_interface_list.resize(len, 0);
        std::fill(v_device_interface_list.begin(), v_device_interface_list.end(), 0);

        cr = CM_Get_Device_Interface_ListW(&interface_class_guid, NULL, &v_device_interface_list[0], v_device_interface_list.size(), CM_GET_DEVICE_INTERFACE_LIST_PRESENT);
        if (cr != CR_SUCCESS && cr != CR_BUFFER_SMALL) {
            _mp::clog::get_instance().log_fmt(L"[E] %ls : Failed to get HID device interface list.\n", __WFUNCTION__);
        }
    } while (cr == CR_BUFFER_SMALL);

    if (cr != CR_SUCCESS) {
        return root;
    }

    std::set<std::wstring> set_before_filter, set_after_filter;
    /* Iterate over each device interface in the HID class, looking for the right one. */
    for (wchar_t* device_interface = &v_device_interface_list[0]; *device_interface; device_interface += wcslen(device_interface) + 1) {
        unsigned short w_v(0), w_p(0);
        int n_v(0), n_p(0);

        std::wstring s_low_path(device_interface);
        _mp::cstring::to_lower(s_low_path);

        n_v = _mp::coperation::get_usb_vid_from_path(s_low_path);
        if (n_v < 0) {
            continue;
        }
        n_p = _mp::coperation::get_usb_pid_from_path(s_low_path);
        if (n_p < 0) {
            continue;
        }

        set_before_filter.insert(s_low_path);
        if (s_low_path.size() >= 3) {
            if (s_low_path.compare(s_low_path.size() - 3, 3, L"kbd") == 0) {
                // the last string of s_low_path is "kbd".
                continue;//ignore keyboard device
            }
            if (s_low_path.compare(s_low_path.size() - 3, 3, L"mou") == 0) {
                // the last string of s_low_path is "mou".
                continue;//ignore mouse device
            }
        }
        set_after_filter.insert(s_low_path);

        w_v = (unsigned short)n_v;
        w_p = (unsigned short)n_p;


        /* Check the VID/PID to see if we should add this
           device to the enumeration list. */
        if ((vendor_id == 0x0 || w_v == vendor_id) &&
            (product_id == 0x0 || w_p == product_id)) {

            /* VID/PID match. Create the record. */
            struct hid_device_info* tmp = _hid_internal_get_device_info(device_interface);

            if (tmp == NULL) {
                continue;
            }

            if (cur_dev) {
                cur_dev->next = tmp;
            }
            else {
                root = tmp;
            }
            cur_dev = tmp;
        }
    }//end for

    if (root == NULL) {
        if (vendor_id == 0 && product_id == 0) {
            //_mp::clog::get_instance().log_fmt(L"[W] %ls : No HID devices found in the system.\n", __WFUNCTION__);
        }
        else {
            //_mp::clog::get_instance().log_fmt(L"[W] %ls : No HID devices with requested VID/PID found in the system.\n", __WFUNCTION__);
        }
    }

    return root;
}
#else
//////////////////////////////////////////////////////////
// for linux
/**
  Max length of the result: "000-000.000.000.000.000.000.000:000.000" (39 chars).
  64 is used for simplicity/alignment.
*/
void _hidapi_get_path(char (*result)[64], libusb_device* dev, int config_number, int interface_number)
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
char* _hidapi_make_path(libusb_device* dev, int config_number, int interface_number)
{
    char str[64];
    _hidapi_get_path(&str, dev, config_number, interface_number);
    return strdup(str);

}

//modified create_device_info_for_device of hidapi
struct hid_device_info* _hidapi_create_device_info_for_device(libusb_device* device, struct libusb_device_descriptor* desc, int config_number, int interface_num)
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

struct hid_device_info* _hid_enumerate(libusb_context* usb_context, unsigned short w_vid, unsigned short w_pid)
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
#endif //_WIN32
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


/**
* member function bodies
*/
_hid_api_briage::_hid_api_briage() :
    m_atll_req_q_check_interval_mmsec(_hid_api_briage::const_default_req_q_check_interval_mmsec_of_child)
	, m_atll_hid_write_interval_mmsec(_hid_api_briage::const_default_hid_write_interval_mmsec)
	, m_atll_hid_read_interval_mmsec(_hid_api_briage::const_default_hid_read_interval_mmsec)
    , m_s_class_name(L"_hid_api_briage")
    , m_b_ini(false)
    , m_n_map_index(_vhid_info::const_map_index_invalid)
{
    m_ptr_usb_lib = std::make_shared<_mp::clibusb>();

    if (hid_init() == 0) {
        m_b_ini = true;

#if defined(__APPLE__) && HID_API_VERSION >= HID_API_MAKE_VERSION(0, 12, 0)
        // To work properly needs to be called before hid_open/hid_open_path after hid_init.
        // Best/recommended option - call it right after hid_init.
        hid_darwin_set_open_exclusive(0);
#endif

    }
}

_hid_api_briage::~_hid_api_briage()
{
    if (m_b_ini) {
        for( auto item : m_map_lpu237_disable_ibutton) {
            if (item.second) {
#if defined(_WIN32) && defined(_DEBUG) && defined(__THIS_FILE_ONLY__)
                struct hid_device_info* p_hid_dev_inf = hid_get_device_info(item.first);
                if (p_hid_dev_inf) {

                    ATLTRACE(" ^________^ device auto closed-%s, vendor_id:0x%x, product_id:0x%x, interface_number:%d.\n",
                        p_hid_dev_inf->path,
                        p_hid_dev_inf->vendor_id,
                        p_hid_dev_inf->product_id,
                        p_hid_dev_inf->interface_number
                    );
                }
#endif
                //_lpu237_ibutton_enable(item.first, true); // enable i-button listening.
            }
		}//end for

        for (auto item : m_map_hid_dev) {
            item.second.second = false;
            hid_close(item.second.first);
        }//end for

        hid_exit();
    }

    m_ptr_usb_lib.reset();
}

bool _hid_api_briage::is_ini() const
{
    return m_b_ini;
}

std::set< std::tuple<std::string, unsigned short, unsigned short, int,std::string> > _hid_api_briage::hid_enumerate()
{
    std::set< std::tuple<std::string, unsigned short, unsigned short, int, std::string> > st;

    struct hid_device_info *p_devs = NULL, *p_devs_org = NULL;

    do {
        std::lock_guard<std::mutex> lock(_hid_api_briage::get_mutex_for_hidapi());
        if (!m_b_ini) {
            continue;
        }
        /**
        * don't use the hid_enumerate() of hidapi. it will occur packer-losting.
        */
        p_devs_org = p_devs = _hid_enumerate(m_ptr_usb_lib->get_context(), 0x0, 0x0);

        while (p_devs) {
            st.emplace(
                std::string(p_devs->path),
                p_devs->vendor_id,
                p_devs->product_id,
                p_devs->interface_number,
                std::string()
            );
            p_devs = p_devs->next;
        }
        _hid_free_enumeration(p_devs_org);
    } while (false);

	return st;
}

/**
* @brief check is open or not
* @param path - primitive path
* @return first true - open, false not open or error, second - the opened device of the index of map. third - true(exclusive open), false(shared open or not open )
*/
std::tuple<bool, int, bool> _hid_api_briage::is_open(const char* path) const
{
    bool b_result(false);
    int n_index(-1);
    bool b_exclusive_open(false);

    do {
        std::lock_guard<std::mutex> lock(_hid_api_briage::get_mutex_for_hidapi());

        if (path == nullptr) {
            continue;
        }
        for (auto item : m_map_hid_dev) {
            hid_device* p_hid = item.second.first;
            if (p_hid == nullptr) {
                continue;
            }
            //
            hid_device_info* p_info = hid_get_device_info(p_hid);
            if (p_info == NULL) {
                continue;
            }
            if (p_info->path == NULL) {
                continue;
            }
            //
            std::string s(p_info->path);
            if (s.compare(path) == 0) {
                n_index = item.first;
                b_exclusive_open = item.second.second;
                b_result = true;
                break; //opened device exit while
            }
        }//end for

    } while (false);
    return std::make_tuple(b_result,n_index,b_exclusive_open);
}


int _hid_api_briage::api_open_path(const char* path)
{
    std::lock_guard<std::mutex> lock(_hid_api_briage::get_mutex_for_hidapi());

    if (!m_b_ini) {
        return -1;
    }

    int n_map_index = m_n_map_index;
    hid_device* p_dev = hid_open_path(path);
    if (p_dev) {
        if (n_map_index == _vhid_info::const_map_index_invalid) {
            n_map_index = _vhid_info::const_map_index_min;
        }
        else if (n_map_index == _vhid_info::const_map_index_max) {
            n_map_index = _vhid_info::const_map_index_min;
        }
        else {
            n_map_index += _vhid_info::const_map_index_inc_unit;
        }
        
        if (m_map_hid_dev.find(n_map_index) != std::end(m_map_hid_dev)) {
            return -1; // already open
            //hid_close(m_map_hid_dev[n_map_index].first);
            //m_map_hid_dev.erase(n_map_index);
        }
        m_map_hid_dev[n_map_index].first = p_dev;
        m_map_hid_dev[n_map_index].second = false;//primitive device always exclusive mode
        m_n_map_index = n_map_index;

        // this is dregon.
        if (p_dev) {
            struct hid_device_info* p_hid_dev_inf = hid_get_device_info(p_dev);
            if (p_hid_dev_inf) {


                do {
                    if (p_hid_dev_inf->vendor_id != _mp::_elpusk::const_usb_vid) {
                        continue;
                    }
                    if (p_hid_dev_inf->product_id != _mp::_elpusk::_lpu237::const_usb_pid && p_hid_dev_inf->product_id != _mp::_elpusk::_lpu238::const_usb_pid) {
                        continue;
                    }
                    // here lpu237 & lpu238 device only
                    /*
                    if (_lpu237_ibutton_enable(p_dev, false)) {// disable i-button listening.
						m_map_lpu237_disable_ibutton[p_dev] = true; // remember disable i-button
                    }
                    */
                } while (false);
#if defined(_WIN32) && defined(_DEBUG) && defined(__THIS_FILE_ONLY__)
                ATLTRACE(" ^________^ device open-%s, vendor_id:0x%x, product_id:0x%x, interface_number:%d.\n",
                    p_hid_dev_inf->path,
                    p_hid_dev_inf->vendor_id,
                    p_hid_dev_inf->product_id,
                    p_hid_dev_inf->interface_number
                );
#endif
            }
        }
        return n_map_index;
    }
    else {
        return -1;
    }
}

void _hid_api_briage::api_close(int n_primitive_map_index)
{
    std::lock_guard<std::mutex> lock(_hid_api_briage::get_mutex_for_hidapi());

    if (!m_b_ini) {
        return;
    }

    hid_device* p_dev = nullptr;
    if (m_map_hid_dev.find(n_primitive_map_index) != std::end(m_map_hid_dev)) {
        p_dev = m_map_hid_dev[n_primitive_map_index].first;
        m_map_hid_dev.erase(n_primitive_map_index);

        // this is dregon.
        if (p_dev) {
            struct hid_device_info* p_hid_dev_inf = hid_get_device_info(p_dev);
            if (p_hid_dev_inf) {

                do {
                    if (p_hid_dev_inf->vendor_id != _mp::_elpusk::const_usb_vid) {
                        continue;
                    }
                    if (p_hid_dev_inf->product_id != _mp::_elpusk::_lpu237::const_usb_pid && p_hid_dev_inf->product_id != _mp::_elpusk::_lpu238::const_usb_pid) {
                        continue;
                    }
                    // here lpu237 & lpu238 device only
					auto it = m_map_lpu237_disable_ibutton.find(p_dev);
                    if(it != std::end(m_map_lpu237_disable_ibutton)) {
                        if (it->second) {
                            // enable i-button listening.
                            //_lpu237_ibutton_enable(p_dev, true); // enable i-button listening.(recover default )
                        }
                        m_map_lpu237_disable_ibutton.erase(it);
                    }
                } while (false);

#if defined(_WIN32) && defined(_DEBUG) && defined(__THIS_FILE_ONLY__)
                ATLTRACE(" ^________^ device closed-%s, vendor_id:0x%x, product_id:0x%x, interface_number:%d.\n",
                    p_hid_dev_inf->path,
                    p_hid_dev_inf->vendor_id,
                    p_hid_dev_inf->product_id,
					p_hid_dev_inf->interface_number
                );
#endif
            }
        }

        hid_close(p_dev);
    }
}

bool _hid_api_briage::_lpu237_ibutton_enable(hid_device* p_dev, bool b_enable)
{
    bool b_result(false);

    do {
        if (p_dev == nullptr) {
            continue;
        }

        long long ll_check_tx_interval_mmsec = get_hid_write_interval_in_child();
        long long ll_check_rx_interval_mmsec = get_hid_read_interval_in_child();

        unsigned char c_cmd = b_enable ? 'F' : 'H';

        int n_try = 100;
		int n_report_out = _mp::_elpusk::_lpu237::const_size_report_out_except_id+1; // 1 is report id

        _mp::type_v_buffer v_tx(n_report_out, 0);
        int n_written = 0;
        int n_totoal_w(0);

		v_tx[1] = c_cmd; // request command

        // TX
        do {
            n_written = hid_write(p_dev, &v_tx[n_totoal_w], v_tx.size()- n_totoal_w);
            if (n_written < 0) {
                // error
                break;
            }

            n_totoal_w += n_written;
            std::this_thread::sleep_for(std::chrono::milliseconds(ll_check_tx_interval_mmsec));
            --n_try;
            if( n_try <= 0) {
                // error
                break;
			}
        } while (n_totoal_w < n_report_out);

        if (n_totoal_w < n_report_out) {
			continue; //error
        }

        // RX . Good - 0xFF
        n_try = 200;
#ifdef _WIN32
        int n_report_in = _mp::_elpusk::_lpu237::const_size_report_in_except_id;
        int n_rx_start = 0;
#else
        int n_report_in = _mp::_elpusk::_lpu237::const_size_report_in_except_id + 1; // 1 is report id
        int n_rx_start = 1;
#endif

        _mp::type_v_buffer v_rx(n_report_in, 0);
        int n_read = 0;
        int n_totoal_r(0);

        do {
            n_read = hid_read(p_dev, &v_rx[n_totoal_r], v_rx.size() - n_totoal_r);
            if (n_read < 0) {
                // error
                break;
            }

			n_totoal_r += n_read;
            std::this_thread::sleep_for(std::chrono::milliseconds(ll_check_rx_interval_mmsec));
            --n_try;
            if (n_try <= 0) {
                // error
                break;
            }
        } while (n_totoal_r < n_report_in);

        if (n_totoal_r < n_report_in) {
            continue; //error
        }

        if( v_rx[n_rx_start] != 'R' || v_rx[n_rx_start+1] != 0xFF) {
			continue; // error
        }

        b_result = true;
    } while (false);
    return b_result;
}

int _hid_api_briage::api_set_nonblocking(int n_primitive_map_index, int nonblock)
{
    std::lock_guard<std::mutex> lock(_hid_api_briage::get_mutex_for_hidapi());

    hid_device* p_dev = nullptr;
    if (m_map_hid_dev.find(n_primitive_map_index) != std::end(m_map_hid_dev)) {
        p_dev = m_map_hid_dev[n_primitive_map_index].first;
        return hid_set_nonblocking(p_dev, nonblock);
    }
    else {
        return -1; //error
    }
    
}


int _hid_api_briage::api_get_report_descriptor(int n_primitive_map_index, unsigned char* buf, size_t buf_size)
{
    std::lock_guard<std::mutex> lock(_hid_api_briage::get_mutex_for_hidapi());

    if (!m_b_ini) {
        return -1;
    }

    hid_device* p_dev = nullptr;
    if (m_map_hid_dev.find(n_primitive_map_index) != std::end(m_map_hid_dev)) {
        p_dev = m_map_hid_dev[n_primitive_map_index].first;
    }
    return hid_get_report_descriptor(p_dev, buf, buf_size);
}

int _hid_api_briage::api_write(int n_primitive_map_index, const unsigned char* data, size_t length, _mp::type_next_io next)
{
    std::lock_guard<std::mutex> lock(_hid_api_briage::get_mutex_for_hidapi());

    if (!m_b_ini) {
        return -1;
    }

    hid_device* p_dev = nullptr;
    if (m_map_hid_dev.find(n_primitive_map_index) != std::end(m_map_hid_dev)) {
        p_dev = m_map_hid_dev[n_primitive_map_index].first;
    }

    int n_written = hid_write(p_dev, data, length);

#if defined(_WIN32) && defined(_DEBUG) && defined(__THIS_FILE_ONLY__)
    if (n_written > 0) {
        ATLTRACE(L" !!!!! api_write(n_primitive_map_index:%d,length:%u)->written:%d.\n", n_primitive_map_index, length, n_written);
    
        std::wstringstream woss;
        for (int i = 0; i < n_written; i++) {
            woss << std::hex << data[i];
            woss << L'.';
        }//end for

        ATLTRACE(L" !!!!! %ls.\n", woss.str().c_str());
    }
#endif
    return n_written;
}

int _hid_api_briage::api_read(int n_primitive_map_index, unsigned char* data, size_t length, size_t n_report)
{
    std::lock_guard<std::mutex> lock(_hid_api_briage::get_mutex_for_hidapi());

    if (!m_b_ini) {
        _mp::clog::get_instance().log_fmt(L"[E] %ls : not m_b_ini.\n", __WFUNCTION__);
        return -1;
    }

    hid_device* p_dev = nullptr;
    if (m_map_hid_dev.find(n_primitive_map_index) != std::end(m_map_hid_dev)) {
        p_dev = m_map_hid_dev[n_primitive_map_index].first;
    }

    int n_result = hid_read(p_dev, data, length);
    if (n_result < 0) {
        _mp::clog::get_instance().log_fmt(L"[E] %ls : n_result = %d.\n", __WFUNCTION__, n_result);
    }
    
#if defined(_WIN32) && defined(_DEBUG) && defined(__THIS_FILE_ONLY__)
    if (n_result > 0) {
        ATLTRACE(L" !!!!! api_read(n_primitive_map_index:%d,length:%u,n_report:%u)->read:%d.\n", n_primitive_map_index, length, n_report, n_result);
    
        std::wstringstream woss;
        for (int i = 0; i < n_result; i++) {
            woss << std::hex << data[i];
            woss << L'.';
        }//end for

        ATLTRACE(L" !!!!! %ls.\n", woss.str().c_str());
    }
#endif

#if defined(_DEBUG) && defined(__VIRTUAL_IBUTTON_DATA__)
    else {
        do {
            // 빠른 실행을 위한 코드
            if (n_result <= 0) {
                continue;
            }
            if (data == NULL) {
                continue;
            }

            // debugging 을 위해, i-button 가상 응답 설정.
            const std::string s_ibutton_postfix("this_is_ibutton_data");
            const size_t n_size_button_data(8);
            const size_t n_len_bytes = 3;
            _mp::type_v_buffer v_code(0);
            if (n_result < (n_len_bytes + n_size_button_data + s_ibutton_postfix.size())) {
                continue;
            }
            if (length < n_len_bytes + n_size_button_data + s_ibutton_postfix.size()) {
                continue;
            }

            std::array<char, 3> ar{ data[0], data[1] ,data[2] };
            if (ar[0] >= 0) {
                continue;
            }
            if (ar[1] >= 0) {
                continue;
            }
            if (ar[2] >= 0) {
                continue;
            }

            // ISO 1,2,3 모두 에러이거나 msr extension format 일때, 가상 ibutton 응답 전송. 
            // ibutton rsp
            // 0 0 0 + 8 bytes + "this_is_ibutton_data"
            size_t i = 0;
            for (i = 0; i < n_len_bytes; i++) {
                data[i] = 0;
            }
            for (; i < n_len_bytes + n_size_button_data; i++) {
                data[i] = 0xF0 + i - n_len_bytes;
            }

            for (; i < n_len_bytes + n_size_button_data + s_ibutton_postfix.size(); i++) {
                data[i] = s_ibutton_postfix[i - (n_len_bytes + n_size_button_data)];
            }
            // n_result 를 변경 할 필요 없음.
            
            std::fill(&data[i], &data[length], 0);// clear remainder buffer.

        } while (false);
    }

#endif// _DEBUG && __VIRTUAL_IBUTTON_DATA__
	return n_result;
}

const wchar_t* _hid_api_briage::api_error(int n_primitive_map_index)
{
    std::lock_guard<std::mutex> lock(_hid_api_briage::get_mutex_for_hidapi());

    if (!m_b_ini) {
        return NULL;
    }

    hid_device* p_dev = nullptr;
    if (m_map_hid_dev.find(n_primitive_map_index) != std::end(m_map_hid_dev)) {
        p_dev = m_map_hid_dev[n_primitive_map_index].first;
    }
    return hid_error(p_dev);
}

_hid_api_briage& _hid_api_briage::set_req_q_check_interval_in_child(long long n_interval_mmsec)
{
	m_atll_req_q_check_interval_mmsec.store(n_interval_mmsec, std::memory_order_relaxed);
    return *this;
}

_hid_api_briage& _hid_api_briage::set_hid_write_interval_in_child(long long n_interval_mmsec)
{
	m_atll_hid_write_interval_mmsec.store(n_interval_mmsec, std::memory_order_relaxed);
	return *this;
}
_hid_api_briage& _hid_api_briage::set_hid_read_interval_in_child(long long n_interval_mmsec)
{
    m_atll_hid_read_interval_mmsec.store(n_interval_mmsec, std::memory_order_relaxed);
    return *this;
}

long long _hid_api_briage::get_req_q_check_interval_in_child() const
{
	return m_atll_req_q_check_interval_mmsec.load(std::memory_order_relaxed);
}

long long _hid_api_briage::get_hid_write_interval_in_child() const
{
    return m_atll_hid_write_interval_mmsec.load(std::memory_order_relaxed);
}
long long _hid_api_briage::get_hid_read_interval_in_child() const
{
    return m_atll_hid_read_interval_mmsec.load(std::memory_order_relaxed);
}
