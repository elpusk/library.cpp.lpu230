# Software components information

package release 2.16

## LiMgmtLpu230

- *Not distributed as an internal test program*
- for Debian 12.(x64)
- executable file name : mgmt_lpu230
- lpu237 device library test program.
- for changing version, none.

## WiMgmtLpu230

- *Not distributed as an internal test program*
- for windows 11.(x64, x86)
- executable file name : mgmt_lpu230.exe
- lpu237 device library test program.
- for changing version, none.

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
- for changing version, change
  - mp_coffee.h : CONST_S_COFFEE_MGMT_VERSION
  - mp_coffee.h : CONST_WS_COFFEE_MGMT_VERSION

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
- for changing version, change
  - mp_coffee.h : CONST_S_COFFEE_MGMT_VERSION
  - mp_coffee.h : CONST_WS_COFFEE_MGMT_VERSION
  - version resource

## wi_coffee_service

- for windows 11.(x64, x86)
- executable file name : coffee_service.exe
- Windows service program.
- run and stop elpusk-hid-d.exe
- for changing version, change version resource

## li_lpu237_dll

- for Debian 12.(x64)
- executable file name : libtg_lpu237_dll.so
- for native client application, shared object.
- supports lpu237 msr functionality.
- for changing version, change
  - vs2022 project property ：　Target Extension
  - vs2022 project property ： Linker-Command Line : -soname option(major version only)
  - vs2022 project property ： Build Events-Remote Post-Build Event : Command Line option(major version only) as like

  ``` shell
  rm $(RemoteOutDir)$(TargetName).so;rm $(RemoteOutDir)$(TargetName).so.6;ln -sf $(RemoteOutDir)$(TargetFileName) $(RemoteOutDir)$(TargetName).so.6;ln -sf $(RemoteOutDir)$(TargetName).so.6 $(RemoteOutDir)$(TargetName).so
  ```

## wi_lpu237_dll

- for windows 11.(x64, x86)
- executable file name : tg_lpu237_dll.dll
- for native client application, dynamic linked library.
- supports lpu237 msr functionality.
- for changing version, change version resource

## li_lpu237_ibutton

- for Debian 12.(x64)
- executable file name : libtg_lpu237_ibutton.so
- for native client application, shared object.
- supports lpu237 ibutton functionality.
- for changing version, change
  - vs2022 project property ：　Target Extension
  - vs2022 project property ： Linker-Command Line : -soname option(major version only)
  - vs2022 project property ： Build Events-Remote Post-Build Event : Command Line option(major version only) as like

  ``` shell
  rm $(RemoteOutDir)$(TargetName).so;rm $(RemoteOutDir)$(TargetName).so.6;ln -sf $(RemoteOutDir)$(TargetFileName) $(RemoteOutDir)$(TargetName).so.6;ln -sf $(RemoteOutDir)$(TargetName).so.6 $(RemoteOutDir)$(TargetName).so
  ```

## wi_lpu237_ibutton

- for windows 11.(x64, x86)
- executable file name : tg_lpu237_ibutton.dll
- for native client application, dynamic linked library.
- supports lpu237 ibutton functionality.
- for changing version, change version resource

## li_lpu237_fw

- from package release 2.11
- for Debian 12.(x64)
- executable file name : libtg_lpu237_fw.so
- for native client application, shared object.
- supports lpu237 firmware update functionality.
- for changing version, change
  - vs2022 project property ：　Target Extension
  - vs2022 project property ： Linker-Command Line : -soname option(major version only)
  - vs2022 project property ： Build Events-Remote Post-Build Event : Command Line option(major version only) as like

  ``` shell
  rm $(RemoteOutDir)$(TargetName).so;rm $(RemoteOutDir)$(TargetName).so.6;ln -sf $(RemoteOutDir)$(TargetFileName) $(RemoteOutDir)$(TargetName).so.6;ln -sf $(RemoteOutDir)$(TargetName).so.6 $(RemoteOutDir)$(TargetName).so
  ```

## wi_lpu237_fw

- from package release 2.11
- for windows 11.(x64, x86)
- executable file name : tg_lpu237_fw.dll
- for native client application, dynamic linked library.
- supports lpu237 firmware update functionality.
- for changing version, change version resource

## wi_lpu237_tools

- from package release 2.14
- for windows 11.(x86)
- executable file name : tg_lpu237_tools.dll
- for native client application, dynamic linked library.
- supports lpu237 basic setting functionality.
- for changing version, change version resource

## li_lpu237_jni

- from package release 2.15
- for Debian 12.(x64)
- executable file name : libtg_lpu237_jni.so
- for native client application, shared object.
- the native library of Javapos Service Object.
- for changing version, change
  - vs2022 project property ：　Target Extension
  - vs2022 project property ： Linker-Command Line : -soname option(major version only)
  - vs2022 project property ： Build Events-Remote Post-Build Event : Command Line option(major version only) as like

  ``` shell
  rm $(RemoteOutDir)$(TargetName).so;rm $(RemoteOutDir)$(TargetName).so.2;ln -sf $(RemoteOutDir)$(TargetFileName) $(RemoteOutDir)$(TargetName).so.2;ln -sf $(RemoteOutDir)$(TargetName).so.2 $(RemoteOutDir)$(TargetName).so
  ```

## wi_lpu237_jni

