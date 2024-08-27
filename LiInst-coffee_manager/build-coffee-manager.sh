#!/bin/bash

# this scripter format must be Unix/Linux LF format.
# setting variables
gPACKAGE_NAME="coffee-manager"
gPACKAGE_VERSION="2.0-1"
gARCHITECTURE="amd64"
gUSR_HOME_DIR="/home/tester"
gBUILD_ROOT_DIR="/home/tester/build_deb"
gBUILD_DIR="${gBUILD_ROOT_DIR}/${gPACKAGE_NAME}_${gPACKAGE_VERSION}_${gARCHITECTURE}"
gDEBIAN_DIR="${gBUILD_DIR}/DEBIAN"
gCF_ROOT_PD_DIR="/usr/share/elpusk/program/00000006/coffee_manager"
gCF_ROOT_DD_DIR="/usr/share/elpusk/programdata/00000006/coffee_manager"
gCF_ROOT_LOG_DIR="/var/log/elpusk/00000006/coffee_manager"
gCA_CERT_FILE="/usr/local/share/ca-certificates/ca-coffee_server.crt"
gCA_CERT_ALIAS="ca-coffee_server"

# create directory 
mkdir -p ${gBUILD_DIR}/etc/systemd/system/
mkdir -p ${gBUILD_DIR}${gCF_ROOT_PD_DIR}/bin/
mkdir -p ${gBUILD_DIR}${gCF_ROOT_PD_DIR}/so/
mkdir -p ${gBUILD_DIR}${gCF_ROOT_DD_DIR}/elpusk-hid-d/
mkdir -p ${gBUILD_DIR}${gCF_ROOT_DD_DIR}/data/filter/
mkdir -p ${gBUILD_DIR}${gCF_ROOT_DD_DIR}/root/
mkdir -p ${gBUILD_DIR}${gCF_ROOT_DD_DIR}/tg_lpu237_dll/
mkdir -p ${gBUILD_DIR}${gCF_ROOT_DD_DIR}/data/server/
mkdir -p ${gBUILD_DIR}${gCF_ROOT_LOG_DIR}/elpusk-hid-d/
mkdir -p ${gBUILD_DIR}${gCF_ROOT_LOG_DIR}/tg_lpu237_dll/


# copy files for building package.
cp ${gUSR_HOME_DIR}/projects/LiElpuskHidDaemon/bin/x64/Release/elpusk-hid-d ${gBUILD_DIR}${gCF_ROOT_PD_DIR}/bin/
cp ${gUSR_HOME_DIR}/projects/li_lpu237_dll/bin/x64/Release/libtg_lpu237_dll.so.6.0.0 ${gBUILD_DIR}${gCF_ROOT_PD_DIR}/so/
cp ${gUSR_HOME_DIR}/projects/LiElpuskHidDaemon/job/library.cpp.lpu230/shared/projects/ElpuskHidDaemon/elpusk-hid-d.xml ${gBUILD_DIR}${gCF_ROOT_DD_DIR}/elpusk-hid-d/
cp ${gUSR_HOME_DIR}/projects/LiElpuskHidDaemon/job/library.cpp.lpu230/shared/projects/ElpuskHidDaemon/device.xml ${gBUILD_DIR}${gCF_ROOT_DD_DIR}/data/filter/
cp ${gUSR_HOME_DIR}/projects/li_lpu237_dll/shared/projects/lpu237_dll/tg_lpu237_dll.xml ${gBUILD_DIR}${gCF_ROOT_DD_DIR}/tg_lpu237_dll/


# create DEBIAN directory 
mkdir -p ${gDEBIAN_DIR}

# create control file
######################################
cat <<EOF > ${gDEBIAN_DIR}/control
Package: ${gPACKAGE_NAME}
Version: ${gPACKAGE_VERSION}
Architecture: ${gARCHITECTURE}
Maintainer: Elpusk<elpusk@naver.com>
Description: Coffee Manager Package
EOF
###################################### end of control


# create coffee-manager.service
######################################
cat <<EOF > ${gBUILD_DIR}/etc/systemd/system/coffee-manager.service
[Unit]
Description=coffee manager v2.01
After=network.target

[Service]
Type=forking
PIDFile=/var/run/elpusk-hid-d.pid
ExecStart=${gCF_ROOT_PD_DIR}/bin/elpusk-hid-d
Restart=on-failure
RestartSec=3s
User=root
Group=root

[Install]
WantedBy=multi-user.target
EOF
###################################### end of coffee-manager.service


# create postinst script (postwork after installation)
######################################
cat <<EOF > ${gDEBIAN_DIR}/postinst
#!/bin/bash
set -e

# run elpusk-hid-d with /cert option. create certificate
${gCF_ROOT_PD_DIR}/bin/elpusk-hid-d /cert

