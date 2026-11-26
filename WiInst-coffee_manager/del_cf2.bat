@echo off
chcp 65001 > nul
setlocal EnableExtensions EnableDelayedExpansion

:: tested x86 2.17
:: tested x64 2.17

:: need admin right 
net session >nul 2>&1
if %errorLevel% neq 0 (
    echo restart to admin...
    powershell -Command "Start-Process '%~f0' -Verb RunAs"
    exit /b
)

:: remove coffee manager 2nd
echo coffee manager 2nd  Remover V1.0

set "ARCH=%PROCESSOR_ARCHITECTURE%"
echo initial ARCH: %ARCH%

if defined PROCESSOR_ARCHITEW6432 (
    set "ARCH=%PROCESSOR_ARCHITEW6432%"
)

echo adjusted OS ARCH: %ARCH%

set "PROGRAM_DIR=%ProgramFiles%"
set "PROGRAM_DIR86="

if /i "%ARCH%"=="AMD64" goto :LARCH_X64
if /i "%ARCH%"=="ARM64" goto :LARCH_ARM64
if /i "%ARCH%"=="x86"   goto :LARCH_X86
echo unknown architect : %ARCH%
exit /b 0

:LARCH_ARM64
echo this arm64 architect. Not support!
exit /b 0

:LARCH_X64
echo x64 architect
set "PROGRAM_DIR86=%ProgramFiles(x86)%"
goto :LARCH_DONE

:LARCH_X86
echo x86 architect

:LARCH_DONE

if "!PROGRAM_DIR86!" == "" goto :LPROGRAM_FILE

:: stop service & remove service
call :SUB_REMOVE_SERVICE "!PROGRAM_DIR86!"

:: unregister service object
call :SUB_UNREGISTER_SO "!PROGRAM_DIR86!" "OposLpu230.dll"
call :SUB_UNREGISTER_SO "!PROGRAM_DIR86!" "OposLpu230Lock.dll"

:: unregister certificate
call :SUB_REMOVE_CERTIFICATE "!PROGRAM_DIR86!"

:: remove program files
call :SUB_REMOVE_PRODUCT_FILE_FOLDER "!PROGRAM_DIR86!" "coffee_manager"

::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
LPROGRAM_FILE:

:: stop service & remove service
call :SUB_REMOVE_SERVICE "%PROGRAM_DIR%"

:: unregister service object
call :SUB_UNREGISTER_SO "%PROGRAM_DIR%" "OposLpu230.dll"
call :SUB_UNREGISTER_SO "%PROGRAM_DIR%" "OposLpu230Lock.dll"

:: unregister certificate
call :SUB_REMOVE_CERTIFICATE "%PROGRAM_DIR%"

:: remove program files
call :SUB_REMOVE_PRODUCT_FILE_FOLDER "%PROGRAM_DIR%" "coffee_manager"

::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
echo Delete data Folder.
for /f "tokens=3*" %%a in ('reg query "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\Shell Folders" /v "Common AppData" 2^>nul') do set "COMMON_APPDATA=%%b"
call set "DAT_DIR=%COMMON_APPDATA%"
call :SUB_REMOVE_PRODUCT_FILE_FOLDER "%DAT_DIR%" "coffee_manager"

::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
echo Delete menu Folder.
for /f "tokens=3*" %%a in ('reg query "HKLM\Software\Microsoft\Windows\CurrentVersion\Explorer\Shell Folders" /v "Common Programs" 2^>nul') do set "PROGRAMS_DIR=%%b"
call set "MENU_DIR=%PROGRAMS_DIR%"
call :SUB_REMOVE_PRODUCT_FILE_FOLDER "%MENU_DIR%" "coffee_manager"

:: remove registry keys

::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: v2.6 ~ 2.17
call :SUB_REMOVE_PRODUCTCODE_KEY "v2.6.0" "{44A389AD-8364-4147-BF79-C209221CB66C}"
call :SUB_REMOVE_PRODUCTCODE_KEY "v2.7.0" "{7EEE6616-C8A2-4946-8799-AB48DE22361D}"
call :SUB_REMOVE_PRODUCTCODE_KEY "v2.8.0" "{37B99C8A-FEE0-4F63-BE7E-E0453C413AEF}"
call :SUB_REMOVE_PRODUCTCODE_KEY "v2.9.0" "{0DB9CDDD-AF68-4C9A-8C18-9EDC7F8BB5A2}"
call :SUB_REMOVE_PRODUCTCODE_KEY "v2.10.0" "{9BEFD1A0-59EB-4492-99C9-78A529FC5B20}"
call :SUB_REMOVE_PRODUCTCODE_KEY "v2.11.0" "{5746120D-5888-4DAE-805B-6123B6B21D09}"
call :SUB_REMOVE_PRODUCTCODE_KEY "v2.12.0" "{9E8A2FA1-C02E-4BB1-BC17-7CC7150EED6B}"
call :SUB_REMOVE_PRODUCTCODE_KEY "v2.13.0" "{36134181-8B24-448C-9392-1CFC6FAE436F}"
call :SUB_REMOVE_PRODUCTCODE_KEY "v2.14.0" "{EB201B8C-2F00-4989-A5C6-6F1D90873F25}"
call :SUB_REMOVE_PRODUCTCODE_KEY "v2.15.0" "{65AFBBB8-0C66-429F-A66E-6D0880B44815}"
call :SUB_REMOVE_PRODUCTCODE_KEY "v2.16.0" "{AD6925A2-0FC8-4445-A68D-83A59772B018}"
call :SUB_REMOVE_PRODUCTCODE_KEY "v2.17.0" "{67A138E2-30A8-451B-B900-057E0E32514F}"

