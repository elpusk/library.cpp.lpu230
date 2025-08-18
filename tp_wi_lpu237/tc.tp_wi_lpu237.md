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
