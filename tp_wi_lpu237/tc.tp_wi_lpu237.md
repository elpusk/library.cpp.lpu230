# Magetic Strip Read

## tc.msr.wi_lpu237_dll.*
+ 목적
  - tg_lpu237_dll.dll 을 사용한 마그네틱 카드 읽기 테스트.
+ 환경
  - Windows 11
  - lpu237 D type ganymede v5.25.
  - ISO 7811-2 포멧의 마그네틱 카드.

## tc.msr.wi_lpu237_dll.threadsafty
+ 목적
  - 스레드에 안전하게 설계된 구조로 tg_lpu237_dll.dll 를 사용한 마그네틱 카드 읽기 테스트.
+ 실행
  - ./elpusk-hid-d.exe --server
  - ./tp_wi_lpu237.exe /msr /threadsafty
  - ISO 1,2,3 트랙에 정보가 있는 신용카드를 읽어 표시되는 카드 정보를 확인.
  - ISO 3 트랙에 정보가 있는 테스트 카드를 읽어 표시되는 내용을 확인.

## tc.msr.wi_lpu237_dll.getdataincb
+ 목적
  - 콜백에서 모든 것을 처리하는 구조로 tg_lpu237_dll.dll 를 사용한 마그네틱 카드 읽기 테스트.
+ 실행
  - ./elpusk-hid-d.exe --server
  - ./tp_wi_lpu237.exe /msr /getdataincallback
  - ISO 1,2,3 트랙에 정보가 있는 신용카드를 읽어 표시되는 카드 정보를 확인.
  - ISO 3 트랙에 정보가 있는 테스트 카드를 읽어 표시되는 내용을 확인.

## tc.msr.wi_lpu237_dll.exclusive
+ 목적
  - tg_lpu237_dll.dll 는 마그네틱 카드 읽기에 대해 독점 모드만 지원.
+ 실행
  - ./elpusk-hid-d.exe --server
  - ./tp_wi_lpu237.exe /msr /threadsafty
  - 다른 콘솔에서 ./tp_wi_lpu237.exe /msr /getdataincallback 실행.
+ 정상
  - 나중에 실행한 msr 읽기는 opnen 실패.

---
# i-button Read

## tc.ibutton.wi_lpu237_ibutton.*
+ 목적
  - tg_lpu237_ibutton.dll 을 사용한 i-button 읽기 테스트.
+ 환경
  - Windows 11
  - lpu237 D type ganymede v5.25.
  - dallas key(i-button).

## tc.ibutton.wi_lpu237_ibutton.threadsafty
+ 목적
  - 스레드에 안전하게 설계된 구조로 tg_lpu237_ibutton.dll 를 사용한 i-button 읽기 테스트.
+ 실행
  - ./elpusk-hid-d.exe --server
  - ./tp_wi_lpu237.exe /ibutton /threadsafty
  - i-button 읽어 표시되는 카드 정보를 확인.

## tc.ibutton.wi_lpu237_ibutton.getdataincb
+ 목적
  - 콜백에서 모든 것을 처리하는 구조로 tg_lpu237_ibutton.dll 를 사용한 i-button 읽기 테스트.
+ 실행
  - ./elpusk-hid-d.exe --server
  - ./tp_wi_lpu237.exe /ibutton /getdataincallback
  - i-button 읽어 표시되는 카드 정보를 확인.

## tc.ibutton.wi_lpu237_ibutton.share
+ 목적
  - tg_lpu237_ibutton.dll 는 i-button 읽기에 대해 공유 모드만 지원.
+ 실행
  - ./elpusk-hid-d.exe --server
  - ./tp_wi_lpu237.exe /ibutton /threadsafty
  - 다른 콘솔에서 ./tp_wi_lpu237.exe /ibutton /getdataincallback 실행.
+ 정상
  - 두 콘솔 모두 i-button 읽기 정상.
