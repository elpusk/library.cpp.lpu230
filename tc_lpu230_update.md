# 정보
- 리눅스에서 coffee-manager(cf) 가 장비 관리시, 다른 프로그램에서 open 불가

# 장비
- LPC1343 을 사용하는 lpu237 d type.
- LPC1343 을 사용하는 lpu238 d type.
- MH1902T 을 사용하는 lpu237 d type.
- MH1902T 을 사용하는 lpu238 d type.

# firmware
- callisto v3.25(LPC1343)
- ganymede v5.25(LPC1343)
- europa v1.3(LPC1343)
- himalia v2.5(MH1902T)

# 테스트
## 공통
- update 가 장비 open 전에 cf cotrol pipe 를 통해, 해당 장비 관리 해제 요청.
- update 종료 후, cf cntrol pipr 를 통해, 해당 장비 관리 다시 요청.
- cmd.exe 를 관리자 모드로 실행(conhost.exe)


## option 없을때
- rom 파일 선택.
- 선태된 rom 파일에서 firmware 선택.
- firmware 업데이트 전, system parameters 자동 보존.
- firmware 정상 업데이트.
- firmware 업데이트 후, system parameters 복원.


## tested option
### win(관리자 권한 필수)
- 옵션없이 실행. //자동으로 system menu 의 close 가 사용불가되고, 종료 후 다시 사용 가능해짐.
- --file lpu23x_00035.rom //파일 선택창 패스됨.
- -q // 정상( windows subsystem 사용 후)
- -qa //정상.( windows subsystem 사용 후)
- Update-Lock 표시 중, CTL+C 누루면 프로그램 종료됨. // 이 건 문제!!!



