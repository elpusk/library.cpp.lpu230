# library.cpp.lpu230

- lpu23x device c++ libraries
- MCP Server by Rust

## env

+ vs2022
+ debian12 remote - user id : tester
+ use static link library
+ boost 1.80 lib
+ nlohmann json lib - path : /usr/local/json/
+ openssl-1.1.1s lib - path : /usr/local/openssl-1.1.1s/
+ libusb1.0 lib - path : /usr/local/libusb/
+ deb package directory : /home/tester/build_deb/
+ FTXUI 6.1.9 lib - path : /home/tester/FTXUI/

## the current version(pkg v2.14)

+ coffee_sevice : 2.3
  + rebuilded with the changed library.
  + Windows only service program for running elpusk-hid-d.exe

+ elpusk-hid-d : v2.13
  + In removeall option, change "Elpusk" -> "elpusk" (Windows only)

+ tg_lpu237_dll : v6.4
  + add virtual lpu238 class

+ tg_lpu237_ibutton : v6.4
  + add virtual lpu238 class

+ tg_lpu237_fw : v6.4
  + add virtual lpu238 class

+ dev_lib : v1.4
  + add virtual lpu238 class

+ lpu230_update : v2.10
  + add virtual lpu238 class

+ tg_rom : v1.5
  + rebuilded with the changed library.

+ tg_rom_build : v1.4
  + rebuilded with the changed library.

+ lpu23x-msr-mcp : v1.1
  + the first release.

+ lpu23x-ibutton-mcp : v1.1
  + the first release.

+ OposLpu230 : v1.8.30
  + the first release.(porting from NDM version)

+ OposLpu230Lock : v1.14.30
  + the first release.(porting from NDM version)
  + Windows x86 only

+ OposLpu230Lock : v1.14.30
  + the first release.(porting from NDM version)
  + Windows x86 only

## build on windows11