::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
set "COMPONENT_ROOT_KEY=HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Installer\UserData\S-1-5-18\Components"
set "REG_BACKUP_FILE=%~dp0components_backup.reg"

echo Backup: Please Waits. "%REG_BACKUP_FILE%"
reg export "%COMPONENT_ROOT_KEY%" "%REG_BACKUP_FILE%" /y >nul
if errorlevel 1 (
    echo [ERROR] backup failed. abort.
    pause
    exit /b 1
)

set /a COUNT=0

call :SUB_REMOVE_SUBKEY_WITH_KEYWORD "elpusk\00000006\coffee_manager" "%COMPONENT_ROOT_KEY%"
call :SUB_REMOVE_SUBKEY_WITH_KEYWORD "Elpusk\00000006\coffee_manager" "%COMPONENT_ROOT_KEY%"

echo.
echo Deleted %COUNT% key(s).
pause
exit /b 0

:: ============================================================================
:: the end of bat
:: ============================================================================

:: ============================================================================
:: sub routines
:: ============================================================================

:: ============================================================================
:: Remove the subkey of the second parameter whose data contains the string of the first parameter.
:: parameter 1'st - keyword string
:: parameter 2'nd - base key
:SUB_REMOVE_SUBKEY_WITH_KEYWORD
echo base key: %~2
echo Searching: %~1
:: /s: recursive, /f: search text, /d: search in value DATA only
:: Each matching subkey is printed once as a "HKEY_..." header line
for /f "delims=" %%K in ('reg query "%~2" /s /f "%~1" /d 2^>nul ^| findstr /b /i "HKEY_"') do (
    echo Deleting: %%K
    reg delete "%%K" /f
    if errorlevel 1 (
        echo [ERROR] delete failed: %%K
    ) else (
        set /a COUNT+=1
    )
)
exit /b 0


:: ============================================================================
:: remove product code & compressed product code key
:: parameter 1'st - product version string 
:: parameter 2'nd - product code string 
:: using ex) call :SUB_REMOVE_PRODUCTCODE_KEY "v1.8.69" "{6E6146BA-E137-44DC-BE02-AF856C0C5D2D}"
:SUB_REMOVE_PRODUCTCODE_KEY
set "INST_VER=%~1"
set "INST_GUID=%~2"

:: get MSI Compressed GUID(INST_COMPRESS_GUID) product code GUID
set "G=%INST_GUID:{=%"
set "G=%G:}=%"
set "P1=%G:~0,8%"
set "P2=%G:~9,4%"
set "P3=%G:~14,4%"
set "P4=%G:~19,2%"
set "P5=%G:~21,2%"
set "P6=%G:~24,2%"
set "P7=%G:~26,2%"
set "P8=%G:~28,2%"
set "P9=%G:~30,2%"
set "P10=%G:~32,2%"
set "P11=%G:~34,2%"

set "INST_COMPRESS_GUID=%P1:~7,1%%P1:~6,1%%P1:~5,1%%P1:~4,1%%P1:~3,1%%P1:~2,1%%P1:~1,1%%P1:~0,1%"
set "INST_COMPRESS_GUID=%INST_COMPRESS_GUID%%P2:~3,1%%P2:~2,1%%P2:~1,1%%P2:~0,1%"
set "INST_COMPRESS_GUID=%INST_COMPRESS_GUID%%P3:~3,1%%P3:~2,1%%P3:~1,1%%P3:~0,1%"
set "INST_COMPRESS_GUID=%INST_COMPRESS_GUID%%P4:~1,1%%P4:~0,1%%P5:~1,1%%P5:~0,1%"
set "INST_COMPRESS_GUID=%INST_COMPRESS_GUID%%P6:~1,1%%P6:~0,1%%P7:~1,1%%P7:~0,1%%P8:~1,1%%P8:~0,1%%P9:~1,1%%P9:~0,1%%P10:~1,1%%P10:~0,1%%P11:~1,1%%P11:~0,1%"

echo [Trying MSI uninstall] %INST_VER%
msiexec /x "%INST_GUID%" /qn /norestart

