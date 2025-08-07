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
+ Win11 : elpusk-hid-d.exe /bye
+ Debian : elpusk-hid-d /bye

### start termainl.
+ control "security websocket server and device manager".
+ Win11 : elpusk-hid-d.exe /terminal
+ Debian : elpusk-hid-d /terminal

## Problems
+ fixed after starting wss, the inserted device is reported that  is used.
+ In li_lpu237_dll project.
  - project setting : for using ld file, --version-script=$(RemoteProjectDir)li_lpu237_dll/li_lpu237_dll.ld 
  - build error "/usr/bin/ld : error : cannot open linker script file ~/projects/li_lpu237_dll/li_lpu237_dll/li_lpu237_dll.ld: No such file or directory"
  - BUT li_lpu237_dll.ld exist in ~/projects/li_lpu237_dll/li_lpu237_dll/.  -_-;;
  - workaround : changing setting, --version-script=/home/tester/projects/li_lpu237_dll/li_lpu237_dll/li_lpu237_dll.ld
+ fixed : when installation, elpusk-hid-d start failure.(exit iwth error)
+ fixed : deb pkg install /uninstall
+ fixed : conversion string data of "::" and ":".( Now native app can read a ms-card)

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

#### Linux build x86 and x64
+ common
  - PID file path : _mp::_coffee::CONST_S_PID_FILE_FULL_PATH = L"/var/run/elpusk-hid-d.pid"

+ debug build
  - certificate file : _mp::_coffee::CONST_S_CERT_ABS_FULL_PATH = L"/home/tester/projects/LiElpuskHidDaemon/bin/x64/Debug/coffee_server.crt"
  - private key file : _mp::_coffee::CONST_S_PRIVATE_KEY_ABS_FULL_PATH = L"/home/tester/projects/LiElpuskHidDaemon/bin/x64/Debug/coffee_server.key"
  - log directory : _mp::_coffee::CONST_S_LOG_DIR_EXCEPT_BACKSLASH = L"/home/tester/projects/LiElpuskHidDaemon/bin/x64/Debug"
  - virtual drive root directory : _mp::_coffee::CONST_S_ROOT_DIR_EXCEPT_BACKSLASH = L"/home/tester/projects/LiElpuskHidDaemon/bin/x64/Debug"
  
+ release build
  - certificate file : _mp::_coffee::CONST_S_CERT_ABS_FULL_PATH = L"/usr/share/elpusk/programdata/00000006/coffee_manager/data/server/coffee_server.crt"
  - private key file : _mp::_coffee::CONST_S_PRIVATE_KEY_ABS_FULL_PATH = L"/usr/share/elpusk/programdata/00000006/coffee_manager/data/server/coffee_server.key"
  - log directory : _mp::_coffee::CONST_S_LOG_DIR_EXCEPT_BACKSLASH = L"/var/log/elpusk/00000006/coffee_manager/elpusk-hid-d"
  - virtual drive root directory : _mp::_coffee::CONST_S_ROOT_DIR_EXCEPT_BACKSLASH = L"/usr/share/elpusk/programdata/00000006/coffee_manager/root"


### tg_lpu27_dll.so file.(tg_lpu27_dll.dll on Windows)

#### common
+ log directory : L"."

