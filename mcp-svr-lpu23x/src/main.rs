use libloading::{Library, Symbol};
use std::sync::Arc;
use tokio::sync::mpsc;
use std::path::PathBuf;

/**
 * dll 이나 so 의 함수가 return 하는 결과값.
 */
pub const LPU237_DLL_RESULT_SUCCESS: libc::c_ulong = 0;
pub const LPU237_DLL_RESULT_ERROR: libc::c_ulong = 0xFFFF_FFFF;
pub const LPU237_DLL_RESULT_CANCEL: libc::c_ulong = 0xFFFF_FFFE;
pub const LPU237_DLL_RESULT_ERROR_MSR: libc::c_ulong = 0xFFFF_FFFD;
pub const LPU237_DLL_RESULT_ICC_INSERTED: libc::c_ulong = 0xFFFF_FFFC;
pub const LPU237_DLL_RESULT_ICC_REMOVED: libc::c_ulong = 0xFFFF_FFFB;

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

/**
 * --- C 라이브러리 함수 시그니처 정의 ---
 */
//unsigned long _CALLTYPE_ LPU237_dll_on();
type LpuOnFn = unsafe extern "system" fn() -> libc::c_ulong;

//unsigned long _CALLTYPE_ LPU237_dll_off();
type LpuOffFn = unsafe extern "system" fn() -> libc::c_ulong;

//unsigned long _CALLTYPE_ LPU237_get_list(wchar_t* ssDevPaths);
type LpuGetListFn = unsafe extern "system" fn(ss_dev_paths:*mut u16) -> libc::c_ulong;

//HANDLE _CALLTYPE_ LPU237_open(const wchar_t* sDevPath);
type LpuOpenFn = unsafe extern "system" fn(s_dev_path:*const u16) -> HANDLE;

//unsigned long _CALLTYPE_ LPU237_close(HANDLE hDev);
type LpuCloseFn = unsafe extern "system" fn(h_dev:HANDLE) -> libc::c_ulong;

//unsigned long _CALLTYPE_ LPU237_enable(HANDLE hDev);
type LpuEnableFn = unsafe extern "system" fn(h_dev:HANDLE) -> libc::c_ulong;

//unsigned long _CALLTYPE_ LPU237_disable(HANDLE hDev);
type LpuDisableFn = unsafe extern "system" fn(h_dev:HANDLE) -> libc::c_ulong;

//unsigned long _CALLTYPE_ LPU237_wait_swipe_with_callback(HANDLE hDev, type_callback pFun, void* pParameter);
//typedef	void (_CALLTYPE_* type_callback)(void*);
type TypeCallback = unsafe extern "system" fn(p_user:*mut libc::c_void);
type LpuWaitSwipeCbFn = unsafe extern "system" fn(h_dev:HANDLE, p_fun:TypeCallback,p_parameter:*mut libc::c_void) -> libc::c_ulong;

//unsigned long _CALLTYPE_ LPU237_get_data(unsigned long dwBufferIndex, unsigned long dwIsoTrack, unsigned char* sTrackData);
type LpuGetDataFn = unsafe extern "system" fn(dw_buffer_index: libc::c_ulong, dw_iso_track:libc::c_ulong, s_track_data:*mut u8) -> libc::c_ulong;

//unsigned long _CALLTYPE_ LPU237_cancel_wait_swipe(HANDLE hDev);
type LpuCancelWaitSwipeFn = unsafe extern "system" fn(h_dev:HANDLE) -> libc::c_ulong;


// local 함수.
fn local_is_invalid(handle: HANDLE) -> bool {
    handle == INVALID_HANDLE_VALUE
}

fn local_get_dll_path() -> PathBuf {
    #[cfg(target_os = "windows")]
    {
        // ProgramFiles 경로 얻기 (x64/x86 구분 포함)
        let base = if cfg!(target_pointer_width = "64") {
            std::env::var("ProgramFiles")
        } else {
            std::env::var("ProgramFiles(x86)")
        }.expect("ProgramFiles env not found");

        let arch_dir = if cfg!(target_pointer_width = "64") {
            "x64"
        } else {
            "x86"
        };

        PathBuf::from(base)
            .join("elpusk")
            .join("00000006")
            .join("coffee_manager")
            .join("dll")
            .join(arch_dir)
            .join("tg_lpu237_dll.dll")
    }

    #[cfg(target_os = "linux")]
    {
        PathBuf::from("/usr/share/elpusk/program/00000006/coffee_manager/so/libtg_lpu237_dll.so")
    }
}

