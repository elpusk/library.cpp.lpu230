# MCP 서버 사용 path 정보

## Windows x64

- lpu23x-ibutton-mcp.exe 에서 사용하는 설정 파일 : %ProgramData%\elpusk\00000006\coffee_manager\mcp\lpu23x-ibutton-mcp.json
- lpu23x-msr-mcp.exe 에서 사용하는 설정 파일 : %ProgramData%\elpusk\00000006\coffee_manager\mcp\lpu23x-msr-mcp.json

## Linux x64

- lpu23x-ibutton-mcp.exe 에서 사용하는 설정 파일 : /usr/share/elpusk/programdata/00000006/coffee_manager/mcp/lpu23x-ibutton-mcp.json
- lpu23x-msr-mcp.exe 에서 사용하는 설정 파일 :  /usr/share/elpusk/programdata/00000006/coffee_manager/mcp/lpu23x-msr-mcp.json

## json 내용

- 파일명이 같으면, Windows 이나 Linux 용, 모두 내용 동일
- 내용은 항상 단순 명확한 영어 문장만 사용
- 키는 tool 이름, 값은 tool 의 description

### lpu23x-ibutton-mcp.json

아래는 lpu23x-ibutton-mcp.json 의 기본 내용

```json
{
    "read_ibutton" : "Synchronous i-button read. Blocks until response or timeout. Use get_ibutton_result() on success.",
    "start_read_ibutton" : "Asynchronous i-button read. Poll get_ibutton_result() every 500 ms–1 s to check for a response.",
    "get_ibutton_result" : "Get i-button result.",
    "cancel_ibutton" : "Set i-button data ignore mode."
}
```

### lpu23x-msr-mcp.json

아래는 lpu23x-msr-mcp.json 의 기본 내용

```json
{
    "read_card" : "Synchronous a magnetic card read. Blocks until response or timeout. Use get_card_result() on success.",
    "start_read_card" : "Asynchronous a magnetic card read. Poll get_card_result() every 500 ms–1 s to check for a response.",
    "get_card_result" : "Get magnetic card result.",
    "cancel_card" : "Set a magnetic card data ignore mode."
}
```
