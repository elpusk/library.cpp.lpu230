# LPU23X iButton API User Manual

Version 5.0

## Overview

This document describes the Application Programming Interface (API) for the LPU23X magnetic card reader with iButton reader functionality. The API allows a user application to retrieve iButton data easily.

For version 3.x, the Next Device Manager (NDM) must be running on the system. Starting from version 4.0, the device I/O mode can be selected via the `lpu230_ibutton_api.ini` file (dual mode).

**Recommendation:** Use "USB HID Vendor mode" for this API.

## API Basic Information

### Characteristics
- **Position:**
  - 32-bit: `[Program Files folder]\Easyset\lpu230\bin\components\x86`
  - 64-bit: `[Program Files folder]\Easyset\lpu230\bin\components\x64`
- **File:** `tg_lpu237_ibutton.dll`
- **Version:** 4.0.0.0
- **Type:** Win32 regular DLL

## Exported Functions of tg_lpu237_ibutton.dll

**Note:** BTC functions and LPU237 functions must be used mutually exclusively.

### BTC Functions (Not for 64-bit OS)

| Exported Name | Prototype | Description |
|---------------|-----------|-------------|
| XHidDev_LoadDriver | BOOL WINAPI XHidDev_LoadDriver(CallBackKeyIn cbOnKeyIn) | Prepare for receiving iButton data. |
| XHidDev_UnloadDriver | void WINAPI XHidDev_UnloadDriver(void) | Ignore iButton data. |
| UCD_SendData | - | No functionality. |

### LPU237 Functions (32-bit or 64-bit OS)

| Exported Name | Prototype | Description |
|---------------|-----------|-------------|
| LPU237Lock_dll_on | DWORD WINAPI LPU237Lock_dll_on() | Create DLL internal worker thread. |
| LPU237Lock_dll_off | DWORD WINAPI LPU237Lock_dll_off() | Remove DLL internal worker thread. |
| LPU237Lock_get_list | DWORD WINAPI LPU237Lock_get_list(LPTSTR ssDevPaths) | Get connected device path list. |
| LPU237Lock_open | HANDLE WINAPI LPU237Lock_open(LPCTSTR sDevPath) | Open device for communication. |
| LPU237Lock_close | DWORD WINAPI LPU237Lock_close(HANDLE hDev) | Close device communication. |
| LPU237Lock_get_id | DWORD WINAPI LPU237Lock_get_id(HANDLE hDev, BYTE *sId) | Get opened device unique ID. |
| LPU237Lock_enable | DWORD WINAPI LPU237Lock_enable(HANDLE hDev) | Enable iButton reading. |
| LPU237Lock_disable | DWORD WINAPI LPU237Lock_disable(HANDLE hDev) | Disable iButton reading. |
| LPU237Lock_cancel_wait_key | DWORD WINAPI LPU237Lock_cancel_wait_key(HANDLE hDev) | Cancel waiting for iButton contact. |
| LPU237Lock_wait_key_with_callback | DWORD WINAPI LPU237Lock_wait_key_with_callback(HANDLE hDev, type_callback pFun, void *pParameter) | Wait for iButton contact (asynchronous). |
| LPU237Lock_get_data | DWORD WINAPI LPU237Lock_get_data(DWORD dwBufferIndex, BYTE *sKey) | Get iButton data. |

## Definition of Return Values

| Symbol | Hex Value | Description |
|--------|-----------|-------------|
| LPU237LOCK_DLL_RESULT_SUCCESS | 0x00000000 | Processing success. |
| LPU237LOCK_DLL_RESULT_ERROR | 0xFFFFFFFF | Processing error. |
| LPU237LOCK_DLL_RESULT_CANCEL | 0xFFFFFFFE | Processing canceled by user request. |
| - | 0x00000001 ~ 0x7FFFFFFF | Buffer size, number of devices, buffer index, or data count. |

## Basic Programming Sequence

