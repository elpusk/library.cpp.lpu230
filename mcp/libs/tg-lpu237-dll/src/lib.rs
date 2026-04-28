use libloading::Library;
use lpu237_common::{HANDLE, INVALID_HANDLE_VALUE};
use std::sync::Arc;
use std::ffi::OsStr;
use libc::c_ulong;

pub const LPU237_DLL_RESULT_SUCCESS: c_ulong = 0;
pub const LPU237_DLL_RESULT_ERROR: c_ulong = !0; // Equivalent to 0xFFFFFFFF on 32-bit and 0xFFFFFFFFFFFFFFFF on 64-bit
pub const LPU237_DLL_RESULT_CANCEL: c_ulong = !0 - 1;
pub const LPU237_DLL_RESULT_ERROR_MSR: c_ulong = !0 - 2;

pub type TypeCallback = extern "C" fn(*mut std::ffi::c_void);

#[cfg(target_os = "windows")]
pub type WChar = u16;

#[cfg(not(target_os = "windows"))]
pub type WChar = u32;

#[derive(Clone)]
pub struct Lpu237Dll {
    _lib: Arc<Library>,//libloading::Library가 drop 되면 함수 포인터가 무효가 되니까, struct 안에 들고 있으면서 lifetime 유지하려는 것
    // Function pointers using libc::c_ulong for unsigned long
    fn_on: unsafe extern "C" fn() -> c_ulong,
    fn_off: unsafe extern "C" fn() -> c_ulong,
    fn_get_list: unsafe extern "C" fn(*mut WChar) -> c_ulong,
    fn_open: unsafe extern "C" fn(*const WChar) -> HANDLE,
    fn_close: unsafe extern "C" fn(HANDLE) -> c_ulong,
    fn_enable: unsafe extern "C" fn(HANDLE) -> c_ulong,
    fn_disable: unsafe extern "C" fn(HANDLE) -> c_ulong,
    fn_cancel_wait_swipe: unsafe extern "C" fn(HANDLE) -> c_ulong,
    fn_wait_swipe_with_callback: unsafe extern "C" fn(HANDLE, TypeCallback, *mut std::ffi::c_void) -> c_ulong,
    fn_get_data: unsafe extern "C" fn(c_ulong, c_ulong, *mut u8) -> c_ulong,
    fn_get_id: unsafe extern "C" fn(HANDLE, *mut u8) -> c_ulong,
}

impl Lpu237Dll {
    pub unsafe fn new<P: AsRef<OsStr>>(path: P) -> Result<Self, Box<dyn std::error::Error>> {
        let _lib = Arc::new(unsafe { Library::new(path.as_ref())? });

        let fn_on = *unsafe { _lib.get(b"LPU237_dll_on")? };
        let fn_off = *unsafe { _lib.get(b"LPU237_dll_off")? };
        let fn_get_list = *unsafe { _lib.get(b"LPU237_get_list")? };
        let fn_open = *unsafe { _lib.get(b"LPU237_open")? };
        let fn_close = *unsafe { _lib.get(b"LPU237_close")? };
        let fn_enable = *unsafe { _lib.get(b"LPU237_enable")? };
        let fn_disable = *unsafe { _lib.get(b"LPU237_disable")? };
        let fn_cancel_wait_swipe = *unsafe { _lib.get(b"LPU237_cancel_wait_swipe")? };
        let fn_wait_swipe_with_callback = *unsafe { _lib.get(b"LPU237_wait_swipe_with_callback")? };
        let fn_get_data = *unsafe { _lib.get(b"LPU237_get_data")? };
        let fn_get_id = *unsafe { _lib.get(b"LPU237_get_id")? };

        Ok(Self {
            _lib,
            fn_on,
            fn_off,
            fn_get_list,
            fn_open,
            fn_close,
            fn_enable,
            fn_disable,
            fn_cancel_wait_swipe,
            fn_wait_swipe_with_callback,
            fn_get_data,
            fn_get_id,
        })
    }

    pub fn dll_on(&self) -> c_ulong {
        unsafe { (self.fn_on)() }
    }

    pub fn dll_off(&self) -> c_ulong {
        unsafe { (self.fn_off)() }
    }

