#!/bin/bash

# this scripter format must be Unix/Linux LF format.

# 패키지 정보 설정
PACKAGE_NAME="coffee-manager-2nd"
VERSION="2.0~beta1"
ARCH="amd64" # 또는 'arm64', 'i386' 등 실제 아키텍처에 맞게 수정
DEB_PACKAGE_NAME="${PACKAGE_NAME}_${VERSION}_${ARCH}.deb"
INSTALL_ROOT="opt/${PACKAGE_NAME}" # 패키지 내부의 임시 설치 경로

# setting variables
gCA_CERT_FILE="/usr/local/share/ca-certificates/ca-coffee_server.crt"
gCA_CERT_ALIAS="ca-coffee_server"

# 원본 파일 경로 (사용자 환경에 맞게 수정 필요)
ORIGIN_ELPUSK_HID_D="/home/tester/projects/LiElpuskHidDaemon/bin/x64/Release/elpusk-hid-d"
ORIGIN_LIBTG_LPU237_DLL_XYZ="/home/tester/projects/li_lpu237_dll/bin/x64/Release/libtg_lpu237_dll.so.6.0.0" # <-- x.y.z 실제 버전으로 변경
ORIGIN_LIBTG_LPU237_IBUTTON_XYZ="/home/tester/projects/li_lpu237_ibutton/bin/x64/Release/libtg_lpu237_ibutton.so.6.0.0" # <-- x.y.z 실제 버전으로 변경
ORIGIN_ELPUSK_HID_D_JSON="/home/tester/projects/LiElpuskHidDaemon/job/library.cpp.lpu230/shared/projects/ElpuskHidDaemon/elpusk-hid-d.json"
ORIGIN_TG_LPU237_DLL_INI="/home/tester/projects/li_lpu237_dll/shared/projects/lpu237_dll/tg_lpu237_dll.ini"
ORIGIN_TG_LPU237_IBUTTON_INI="/home/tester/projects/li_lpu237_ibutton/shared/projects/lpu237_ibutton/tg_lpu237_ibutton.ini"

# 임시 빌드 디렉토리 생성
DEB_DIR="./${PACKAGE_NAME}-${VERSION}"
mkdir -p "${DEB_DIR}/DEBIAN"
mkdir -p "${DEB_DIR}/usr/share/elpusk/programdata/00000006/coffee_manager/elpusk-hid-d"
mkdir -p "${DEB_DIR}/usr/share/elpusk/programdata/00000006/coffee_manager/data/server"
mkdir -p "${DEB_DIR}/usr/share/elpusk/programdata/00000006/coffee_manager/root/temp" # root/temp를 만들면 root도 자동으로 생성됨
mkdir -p "${DEB_DIR}/var/log/elpusk/00000006/coffee_manager/elpusk-hid-d"
mkdir -p "${DEB_DIR}/usr/share/elpusk/programdata/00000006/coffee_manager/tg_lpu237_dll"
mkdir -p "${DEB_DIR}/var/log/elpusk/00000006/coffee_manager/tg_lpu237_dll"
mkdir -p "${DEB_DIR}/usr/share/elpusk/programdata/00000006/coffee_manager/tg_lpu237_ibutton"
mkdir -p "${DEB_DIR}/var/log/elpusk/00000006/coffee_manager/tg_lpu237_ibutton"
mkdir -p "${DEB_DIR}/usr/share/elpusk/program/00000006/coffee_manager/bin"
mkdir -p "${DEB_DIR}/usr/share/elpusk/program/00000006/coffee_manager/so"


# 파일 복사
echo "파일 복사 중..."
cp "${ORIGIN_ELPUSK_HID_D_JSON}" "${DEB_DIR}/usr/share/elpusk/programdata/00000006/coffee_manager/elpusk-hid-d/"
cp "${ORIGIN_TG_LPU237_DLL_INI}" "${DEB_DIR}/usr/share/elpusk/programdata/00000006/coffee_manager/tg_lpu237_dll/"
cp "${ORIGIN_TG_LPU237_IBUTTON_INI}" "${DEB_DIR}/usr/share/elpusk/programdata/00000006/coffee_manager/tg_lpu237_ibutton/"
cp "${ORIGIN_ELPUSK_HID_D}" "${DEB_DIR}/usr/share/elpusk/program/00000006/coffee_manager/bin/"
cp "${ORIGIN_LIBTG_LPU237_DLL_XYZ}" "${DEB_DIR}/usr/share/elpusk/program/00000006/coffee_manager/so/"
cp "${ORIGIN_LIBTG_LPU237_IBUTTON_XYZ}" "${DEB_DIR}/usr/share/elpusk/program/00000006/coffee_manager/so/"

