/**
 * OS 별, 차이에 따른 정의
 */
#[cfg(windows)]
#[repr(transparent)]
#[derive(Copy, Clone, PartialEq, Eq, Debug)]
pub struct HANDLE(pub *mut core::ffi::c_void);

#[cfg(not(windows))]
#[repr(transparent)]
#[derive(Copy, Clone, PartialEq, Eq, Debug)]
pub struct HANDLE(pub u32);

#[cfg(windows)]
pub const INVALID_HANDLE_VALUE: HANDLE = HANDLE((-1isize) as *mut core::ffi::c_void);

#[cfg(not(windows))]
pub const INVALID_HANDLE_VALUE: HANDLE = HANDLE((-1i32) as u32);

// Derived traits for HANDLE to make it easier to use in Rust
#[cfg(windows)]
unsafe impl Send for HANDLE {}
#[cfg(windows)]
unsafe impl Sync for HANDLE {}
