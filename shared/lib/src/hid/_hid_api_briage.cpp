#include <mp_cstring.h>
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
            _mp::clog::get_instance().log_fmt(L"[W] %ls : No HID devices found in the system.\n", __WFUNCTION__);
        }
        else {
            _mp::clog::get_instance().log_fmt(L"[W] %ls : No HID devices with requested VID/PID found in the system.\n", __WFUNCTION__);
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
    m_s_class_name(L"_hid_api_briage"),
    m_b_ini(false), 
    m_n_map_index(_vhid_info::const_map_index_invalid)
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

    struct hid_device_info* p_devs = NULL;

    do {
        std::lock_guard<std::mutex> lock(_hid_api_briage::get_mutex_for_hidapi());
        if (!m_b_ini) {
            continue;
        }
        /**
        * don't use the hid_enumerate() of hidapi. it will occur packer-losting.
        */
        p_devs = _hid_enumerate(m_ptr_usb_lib->get_context(), 0x0, 0x0);

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
        _hid_free_enumeration(p_devs);
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
        return n_map_index;
    }
    else {
        return -1;
    }
}

void _hid_api_briage::api_close(int n_map_index)
{
    std::lock_guard<std::mutex> lock(_hid_api_briage::get_mutex_for_hidapi());

    if (!m_b_ini) {
        return;
    }

    hid_device* p_dev = nullptr;
    if (m_map_hid_dev.find(n_map_index) != std::end(m_map_hid_dev)) {
        p_dev = m_map_hid_dev[n_map_index].first;
        m_map_hid_dev.erase(n_map_index);
        hid_close(p_dev);
    }
}

int _hid_api_briage::api_set_nonblocking(int n_map_index, int nonblock)
{
    std::lock_guard<std::mutex> lock(_hid_api_briage::get_mutex_for_hidapi());

    hid_device* p_dev = nullptr;
    if (m_map_hid_dev.find(n_map_index) != std::end(m_map_hid_dev)) {
        p_dev = m_map_hid_dev[n_map_index].first;
    }
    return hid_set_nonblocking(p_dev, nonblock);
}


int _hid_api_briage::api_get_report_descriptor(int n_map_index, unsigned char* buf, size_t buf_size)
{
    std::lock_guard<std::mutex> lock(_hid_api_briage::get_mutex_for_hidapi());

    if (!m_b_ini) {
        return -1;
    }

    hid_device* p_dev = nullptr;
    if (m_map_hid_dev.find(n_map_index) != std::end(m_map_hid_dev)) {
        p_dev = m_map_hid_dev[n_map_index].first;
    }
    return hid_get_report_descriptor(p_dev, buf, buf_size);
}

int _hid_api_briage::api_write(int n_map_index, const unsigned char* data, size_t length, _hid_api_briage::type_next_io next)
{
    std::lock_guard<std::mutex> lock(_hid_api_briage::get_mutex_for_hidapi());

    if (!m_b_ini) {
        return -1;
    }

    hid_device* p_dev = nullptr;
    if (m_map_hid_dev.find(n_map_index) != std::end(m_map_hid_dev)) {
        p_dev = m_map_hid_dev[n_map_index].first;
    }
    return hid_write(p_dev, data, length);
}

int _hid_api_briage::api_read(int n_map_index, unsigned char* data, size_t length, size_t n_report)
{
    std::lock_guard<std::mutex> lock(_hid_api_briage::get_mutex_for_hidapi());

    if (!m_b_ini) {
        return -1;
    }

    hid_device* p_dev = nullptr;
    if (m_map_hid_dev.find(n_map_index) != std::end(m_map_hid_dev)) {
        p_dev = m_map_hid_dev[n_map_index].first;
    }

    int n_result = hid_read(p_dev, data, length);
#ifdef _WIN32
#ifdef _DEBUG
    if(n_result >0 ){
        if (data[0] == 'R') {
            ATLTRACE(L"0x%08X-RX[0] is 'R'.\n", n_map_index);
        }
    }
    
#endif
#endif
	return n_result;
}

const wchar_t* _hid_api_briage::api_error(int n_map_index)
{
    std::lock_guard<std::mutex> lock(_hid_api_briage::get_mutex_for_hidapi());

    if (!m_b_ini) {
        return NULL;
    }

    hid_device* p_dev = nullptr;
    if (m_map_hid_dev.find(n_map_index) != std::end(m_map_hid_dev)) {
        p_dev = m_map_hid_dev[n_map_index].first;
    }
    return hid_error(p_dev);
}