    pub fn get_list(&self) -> Result<Vec<String>, c_ulong> {
        unsafe {
            let size = (self.fn_get_list)(std::ptr::null_mut());
            if size == 0 {
                return Ok(Vec::new());
            }

            let mut buffer = vec![0 as WChar; size as usize];
            let count = (self.fn_get_list)(buffer.as_mut_ptr());
            
            if count == LPU237_DLL_RESULT_ERROR {
                return Err(LPU237_DLL_RESULT_ERROR);
            }

            let mut result = Vec::new();
            let mut current_pos = 0;
            while current_pos < buffer.len() && buffer[current_pos] != 0 {
                #[cfg(target_os = "windows")]
                {
                    use widestring::U16CString;

                    let s = U16CString::from_ptr_str(buffer.as_ptr().add(current_pos));
                    result.push(s.to_string_lossy());
                    current_pos += s.len() + 1;
                }

                #[cfg(not(target_os = "windows"))]
                {
                    let start = current_pos;
                    let mut end = start;

                    while end < buffer.len() && buffer[end] != 0 {
                        end += 1;
                    }

                    let slice = &buffer[start..end];
                    let s: String = slice.iter().map(|&c| char::from_u32(c).unwrap_or('\u{FFFD}')).collect();
                    result.push(s);

                    current_pos = end + 1;
                }                
            }
            Ok(result)
        }
    }

    pub fn open(&self, path: &str) -> Result<HANDLE, HANDLE> {
        #[cfg(target_os = "windows")]
        let wpath: Vec<WChar> = {
            use widestring::U16CString;
            U16CString::from_str(path)
                .map_err(|_| INVALID_HANDLE_VALUE)?
                .into_vec_with_nul()
        };

        #[cfg(not(target_os = "windows"))]
        let wpath: Vec<WChar> = {
            let mut v: Vec<u32> = path.chars().map(|c| c as u32).collect();
            v.push(0);
            v
        };        

        unsafe {
            let h = (self.fn_open)(wpath.as_ptr());
            if h == INVALID_HANDLE_VALUE {
                Err(INVALID_HANDLE_VALUE)
            } else {
                Ok(h)
            }
        }
    }

    pub fn close(&self, h_dev: HANDLE) -> c_ulong {
        unsafe { (self.fn_close)(h_dev) }
    }

    pub fn enable(&self, h_dev: HANDLE) -> c_ulong {
        unsafe { (self.fn_enable)(h_dev) }
    }

    pub fn disable(&self, h_dev: HANDLE) -> c_ulong {
        unsafe { (self.fn_disable)(h_dev) }
    }

    pub fn cancel_wait_swipe(&self, h_dev: HANDLE) -> c_ulong {
        unsafe { (self.fn_cancel_wait_swipe)(h_dev) }
    }

    pub fn wait_swipe_with_callback(&self, h_dev: HANDLE, cb: TypeCallback, param: *mut std::ffi::c_void) -> c_ulong {
        unsafe { (self.fn_wait_swipe_with_callback)(h_dev, cb, param) }
    }

    pub fn get_data(&self, index: c_ulong, track: c_ulong) -> Result<Vec<u8>, c_ulong> {
        unsafe {
            let len = (self.fn_get_data)(index, track, std::ptr::null_mut());
            if len == LPU237_DLL_RESULT_ERROR {
                return Err(LPU237_DLL_RESULT_ERROR);
            }
            if len == LPU237_DLL_RESULT_CANCEL {
                return Err(LPU237_DLL_RESULT_CANCEL);
            }
            
            let mut buffer = vec![0u8; len as usize];
            let actual_len = (self.fn_get_data)(index, track, buffer.as_mut_ptr());
            if actual_len != len {
                return Err(LPU237_DLL_RESULT_ERROR);
            }
            Ok(buffer)
        }
    }

    pub fn get_id(&self, h_dev: HANDLE) -> Result<Vec<u8>, c_ulong> {
        unsafe {
            let len = (self.fn_get_id)(h_dev, std::ptr::null_mut());
            if len == LPU237_DLL_RESULT_ERROR {
                return Err(LPU237_DLL_RESULT_ERROR);
            }
            
            let mut buffer = vec![0u8; len as usize];
            let actual_len = (self.fn_get_id)(h_dev, buffer.as_mut_ptr());
            if actual_len != len {
                return Err(LPU237_DLL_RESULT_ERROR);
            }
            Ok(buffer)
        }
    }
}
