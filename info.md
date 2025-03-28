# Software components information

## LiMgmtLpu230
- for Debian 12.(x64)
- executable file name : mgmt_lpu230
- lpu237 device library test program.

## WiMgmtLpu230
- for windows 11.(x64, x86)
- executable file name : mgmt_lpu230.exe
- lpu237 device library test program.

## LiElpuskHidDaemon
- for Debian 12.(x64)
- executable file name : elpusk-hid-d
- secure websoecket server deamon.(only single instance) with /server option or none(default)
- supports lpu237 device IO.
- supports generation & installation self-signed certificate. with /cert option.
- supports remove self-signed certificate. with /removecert option.
- display runtime tracking message of the current elpusk-hid-d. with /trace option.

## WiElpuskHidDaemon
- for windows 11.(x64, x86)
- executable file name : elpusk-hid-d.exe
- secure websoecket server exe file.(only single instance) with /server option or none(default)
- this exe will be executed by service program.
- supports lpu237 device IO.
- supports generation & installation self-signed certificate. with /cert option.
- supports remove self-signed certificate. with /removecert option.
- display runtime tracking message of the current elpusk-hid-d. with /trace option.

## li_lpu237_dll
- for Debian 12.(x64)
- executable file name : libtg_lpu237_dll.so
- for native client application, shared object.
- supports lpu237 msr functionality.

## wi_lpu237_dll
- for windows 11.(x64, x86)
- executable file name : tg_lpu237_dll.dll
- for native client application, dynamic linked library.
- supports lpu237 msr functionality.

## tp_li_lpu237
- for Debian 12.(x64)
- executable file name : tp_li_lpu237.out
- libtg_lpu237_dll.so test program.(msr test)

## tp_wi_lpu237
- for windows 11.(x64, x86)
- executable file name : tp_wi_lpu237.exe
- tg_lpu237_dll.dll test program.(msr test)