# register self-signed ca-root certificate to Chromium & Firefox.
# loop for all user
for USER_HOME in /home/*; do
    # is home?
    if [ -d "\${USER_HOME}" ]; then
        # find Firefox profile directory
        for PROFILE_DIR in "\${USER_HOME}"/.mozilla/firefox/*.default-*; do
            if [ -d "\${PROFILE_DIR}" ]; then
                # Add cert to Firefox profile NSS DB
                certutil -A -n "${gCA_CERT_ALIAS}" -t "C,," -i "${gCA_CERT_FILE}" -d sql:"\${PROFILE_DIR}"

                if [ $? -eq 0 ]; then
                    echo "Successfully installed certificate to \${PROFILE_DIR}"
                else
                    echo "Failed to install certificate to \${PROFILE_DIR}"
                fi
            fi
        done

        # find Chromium nssdb directory
        NSSDB_DIR="\${USER_HOME}/.pki/nssdb" 
        if [ -d "\${NSSDB_DIR}" ]; then
            # Add cert to Chromium NSS DB
            certutil -A -n "${gCA_CERT_ALIAS}" -t "C,," -i "${gCA_CERT_FILE}" -d sql:"\${NSSDB_DIR}"

            if [ $? -eq 0 ]; then
                echo "Successfully installed certificate. \${NSSDB_DIR}"
            else
                echo "Failed to install certificate. \${NSSDB_DIR}"
            fi
        fi
    fi
done

# register elpusk-hid-d daemon. and start it.
chmod +x ${gCF_ROOT_PD_DIR}/bin/elpusk-hid-d
systemctl daemon-reload
systemctl enable coffee-manager.service
systemctl start coffee-manager.service

exit 0
EOF
###################################### end of postinst


# postinst . set permission to file
chmod 755 ${gDEBIAN_DIR}/postinst

# prerm create script(before uninstallation)
######################################
cat <<EOF > ${gDEBIAN_DIR}/prerm
#!/bin/bash
set -e

# stop daemon and remove daemon
if [ -f /etc/systemd/system/coffee-manager.service ]; then
    systemctl stop coffee-manager.service
    systemctl disable coffee-manager.service
    rm -f /etc/systemd/system/coffee-manager.service
    systemctl daemon-reload
fi

# unregister self-signed ca-root certificate from Chromium & Firefox.
# loop for all user
for USER_HOME in /home/*; do
    # is home?
    if [ -d "\${USER_HOME}" ]; then
        # find Firefox profile directory
        for PROFILE_DIR in "\${USER_HOME}"/.mozilla/firefox/*.default-*; do
            if [ -d "\${PROFILE_DIR}" ]; then
                # remove cert from Firefox profile NSS DB
                certutil -D -n "${gCA_CERT_ALIAS}" -d sql:"\${PROFILE_DIR}"

                if [ $? -eq 0 ]; then
                    echo "Successfully removed certificate to \${PROFILE_DIR}"
                else
                    echo "Failed to remove certificate to \${PROFILE_DIR}"
                fi
            fi
        done

        # find Chromium nssdb directory
        NSSDB_DIR="\${USER_HOME}/.pki/nssdb" 
        if [ -d "\${NSSDB_DIR}" ]; then
            # Add cert to Chromium NSS DB
            certutil -D -n "${gCA_CERT_ALIAS}" -d sql:"\${NSSDB_DIR}"

            if [ $? -eq 0 ]; then
                echo "Successfully installed certificate \${NSSDB_DIR}"
            else
                echo "Failed to install certificate \${NSSDB_DIR}"
            fi
        fi

    fi
done

# run elpusk-hid-d with /removecert option. remove certificate
${gCF_ROOT_PD_DIR}/bin/elpusk-hid-d /removecert

# removed the generated cert & key files
if [ -f ${gCF_ROOT_DD_DIR}/data/server/coffee_server.crt ]; then
    rm -f ${gCF_ROOT_DD_DIR}/data/server/coffee_server.crt
fi

if [ -f ${gCF_ROOT_DD_DIR}/data/server/coffee_server.key ]; then
    rm -f ${gCF_ROOT_DD_DIR}/data/server/coffee_server.key
fi

# remove log files
if [ -d ${gCF_ROOT_LOG_DIR}/elpusk-hid-d ]; then
    rm -rf ${gCF_ROOT_LOG_DIR}/elpusk-hid-d/*
fi

if [ -d ${gCF_ROOT_LOG_DIR}/tg_lpu237_dll ]; then
    rm -rf ${gCF_ROOT_LOG_DIR}/tg_lpu237_dll/*
fi

exit 0
EOF
###################################### end of prerm

# prerm  set execution permission to file.
chmod 755 ${gDEBIAN_DIR}/prerm

# create .deb package.
dpkg-deb --build ${gBUILD_DIR}
