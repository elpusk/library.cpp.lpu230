# library.cpp.lpu230
lpu23x device c++ library

## env
+ vs2022
+ debian12 remote
+ boost 1.80 lib
+ nlohmann json lib
+ openssl-1.1.1s lib
+ libusb1.0 lib

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

## coding rules
+ Wherever the file log is, the trace must be present.

## build & install
### deb package
+ In Debian12, copy build-coffee-manager.sh to /home/tester/build_deb/
+ run as like, sudo sh build-coffee-manager.sh
+ installation, sudo dpkg -i coffee-manager_x.y-z_amd64.deb
+ uninstallation, sudo dpkg -r coffee-manager