# dpkg 빌드 시에는 심볼릭 링크를 직접 생성하지 않습니다.
# 대신, postinst 스크립트에서 생성하도록 처리합니다.

# DEBIAN/control 파일 생성
echo "DEBIAN/control 파일 생성 중..."
cat <<EOF > "${DEB_DIR}/DEBIAN/control"
Package: ${PACKAGE_NAME}
Version: ${VERSION}
Architecture: ${ARCH}
Pre-Depends: libnss3-tools
Maintainer: Elpusk<elpusk@naver.com>
Description: Coffee Manager 2nd Daemon 2.0~beta1
 Provides the necessary daemon and libraries for the coffee manager 2nd system.
EOF


# Systemd 서비스 파일 생성
SYSTEMD_SERVICE_DIR="${DEB_DIR}/etc/systemd/system"
mkdir -p "${SYSTEMD_SERVICE_DIR}"
cat <<EOF > "${SYSTEMD_SERVICE_DIR}/coffee-manager-2nd.service"
[Unit]
Description="coffee manager 2nd"
After=network.target

[Service]
Type=forking
PIDFile=/var/run/elpusk-hid-d.pid
ExecStart=/usr/share/elpusk/program/00000006/coffee_manager/bin/elpusk-hid-d
Restart=always
User=root
Group=root
WorkingDirectory=/usr/share/elpusk/program/00000006/coffee_manager/bin/

[Install]
WantedBy=multi-user.target
EOF

# DEBIAN/preinst 파일 생성 (설치 전 실행)
echo "DEBIAN/preinst 파일 생성 중..."
cat <<EOF > "${DEB_DIR}/DEBIAN/preinst"
#!/bin/bash
set -e

# 기존 서비스 중지 및 등록 말소 (업그레이드 시)
if systemctl is-active --quiet coffee-manager-2nd.service; then
    echo "Stopping coffee-manager-2nd service..."
    systemctl stop coffee-manager-2nd.service || true
fi
if systemctl is-enabled --quiet coffee-manager-2nd.service; then
    echo "Disabling coffee-manager-2nd service..."
    systemctl disable coffee-manager-2nd.service || true
    systemctl daemon-reload || true
fi

# /removecert 및 /removeall 실행은 postrm에서 처리하는 것이 더 적절합니다.
# preinst에서는 주로 설치 전 필요한 정리 작업을 수행합니다.

exit 0
EOF
chmod 755 "${DEB_DIR}/DEBIAN/preinst"

# DEBIAN/postinst 파일 생성 (설치 후 실행)
echo "DEBIAN/postinst 파일 생성 중..."
cat <<EOF > "${DEB_DIR}/DEBIAN/postinst"
#!/bin/bash
set -e

# 설치된 파일/폴더 소유자와 권한 설정
chown -R root:root /usr/share/elpusk/program/00000006/coffee_manager
chown -R root:root /usr/share/elpusk/programdata/00000006/coffee_manager
chown -R root:root /var/log/elpusk/00000006/coffee_manager

chmod 755 /usr/share/elpusk/program/00000006/coffee_manager/bin/elpusk-hid-d
chmod 644 /usr/share/elpusk/programdata/00000006/coffee_manager/elpusk-hid-d/elpusk-hid-d.json
chmod 644 /usr/share/elpusk/programdata/00000006/coffee_manager/tg_lpu237_dll/tg_lpu237_dll.ini
chmod 644 /usr/share/elpusk/programdata/00000006/coffee_manager/tg_lpu237_ibutton/tg_lpu237_ibutton.ini
chmod 755 /var/log/elpusk/00000006/coffee_manager -R

# 서비스 심볼릭 링크 업데이트 및 데몬 리로드

# 심볼릭 링크 생성
SO_DIR="/usr/share/elpusk/program/00000006/coffee_manager/so"
LIBTG_LPU237_DLL_XYZ="libtg_lpu237_dll.so.6.0.0" # <-- x.y.z 실제 버전으로 변경
LIBTG_LPU237_IBUTTON_XYZ="libtg_lpu237_ibutton.so.6.0.0" # <-- x.y.z 실제 버전으로 변경