1. Call `LPU237Lock_dll_on()` to create the DLL's internal worker thread.
2. Call `LPU237Lock_get_list()` to get the connected device path list.
3. Call `LPU237Lock_open()` to get a device handle.
4. Call `LPU237Lock_enable()` to enable iButton reading (device sends iButton data to DLL upon contact).
5. Call `LPU237Lock_wait_key_with_callback()` to wait for iButton contact (DLL notifies application via callback).
6. Contact an iButton or call `LPU237Lock_cancel_wait_key()`.
7. Call `LPU237Lock_get_data()` to retrieve iButton data.
8. Call `LPU237Lock_disable()` to disable iButton reading (device stops sending data).
9. Call `LPU237Lock_dll_off()` to terminate the DLL's worker thread.

**Note:** Steps 5–7 can be repeated to retrieve iButton data multiple times.

## Function Details

### XHidDev_LoadDriver

**Prototype:**
```c
BOOL WINAPI XHidDev_LoadDriver(CallBackKeyIn cbOnKeyIn)
```

**Parameters:**
- `cbOnKeyIn`: [in] Callback function pointer, called on iButton contact or disconnect.

**Return:**
- Success: `1` (ready for receiving iButton).
- Failure: `0`.

### XHidDev_UnloadDriver

**Prototype:**
```c
void WINAPI XHidDev_UnloadDriver(void)
```

**Parameters:** None

**Return:** None

Ignores iButton data.

### UCD_SendData

No functionality.

### LPU237Lock_dll_on

Creates and runs the DLL's internal worker thread for device communication. Do not call in `DllMain()`.

**Prototype:**
```c
DWORD WINAPI LPU237Lock_dll_on()
```

**Parameters:** None

**Return:**
- Success: `LPU237LOCK_DLL_RESULT_SUCCESS`

### LPU237Lock_dll_off

Terminates the DLL's worker thread. Do not call in `DllMain()`.

**Prototype:**
```c
DWORD WINAPI LPU237Lock_dll_off()
```

**Parameters:** None

**Return:**
- Success: `LPU237LOCK_DLL_RESULT_SUCCESS`

### LPU237Lock_get_list

Gets the connected device path list.

**Prototype:**
```c
DWORD WINAPI LPU237Lock_get_list(LPTSTR ssDevPaths)
```

**Parameters:**
- `ssDevPaths`: [in/out] Device path list buffer (Unicode, multi-string).

**Return:**
- If `ssDevPaths` is NULL: Required buffer size (bytes).
- Success: Number of device paths.
- Error: `LPU237LOCK_DLL_RESULT_ERROR`.

**Example:** For two devices with paths "ab" and "12":
| Offset | Value | Description |
|--------|-------|-------------|
| 0      | 0x61  | Unicode 'a' |
| 1      | 0x00  |             |
| 2      | 0x62  | Unicode 'b' |
| 3      | 0x00  |             |
| 4      | 0x00  | Unicode NULL (end of "ab") |
| 5      | 0x00  |             |
| 6      | 0x31  | Unicode '1' |
| 7      | 0x00  |             |
| 8      | 0x32  | Unicode '2' |
| 9      | 0x00  |             |
| 10     | 0x00  | Unicode NULL (end of "12") |
| 11     | 0x00  |             |
| 12     | 0x00  | Unicode NULL (end of multi-string) |
| 13     | 0x00  |             |

### LPU237Lock_open

Opens the device for communication.

**Prototype:**
```c
HANDLE WINAPI LPU237Lock_open(LPCTSTR sDevPath)
```

**Parameters:**
- `sDevPath`: [in] Device path (Unicode, zero-string).

**Return:**
- Success: Device handle.
- Error: `INVALID_HANDLE_VALUE`.

### LPU237Lock_close

Closes the device communication.

**Prototype:**
```c
DWORD WINAPI LPU237Lock_close(HANDLE hDev)
```

**Parameters:**
- `hDev`: [in] Valid handle from `LPU237Lock_open()`.

**Return:**
- Success: `LPU237LOCK_DLL_RESULT_SUCCESS`.
- Error: `LPU237LOCK_DLL_RESULT_ERROR`.

### LPU237Lock_enable

Enables the device to read iButton data.

**Prototype:**
```c
DWORD WINAPI LPU237Lock_enable(HANDLE hDev)
```

**Parameters:**
- `hDev`: [in] Valid handle from `LPU237Lock_open()`.