fn local_parse_multi_sz(buf: &[u16]) -> Vec<String> {
    let mut result = Vec::new();
    let mut start = 0;

    for i in 0..buf.len() {
        if buf[i] == 0 {
            if start == i {
                // double null → 끝
                break;
            }

            let slice = &buf[start..i];
            result.push(String::from_utf16_lossy(slice));

            start = i + 1;
        }
    }

    result
}

fn local_to_wide_null(s: &str) -> Vec<u16> {
    s.encode_utf16().chain(std::iter::once(0)).collect()
}

struct LpuManager {
    lib: Library,
}

impl LpuManager {

    /**
     * 생성자로 tg_lpu237_dll.dll 이나 lintg_lpu237_dll.so( 이하 dll ) 를 dynamic binding 한다.
     */
    fn new() -> Self {
        let dll_path = local_get_dll_path();
        let lib = unsafe { Library::new(dll_path).expect("Failed to load cf2 library") };
        Self { lib }
    }

    /**
     * dll 의 초기화 함수를 호출.
     */
    unsafe fn dll_on(&self) -> bool {
        let mut b_result: bool = false;
        let func: Symbol<LpuOnFn> = unsafe { self.lib.get(b"LPU237_dll_on").unwrap() };

        unsafe {
        if func() == LPU237_DLL_RESULT_SUCCESS{
            b_result = true;
        }
        }

        return b_result;
    }

    /**
     * dll unbinding 하기 전에, 정리한다.
     */
    unsafe fn dll_off(&self){
        let func: Symbol<LpuOffFn> = unsafe { self.lib.get(b"LPU237_dll_off").unwrap() };
        unsafe { 
        func();
        }
    }

    /**
     * 현재 연결된 lpu237 장비의 모든 device path 를 얻는다.
     */
    unsafe fn dll_get_get_list(&self) -> Vec<String>{
        let mut  l_result:libc::c_ulong =  LPU237_DLL_RESULT_ERROR;
        let func: Symbol<LpuGetListFn> = unsafe { self.lib.get(b"LPU237_get_list").unwrap() };
        unsafe { 
            l_result = func(std::ptr::null_mut());
            if l_result == LPU237_DLL_RESULT_ERROR{
                return Vec::new();
            }
            //
            let mut v_device_paths: Vec<u16> = vec![0; l_result as usize];
            l_result = func(v_device_paths.as_mut_ptr());
            if l_result == LPU237_DLL_RESULT_ERROR{
                return Vec::new();
            }

            let v_paths: Vec<String> = local_parse_multi_sz(&v_device_paths);
            return v_paths;
        }
    }

    /**
     * 주어진 device path 에 해당하는 lpu237 장비의 통신 채널을 연다.
     */
    unsafe fn dll_open(&self,s_path:&String) -> HANDLE{
        let mut h_dev:HANDLE = INVALID_HANDLE_VALUE;
        let func: Symbol<LpuOpenFn> = unsafe { self.lib.get(b"LPU237_open").unwrap() };
        unsafe { 
            let v_path = local_to_wide_null(s_path);
            h_dev = func(v_path.as_ptr());
        }
        return h_dev;
    }

    /**
     * 해당 device handle 의 lpu237 장비 통신 채널을 닫는다.
     */
    unsafe fn dll_close(&self,h_dev:HANDLE){
        let func: Symbol<LpuCloseFn> = unsafe { self.lib.get(b"LPU237_close").unwrap() };
        unsafe { 
            func(h_dev);
        }
    }

    /**
     * 해당 device handle 의 lpu237 장비가 수신되는 카드 데이터 인식하도록 한다.
     */
    unsafe fn dll_enable(&self,h_dev:HANDLE) -> bool{
        let mut b_result = false;
        let func: Symbol<LpuEnableFn> = unsafe { self.lib.get(b"LPU237_enable").unwrap() };
        unsafe { 
            if func(h_dev) == LPU237_DLL_RESULT_SUCCESS{
                b_result = true;
            }
        }
        return b_result;
    }

    /**
     * 해당 device handle 의 lpu237 장비가 수신되는 카드 데이터 무시하도록 한다.
     */
    unsafe fn dll_disable(&self,h_dev:HANDLE) -> bool{
        let mut b_result = false;
        let func: Symbol<LpuDisableFn> = unsafe { self.lib.get(b"LPU237_disable").unwrap() };
        unsafe { 
            if func(h_dev) == LPU237_DLL_RESULT_SUCCESS{
                b_result = true;
            }
        }
        return b_result;
    }

