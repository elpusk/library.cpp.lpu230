# Software components information
package release 2.14

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
- supports lpu237-fw update by webapp.
- "In Linux package version 2.6, after stopping the service with systemctl stop, running elpusk-hid-d --removecert followed by elpusk-hid-d --cert generates a certificate. However, executing it results in an authentication certificate error." -> remove the installed pkg and reinstall it.

## WiElpuskHidDaemon
- for windows 11.(x64, x86)
- executable file name : elpusk-hid-d.exe
- secure websoecket server exe file.(only single instance) with /server option or none(default)
- this exe will be executed by service program.
- supports lpu237 device IO.
- supports generation & installation self-signed certificate. with /cert option.
- supports remove self-signed certificate. with /removecert option.
- display runtime tracking message of the current elpusk-hid-d. with /trace option.
- supports lpu237-fw update by webapp.

## wi_coffee_service
- for windows 11.(x64, x86)
- executable file name : coffee_service.exe
- Windows service program.
- run and stop elpusk-hid-d.exe

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

## li_lpu237_ibutton
- for Debian 12.(x64)
- executable file name : libtg_lpu237_ibutton.so
- for native client application, shared object.
- supports lpu237 ibutton functionality.

## wi_lpu237_ibutton
- for windows 11.(x64, x86)
- executable file name : tg_lpu237_ibutton.dll
- for native client application, dynamic linked library.
- supports lpu237 ibutton functionality.

## li_lpu237_fw
- from package release 2.11
- for Debian 12.(x64)
- executable file name : libtg_lpu237_fw.so
- for native client application, shared object.
- supports lpu237 firmware update functionality.

## wi_lpu237_fw
- from package release 2.11
- for windows 11.(x64, x86)
- executable file name : tg_lpu237_fw.dll
- for native client application, dynamic linked library.
- supports lpu237 firmware update functionality.

## wi_lpu237_tools
- from package release 2.14
- for windows 11.(x86)
- executable file name : tg_lpu237_tools.dll
- for native client application, dynamic linked library.
- supports lpu237 basic setting functionality.

## tp_li_lpu237
- for Debian 12.(x64)
- executable file name : tp_li_lpu237.out
- libtg_lpu237_dll.so test program.(msr test)

## tp_wi_lpu237
- for windows 11.(x64, x86)
- executable file name : tp_wi_lpu237.exe
- tg_lpu237_dll.dll test program.(msr test)

## li_lpu237_update
- for Debian 12.(x64)
- executable file name : lpu237_update.out or lpu237_update
- lpu237 firmware update program.
- can be executed by elpusk-hid-d.

## wi_lpu237_update
- for windows 11.(x64, x86)
- executable file name : lpu237_update.exe
- lpu237 firmware update program.
- can be executed by elpusk-hid-d.exe.

## li_rom
- for Debian 12.(x64)
- executable file name : libtg_rom.so
- rom file helper dynamic linked library.

## wi_rom
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
- for windows 11.(x64, x86)
- executable file name : dev_lib.dll
- for native client application, dynamic linked library.
- for elpusk-hid-d.exe and lpu230_update.exe, service hid device io.

## li_dev_lib
- for Debian 12.(x64)
- executable file name : libdev_lib.so
- for native client application, shared object.
- for elpusk-hid-d and lpu230_update(lpu230_update.out), service hid device io.

## OPOS Service Object

### OposLpu230
- from package release 2.14
- for windows 11.(x86)
- executable file name : OposLpu230.dll
- for OPOS CCO, COM object.
- OPOS Msr Service Object of lpu237.

### OposLpu230Lock
- from package release 2.14
- for windows 11.(x86)
- executable file name : OposLpu230Lock.dll
- for OPOS CCO, COM object.
- OPOS Keylock Service Object of lpu237.

## MCP

### ibutton-mcp
- for win11(x64,Claude Desktop and Cursor) & Debian 12.(x64, Cursor)  
- executable file name : lpu23x-ibutton-mcp.exe(lpu23x-ibutton-mcp)
- AI agent MCP of magnetic card read(tg_lpu237_dll.dll/libtg_lpu237_dll.so)
- the description of Tool is dynamic loading from lpu23x-ibutton-mcp.json

### msr-mcp
- for win11(x64,Claude Desktop and Cursor) & Debian 12.(x64, Cursor)
- executable file name : lpu23x-msr-mcp.exe(lpu23x-msr-mcp)
- AI agent MCP of i-button read(tg_lpu237_ibutton.dll/libtg_lpu237_ibutton.so)
- the description of Tool is dynamic loading from lpu23x-msr-mcp.json
