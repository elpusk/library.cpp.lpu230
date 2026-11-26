@echo off
echo %1

echo compressed GUID generator V1.0

call :SUB_GEN_COMPRESS_GUID_FROM_GUID "%1"

pause
exit /b 0

:: ============================================================================
:: the end of bat
:: ============================================================================

:: ============================================================================
:: sub routines
:: ============================================================================


:: ============================================================================
:: remove product code & compressed product code key
:: parameter 1'st - GUID
:: using ex) call :SUB_GEN_COMPRESS_GUID_FROM_GUID "{6E6146BA-E137-44DC-BE02-AF856C0C5D2D}"
:SUB_GEN_COMPRESS_GUID_FROM_GUID
set "INST_GUID=%~1"

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

echo ORG : %INST_GUID%
echo COMPRESSED GUID : %INST_COMPRESS_GUID%

exit /b 0
