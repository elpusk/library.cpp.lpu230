#pragma once

namespace _mp{
    namespace _elpusk{

        enum : int {
            const_usb_vid = 0x134b
        };
        enum : int {
            const_usb_pid_hidbl = 0x0fff, //elpusk hid bootloader vid
            const_usb_inf_hidbl = -1 //elpusk hid bootloader inteface(none)
        };
        enum : int {
            const_size_hidbl_report_in_except_id = 64,
        };
        enum : int {
            const_size_hidbl_report_out_except_id = 64
        };

        namespace _lpu237 {
            enum : int {
                const_usb_pid = 0x0206,
                const_usb_inf_hid = 1,
                const_size_report_in_except_id = 220,
                const_size_report_out_except_id = 64
            };
        }
        namespace _lpu238 {
            enum : int {
                const_usb_pid = 0x0214,
                const_usb_inf_hid = 0,
                const_usb_inf_vcom = 1,
                const_size_report_in_except_id = 220,
                const_size_report_out_except_id = 64
            };

        }
       

    }
}