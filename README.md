# library.cpp.lpu230
lpu23x device c++ library

## env
+ vs2022
+ debian12 remote - user id : tester
+ use static link library. 
+ boost 1.80 lib
+ nlohmann json lib - path : /usr/local/json/
+ openssl-1.1.1s lib - path : /usr/local/openssl-1.1.1s/
+ libusb1.0 lib - path : /usr/local/libusb/
+ deb package directory : /home/tester/build_deb/
+ FTXUI 6.1.9 lib - path : /home/tester/FTXUI/

## the current developing version(pkg v2.5)
+ dev_lib : v1.0
+ lpu230_update : v2.2
  - at windows, logging folder is equal to lpu230_update.exe_ logging folder.
  - run by elpusk-hid-d server.
  
+ tg_lpu237_dll : v6.0
+ tg_lpu237_ibutton : v6.0
+ tg_rom : v1.2
+ tg_rom_build : v1.2
+ elpusk-hid-d : v2.5
  - clog miss code; fixed this error.
+ coffee_sevice : 2.0
  - Windows only service program for running elpusk-hid-d.exe

## build on debian12.
+ notice
  - when linking the library, -lssl must be in front of -lcrypto.
    * as like linker error - ... undefined reference to COMP_CTX_...
  - udev must be linked the last order.
    * as like linker error - ... undefined reference to udev...
  - add path on /etc/profile file.
    * export PATH=$PATH:/usr/sbin:/usr/local/sbin:/sbin:/usr/local/openssl-1.1.1s/bin

