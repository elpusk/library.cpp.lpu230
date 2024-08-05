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

## coding rules
+ Wherever the file log is, the trace must be present.
