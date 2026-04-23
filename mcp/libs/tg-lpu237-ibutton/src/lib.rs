use libloading::{Library, Symbol};
use lpu237_common::{HANDLE, INVALID_HANDLE_VALUE};
use std::sync::Arc;
use widestring::U16CString;
use std::ffi::OsStr;
use libc::c_ulong;

pub const LPU237LOCK_DLL_RESULT_SUCCESS: c_ulong = 0;
pub const LPU237LOCK_DLL_RESULT_ERROR: c_ulong = !0;
pub const LPU237LOCK_DLL_RESULT_CANCEL: c_ulong = !0 - 1;

pub type TypeKeyCallback = extern "C" fn(*mut std::ffi::c_void);

pub struct Lpu237IButton {
    lib: Arc<Library>,
    // Function pointers using libc::c_ulong for unsigned long
    fn_on: unsafe extern "C" fn() -> c_ulong,
    fn_off: unsafe extern "C" fn() -> c_ulong,
    fn_get_list: unsafe extern "C" fn(*mut u16) -> c_ulong,
    fn_open: unsafe extern "C" fn(*const u16) -> HANDLE,
    fn_close: unsafe extern "C" fn(HANDLE) -> c_ulong,
    fn_enable: unsafe extern "C" fn(HANDLE) -> c_ulong,
    fn_disable: unsafe extern "C" fn(HANDLE) -> c_ulong,
    fn_cancel_wait_key: unsafe extern "C" fn(HANDLE) -> c_ulong,
    fn_wait_key_with_callback: unsafe extern "C" fn(HANDLE, TypeKeyCallback, *mut std::ffi::c_void) -> c_ulong,
    fn_get_data: unsafe extern "C" fn(c_ulong, *mut u8) -> c_ulong,
    fn_get_id: unsafe extern "C" fn(HANDLE, *mut u8) -> c_ulong,
}

impl Lpu237IButton {
    pub unsafe fn new<P: AsRef<OsStr>>(path: P) -> Result<Self, Box<dyn std::error::Error>> {
        let lib = Arc::new(Library::new(path)?);

        let fn_on = *lib.get(b"LPU237Lock_dll_on")?;
        let fn_off = *lib.get(b"LPU237Lock_dll_off")?;
        let fn_get_list = *lib.get(b"LPU237Lock_get_list")?;
        let fn_open = *lib.get(b"LPU237Lock_open")?;
        let fn_close = *lib.get(b"LPU237Lock_close")?;
        let fn_enable = *lib.get(b"LPU237Lock_enable")?;
        let fn_disable = *lib.get(b"LPU237Lock_disable")?;
        let fn_cancel_wait_key = *lib.get(b"LPU237Lock_cancel_wait_key")?;
        let fn_wait_key_with_callback = *lib.get(b"LPU237Lock_wait_key_with_callback")?;
        let fn_get_data = *lib.get(b"LPU237Lock_get_data")?;
        let fn_get_id = *lib.get(b"LPU237Lock_get_id")?;

        Ok(Self {
            lib,
            fn_on,
            fn_off,
            fn_get_list,
            fn_open,
            fn_close,
            fn_enable,
            fn_disable,
            fn_cancel_wait_key,
            fn_wait_key_with_callback,
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

            let mut buffer = vec![0u16; size as usize];
            let count = (self.fn_get_list)(buffer.as_mut_ptr());
            
            if count == LPU237LOCK_DLL_RESULT_ERROR {
                return Err(LPU237LOCK_DLL_RESULT_ERROR);
            }

            let mut result = Vec::new();
            let mut current_pos = 0;
            while current_pos < buffer.len() && buffer[current_pos] != 0 {
                let s = U16CString::from_ptr_with_nul(buffer.as_ptr().add(current_pos), buffer.len() - current_pos)
                    .map_err(|_| LPU237LOCK_DLL_RESULT_ERROR)?;
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

    pub fn close(&self, h_dev: HANDLE) -> c_ulong {
        unsafe { (self.fn_close)(h_dev) }
    }

    pub fn enable(&self, h_dev: HANDLE) -> c_ulong {
        unsafe { (self.fn_enable)(h_dev) }
    }

    pub fn disable(&self, h_dev: HANDLE) -> c_ulong {
        unsafe { (self.fn_disable)(h_dev) }
    }

    pub fn cancel_wait_key(&self, h_dev: HANDLE) -> c_ulong {
        unsafe { (self.fn_cancel_wait_key)(h_dev) }
    }

    pub fn wait_key_with_callback(&self, h_dev: HANDLE, cb: TypeKeyCallback, param: *mut std::ffi::c_void) -> c_ulong {
        unsafe { (self.fn_wait_key_with_callback)(h_dev, cb, param) }
    }

    pub fn get_data(&self, index: c_ulong) -> Result<Vec<u8>, c_ulong> {
        unsafe {
            let len = (self.fn_get_data)(index, std::ptr::null_mut());
            if len == LPU237LOCK_DLL_RESULT_ERROR {
                return Err(LPU237LOCK_DLL_RESULT_ERROR);
            }
            if len == LPU237LOCK_DLL_RESULT_CANCEL {
                return Err(LPU237LOCK_DLL_RESULT_CANCEL);
            }
            
            let mut buffer = vec![0u8; len as usize];
            let actual_len = (self.fn_get_data)(index, buffer.as_mut_ptr());
            if actual_len != len {
                return Err(LPU237LOCK_DLL_RESULT_ERROR);
            }
            Ok(buffer)
        }
    }

    pub fn get_id(&self, h_dev: HANDLE) -> Result<Vec<u8>, c_ulong> {
        unsafe {
            let len = (self.fn_get_id)(h_dev, std::ptr::null_mut());
            if len == LPU237LOCK_DLL_RESULT_ERROR {
                return Err(LPU237LOCK_DLL_RESULT_ERROR);
            }
            
            let mut buffer = vec![0u8; len as usize];
            let actual_len = (self.fn_get_id)(h_dev, buffer.as_mut_ptr());
            if actual_len != len {
                return Err(LPU237LOCK_DLL_RESULT_ERROR);
            }
            Ok(buffer)
        }
    }
}