**Return:**
- Success: `LPU237LOCK_DLL_RESULT_SUCCESS`.
- Error: `LPU237LOCK_DLL_RESULT_ERROR`.

### LPU237Lock_disable

Disables iButton reading (device ignores iButton contact).

**Prototype:**
```c
DWORD WINAPI LPU237Lock_disable(HANDLE hDev)
```

**Parameters:**
- `hDev`: [in] Valid handle from `LPU237Lock_open()`.

**Return:**
- Success: `LPU237LOCK_DLL_RESULT_SUCCESS`.
- Error: `LPU237LOCK_DLL_RESULT_ERROR`.

### LPU237Lock_cancel_wait_key

Stops the `LPU237Lock_wait_key_with_callback()` operation.

**Prototype:**
```c
DWORD WINAPI LPU237Lock_cancel_wait_key(HANDLE hDev)
```

**Parameters:**
- `hDev`: [in] Valid handle from `LPU237Lock_open()`.

**Return:**
- Success: `LPU237LOCK_DLL_RESULT_SUCCESS`.
- Error: `LPU237LOCK_DLL_RESULT_ERROR`.

### LPU237Lock_wait_key_with_callback

Waits for iButton contact (asynchronous). Returns immediately and executes the callback function when an iButton is contacted.

**Prototype:**
```c
DWORD WINAPI LPU237Lock_wait_key_with_callback(HANDLE hDev, type_callback pFun, void *pParameter)
```

**Parameters:**
- `hDev`: [in] Valid handle from `LPU237Lock_open()`.
- `pFun`: [in] Callback function pointer.
- `pParameter`: [in] Callback function parameter.

**Return:**
- Success: Buffer index number (used in `LPU237Lock_get_data()`).
- Error: `LPU237LOCK_DLL_RESULT_ERROR`.

### LPU237Lock_get_data

Retrieves iButton data.

**Prototype:**
```c
DWORD WINAPI LPU237Lock_get_data(DWORD dwBufferIndex, BYTE *sKey)
```

**Parameters:**
- `dwBufferIndex`: [in] Buffer index from `LPU237Lock_wait_key_with_callback()`.
- `sKey`: [in/out] Buffer for iButton data (can be NULL).

**Return:**
- Success: Number of iButton data bytes.
- Error: `LPU237LOCK_DLL_RESULT_ERROR`.
- Cancel: `LPU237LOCK_DLL_RESULT_CANCEL`.

**Example iButton Data:** "9A0000127DD8F701"
| Offset | Value | Description |
|--------|-------|-------------|
| 0      | 0x9A  | Raw data    |
| 1      | 0x00  | Raw data    |
| 2      | 0x00  | Raw data    |
| 3      | 0x12  | Raw data    |
| 4      | 0x7D  | Raw data    |
| 5      | 0xD8  | Raw data    |
| 6      | 0xF7  | Raw data    |
| 7      | 0x01  | Raw data    |

### LPU237Lock_get_id

Gets the device's unique ID (always 16 bytes).

**Prototype:**
```c
DWORD WINAPI LPU237Lock_get_id(HANDLE hDev, BYTE *sId)
```

**Parameters:**
- `hDev`: [in] Valid handle from `LPU237Lock_open()`.
- `sId`: [in/out] Buffer for device ID (can be NULL).

**Return:**
- Success: ID size (16 bytes).
- Error: `LPU237LOCK_DLL_RESULT_ERROR`.

## Dual Mode

The device I/O mode (auto, NDM, or direct) can be selected via `lpu230_fw.ini`.

### NDM Mode
Requires Next Device Manager (version 3.x).

### Direct Mode
Supported from version 4.0.

## History

- 2014.11.25: V1.0.0.1 – First release.
- 2017.09.19: V2.1 – Supported standalone KeyLock service object.
- 2017.11.28: V3.0 – Dropped standalone KeyLock, supported NDM KeyLock service.
- 2023.09.21: V4.0 – Added LPU238 support and dual mode.
- 2023.10.24: V4.1 – Rebuild due to dependency library changes (no functional changes).
- 2024.02.28: V4.2 – Rebuild due to dependency library changes (no functional changes).
- 2024.05.10: V5.0 – Rebuild due to dependency library changes (no functional changes).