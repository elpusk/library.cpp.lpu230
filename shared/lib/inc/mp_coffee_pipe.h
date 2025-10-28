#pragma once

#include <string>
#include <cstdint>
#include <tuple>
#include <iostream>
#include <sstream>
#include <iomanip>

#ifndef _WIN32
#include <unistd.h>
#endif

#include <mp_coffee.h>
#include <mp_cstring.h>

namespace _mp{

    /**
    * pipe control class of coffee manager 
    */
    class ccoffee_pipe
    {
    public:

        /**
        * @brief request string is server-terminate-request or not
        * @param s_request : request string
        */
        static bool is_ctl_request_for_terminate_server(const std::wstring& s_request)
        {
            if (s_request.compare(_mp::_coffee::CONST_S_COFFEE_MGMT_CTL_REQ) == 0) {
                return true;
            }
            return false;
        }

        /**
        * @brief request string is consider-to-removed or not
        * @param s_request : request string
        * @return first(true) : request is consider-to-removed.
        * @return second : usb vendor id.
        * @return third : usb product id.
        */
        static std::tuple<bool,int,int> is_ctl_request_for_consider_to_removed(const std::wstring& s_request)
        {
            bool b_result(false);
            int n_vid = 0, n_pid =0;

            do {
                size_t vid_pos = s_request.find(L"VID_");
                if (vid_pos == std::wstring::npos) {
                    continue;
                }
                size_t pid_pos = s_request.find(L"PID_");
                if (pid_pos == std::wstring::npos) {
                    continue;
                }
                std::wstring s_prefix = s_request.substr(0, vid_pos); // "DEV_STOP:USB:"
                if (s_prefix.compare(_mp::_coffee::CONST_S_COFFEE_MGMT_CTL_REQ_STOP_DEV_PREFIX) != 0) {
                    continue;
                }

                try {
                    std::wstring vid_hex = s_request.substr(vid_pos + 4, 4);
                    n_vid = std::stoi(vid_hex, nullptr, 16);

                    std::wstring pid_hex = s_request.substr(pid_pos + 4, 4);
                    n_pid = std::stoi(pid_hex, nullptr, 16);
                }
                catch (...) {
                    // all error
                    continue;
                }

                if (n_vid == 0xFFFF) {
                    n_vid = -1;
                }
                if (n_pid == 0xFFFF) {
                    n_pid = -1;
                }
                b_result = true;
            } while (false);
            return std::make_tuple(b_result, n_vid, n_pid);
        }

        /**
        * @brief request string is cancel-consider-to-removed or not
        * @param s_request : request string
        * @return first(true) : request is consider-to-removed.
        * @return second : usb vendor id.
        * @return third : usb product id.
        */
        static std::tuple<bool, int, int> is_ctl_request_for_cancel_consider_to_removed(const std::wstring& s_request)
        {
            bool b_result(false);
            int n_vid = 0, n_pid = 0;

            do {
                size_t vid_pos = s_request.find(L"VID_");
                if (vid_pos == std::wstring::npos) {
                    continue;
                }
                size_t pid_pos = s_request.find(L"PID_");
                if (pid_pos == std::wstring::npos) {
                    continue;
                }
                std::wstring s_prefix = s_request.substr(0, vid_pos); // "DEV_STOP:USB:"
                if (s_prefix.compare(_mp::_coffee::CONST_S_COFFEE_MGMT_CTL_REQ_START_DEV_PREFIX) != 0) {
                    continue;
                }

                try {
                    std::wstring vid_hex = s_request.substr(vid_pos + 4, 4);
                    n_vid = std::stoi(vid_hex, nullptr, 16);

                    std::wstring pid_hex = s_request.substr(pid_pos + 4, 4);
                    n_pid = std::stoi(pid_hex, nullptr, 16);
                }
                catch (...) {
                    // all error
                    continue;
                }

                if (n_vid == 0xFFFF) {
                    n_vid = -1;
                }
                if (n_pid == 0xFFFF) {
                    n_pid = -1;
                }
                b_result = true;
            } while (false);
            return std::make_tuple(b_result, n_vid, n_pid);
        }

        /**
        * @brief generate the control request which manager consider a device to be removed.
        * @param n_vid : usb vendor id
        * @param n_pid : usb product id
        * @return th generted request-string
        */
        static std::wstring generate_ctl_request_for_consider_to_removed(int n_vid, int n_pid)
        {
            std::wstring s;
            uint16_t w_v, w_p;

            if (n_vid < 0) {
                w_v = 0xFFFF;
            }
            else if (n_vid > 0xFFFF) {
                w_v = 0;
            }
            else {
                w_v = (uint16_t)n_vid;
            }

            if (n_pid < 0) {
                w_p = 0xFFFF;
            }
            else if (n_pid > 0xFFFF) {
                w_p = 0;
            }
            else {
                w_p = (uint16_t)n_pid;
            }

            _mp::cstring::format_c_style(s, L"%lsVID_%04X:PID_%04X",_mp::_coffee::CONST_S_COFFEE_MGMT_CTL_REQ_STOP_DEV_PREFIX, w_v, w_p);
            return s;
        }

        /**
        * @brief generate the control cancel request which manager  consider a device to be removed.
        * @param n_vid : usb vendor id. -1 : all vendor
        * @param n_pid : usb product id. -1 : all product
        * @return th generted request-string
        */
        static std::wstring generate_ctl_request_for_cancel_considering_dev_as_removed(int n_vid = -1, int n_pid = -1)
        {
            std::wstring s;
            uint16_t w_v, w_p;

            if (n_vid < 0) {
                w_v = 0xFFFF;
            }
            else if (n_vid > 0xFFFF) {
                w_v = 0;
            }
            else {
                w_v = (uint16_t)n_vid;
            }

            if (n_pid < 0) {
                w_p = 0xFFFF;
            }
            else if (n_pid > 0xFFFF) {
                w_p = 0;
            }
            else {
                w_p = (uint16_t)n_pid;
            }

            _mp::cstring::format_c_style(s, L"%lsVID_%04X:PID_%04X", _mp::_coffee::CONST_S_COFFEE_MGMT_CTL_REQ_START_DEV_PREFIX, w_v, w_p);
            return s;
        }

    private:
        ccoffee_pipe() = delete;
        ccoffee_pipe(const ccoffee_pipe&) = delete;
        ccoffee_pipe& operator=(const ccoffee_pipe&) = delete;
	};
}