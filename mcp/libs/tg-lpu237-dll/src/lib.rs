use libloading::{Library, Symbol};
use lpu237_common::{HANDLE, INVALID_HANDLE_VALUE};
use std::sync::Arc;
use widestring::U16CString;
use std::ffi::OsStr;

pub const LPU237_DLL_RESULT_SUCCESS: u32 = 0;
pub const LPU237_DLL_RESULT_ERROR: u32 = 0xFFFFFFFF;
pub const LPU237_DLL_RESULT_CANCEL: u32 = 0xFFFFFFFE;
pub const LPU237_DLL_RESULT_ERROR_MSR: u32 = 0xFFFFFFFD;

pub type TypeCallback = extern "C" fn(*mut std::ffi::c_void);

pub struct Lpu237Dll {
    lib: Arc<Library>,
    // Function pointers
    fn_on: unsafe extern "C" fn() -> u32,
    fn_off: unsafe extern "C" fn() -> u32,
    fn_get_list: unsafe extern "C" fn(*mut u16) -> u32,
    fn_open: unsafe extern "C" fn(*const u16) -> HANDLE,
    fn_close: unsafe extern "C" fn(HANDLE) -> u32,
    fn_enable: unsafe extern "C" fn(HANDLE) -> u32,
    fn_disable: unsafe extern "C" fn(HANDLE) -> u32,
    fn_cancel_wait_swipe: unsafe extern "C" fn(HANDLE) -> u32,
    fn_wait_swipe_with_callback: unsafe extern "C" fn(HANDLE, TypeCallback, *mut std::ffi::c_void) -> u32,
    fn_get_data: unsafe extern "C" fn(u32, u32, *mut u8) -> u32,
    fn_get_id: unsafe extern "C" fn(HANDLE, *mut u8) -> u32,
}

impl Lpu237Dll {
    pub unsafe fn new<P: AsRef<OsStr>>(path: P) -> Result<Self, Box<dyn std::error::Error>> {
        let lib = Arc::new(Library::new(path)?);

        let fn_on = *lib.get(b"LPU237_dll_on")?;
        let fn_off = *lib.get(b"LPU237_dll_off")?;
        let fn_get_list = *lib.get(b"LPU237_get_list")?;
        let fn_open = *lib.get(b"LPU237_open")?;
        let fn_close = *lib.get(b"LPU237_close")?;
        let fn_enable = *lib.get(b"LPU237_enable")?;
        let fn_disable = *lib.get(b"LPU237_disable")?;
        let fn_cancel_wait_swipe = *lib.get(b"LPU237_cancel_wait_swipe")?;
        let fn_wait_swipe_with_callback = *lib.get(b"LPU237_wait_swipe_with_callback")?;
        let fn_get_data = *lib.get(b"LPU237_get_data")?;
        let fn_get_id = *lib.get(b"LPU237_get_id")?;

        Ok(Self {
            lib,
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

    pub fn dll_on(&self) -> u32 {
        unsafe { (self.fn_on)() }
    }

    pub fn dll_off(&self) -> u32 {
        unsafe { (self.fn_off)() }
    }

    pub fn get_list(&self) -> Result<Vec<String>, u32> {
        unsafe {
            let size = (self.fn_get_list)(std::ptr::null_mut());
            if size == 0 {
                return Ok(Vec::new());
            }

            let mut buffer = vec![0u16; size as usize];
            let count = (self.fn_get_list)(buffer.as_mut_ptr());
            
            if count == LPU237_DLL_RESULT_ERROR {
                return Err(LPU237_DLL_RESULT_ERROR);
            }

            // The buffer contains multiple null-terminated strings, ending with an extra null.
            let mut result = Vec::new();
            let mut current_pos = 0;
            while current_pos < buffer.len() && buffer[current_pos] != 0 {
                let s = U16CString::from_ptr_with_nul(buffer.as_ptr().add(current_pos), buffer.len() - current_pos)
                    .map_err(|_| LPU237_DLL_RESULT_ERROR)?;
                result.push(s.to_string_lossy());
                current_pos += s.len() + 1;
            }
            Ok(result)
        }
    }

    pub fn open(&self, path: &str) -> Result<HANDLE, HANDLE> {
        let wpath = U16CString::from_str(path).map_err(|_| INVALID_HANDLE_VALUE)?;
        unsafe {
            let h = (self.fn_open)(wpath.as_ptr());
            if h == INVALID_HANDLE_VALUE {
                Err(INVALID_HANDLE_VALUE)
            } else {
                Ok(h)
            }
        }
    }

    pub fn close(&self, h_dev: HANDLE) -> u32 {
        unsafe { (self.fn_close)(h_dev) }
    }

    pub fn enable(&self, h_dev: HANDLE) -> u32 {
        unsafe { (self.fn_enable)(h_dev) }
    }

    pub fn disable(&self, h_dev: HANDLE) -> u32 {
        unsafe { (self.fn_disable)(h_dev) }
    }

    pub fn cancel_wait_swipe(&self, h_dev: HANDLE) -> u32 {
        unsafe { (self.fn_cancel_wait_swipe)(h_dev) }
    }

    pub fn wait_swipe_with_callback(&self, h_dev: HANDLE, cb: TypeCallback, param: *mut std::ffi::c_void) -> u32 {
        unsafe { (self.fn_wait_swipe_with_callback)(h_dev, cb, param) }
    }

    pub fn get_data(&self, index: u32, track: u32) -> Result<Vec<u8>, u32> {
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

    pub fn get_id(&self, h_dev: HANDLE) -> Result<Vec<u8>, u32> {
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

// Ensure the library is not dropped while the struct is alive.
// Library is wrapped in Arc so it can be cloned if needed.
