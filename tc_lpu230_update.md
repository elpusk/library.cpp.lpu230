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


## option 없을때
- rom 파일 선택.
- 선태된 rom 파일에서 firmware 선택.
- firmware 업데이트 전, system parameters 자동 보존.
- firmware 정상 업데이트.
- firmware 업데이트 후, system parameters 복원.


## tested option
- --file lpu23x_00035.rom
