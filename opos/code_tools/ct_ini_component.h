#pragma once

#include <array>
#include <ct_type.h>
#include <ct_convert.h>
#include <ct_file.h>
#include <ct_system.h>

namespace _ns_tools
{
	class ct_ini_component
	{
	public:
        ~ct_ini_component() {}

        static ct_ini_component& get_instance()
        {
            static ct_ini_component obj;
            return obj;
        }

        HMODULE get_module() const
        {
            return m_h_module;
        }
        ct_ini_component& set_module(HMODULE h_module)
        {
            do {
                m_h_module = h_module;
                m_s_cur_dll_file_name.clear();

                if (!m_h_module) {
                    continue;
                }
                std::vector <wchar_t> v_n(MAX_PATH + 1, 0);
                if (GetModuleFileName(m_h_module, &v_n[0], v_n.size()) == 0) {
                    continue;
                }
                std::wstring s_n = &v_n[0];

                m_s_cur_module = s_n;

                std::wstring s_out_drive;
                std::wstring s_out_dir;
                std::wstring s_out_file_name;
                std::wstring s_out_ext;

                if (!_ns_tools::ct_file::split_path(s_n, s_out_drive, s_out_dir, s_out_file_name, s_out_ext)) {
                    continue;
                }
                m_s_cur_dll_file_name = s_out_file_name;
                m_s_cur_folder_ini_abs_file_path = s_out_drive + s_out_dir + s_out_file_name + L".ini";
            } while (false);

            return *this;
        }

        /**
        * set_old_ini_file - ex) lpu230_api.ini or lpu230_fw.ini .......
        */
        ct_ini_component& set_old_ini_file(const std::wstring& s_ini_file_name_and_ext)
        {
            m_s_old_ini_file_name_and_ext = s_ini_file_name_and_ext;
            return *this;
        }

        std::wstring get_cur_module_name() const
        {
            return m_s_cur_module;
        }
        std::wstring get_old_ini_file_name_and_ext() const
        {
            return m_s_old_ini_file_name_and_ext;
        }

        /**
        * if c:\return\abc.dll -> return abc
        */
        std::wstring get_cur_module_file_name() const
        {
            return m_s_cur_dll_file_name;
        }

        bool load_ini_file()
        {
            bool b_result(false);

            do {
                std::wstring s_ini_abs_path(get_ini_path());
                if (s_ini_abs_path.empty()) {
                    continue;
                }
                //
                DWORD dw_result;
                TCHAR s_value[_MAX_PATH];
                int nValue = 0;

                //setting enable property.......
                dw_result = ::GetPrivateProfileString(L"control", L"io", L"0", s_value, _MAX_PATH, s_ini_abs_path.c_str());
                if (dw_result == 0) {
                    continue;
                }

                nValue = _tstoi(s_value);

                switch (nValue) {
                case 1://dircet mode
                    m_device_io_mode = _ns_tools::device_io_direct;
                    break;
                case 2://NDM mode.
                    m_device_io_mode = _ns_tools::device_io_ng_devmgmt;
                    break;
                default://automation mode
                    b_result = false;
                    continue;;
                }//end switch

                b_result = true;
            } while (false);

            if (!b_result) {
                if (_is_NDM()) {
                    m_device_io_mode = _ns_tools::device_io_ng_devmgmt;
                }
                else {
                    m_device_io_mode = _ns_tools::device_io_direct;
                }
            }

            return b_result;
        }

        _ns_tools::type_device_io_mode get_io_type() const
        {
            return m_device_io_mode;
        }
        std::wstring get_ini_path()
        {
            std::wstring s_abs_ini;

            do {
                std::wstring s_path;
                if (m_s_cur_dll_file_name.empty())
                    continue;
                
                if (_ns_tools::ct_file::is_exist_file(m_s_cur_folder_ini_abs_file_path)) {
                    s_abs_ini = m_s_cur_folder_ini_abs_file_path;
                    continue;
                }

                s_path = _ns_tools::ct_file::get_default_initial_file_abs_path_without_backslash_and_file_name(L"Elpusk", 6, m_s_cur_dll_file_name);
                s_path += L"\\";	s_path += m_s_cur_dll_file_name;	s_path += L".ini";
                if (_ns_tools::ct_file::is_exist_file(s_path)) {
                    s_abs_ini = s_path;
                    continue;
                }

                s_path = _ns_tools::ct_file::get_default_initial_file_abs_path_without_backslash_and_file_name(L"Easyset", 0, m_s_cur_dll_file_name);
                s_path += L"\\";	s_path += m_s_cur_dll_file_name;	s_path += L".ini";
                if (_ns_tools::ct_file::is_exist_file(s_path)) {
                    s_abs_ini = s_path;
                    continue;
                }

                if (m_s_old_ini_file_name_and_ext.empty())
                    continue;
                //
                TCHAR sPersonalFolder[MAX_PATH];
                TCHAR sIniPath[MAX_PATH];
                std::wstring s_ini_path;

                if (FAILED(
                    ::SHGetFolderPath(NULL,
                        CSIDL_PERSONAL | CSIDL_FLAG_CREATE,
                        NULL,
                        SHGFP_TYPE_CURRENT,
                        sPersonalFolder
                    )))
                    continue;
                //
                ::_tcscpy(sIniPath, sPersonalFolder);
                s_ini_path = sIniPath;
                s_ini_path += L"\\Elpusk\\lpu230\\";
                s_ini_path += m_s_old_ini_file_name_and_ext;
                if (_ns_tools::ct_file::is_exist_file(s_ini_path)) {
                    s_abs_ini = s_ini_path;
                    continue;
                }
                //
                s_ini_path = sIniPath;
                s_ini_path += L"\\Easyset\\lpu230\\";
                s_ini_path += m_s_old_ini_file_name_and_ext;
                if (_ns_tools::ct_file::is_exist_file(s_ini_path)) {
                    s_abs_ini = s_ini_path;
                    continue;
                }

            } while (false);
            return s_abs_ini;
        }

    private:
        ct_ini_component() :
            m_device_io_mode(_ns_tools::device_io_ng_devmgmt),
            m_h_module(NULL)
        {
            if (_is_NDM()) {
                m_device_io_mode = _ns_tools::device_io_ng_devmgmt;
            }
            else {
                m_device_io_mode = _ns_tools::device_io_direct;
            }
        }

        // check function
        // NDK is setup OK?
        bool _is_NDM()
        {
            bool b_result = false;

            //GetNameNDMSetupOk()
            std::wstring s_mutex_name_NDM_setup_ok(L"_Next_Generation_NDM_setup_ok_20101006_");
            if (_ns_tools::ct_system::is_exist_named_mutex(s_mutex_name_NDM_setup_ok))
                b_result = true;

            return b_result;
        }

    private:
        _ns_tools::type_device_io_mode m_device_io_mode;
        HMODULE m_h_module;
        std::wstring m_s_old_ini_file_name_and_ext;
        std::wstring m_s_cur_dll_file_name;
        std::wstring m_s_cur_folder_ini_abs_file_path;
        std::wstring m_s_cur_module;

    private://don't call these methods
        ct_ini_component(const ct_ini_component&);
        ct_ini_component& operator=(const ct_ini_component&);

	};
}
