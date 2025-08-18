# 솔루션
## 이름 
- coffee manager second edition.(이하 cf.2nd)

## 설명
- coffee manager 의 두번째 버전
- secure web socket(이하 wss) 서버를 통한 lpu237,hid 장비 IO 지원.
- Windows 11, Linux(debian 12 와 Ubuntu 24.04) 에서 테스트.

# 개발환경
- C++ 14
- Vsiual Studio 2022(이하 vs2022)
- Linux 는 vs2022 remote ssh 연결을 통한 개발.
- 사용 라이브러리는 README.md 에 영어로 명시.
- 빌드 환경은 README.md 에 영어로 명시.


## 하위 프로젝트
- 프로젝트 이름 규칙
  + L[l]i 로 시작하는 프로젝트,
	+ 64 비트, linux 에서 테스트.
	+ cf.2nd 구성요소 또는 installer.
  + W[w]i 로 시작하는 프로젝트.
	+ 32비트, 64 비트, windows 11 에서 테스트.
	+ cf.2nd 구성요소 또는 installer.
  + tp_li_ 로 시작하는 프로젝트.
	+ 64 비트, linux 에서 테스트.
	+ cf.2nd 구성요소를 테스트하기 위한 테스트 프로그램.
  + tp_wi_ 로 시작하는 프로젝트.
	+ 64 비트, 32 비트, windows 11 에서 테스트.
	+ cf.2nd 구성요소를 테스트하기 위한 테스트 프로그램.
  + L[l]i, W[w]i 또는 tp_li_, tp_wi_ 이 후 이름이 같은 경우.
	+ 동일한 소스코드 공유한 동일한 기능의 프로젝트.

- 프로젝트 설명 : info.md 에 영어로 명시.

## 작업 방법.
- 테스트 전 test case(이하 tc)를 md 파일로 작성.
- tc 파일 이름은 tc.<프로젝트 이름>.md