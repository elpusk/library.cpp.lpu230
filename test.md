# TEST

- setting 프로그램은 단독 실행 시, 이상 없으면 OK
- msr read 는 단독 실행 시, 이상 없으면 OK
- ibutton read 는 1, 2 개 단독, 동시 실행 시, 이상 없으면 OK
- msr read 1개, ibutton read 1, 2 개 실행 시, 이상 없으면 OK

## Windows

### single

다음 각 프로그램을 단독 동작 시.

- Webapp
  - [Webmapper](https://elpusk.github.io/library.js.coffee.2nd/app/webmapper/) : OK
  - [webmsr-read](https://elpusk.github.io/library.js.coffee.2nd/app/webmsr-read/) : OK
  - [webibutton-read](https://elpusk.github.io/library.js.coffee.2nd/app/webibutton-read/) : OK

- native app
  - msr read : test_mfc_tg_lpu237_dll.exe : OK
  - ibutton read : test_tg_lpu237_ibutton.exe : OK
  - fw update : test_tg_lpu237_fw.exe : OK

- OPOS app
  - msr read : OposTest2.exe : OK
  - ibutton read : test_opos_lock.exe : OK

- MCP
  - Claude amd64 only : OK

### double

다음 프로그램을 두 개 동작 시.

- Webapp
  - [webmsr-read](https://elpusk.github.io/library.js.coffee.2nd/app/webmsr-read/) 2개 : OK
  - [webibutton-read](https://elpusk.github.io/library.js.coffee.2nd/app/webibutton-read/) 2개 : OK
  - [webmsr-read](https://elpusk.github.io/library.js.coffee.2nd/app/webmsr-read/) 과 [webibutton-read](https://elpusk.github.io/library.js.coffee.2nd/app/webibutton-read/) : OK

- native app
  - msr read : test_mfc_tg_lpu237_dll.exe 2개 : OK
  - ibutton read : test_tg_lpu237_ibutton.exe 2개 : OK
  - msr & ibutton read : test_mfc_tg_lpu237_dll.exe & test_tg_lpu237_ibutton.exe : OK

- OPOS app
  - msr read : OposTest2.exe 2개 : OK
  - ibutton read : test_opos_lock.exe 2개 : OK

### triple
 
다음 프로그램을 세 개 동작 시.

- msr read : OK
  - [webmsr-read](https://elpusk.github.io/library.js.coffee.2nd/app/webmsr-read/)
  - test_mfc_tg_lpu237_dll.exe
  - OposTest2.exe

- ibutton read : OK
  - [webibutton-read](https://elpusk.github.io/library.js.coffee.2nd/app/webibutton-read/)
  - test_tg_lpu237_ibutton.exe
  - test_opos_lock.exe

- msr & ibutton read
  - case 1 : OK
    - [webmsr-read](https://elpusk.github.io/library.js.coffee.2nd/app/webmsr-read/)
    - [webibutton-read](https://elpusk.github.io/library.js.coffee.2nd/app/webibutton-read/)
    - test_tg_lpu237_ibutton.exe
  - case 2 : OK
    - [webmsr-read](https://elpusk.github.io/library.js.coffee.2nd/app/webmsr-read/)
    - [webibutton-read](https://elpusk.github.io/library.js.coffee.2nd/app/webibutton-read/)
    - test_opos_lock.exe
  - case 3 : OK
    - [webmsr-read](https://elpusk.github.io/library.js.coffee.2nd/app/webmsr-read/)
    - test_tg_lpu237_ibutton.exe
    - test_opos_lock.exe

## Debian12

### single

다음 각 프로그램을 단독 동작 시.

- Webapp
  - [Webmapper](https://elpusk.github.io/library.js.coffee.2nd/app/webmapper/) : OK
  - [webmsr-read](https://elpusk.github.io/library.js.coffee.2nd/app/webmsr-read/) : OK
  - [webibutton-read](https://elpusk.github.io/library.js.coffee.2nd/app/webibutton-read/) : OK

- native app
  - msr read : tp_li_lpu237.out /msr : OK
  - ibutton read : tp_li_lpu237.out /ibutton : OK
  - fw update : lpu230_update : OK

- MCP
  - Cursor : OK

### double

다음 프로그램을 두 개 동작 시.

- Webapp
  - [webmsr-read](https://elpusk.github.io/library.js.coffee.2nd/app/webmsr-read/) 2개 : OK
  - [webibutton-read](https://elpusk.github.io/library.js.coffee.2nd/app/webibutton-read/) 2개 : OK
  - [webmsr-read](https://elpusk.github.io/library.js.coffee.2nd/app/webmsr-read/) 과 [webibutton-read](https://elpusk.github.io/library.js.coffee.2nd/app/webibutton-read/) : OK

- native app
  - msr read : tp_li_lpu237.out /msr 2개 : OK
  - ibutton read : tp_li_lpu237.out /ibutton 2개 : OK
  - msr & ibutton read : tp_li_lpu237.out /msr & tp_li_lpu237.out /ibutton : OK

### triple
 
다음 프로그램을 세 개 동작 시.

- msr & ibutton read : OK
  - [webmsr-read](https://elpusk.github.io/library.js.coffee.2nd/app/webmsr-read/)
  - [webibutton-read](https://elpusk.github.io/library.js.coffee.2nd/app/webibutton-read/)
  - tp_li_lpu237.out /ibutton
