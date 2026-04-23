# lpu237 용 MCP server

- rust 언어 사용.
- RMCP sdk 사용.
- msr(magnetic card reading) 를 위한 MCP server 와 ibutton(i-button reading) 를 위한 MCP server 두개 제작.
- 각 MCP server 는 windows x86, x64 용과, 리눅스 x64 용으로 빌드 가능해야함.
- 개발 환경은 vscode 를 표준으로 한다.
- 모든 하위 project 에서 공용으로 사용하는 것은 별도의 재사용 가능한 라이브러리로 만들어서 사용.
  - 특히 HANDLE 이나 INVALID_HANDLE_VALUE 은 별도 라이브러리로 독립 시키고 아래 와 같이 정의해서 사용 할 것.

``` rust
/**
 * OS 별, 차이에 따른 정의
 */
#[cfg(windows)]
#[repr(transparent)]
#[derive(Copy, Clone, PartialEq, Eq)]
pub struct HANDLE(pub *mut core::ffi::c_void);

#[cfg(not(windows))]
#[repr(transparent)]
pub struct HANDLE(pub u32);

#[cfg(windows)]
pub const INVALID_HANDLE_VALUE: HANDLE = HANDLE((-1isize) as *mut core::ffi::c_void);

#[cfg(not(windows))]
pub const INVALID_HANDLE_VALUE: HANDLE = (-1i32) as u32;
```

## msr 용 MCP server

- lpu237 msr 은 ../shared/projects/lpu237_dll/inc/tg_lpu237_dll.h 를 참고, tg_lpu237_dll.dll(리눅스 경우 libtg_lpu237_dll.so) 를 동적 바인딩하여 사용.
- tg_lpu237_dll.dll 사용을 위한 rust wrapper struct 를 재사용 가능한 라이브러리로 만들어서 사용.
- MCR server 는 tg_lpu237_dll.dll 용 라이브러리를 사용해서 제작.

## ibutton 용 MCP server

- lpu237 ibutton 은 ../shared/projects/lpu237_ibutton/inc/tg_lpu237_ibutton.h 를 참고, tg_lpu237_ibutton.dll(리눅스 경우 libtg_lpu237_ibutton.so) 를 동적 바인딩하여 사용.
- tg_lpu237_ibutton.dll 사용을 위한 rust wrapper struct 를 재사용 가능한 라이브러리로 만들어서 사용.
- MCR server 는 tg_lpu237_ibutton.dll 용 라이브러리를 사용해서 제작.