    /**
     * 해당 device handle 의 lpu237 장비를 카드 데이터 수신 대기 상태로 전환한다.
     * 정상 카드 데이터 수신 또는 에러 수신시, p_fun 콜백함수가 p_parameter 를 인자로 호출된다.
     * 정상적으로 대기상태가 되면 LPU237_DLL_RESULT_ERROR 가 아닌 수신 데이터 저장 버퍼의  index 값을 반환.
     */
    unsafe fn dll_wait_swipe_with_callback(&self,h_dev:HANDLE,p_fun:TypeCallback,p_parameter:*mut libc::c_void) ->libc::c_ulong{
        let mut  l_result:libc::c_ulong =  LPU237_DLL_RESULT_ERROR;
        let func: Symbol<LpuWaitSwipeCbFn> = unsafe { self.lib.get(b"LPU237_wait_swipe_with_callback").unwrap() };
        unsafe { 
            l_result = func(h_dev,p_fun,p_parameter);
        }

        return l_result;
    }

    /**
     * 해당 device handle 의 lpu237 장비가 현재 카드 데이터 수신 대기 상태이면, 대기를 취소한다.
     */
    unsafe fn dll_cancel_wait_swipe(&self,h_dev:HANDLE) ->bool{
        let mut b_result = false;
        let func: Symbol<LpuCancelWaitSwipeFn> = unsafe { self.lib.get(b"LPU237_cancel_wait_swipe").unwrap() };
        unsafe { 
            if func(h_dev) == LPU237_DLL_RESULT_SUCCESS{
                b_result = true;
            }
        }
        return b_result;
    }

    /**
     * 수신된 ASCII code 카드 데이터를  얻는다.
     * 
     * dw_buffer_index - dll_wait_swipe_with_callback() 로 부터 얻은 수신 버퍼 index.
     */
    unsafe  fn dll_get_date(&self
        , dw_buffer_index: libc::c_ulong
    ) -> bool{
        let mut b_result = false;
        let mut u_result: libc::c_ulong = LPU237_DLL_RESULT_ERROR;

        let func: Symbol<LpuGetDataFn> = unsafe { self.lib.get(b"LPU237_get_data").unwrap() };
        let mut  ss_iso = ["","",""];
        unsafe {
            for n_tack in [1,2,3] {
                b_result = true;

                u_result = func(dw_buffer_index,n_tack,std::ptr::null_mut());
                if u_result == LPU237_DLL_RESULT_ERROR{
                    b_result = false;
                }
                else if u_result == LPU237_DLL_RESULT_CANCEL{
                    b_result = false;
                }
                else{
                    let mut v_iso: Vec<u8> = vec![0; u_result as usize];
                    func(dw_buffer_index,n_tack,v_iso.as_mut_ptr());
                    // ASCII code 가 들어 있는 v_iso 버퍼(NULL 로 끝나지 않음) 값을 String 값으로 변경한다.
                    ss_iso[n_tack-1] = String::from_utf8_lossy(&v_iso).into_owned();
                }
            }//end for
        }
        return b_result;
    }
}

#[tokio::main]
async fn main() {
    let lpu = Arc::new(LpuManager::new());

    let mut l_result = LPU237_DLL_RESULT_ERROR;
    let mut  b_result = false;

    loop {
        b_result = unsafe{ lpu.dll_on() };
        if !b_result{
            break;
        }
        
        let mut v_dev_paths = unsafe{ lpu.dll_get_get_list() };
        if v_dev_paths.len() == 0{
            break;
        }

        let h_dev = unsafe{ lpu.dll_open(&v_dev_paths[0]) };
        if h_dev == INVALID_HANDLE_VALUE{
            break;
        }

        b_result = unsafe{ lpu.dll_enable(h_dev) };
        if !b_result{
            unsafe{ lpu.dll_close(h_dev) };
            break;
        }

        l_result = unsafe{ lpu.dll_wait_swipe_with_callback(h_dev,cb_msr,NULL) };
        if l_result == LPU237_DLL_RESULT_ERROR{
            unsafe{ lpu.dll_close(h_dev) };
            break;
        }

        b_result = unsafe{ lpu.dll_disable(h_dev) };
        if !b_result{
            unsafe{ lpu.dll_close(h_dev) };
            break;
        }

        unsafe{ lpu.dll_close(h_dev) };
        unsafe{ lpu.dll_off() };
        
    }
    


}