+ boost library
  + use version 1.80.0.0, static lib, target vs2022
  + build (on C:\local\boost_1_80_0)
    + download [boost_1_80_0.zip](https://www.boost.org/releases/1.80.0/) to C:\local directory
    + extract boost_1_80_0.zip
    + cd boost_1_80_0
    + run "Developer Command Prompt for VS 2022"
    + edit C:\local\boost_1_80_0\tools\build\src\tools\msvc.jam line 1122. see [issue](https://github.com/boostorg/boost/issues/914)
    + run bootstrap.bat
    + b2 -j4 -a toolset=msvc-14.3 architecture=x86 variant=debug,release link=static threading=multi address-model=32 runtime-link=static
    + b2 -j4 -a toolset=msvc-14.3 architecture=x86 variant=debug,release link=static threading=multi address-model=64 runtime-link=static
    + output lib files are in C:\local\boost_1_80_0\stage\lib

+ libusb
  + use version 1.0, static lib
  + build
    + cd C:\local\libusb
    + git clone https://github.com/libusb/libusb.git
    + see C:\local\libusb\INSTALL_WIN.txt

+ openssl
  + use version 1.1.1s, static lib, strawberry-perl
  + buid
    + cd C:\local
    + download openssl-1.1.1s.tar.gz from https://openssl-library.org/source/old/1.1.1/
    + tar xvfz openssl-1.1.1s.tar.gz
    + perl Configure VC-WIN64A no-idea no-md2 no-mdc2 no-rc5 no-rc4  no-asm enable-static-engine --openssldir=C:\local\openssl-1.1.1s\x64\SSL --prefix=C:\local\openssl-1.1.1s\x64\OpenSSL
    + nmake
    + nmake test
    + nmake install
    + perl Configure VC-WIN32 no-idea no-md2 no-mdc2 no-rc5 no-rc4  no-asm enable-static-engine --openssldir=C:\local\openssl-1.1.1s\x86\SSL --prefix=C:\local\openssl-1.1.1s\x86\OpenSSL
    + nmake
    + nmake test
    + nmake install

+ nlohmann/json
  + build : no need compile, develop branch, header only No need build
    + cd C:\local
    + mkdir nlohmann
    + git clone https://github.com/nlohmann/json.git

+ FTXUI
  + use version 6.1.9, static lib
  + build
    + cd C:\local
    + git clone https://github.com/ArthurSonzogni/FTXUI.git
    + cd FTXUI
    + mkdir build_x64
    + cd build_x64
    + cmake .. -G "Visual Studio 17 2022" -A x64
    + load & build the created solution on vs2022
    + mkdir C:\local\FTXU\build_x86
    + cd C:\local\FTXU\build_x86
    + cmake .. -G "Visual Studio 17 2022" -A Win32
    + load & build the created solution on vs2022

+ OPOS
  + use version 1.14.001, static lib
  + build
    + download and install from [OPOS CCO](http://www.monroecs.com/files/OPOS_CCOs_1.14.001.msi)

if boost lib building error, 
```
msvc.jam file : 
        else
        {
            if [ MATCH "(14.4)" : $(version) ]
            {
                if $(.debug-configuration)
                {
                    ECHO "notice: [generate-setup-cmd] $(version) is 14.4x" ;
                }
                parent = [ path.native [ path.join  $(parent) "..\\..\\..\\..\\..\\Auxiliary\\Build" ] ] ;
            }
            if [ MATCH "(14.3)" : $(version) ]
            ...
```

## build on debian12

+ notice
  + when linking the library, -lssl must be in front of -lcrypto.
    + as like linker error - ... undefined reference to COMP_CTX_...
  + udev must be linked the last order.
    + as like linker error - ... undefined reference to udev...
  + add path on /etc/profile file.
    + export PATH=$PATH:/usr/sbin:/usr/local/sbin:/sbin:/usr/local/openssl-1.1.1s/bin

+ boost library
  + use version 1.80.0.0, static lib
  + build
    + wget https://boostorg.jfrog.io/artifactory/main/release/1.80.0/source/boost_1_80_0.tar.bz2
    + tar --bzip2 -xf boost_1_80_0.tar.bz2
    + cd boost_1_80_0
    + ./bootstrap.sh
    + ./b2 -j4 -a --build-dir=build/debug --stagedir=stage/debug --toolset=gcc --architecture=x86_64 variant=debug link=static threading=multi address-model=64 runtime-link=static
    + ./b2 -j4 -a --build-dir=build/release --stagedir=stage/release --toolset=gcc --architecture=x86_64 variant=release link=static threading=multi address-model=64 runtime-link=static
    + sudo cp -r ./boost /usr/local/include/
    + sudo cp -r ./stage/release/lib/* /usr/local/lib/

+ libusb
  + use version 1.0, static lib
  + build
    + git clone https://github.com/libusb/libusb.git
    + ./autogen.sh
    + ./configure --prefix=/usr/local/libusb  --enable-static --disable-shared
    + make
    + sudo make install
    + sudo cp -r /usr/local/libusb/include/* /usr/local/include/
    + sudo cp -r /usr/local/libusb/lib/* /usr/local/lib/

+ openssl
  + use version 1.1.1s, static lib
  + buid
    + wget https://www.openssl.org/source/openssl-1.1.1s.tar.gz
    + tar xvfz openssl-1.1.1s.tar.gz
    + ./Configure linux-x86_64 no-shared  no-md2 no-mdc2 no-rc5 no-rc4  --prefix=/usr/local/openssl-1.1.1s
    + make depend
    + make
    + sudo make install

+ nlohmann/json
  + build : no need compile, develop branch, 
    + cd /usr/local
    + sudo git clone https://github.com/nlohmann/json.git

+ FTXUI
  + use version 6.1.9, static lib
  + build
    + git clone https://github.com/ArthurSonzogni/FTXUI.git
    + cd FTXUI
    + mkdir build
    + cd build
    + cmake .. -DBUILD_SHARED_LIBS=OFF
    + make -j

## run

+ Windows11 - with admin permision.
+ Debian12 - with sudo.

## Using

### security websocket server and device manager

+ Win11 : elpusk-hid-d.exe
+ Debian : elpusk-hid-d

### trace console

+ Win11 : elpusk-hid-d.exe /trace
+ Debian : elpusk-hid-d /trace

### installation self signed CA-root & server certificate and key file

+ Win11 : elpusk-hid-d.exe /cert
+ Debian : elpusk-hid-d /cert

### uninstallation self signed CA-root & server certificate and key file

+ Win11 : elpusk-hid-d.exe /removecert
+ Debian : elpusk-hid-d /removecert

### remove all installed files & add path

+ Win11 : elpusk-hid-d.exe /removeall
+ Debian : elpusk-hid-d /removeall

### terminate security websocket server

+ Win11 : use services.msc
+ Debian : sudo systemctl stop coffee-manager-2nd

### start terminal

+ control "security websocket server and device manager".
+ Win11 : elpusk-hid-d.exe /terminal
+ Debian : elpusk-hid-d /terminal

## Problems

+ not yet solved : In Debian12, "lpu230_update" run by "elpusk-hid-d" does not always display the UI.

## coding rules

+ Wherever the file log is, the trace must be present.

## build & install

### deb package

+ In Debian12, copy build-coffee-manager-2nd.sh to /home/tester/build_deb/
+ run as like, sh build-coffee-manager-2nd.sh
+ installation, sudo dpkg -i coffee-manager-2nd_x.y_amd64.deb
+ uninstallation(pure), sudo dpkg -P coffee-manager-2nd

### remove forcefully

sudo dpkg --remove --force-remove-reinstreq coffee-manager-2nd
sudo rm /var/lib/dpkg/info/coffee-manager*

## running environment

### elpusk-hid-d file.(elpusk-hid-d.exe on Windows)

#### common

+ trace pipe name : _mp::_coffee::CONST_S_COFFEE_MGMT_TRACE_PIPE_NAME = L"PIPE_NAME_COFFEE_MGMT_TRACE_036423FC_2189_423D_8D0E_75992725F843"
+ for single instance, file lock : _mp::_coffee::CONST_S_COFFEE_MGMT_FILE_LOCK_FOR_SINGLE = L"FILE_LOCK_COFFEE_MGMT_E0A38B4D_DBE7_4F77_A657_45BD8A19B923"
+ for controll object, pipe name : _mp::_coffee::CONST_S_COFFEE_MGMT_CTL_PIPE_NAME = L"PIPE_NAME_COFFEE_MGMT_CTL_6B092EC7_0D20_4123_8165_2C1DE27C9AAF"
+ for controll object, pipe name : _mp::_coffee::CONST_S_COFFEE_MGMT_CTL_PIPE_NAME_OF_SERVER_RESPONSE = L"PIPE_NAME_COFFEE_MGMT_CTL_SV_RSP_867621B4_B234_4823_82EC_D5562655EFA3"
+ security websocket port : mp::_ws_tools::WEBSOCKET_SECURITY_SERVER_PORT_COFFEE_MANAGER =  443

#### Windows x86 and x64

+ common
  + PID file path : none
+ debug build
  + certificate file : _mp::_coffee::CONST_S_CERT_ABS_FULL_PATH = L"C:\\job\\library.cpp.lpu230\\shared\\data_for_debug\\cert\\coffee_server.crt"
  + private key file : _mp::_coffee::CONST_S_PRIVATE_KEY_ABS_FULL_PATH = L"C:\\job\\library.cpp.lpu230\\shared\\data_for_debug\\cert\\coffee_server.key"
  + log directory : _mp::_coffee::CONST_S_LOG_DIR_EXCEPT_BACKSLASH = L"C:\\ProgramData\\Elpusk\\00000006\\elpusk-hid-d\\log"
  + virtual drive root directory : _mp::_coffee::CONST_S_ROOT_DIR_EXCEPT_BACKSLASH = L"C:\\job\\library.cpp.lpu230\\shared\\data_for_debug\\win\\ProgramData\\elpusk\\00000006\\vroot"
  
+ release build
  + certificate file : _mp::_coffee::CONST_S_CERT_ABS_FULL_PATH = L"%ProgramData%\\elpusk\\programdata\\00000006\\coffee_manager\\data\\server\\coffee_server.crt"
  + private key file : _mp::_coffee::CONST_S_PRIVATE_KEY_ABS_FULL_PATH = L"%ProgramData%\\elpusk\\programdata\\00000006\\coffee_manager\\data\\server\\coffee_server.key"
  + log directory : _mp::_coffee::CONST_S_LOG_DIR_EXCEPT_BACKSLASH = L"%ProgramData%\\elpusk\\00000006\\coffee_manager\\elpusk-hid-d\\log"
  + virtual drive root directory : _mp::_coffee::CONST_S_ROOT_DIR_EXCEPT_BACKSLASH = L"%ProgramData%\\elpusk\\programdata\\00000006\\coffee_manager\\root"

#### Linux build x64

+ common
  + PID file path : _mp::_coffee::CONST_S_PID_FILE_FULL_PATH = L"/var/run/elpusk-hid-d.pid"

+ debug build
  + certificate file : _mp::_coffee::CONST_S_CERT_ABS_FULL_PATH = L"/home/tester/projects/LiElpuskHidDaemon/bin/x64/Debug/coffee_server.crt"
  + private key file : _mp::_coffee::CONST_S_PRIVATE_KEY_ABS_FULL_PATH = L"/home/tester/projects/LiElpuskHidDaemon/bin/x64/Debug/coffee_server.key"
  + log directory : _mp::_coffee::CONST_S_LOG_DIR_EXCEPT_BACKSLASH = L"/home/tester/projects/LiElpuskHidDaemon/bin/x64/Debug/00000006/coffee_manager/elpusk-hid-d"
  + virtual drive root directory : _mp::_coffee::CONST_S_ROOT_DIR_EXCEPT_BACKSLASH = L"/home/tester/projects/LiElpuskHidDaemon/bin/x64/Debug"
  
+ release build
  + certificate file : _mp::_coffee::CONST_S_CERT_ABS_FULL_PATH = L"/usr/share/elpusk/programdata/00000006/coffee_manager/data/server/coffee_server.crt"
  + private key file : _mp::_coffee::CONST_S_PRIVATE_KEY_ABS_FULL_PATH = L"/usr/share/elpusk/programdata/00000006/coffee_manager/data/server/coffee_server.key"
  + log directory : _mp::_coffee::CONST_S_LOG_DIR_EXCEPT_BACKSLASH = L"/var/log/elpusk/00000006/coffee_manager/elpusk-hid-d"
  + virtual drive root directory : _mp::_coffee::CONST_S_ROOT_DIR_EXCEPT_BACKSLASH = L"/usr/share/elpusk/programdata/00000006/coffee_manager/root"

### libtg_lpu237_dll.so file.(tg_lpu237_dll.dll on Windows)

#### Common

+ none info

#### Windows x86 and x64

+ debug build
  + ini file : L"C:\\job\\library.cpp.lpu230\\shared\\projects\\lpu237_dll\\tg_lpu237_dll.ini"
  + log directory : L"C:\\ProgramData\\Elpusk\\00000006\\tg_lpu237_dll\\log"
+ release build
  + ini file : L"C:\\ProgramData\\Elpusk\\00000006\\tg_lpu237_dll\\tg_lpu237_dll.ini"
  + log directory : L"C:\\ProgramData\\Elpusk\\00000006\\tg_lpu237_dll\\log"

#### Linux build x64

+ debug build
  + ini file : L"/home/tester/projects/LiElpuskHidDaemon/job/library.cpp.lpu230/shared/projects/tg_lpu237_dll/tg_lpu237_dll.ini"
  + log directory
    + root user : L"/home/tester/projects/LiElpuskHidDaemon/bin/x64/Debug/00000006/coffee_manager/tg_lpu237_dll"
    + normal user : L"/home/tester/projects/LiElpuskHidDaemon/bin/x64/Debug/00000006/coffee_manager/tg_lpu237_dll"
+ release build
  + ini file : L"/usr/share/elpusk/programdata/00000006/coffee_manager/tg_lpu237_dll/tg_lpu237_dll.ini"
  + log directory
    + root user : L"/var/log/elpusk/00000006/coffee_manager/tg_lpu237_dll"
    + normal user : L"~/.elpusk/log/00000006/coffee_manager/tg_lpu237_dll"

### libtg_lpu237_ibutton.so file.(tg_lpu237_ibutton.dll on Windows)

#### Common

#### Windows x86 and x64

+ debug build
  + ini file : L"C:\\job\\library.cpp.lpu230\\shared\\projects\\lpu237_ibutton\\tg_lpu237_ibutton.ini"
  + log directory : L"C:\\ProgramData\\Elpusk\\00000006\\tg_lpu237_ibutton\\log"
+ release build
  + ini file : L"C:\\ProgramData\\Elpusk\\00000006\\tg_lpu237_ibutton\\tg_lpu237_ibutton.ini"
  + log directory : L"C:\\ProgramData\\Elpusk\\00000006\\tg_lpu237_ibutton\\log"

#### Linux build x64

+ debug build
  + ini file : L"/home/tester/projects/LiElpuskHidDaemon/job/library.cpp.lpu230/shared/projects/tg_lpu237_ibutton/tg_lpu237_ibutton.ini"
  + log directory
    + root user : L"/home/tester/projects/LiElpuskHidDaemon/bin/x64/Debug/00000006/coffee_manager/tg_lpu237_ibutton"
    + normal user : L"/home/tester/projects/LiElpuskHidDaemon/bin/x64/Debug/00000006/coffee_manager/tg_lpu237_ibutton"
+ release build
  + ini file : L"/usr/share/elpusk/programdata/00000006/coffee_manager/tg_lpu237_ibutton/tg_lpu237_ibutton.ini"
  + log directory
    + root user : L"/var/log/elpusk/00000006/coffee_manager/tg_lpu237_ibutton"
    + normal user : L"~/.elpusk/log/00000006/coffee_manager/tg_lpu237_ibutton"

### libtg_lpu237_fw.so file.(tg_lpu237_fw.dll on Windows)

#### Common

#### Windows x86 and x64

+ debug build
  + ini file : L"C:\\job\\library.cpp.lpu230\\shared\\projects\\lpu237_fw\\tg_lpu237_fw.ini"
  + log directory : L"C:\\ProgramData\\Elpusk\\00000006\\tg_lpu237_fw\\log"
+ release build
  + ini file : L"C:\\ProgramData\\Elpusk\\00000006\\tg_lpu237_fw\\tg_lpu237_fw.ini"
  + log directory : L"C:\\ProgramData\\Elpusk\\00000006\\tg_lpu237_fw\\log"

#### Linux build x64

+ debug build
  + ini file : L"/home/tester/projects/LiElpuskHidDaemon/job/library.cpp.lpu230/shared/projects/tg_lpu237_fw/tg_lpu237_fw.ini"
  + log directory
    + root user : L"/home/tester/projects/LiElpuskHidDaemon/bin/x64/Debug/00000006/coffee_manager/tg_lpu237_fw"
    + normal user : L"/home/tester/projects/LiElpuskHidDaemon/bin/x64/Debug/00000006/coffee_manager/tg_lpu237_fw"
+ release build
  + ini file : L"/usr/share/elpusk/programdata/00000006/coffee_manager/tg_lpu237_fw/tg_lpu237_fw.ini"
  + log directory
    + root user : L"/var/log/elpusk/00000006/coffee_manager/tg_lpu237_fw"
    + normal user : L"~/.elpusk/log/00000006/coffee_manager/tg_lpu237_fw"

---------------------------------------------
---------------------------------------------

## MCP Server

### env

+ Windows host
+ installed docker
+ vscode IDE
+ rust language
+ stdio type MCP Server

### build

+ For Windows x64
  + debug - `solution root/mcp/cargo build`
  + release - `solution root/mcp/cargo build --release`
  + test - Claude desktop nad Cursor
+ For Linux x64
  + common setup
    + install [docker](https://www.docker.com/)
    + install cross - `solution root/mcp/cargo install cross`
    + setup target - `solution root/mcp/rustup target add x86_64-unknown-linux-gnu` __DONT WITH musl__
    + cross target setting  - `solution root/mcp/Cross.toml`
  + release - `solution root/mcp/cross build --release --target x86_64-unknown-linux-gnu` __DONT WITH musl__
  + test - Cursor

### etc

+ clear builded data - `solution root/mcp/cargo clear`
+ check source code error - `solution root/mcp/cargo check`

### projects

+ ibutton-mcp : i-button reader MCP server
  + using tg_lpu237_ibutton.dll(libtg_lpu237_ibutton.so)
  + stdio type MCP server
  + dynamic tool description
    + Windows : %ProgramData%\elpusk\00000006\coffee_manager\mcp\lpu23x-ibutton-mcp.json
    + Debian12 : /usr/share/elpusk/programdata/00000006/coffee_manager/mcp/lpu23x-ibutton-mcp.json
  + exported tools
    + start_read_ibutton : start a waitig a i-button data
    + cancel_ibutton : stop a waitig a i-button data
    + get_ibutton_result : get the received i-button data
    + read_ibutton : read a i-button by sync method

+ msr-mcp : msr reader MCP server
  + using tg_lpu237_msr.dll(libtg_lpu237_msr.so)
  + stdio type MCP server
  + dynamic tool description
    + Windows : %ProgramData%\elpusk\00000006\coffee_manager\mcp\lpu23x-msr-mcp.json
    + Debian12 : /usr/share/elpusk/programdata/00000006/coffee_manager/mcp/lpu23x-msr-mcp.json
  + exported tools
    + start_read_card : start a waitig a card data
    + cancel_card : stop a waitig a card data
    + get_read_card_result : get the received card data
    + read_card : read a magnetic card by sync method

### test setup

+ For Windows
  + Claude Desktop - open C:\Users\your-account\AppData\Roaming\Claude\claude_desktop_config.json
  + Cursor - open  C:\Users\your-account\.cursor\mcp.json

``` json
{
  ...

  "mcpServers":{
    "msr-mcp": {
    "command": "C:\\Program Files\\elpusk\\00000006\\coffee_manager\\mcp\\lpu23x-msr-mcp"
    },
    "ibutton-mcp": {
    "command": "C:\\Program Files\\elpusk\\00000006\\coffee_manager\\mcp\\lpu23x-ibutton-mcp"
    }
  }

  ...
}
```

+ For Linux
  + Cursor - open  ~/.cursor/mcp.json
  
``` json
{
  ...

  "mcpServers":{
    "msr-mcp": {
    "command": "/usr/share/elpusk/program/00000006/coffee_manager/mcp/lpu23x-msr-mcp"
    },
    "ibutton-mcp": {
    "command": "/usr/share/elpusk/program/00000006/coffee_manager/mcp/lpu23x-ibutton-mcp"
    }
  }

  ...
}
```