echo [Cleaning registry] ORG : %INST_GUID%
echo [Cleaning registry] INST_COMPRESS_GUID : %INST_COMPRESS_GUID%
reg delete "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\%INST_GUID%" /f
reg delete "HKLM\SOFTWARE\WOW6432Node\Microsoft\Windows\CurrentVersion\Uninstall\%INST_GUID%" /f
reg delete "HKLM\SOFTWARE\Classes\Installer\Products\%INST_COMPRESS_GUID%" /f
reg delete "HKLM\SOFTWARE\Classes\Installer\Features\%INST_COMPRESS_GUID%" /f
reg delete "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Installer\UserData\S-1-5-18\Products\%INST_COMPRESS_GUID%" /f

exit /b 0

:: ============================================================================
:: remove product file folder
:: parameter 1'st - OS program file folder. "%PROGRAM_DIR%" or "%PROGRAM_DIR86%"
:: parameter 2'nd - product name string. "coffee_manager"
:: using ex) call :SUB_REMOVE_PRODUCT_FILE_FOLDER "%PROGRAM_DIR%" "coffee_manager"
:SUB_REMOVE_PRODUCT_FILE_FOLDER
set "PF_FOLDER=%~1"
set "PRODUCT_NAME=%~2"

set "PRODUCT_FOLDER=%PF_FOLDER%\elpusk\00000006\%PRODUCT_NAME%"

echo delete folder : "%PRODUCT_FOLDER%"

if exist "!PRODUCT_FOLDER!" (
    echo "!PRODUCT_FOLDER!" deleting folder...
    rd /s /q "!PRODUCT_FOLDER!"
    
    if exist "!PRODUCT_FOLDER!" (
        echo [ERROR] deleting folder. Check a running program.
    ) else (
        echo [SUCCESS] delete OK : "!PRODUCT_FOLDER!"

        if exist "!PF_FOLDER!\elpusk\00000006\" (
            rd "!PF_FOLDER!\elpusk\00000006" 2>nul
            if not exist "!PF_FOLDER!\elpusk\00000006" (
                echo [SUCCESS] delete OK : "!PF_FOLDER!\elpusk\00000006"

                if exist "!PF_FOLDER!\elpusk\" (
                    rd "!PF_FOLDER!\elpusk" 2>nul
                    if not exist "!PF_FOLDER!\elpusk" (
                        echo [SUCCESS] delete OK : "!PF_FOLDER!\elpusk"
                    )
                    ) else (
                        echo [INFO] deleting folder. NOT empty folders : "!PF_FOLDER!\elpusk"
                    )
                )
            ) else (
                echo [INFO] deleting folder. NOT empty folders : "!PF_FOLDER!\elpusk\00000006"
            )
        )
    )
) else (
    echo [info] Not found "!PRODUCT_FOLDER!".
)

exit /b 0

:: ============================================================================
:: remove service
:: parameter 1'st - OS program file folder. "%PROGRAM_DIR%" or "%PROGRAM_DIR86%"
:: using ex) call :SUB_REMOVE_CERTIFICATE "%PROGRAM_DIR%"
:SUB_REMOVE_CERTIFICATE
set "PF_FOLDER=%~1"

set "MANAGER_FILE=%PF_FOLDER%\elpusk\00000006\coffee_manager\bin\elpusk-hid-d.exe"

echo remove certificate : "%MANAGER_FILE%"
"%MANAGER_FILE%" --removecert

echo remove all logging : "%MANAGER_FILE%"
"%MANAGER_FILE%" --removeall

exit /b 0

:: ============================================================================
:: remove service
:: parameter 1'st - OS program file folder. "%PROGRAM_DIR%" or "%PROGRAM_DIR86%"
:: using ex) call :SUB_REMOVE_SERVICE "%PROGRAM_DIR%"
:SUB_REMOVE_SERVICE
set "PF_FOLDER=%~1"

set "SERVICE_FILE=%PF_FOLDER%\elpusk\00000006\coffee_manager\bin\coffee_service.exe"

echo stop serivce : "%SERVICE_FILE%"
"%SERVICE_FILE%" -k

echo uninstall service : "%SERVICE_FILE%"
"%SERVICE_FILE%" -u

echo restop server : "%PF_FOLDER%\elpusk\00000006\coffee_manager\bin\elpusk-hid-d.exe"
"%PF_FOLDER%\elpusk\00000006\coffee_manager\bin\elpusk-hid-d.exe" --bye

echo delete service.
sc delete elpusk-hid-d

exit /b 0

:: ============================================================================
:: unregister OPOS service object
:: parameter 1'st - OS program file folder. "%PROGRAM_DIR%" or "%PROGRAM_DIR86%"
:: parameter 2'nd - OPOS service object file name "OposLpu230.dll" or "OposLpu230Lock.dll"
:: using ex) call :SUB_UNREGISTER_SO "%PROGRAM_DIR%" "OposLpu230.dll"
:SUB_UNREGISTER_SO
set "PF_FOLDER=%~1"
set "SO_FNAME=%~2"

set "SO_PATH=%PF_FOLDER%\elpusk\00000006\coffee_manager\dll\x86\%SO_FNAME%"

echo unregister service object : "%SO_PATH%"
regsvr32.exe /s /u "%SO_PATH%"

exit /b 0