# libtg_lpu237_dll.so.x.y.z -> libtg_lpu237_dll.so.x
if [ -f "\$SO_DIR/\$LIBTG_LPU237_DLL_XYZ" ]; then
    VERSION_DLL=\$(echo "\$LIBTG_LPU237_DLL_XYZ" | sed -E 's/libtg_lpu237_dll.so.([0-9]+).([0-9]+).([0-9]+)/\1/')
    ln -sf "\$SO_DIR/\$LIBTG_LPU237_DLL_XYZ" "\$SO_DIR/libtg_lpu237_dll.so.\$VERSION_DLL"
    ln -sf "\$SO_DIR/libtg_lpu237_dll.so.\$VERSION_DLL" "\$SO_DIR/libtg_lpu237_dll.so"
fi

# libtg_lpu237_ibutton.so.x.y.z -> libtg_lpu237_ibutton.so.x
if [ -f "\$SO_DIR/\$LIBTG_LPU237_IBUTTON_XYZ" ]; then
    VERSION_IBUTTON=\$(echo "\$LIBTG_LPU237_IBUTTON_XYZ" | sed -E 's/libtg_lpu237_ibutton.so.([0-9]+).([0-9]+).([0-9]+)/\1/')
    ln -sf "\$SO_DIR/\$LIBTG_LPU237_IBUTTON_XYZ" "\$SO_DIR/libtg_lpu237_ibutton.so.\$VERSION_IBUTTON"
    ln -sf "\$SO_DIR/libtg_lpu237_ibutton.so.\$VERSION_IBUTTON" "\$SO_DIR/libtg_lpu237_ibutton.so"
fi


# elpusk-hid-d /cert 실행
BIN_DIR="/usr/share/elpusk/program/00000006/coffee_manager/bin"
if [ -x "\$BIN_DIR/elpusk-hid-d" ]; then
    echo "Running elpusk-hid-d /cert..."
    (cd "\$BIN_DIR" && "\$BIN_DIR/elpusk-hid-d" /cert)
fi

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

echo "Reloading systemd daemon..."
systemctl daemon-reload

# 서비스 활성화 및 시작
echo "Enabling and starting coffee-manager-2nd service..."
systemctl enable coffee-manager-2nd.service
systemctl start coffee-manager-2nd.service

exit 0
EOF
chmod 755 "${DEB_DIR}/DEBIAN/postinst"

# DEBIAN/prerm 파일 생성 (제거 전 실행)
echo "DEBIAN/prerm 파일 생성 중..."
cat <<EOF > "${DEB_DIR}/DEBIAN/prerm"
#!/bin/bash
set -e

# 서비스 중지 및 등록 말소
if systemctl is-active --quiet coffee-manager-2nd.service; then
    echo "Stopping coffee-manager-2nd service..."
    systemctl stop coffee-manager-2nd.service || true
fi
if systemctl is-enabled --quiet coffee-manager-2nd.service; then
    echo "Disabling coffee-manager-2nd service..."
    systemctl disable coffee-manager-2nd.service || true
    systemctl daemon-reload || true
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

# /removecert 실행
BIN_DIR="/usr/share/elpusk/program/00000006/coffee_manager/bin"
if [ "\$1" = "remove" ] || [ "\$1" = "deconfigure" ]; then
    if [ -x "\$BIN_DIR/elpusk-hid-d" ]; then
        echo "Running elpusk-hid-d /removecert..."
        (cd "\$BIN_DIR" && "\$BIN_DIR/elpusk-hid-d" /removecert) || true
    fi
fi

exit 0
EOF
chmod 755 "${DEB_DIR}/DEBIAN/prerm"

# DEBIAN/postrm 파일 생성 (제거 후 실행)
echo "DEBIAN/postrm 파일 생성 중..."
cat <<EOF > "${DEB_DIR}/DEBIAN/postrm"
#!/bin/bash
set -e

# 완전 제거 시에만 추가 정리 작업 수행
# 일반 사용자 계정으로 실행해서 만든 log 파일도 제거.