- from package release 2.15
- for windows 11.(x64, x86)
- executable file name : tg_lpu237_jni.dll
- for native client application, dynamic linked library.
- the native library of Javapos Service Object.
- for changing version, change version resource

## tp_li_lpu237

- *Not distributed as an internal test program*
- for Debian 12.(x64)
- executable file name : tp_li_lpu237.out
- libtg_lpu237_dll.so test program.(msr test)
- for changing version, none.

## tp_wi_lpu237

- *Not distributed as an internal test program*
- for windows 11.(x64, x86)
- executable file name : tp_wi_lpu237.exe
- tg_lpu237_dll.dll test program.(msr test)
- for changing version, none.

## li_lpu237_update

- for Debian 12.(x64)
- executable file name : lpu237_update.out or lpu237_update
- lpu237 firmware update program.
- can be executed by elpusk-hid-d.
- for changing version, change
  - lpu230_update.cpp : _print_help()

## wi_lpu237_update

- for windows 11.(x64, x86)
- executable file name : lpu237_update.exe
- lpu237 firmware update program.
- can be executed by elpusk-hid-d.exe.
- for changing version, change
  - lpu230_update.cpp : _print_help()
  - change version resource

## li_rom

- for Debian 12.(x64)
- executable file name : libtg_rom.so
- rom file helper dynamic linked library.
- for changing version, change
  - vs2022 project property ：　Target Extension
  - vs2022 project property ： Linker-Command Line : -soname option(major,minor version only)
  - vs2022 project property ： Build Events-Remote Post-Build Event : Command Line option(major version only) as like

  ``` shell
  rm $(RemoteOutDir)$(TargetName).so;rm $(RemoteOutDir)$(TargetName).so.1;ln -sf $(RemoteOutDir)$(TargetFileName) $(RemoteOutDir)$(TargetName).so.1;ln -sf $(RemoteOutDir)$(TargetName).so.1 $(RemoteOutDir)$(TargetName).so
  ```

## wi_rom

- for windows 11.(x64, x86)
- executable file name : tg_rom.dll
- rom file helper dynamic linked library.
- for changing version, change
  - change version resource

## li_rom_build

- *Not distributed as an internal using program*
- for Debian 12.(x64)
- executable file name : tg_rom_build.out
- rom file build program.
- for changing version, none.

## wi_rom_build

- *Not distributed as an internal using program*
- for windows 11.(x64, x86)
- executable file name : tg_rom_build.exe
- rom file build program.
- for changing version, change
  - change version resource

## wi_dev_lib

- for windows 11.(x64, x86)
- executable file name : dev_lib.dll
- for native client application, dynamic linked library.
- for elpusk-hid-d.exe and lpu230_update.exe, service hid device io.
- for changing version, change
  - change version resource

## li_dev_lib

- for Debian 12.(x64)
- executable file name : libdev_lib.so
- for native client application, shared object.
- for elpusk-hid-d and lpu230_update(lpu230_update.out), service hid device io.
- for changing version, change
  - vs2022 project property ：　Target Extension
  - vs2022 project property ： Linker-Command Line : -soname option(major version only)
  - vs2022 project property ： Build Events-Remote Post-Build Event : Command Line option(major version only) as like

  ``` shell
  rm $(RemoteOutDir)$(TargetName).so;rm $(RemoteOutDir)$(TargetName).so.1;ln -sf $(RemoteOutDir)$(TargetFileName) $(RemoteOutDir)$(TargetName).so.1;ln -sf $(RemoteOutDir)$(TargetName).so.1 $(RemoteOutDir)$(TargetName).so
  ```

## OPOS Service Object

### OposLpu230

- from package release 2.14
- for windows 11.(x86)
- executable file name : OposLpu230.dll
- for OPOS CCO, COM object.
- OPOS Msr Service Object of lpu237.
- for changing version, change
  - SoMsr.h : LPU230_SO_VERSION
  - version resource

### OposLpu230Lock

- from package release 2.14
- for windows 11.(x86)
- executable file name : OposLpu230Lock.dll
- for OPOS CCO, COM object.
- OPOS Keylock Service Object of lpu237.
- for changing version, change
  - SoLock.h : LPU230LOCK_SO_VERSION
  - SoLock.h : Don't change *LPU230LOCK_SO_VERSION_1_14_1*
  - version resource

## MCP

### ibutton-mcp

- for win11(x64,Claude Desktop and Cursor) & Debian 12.(x64, Cursor)  
- executable file name : lpu23x-ibutton-mcp.exe(lpu23x-ibutton-mcp)
- AI agent MCP of magnetic card read(tg_lpu237_dll.dll/libtg_lpu237_dll.so)
- the description of Tool is dynamic loading from lpu23x-ibutton-mcp.json
- for changing version, change
  - solution root\mcp\servers\ibutton-mcp\Cargo.toml : package session, version key

### msr-mcp

- for win11(x64,Claude Desktop and Cursor) & Debian 12.(x64, Cursor)
- executable file name : lpu23x-msr-mcp.exe(lpu23x-msr-mcp)
- AI agent MCP of i-button read(tg_lpu237_ibutton.dll/libtg_lpu237_ibutton.so)
- the description of Tool is dynamic loading from lpu23x-msr-mcp.json
- for changing version, change
  - solution root\mcp\servers\msr-mcp\Cargo.toml : package session, version key
