# 정보
- 리눅스에서 coffee-manager(cf,elpusk-hid-d) 가 장비 관리시, 다른 프로그램에서 open 불가

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

- [web app](https://elpusk.github.io/library.js.coffee/tools_lpu237_full.html)
  + lpu237 parameters 일고, 쓰고 저장.
- native application(tp_wi_lpu237.exe, tp_li_lpu237.out)
  + tg_lpu37_dll.dll(libtg_lpu237_dll.so) 를 사용한 상호 배타적 MSR 읽기 기능.
  + tg_lpu37_ibutton.dll(libtg_lpu237_ibutton.so) 를 사용한 공유된 i-button 읽기 기능.