# 완전 제거
if [ "\$1" = "purge" ]; then
    BIN_DIR="/usr/share/elpusk/program/00000006/coffee_manager/bin"
    if [ -x "\$BIN_DIR/elpusk-hid-d" ]; then
        echo "Running elpusk-hid-d /removeall..."
        (cd "\$BIN_DIR" && "\$BIN_DIR/elpusk-hid-d" /removeall) || true
    fi
fi

BIN_DIR="/usr/share/elpusk/program/00000006/coffee_manager/bin"

echo "Removing directories created by coffee-manager-2nd..."
rm -rf /usr/share/elpusk/programdata/00000006/coffee_manager/ || true
rm -rf /usr/share/elpusk/program/00000006/coffee_manager/ || true
rm -rf /var/log/elpusk/00000006/coffee_manager/ || true

# 상위 디렉토리 정리 (비어있을 경우)
rmdir --ignore-fail-on-non-empty /usr/share/elpusk/programdata/00000006/ || true
rmdir --ignore-fail-on-non-empty /usr/share/elpusk/programdata/ || true
rmdir --ignore-fail-on-non-empty /usr/share/elpusk/program/00000006/ || true
rmdir --ignore-fail-on-non-empty /usr/share/elpusk/program/ || true
rmdir --ignore-fail-on-non-empty /usr/share/elpusk/ || true
rmdir --ignore-fail-on-non-empty /var/log/elpusk/00000006/ || true
rmdir --ignore-fail-on-non-empty /var/log/elpusk/ || true

for USER_HOME in /home/*; do
    if [ -d "\${USER_HOME}" ]; then
        BASE_DIR="\${USER_HOME}/.elpusk"

        # 로그 파일 삭제 (존재하는 경우)
        LOG_DIR="\${BASE_DIR}/log/00000006/coffee_manager"
        echo "find log directory: \${LOG_DIR}/tg_lpu237_dll"

        if [ -d "\${LOG_DIR}/tg_lpu237_dll" ]; then
            echo "user log directory: \${LOG_DIR}/tg_lpu237_dll"
            echo "delete log file:"
            find "\${LOG_DIR}/tg_lpu237_dll" -type f -name "*.txt"
            find "\${LOG_DIR}/tg_lpu237_dll" -type f -name "*.txt" -exec rm -f {} \;
        fi

        echo "find log directory: \${LOG_DIR}/tg_lpu237_ibutton"
        if [ -d "\${LOG_DIR}/tg_lpu237_ibutton" ]; then
            echo "user log directory: \${LOG_DIR}/tg_lpu237_ibutton"
            echo "delete log file:"
            find "\${LOG_DIR}/tg_lpu237_ibutton" -type f -name "*.txt"
            find "\${LOG_DIR}/tg_lpu237_ibutton" -type f -name "*.txt" -exec rm -f {} \;
        fi

        # .elpusk 하위 비어 있는 디렉토리 제거 (깊은 곳부터)
        if [ -d "\${BASE_DIR}" ]; then
            find "\${BASE_DIR}" -depth -type d -empty -exec rmdir {} \;
        fi
    fi
done

exit 0
EOF
chmod 755 "${DEB_DIR}/DEBIAN/postrm"

# DEBIAN/conffiles 파일 생성 (설정 파일 명시, 제거 시 삭제되지 않도록)
echo "DEBIAN/conffiles 파일 생성 중..."
cat <<EOF > "${DEB_DIR}/DEBIAN/conffiles"
/etc/systemd/system/coffee-manager-2nd.service
EOF


# dpkg-deb로 .deb 패키지 빌드
echo "deb 패키지 빌드 중..."
dpkg-deb --build --root-owner-group "${DEB_DIR}" "${DEB_PACKAGE_NAME}"

# 임시 디렉토리 정리
echo "임시 디렉토리 정리 중..."
rm -rf "${DEB_DIR}"

echo "==================================================="
echo "패키지 빌드 완료: ${DEB_PACKAGE_NAME}"
echo "==================================================="
echo "설치 명령어: sudo dpkg -i ${DEB_PACKAGE_NAME}"
# echo "제거 명령어: sudo dpkg -r ${PACKAGE_NAME}"
echo "완전 제거 명령어: sudo dpkg -P ${PACKAGE_NAME}"
echo ""
echo "!!! 중요: 'libtg_lpu237_dll.so.x.y.z'와 'libtg_lpu237_ibutton.so.x.y.z'의 'x.y.z' 부분을 실제 파일 버전에 맞게 수정해야 합니다."