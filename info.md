# Software components information
release 2.3

## LiMgmtLpu230
- for Debian 12.(x64)
- executable file name : mgmt_lpu230
- lpu237 device library test program.

## WiMgmtLpu230
- for windows 11.(x64, x86)
- executable file name : mgmt_lpu230.exe
- lpu237 device library test program.

## LiElpuskHidDaemon
- v2.3
- for Debian 12.(x64)
- executable file name : elpusk-hid-d
- secure websoecket server deamon.(only single instance) with /server option or none(default)
- supports lpu237 device IO.
- supports generation & installation self-signed certificate. with /cert option.
- supports remove self-signed certificate. with /removecert option.
- display runtime tracking message of the current elpusk-hid-d. with /trace option.
- supports lpu237-fw update by webapp.

## WiElpuskHidDaemon
- v2.3
- for windows 11.(x64, x86)
- executable file name : elpusk-hid-d.exe
- secure websoecket server exe file.(only single instance) with /server option or none(default)
- this exe will be executed by service program.
- supports lpu237 device IO.
- supports generation & installation self-signed certificate. with /cert option.
- supports remove self-signed certificate. with /removecert option.
- display runtime tracking message of the current elpusk-hid-d. with /trace option.
- supports lpu237-fw update by webapp.

## li_lpu237_dll
- v6.0
- for Debian 12.(x64)
- executable file name : libtg_lpu237_dll.so
- for native client application, shared object.
- supports lpu237 msr functionality.

## wi_lpu237_dll
- v6.0
- for windows 11.(x64, x86)
- executable file name : tg_lpu237_dll.dll
- for native client application, dynamic linked library.
- supports lpu237 msr functionality.

## li_lpu237_ibutton
- v6.0
- for Debian 12.(x64)
- executable file name : libtg_lpu237_ibutton.so
- for native client application, shared object.
- supports lpu237 ibutton functionality.

## wi_lpu237_ibutton
- v6.0
- for windows 11.(x64, x86)
- executable file name : tg_lpu237_ibutton.dll
- for native client application, dynamic linked library.
- supports lpu237 ibutton functionality.

## tp_li_lpu237
- for Debian 12.(x64)
- executable file name : tp_li_lpu237.out
- libtg_lpu237_dll.so test program.(msr test)

## tp_wi_lpu237
- for windows 11.(x64, x86)
- executable file name : tp_wi_lpu237.exe
- tg_lpu237_dll.dll test program.(msr test)

## li_lpu237_update
- v2.2
- for Debian 12.(x64)
- executable file name : lpu237_update.out or lpu237_update
- lpu237 firmware update program.
- can be executed by elpusk-hid-d.

## wi_lpu237_update
- v2.2
- for windows 11.(x64, x86)
- executable file name : lpu237_update.exe
- lpu237 firmware update program.
- can be executed by elpusk-hid-d.exe.

## li_rom
- v1.2
- for Debian 12.(x64)
- executable file name : libtg_rom.so
- rom file helper dynamic linked library.

## wi_rom
- v1.2
- for windows 11.(x64, x86)
- executable file name : tg_rom.dll
- rom file helper dynamic linked library.

## li_rom_build
- for Debian 12.(x64)
- executable file name : tg_rom_build.out
- rom file build program.

## wi_rom_build
- for windows 11.(x64, x86)
- executable file name : tg_rom_build.exe
- rom file build program.

## wi_dev_lib
- v1.0
- for windows 11.(x64, x86)
- executable file name : dev_lib.dll
- for native client application, dynamic linked library.
- for elpusk-hid-d.exe and lpu230_update.exe, service hid device io.

## li_dev_lib
- v1.0
- for Debian 12.(x64)
- executable file name : libdev_lib.so
- for native client application, shared object.
- for elpusk-hid-d and lpu230_update(lpu230_update.out), service hid device io.