+ boost library
  - use version 1.80.0.0, static lib
  - build
    * wget https://boostorg.jfrog.io/artifactory/main/release/1.80.0/source/boost_1_80_0.tar.bz2
    * tar --bzip2 -xf boost_1_80_0.tar.bz2
    * cd boost_1_80_0
    * ./bootstrap.sh
    * ./b2 -j4 -a --build-dir=build/debug --stagedir=stage/debug --toolset=gcc --architecture=x86_64 variant=debug link=static threading=multi address-model=64 runtime-link=static
    * ./b2 -j4 -a --build-dir=build/release --stagedir=stage/release --toolset=gcc --architecture=x86_64 variant=release link=static threading=multi address-model=64 runtime-link=static
    * sudo cp -r ./boost /usr/local/include/
    * sudo cp -r ./stage/release/lib/* /usr/local/lib/

+ libusb
  - use version 1.0, static lib
  - build
    * git clone https://github.com/libusb/libusb.git
    * ./autogen.sh
    * ./configure --prefix=/usr/local/libusb  --enable-static --disable-shared
    * make
    * sudo make install
    * sudo cp -r /usr/local/libusb/include/* /usr/local/include/
    * sudo cp -r /usr/local/libusb/lib/* /usr/local/lib/
            
+ openssl
  - use version 1.1.1s, static lib
  - buid
    * wget https://www.openssl.org/source/openssl-1.1.1s.tar.gz
    * tar xvfz openssl-1.1.1s.tar.gz
    * ./Configure linux-x86_64 no-shared  no-md2 no-mdc2 no-rc5 no-rc4  --prefix=/usr/local/openssl-1.1.1s
    * make depend
    * make
    * sudo make install

+ nlohmann/json
  - build : no need compile, develop branch, 
    * cd /usr/local
    * sudo git clone https://github.com/nlohmann/json.git

+ FTXUI
  - use version 6.1.9, static lib
  - build
    * git clone https://github.com/ArthurSonzogni/FTXUI.git
    * cd FTXUI
    * mkdir build
    * cd build
    * cmake .. -DBUILD_SHARED_LIBS=OFF
    * make -j


## run
+ Windows11 - with admin permision.
+ Debian12 - with sudo.

## Using
### security websocket server and device manager.
+ Win11 : elpusk-hid-d.exe
+ Debian : elpusk-hid-d

### trace console.
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

### terminate security websocket server.
+ Win11 : use services.msc
+ Debian : sudo systemctl stop coffee-manager-2nd

### start termainl.
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
+ run as like, sudo sh build-coffee-manager-2nd.sh
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
  - PID file path : none
+ debug build
  - certificate file : _mp::_coffee::CONST_S_CERT_ABS_FULL_PATH = L"C:\\job\\library.cpp.lpu230\\shared\\data_for_debug\\cert\\coffee_server.crt"
  - private key file : _mp::_coffee::CONST_S_PRIVATE_KEY_ABS_FULL_PATH = L"C:\\job\\library.cpp.lpu230\\shared\\data_for_debug\\cert\\coffee_server.key"
  - log directory : _mp::_coffee::CONST_S_LOG_DIR_EXCEPT_BACKSLASH = L"C:\\ProgramData\\Elpusk\\00000006\\elpusk-hid-d\\log"
  - virtual drive root directory : _mp::_coffee::CONST_S_ROOT_DIR_EXCEPT_BACKSLASH = L"C:\\job\\library.cpp.lpu230\\shared\\data_for_debug\\win\\ProgramData\\elpusk\\00000006\\vroot"
  
+ release build
  - certificate file : _mp::_coffee::CONST_S_CERT_ABS_FULL_PATH = L"%ProgramData%\\elpusk\\programdata\\00000006\\coffee_manager\\data\\server\\coffee_server.crt"
  - private key file : _mp::_coffee::CONST_S_PRIVATE_KEY_ABS_FULL_PATH = L"%ProgramData%\\elpusk\\programdata\\00000006\\coffee_manager\\data\\server\\coffee_server.key"
  - log directory : _mp::_coffee::CONST_S_LOG_DIR_EXCEPT_BACKSLASH = L"%ProgramData%\\elpusk\\00000006\\coffee_manager\\elpusk-hid-d\\log"
  - virtual drive root directory : _mp::_coffee::CONST_S_ROOT_DIR_EXCEPT_BACKSLASH = L"%ProgramData%\\elpusk\\programdata\\00000006\\coffee_manager\\root"

#### Linux build x64
+ common
  - PID file path : _mp::_coffee::CONST_S_PID_FILE_FULL_PATH = L"/var/run/elpusk-hid-d.pid"

+ debug build
  - certificate file : _mp::_coffee::CONST_S_CERT_ABS_FULL_PATH = L"/home/tester/projects/LiElpuskHidDaemon/bin/x64/Debug/coffee_server.crt"
  - private key file : _mp::_coffee::CONST_S_PRIVATE_KEY_ABS_FULL_PATH = L"/home/tester/projects/LiElpuskHidDaemon/bin/x64/Debug/coffee_server.key"
  - log directory : _mp::_coffee::CONST_S_LOG_DIR_EXCEPT_BACKSLASH = L"/home/tester/projects/LiElpuskHidDaemon/bin/x64/Debug/00000006/coffee_manager/elpusk-hid-d"
  - virtual drive root directory : _mp::_coffee::CONST_S_ROOT_DIR_EXCEPT_BACKSLASH = L"/home/tester/projects/LiElpuskHidDaemon/bin/x64/Debug"
  
+ release build
  - certificate file : _mp::_coffee::CONST_S_CERT_ABS_FULL_PATH = L"/usr/share/elpusk/programdata/00000006/coffee_manager/data/server/coffee_server.crt"
  - private key file : _mp::_coffee::CONST_S_PRIVATE_KEY_ABS_FULL_PATH = L"/usr/share/elpusk/programdata/00000006/coffee_manager/data/server/coffee_server.key"
  - log directory : _mp::_coffee::CONST_S_LOG_DIR_EXCEPT_BACKSLASH = L"/var/log/elpusk/00000006/coffee_manager/elpusk-hid-d"
  - virtual drive root directory : _mp::_coffee::CONST_S_ROOT_DIR_EXCEPT_BACKSLASH = L"/usr/share/elpusk/programdata/00000006/coffee_manager/root"


### libtg_lpu237_dll.so file.(tg_lpu237_dll.dll on Windows)

#### common
#### Windows x86 and x64
+ debug build
  - ini file : L"C:\\job\\library.cpp.lpu230\\shared\\projects\\lpu237_dll\\tg_lpu237_dll.ini"
  - log directory : L"C:\\ProgramData\\Elpusk\\00000006\\tg_lpu237_dll\\log"
+ release build
  - ini file : L"C:\\ProgramData\\Elpusk\\00000006\\tg_lpu237_dll\\tg_lpu237_dll.ini"
  - log directory : L"C:\\ProgramData\\Elpusk\\00000006\\tg_lpu237_dll\\log"

#### Linux build x64
+ debug build
  - ini file : L"/home/tester/projects/LiElpuskHidDaemon/job/library.cpp.lpu230/shared/projects/tg_lpu237_dll/tg_lpu237_dll.ini"
  - log directory
    + root user : L"/home/tester/projects/LiElpuskHidDaemon/bin/x64/Debug/00000006/coffee_manager/tg_lpu237_dll"
    + normal user : L"/home/tester/projects/LiElpuskHidDaemon/bin/x64/Debug/00000006/coffee_manager/tg_lpu237_dll"
+ release build
  - ini file : L"/usr/share/elpusk/programdata/00000006/coffee_manager/tg_lpu237_dll/tg_lpu237_dll.ini"
  - log directory
    + root user : L"/var/log/elpusk/00000006/coffee_manager/tg_lpu237_dll"
    + normal user : L"~/.elpusk/log/00000006/coffee_manager/tg_lpu237_dll"


### libtg_lpu237_ibutton.so file.(tg_lpu237_ibutton.dll on Windows)

#### common
#### Windows x86 and x64
+ debug build
  - ini file : L"C:\\job\\library.cpp.lpu230\\shared\\projects\\lpu237_ibutton\\tg_lpu237_ibutton.ini"
  - log directory : L"C:\\ProgramData\\Elpusk\\00000006\\tg_lpu237_ibutton\\log"
+ release build
  - ini file : L"C:\\ProgramData\\Elpusk\\00000006\\tg_lpu237_ibutton\\tg_lpu237_ibutton.ini"
  - log directory : L"C:\\ProgramData\\Elpusk\\00000006\\tg_lpu237_ibutton\\log"

#### Linux build x64
+ debug build
  - ini file : L"/home/tester/projects/LiElpuskHidDaemon/job/library.cpp.lpu230/shared/projects/tg_lpu237_ibutton/tg_lpu237_ibutton.ini"
  - log directory
    + root user : L"/home/tester/projects/LiElpuskHidDaemon/bin/x64/Debug/00000006/coffee_manager/tg_lpu237_ibutton"
    + normal user : L"/home/tester/projects/LiElpuskHidDaemon/bin/x64/Debug/00000006/coffee_manager/tg_lpu237_ibutton"
+ release build
  - ini file : L"/usr/share/elpusk/programdata/00000006/coffee_manager/tg_lpu237_ibutton/tg_lpu237_ibutton.ini"
  - log directory
    + root user : L"/var/log/elpusk/00000006/coffee_manager/tg_lpu237_ibutton"
    + normal user : L"~/.elpusk/log/00000006/coffee_manager/tg_lpu237_ibutton"

