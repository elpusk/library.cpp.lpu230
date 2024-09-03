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
+ In Debian12, copy build-coffee-manager.sh to /home/tester/build_deb/
+ run as like, sudo sh build-coffee-manager.sh
+ installation, sudo dpkg -i coffee-manager_x.y-z_amd64.deb
+ uninstallation, sudo dpkg -r coffee-manager

### remove forcefully
sudo dpkg --remove --force-remove-reinstreq coffee-manager
sudo rm /var/lib/dpkg/info/coffee-manager*



