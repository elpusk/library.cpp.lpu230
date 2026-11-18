# lpu230_opos.exe TestCase

## msr or keylock SO 가 등록되기 전
- 둘 중 하나라도 등록이 되어있지 않으면 에러표시 후, 종료(v1.0)
- 둘 모두 등록이 되어있지 않으면 에러표시 후, 종료(v1.1)

## New Device Name 의 유효성
- New Device Name 아래와 같은 조건을 만족하면 해당 Apply button 활성화
  - 1~30자 사이의 길이
  - 공백 문자 없음
  - backslash(\) 없음
  - 화면에 표시 가능한 영문 및 특수문자만 사용(ASCII 0x21~0x7E))

## Device Name 변경
- 변경된 Physical Device Name이 registry에 적용
- 변경된 Logical Device Name이 registry에 적용

## 프로그램 종료
- 변경된 사항이 있을 때
  - 변경된 사항을 저장할 것인지 묻는 메시지 표시
  - cancel 선택하면 프로그램 유지
  - No 선택 시 프로그램 종료
  - Yes 선택 시 변경된 사항을 registry에 적용 후 프로그램 종료
	
- 변경된 사항이 없을 때
  - 프로그램 종료

## 등록 삭제
- Device Name 변경 후, 등록된 SO를 삭제하면 registry에 적용
  - `regsvr32 /u oposlpu230.dll`
  - `regsvr32 /u oposlpu230Lock.dll`

## 삭제 후, 재등록
- 재등록하면 기본 device name 으로 등록
 - `regsvr32 oposlpu230.dll` : Lpu230MSR 으로 등록
 - `regsvr32 oposlpu230Lock.dll` : Lpu230Lock 으로 등록