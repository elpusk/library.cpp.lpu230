#!/bin/bash

# setting variables
PACKAGE_NAME="coffee-manager"
PACKAGE_VERSION="2.0-1"
ARCHITECTURE="amd64"
USR_HOME_DIR="/home/tester"
BUILD_DIR="$USR_HOME_DIR/build_deb/${PACKAGE_NAME}_${PACKAGE_VERSION}_${ARCHITECTURE}"
DEBIAN_DIR="$BUILD_DIR/DEBIAN"
CF_ROOT_PD_DIR="/usr/share/elpusk/program/00000006/coffee_manager"
CF_ROOT_DD_DIR="/usr/share/elpusk/programdata/00000006/coffee_manager"
CF_ROOT_LOG_DIR="/var/log/elpusk/00000006/coffee_manager"

# create directory 
mkdir -p ${BUILD_DIR}${CF_ROOT_PD_DIR}/bin/
mkdir -p ${BUILD_DIR}${CF_ROOT_PD_DIR}/so/
mkdir -p ${BUILD_DIR}${CF_ROOT_DD_DIR}/elpusk-hid-d/
mkdir -p ${BUILD_DIR}${CF_ROOT_DD_DIR}/data/filter/
mkdir -p ${BUILD_DIR}${CF_ROOT_DD_DIR}/root/
mkdir -p ${BUILD_DIR}${CF_ROOT_DD_DIR}/tg_lpu237_dll/
mkdir -p ${BUILD_DIR}${CF_ROOT_DD_DIR}/data/server/
mkdir -p ${BUILD_DIR}${CF_ROOT_LOG_DIR}/elpusk-hid-d/
mkdir -p ${BUILD_DIR}${CF_ROOT_LOG_DIR}/tg_lpu237_dll/

# copy files
cp ${USR_HOME_DIR}/projects/LiElpuskHidDaemon/bin/x64/Release/elpusk-hid-d ${BUILD_DIR}${CF_ROOT_PD_DIR}/bin/
cp ${USR_HOME_DIR}/projects/li_lpu237_dll/bin/x64/Release/libtg_lpu237_dll.so.6.0.0 ${BUILD_DIR}${CF_ROOT_PD_DIR}/so/
cp ${USR_HOME_DIR}/projects/LiElpuskHidDaemon/job/library.cpp.lpu230/shared/projects/ElpuskHidDaemon/elpusk-hid-d.xml ${BUILD_DIR}${CF_ROOT_DD_DIR}/elpusk-hid-d/
cp ${USR_HOME_DIR}/projects/LiElpuskHidDaemon/job/library.cpp.lpu230/shared/projects/ElpuskHidDaemon/device.xml ${BUILD_DIR}${CF_ROOT_DD_DIR}/data/filter/
cp ${USR_HOME_DIR}/projects/li_lpu237_dll/shared/projects/lpu237_dll/tg_lpu237_dll.xml ${BUILD_DIR}${CF_ROOT_DD_DIR}/tg_lpu237_dll/

# create DEBIAN directory 
mkdir -p ${DEBIAN_DIR}

# create control file
cat <<EOF > ${DEBIAN_DIR}/control
Package: ${PACKAGE_NAME}
Version: ${PACKAGE_VERSION}
Architecture: ${ARCHITECTURE}
Maintainer: Elpusk<elpusk@naver.com>
Description: Coffee Manager Package
EOF

# create postinst scritper (postwork after installation)
cat <<EOF > ${DEBIAN_DIR}/postinst
#!/bin/bash
set -e

# run elpusk-hid-d with /cert option. create certificate
${CF_ROOT_PD_DIR}/bin/elpusk-hid-d /cert

# register elpusk-hid-d daemon. and start it.
cp ${CF_ROOT_PD_DIR}/bin/elpusk-hid-d /etc/init.d/elpusk-hid-d
chmod +x /etc/init.d/elpusk-hid-d
update-rc.d elpusk-hid-d defaults
service elpusk-hid-d start

exit 0
EOF

# postinst . set permission to file
chmod 755 ${DEBIAN_DIR}/postinst

# prerm create scripter(before uninstallation)
cat <<EOF > ${DEBIAN_DIR}/prerm
#!/bin/bash
set -e

# stop daemon and remove daemon
if [ -f /etc/init.d/elpusk-hid-d ]; then
    service elpusk-hid-d stop
    update-rc.d -f elpusk-hid-d remove
    rm -f /etc/init.d/elpusk-hid-d
fi

exit 0
EOF

# prerm  set execution permission to file.
chmod 755 ${DEBIAN_DIR}/prerm

# create .deb package.
dpkg-deb --build ${BUILD_DIR}
