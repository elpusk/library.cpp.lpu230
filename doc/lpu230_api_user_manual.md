# LPU230 API User Manual

Version 6.0

## API Basic Information

### Exported Functions of tg_lpu237_dll.dll

| Exported Name | Prototype | Description |
|---------------|-----------|-------------|
| LPU237_dll_on | DWORD WINAPI LPU237_dll_on() | Create dll internal worker thread. |
| LPU237_dll_off | DWORD WINAPI LPU237_dll_off() | Remove dll internal worker thread. |
| LPU237_get_list | DWORD WINAPI LPU237_get_list(LPTSTR ssDevPaths) | Get the connected MSR path list. |
| LPU237_open | HANDLE WINAPI LPU237_open(LPCTSTR sDevPath) | Open MSR for communication. |
| LPU237_close | DWORD WINAPI LPU237_close(HANDLE hDev) | Close MSR communication. |
| LPU237_get_id | DWORD WINAPI LPU237_get_id(HANDLE hDev, BYTE *sId) | Get the opened MSR unique ID. |
| LPU237_enable | DWORD WINAPI LPU237_enable(HANDLE hDev) | Enable magnetic card reading. |
| LPU237_disable | DWORD WINAPI LPU237_disable(HANDLE hDev) | Disable magnetic card reading. |
| LPU237_cancel_wait_swipe | DWORD WINAPI LPU237_cancel_wait_swipe(HANDLE hDev) | Cancel the waiting status for card swipe. |
| LPU237_wait_swipe_with_waits(deprecated) | DWORD WINAPI LPU237_wait_swipe_with_waits(HANDLE hDev) | Waits for card swipe (synchronous). |
| LPU237_wait_swipe_with_callback | DWORD WINAPI LPU237_wait_swipe_with_callback(HANDLE hDev, type_callback pFun, void *pParameter) | Waits for card swipe (asynchronous with callback). |
| LPU237_wait_swipe_with_message(deprecated) | DWORD WINAPI LPU237_wait_swipe_with_message(HANDLE hDev, HWND hWnd, UINT nMsg) | Waits for card swipe (asynchronous with message). |
| LPU237_get_data | DWORD WINAPI LPU237_get_data(DWORD dwBufferIndex, DWORD dwIsoTrack, BYTE *sTrackData) | Get the card data. |

### Definition of Return Values

| Symbol | Hex Value | Description |
|--------|-----------|-------------|
| LPU237_DLL_RESULT_SUCCESS | 0x00000000 | Processing success. |
| LPU237_DLL_RESULT_ERROR | 0xFFFFFFFF | Processing error. |
| LPU237_DLL_RESULT_CANCEL | 0xFFFFFFFE | Processing canceled by user. |
| LPU237_DLL_RESULT_ERROR_MSR | 0xFFFFFFFD | Card reading error. |
| - | 0x00000001 ~ 0x7FFFFFFF | Buffer size, number of devices, buffer index, or data count. |

## Basic Programming Sequences

### Plaintext Card Data

1. Call `LPU237_dll_on()` to create DLL internal worker thread.
2. Call `LPU237_get_list()` to get connected device path list.
3. Call `LPU237_open()` to get device handle.
4. Call `LPU237_enable()` to enable card reading.
5. Call `LPU237_wait_swipe_with_x()` to wait for swipe.
6. Swipe card or call `LPU237_cancel_wait_swipe()`.
7. Call `LPU237_get_data()` to get card data.
8. Call `LPU237_disable()` to disable reading.
9. Call `LPU237_dll_off()` to delete worker thread.

Repeat steps 5-7 for multiple reads.


## Function Details

### LPU237_dll_on

**Prototype:**  
```c
DWORD WINAPI LPU237_dll_on()
```

**Parameters:** None

**Return:**  
- Success: `LPU237_DLL_RESULT_SUCCESS`

Creates DLL internal worker thread.

### LPU237_dll_off

**Prototype:**  
```c
DWORD WINAPI LPU237_dll_off()
```

**Parameters:** None

**Return:**  
- Success: `LPU237_DLL_RESULT_SUCCESS`

Terminates DLL worker thread.

### LPU237_get_list

**Prototype:**  
```c
DWORD WINAPI LPU237_get_list(LPTSTR ssDevPaths)
```

**Parameters:**  
- `ssDevPaths`: [in/out] Device path list buffer (Unicode multi-string).

**Return:**  
- If NULL: Required buffer size (bytes).  
- Success: Number of device paths.  
- Error: `LPU237_DLL_RESULT_ERROR`.

### LPU237_open

**Prototype:**  
```c
HANDLE WINAPI LPU237_open(LPCTSTR sDevPath)
```

**Parameters:**  
- `sDevPath`: [in] Device path (Unicode zero-string).

**Return:**  
- Success: Device handle.  
- Error: `INVALID_HANDLE_VALUE`.

### LPU237_close

**Prototype:**  
```c
DWORD WINAPI LPU237_close(HANDLE hDev)
```

**Parameters:**  
- `hDev`: [in] Open handle.

**Return:**  
- Success: `LPU237_DLL_RESULT_SUCCESS`.  
- Error: `LPU237_DLL_RESULT_ERROR`.

### LPU237_enable

**Prototype:**  
```c
DWORD WINAPI LPU237_enable(HANDLE hDev)
```

**Parameters:**  
- `hDev`: [in] Open handle.

**Return:**  
- Success: `LPU237_DLL_RESULT_SUCCESS`.  
- Error: `LPU237_DLL_RESULT_ERROR`.

Enables card reading.

### LPU237_disable

**Prototype:**  
```c
DWORD WINAPI LPU237_disable(HANDLE hDev)
```

**Parameters:**  
- `hDev`: [in] Open handle.

**Return:**  
- Success: `LPU237_DLL_RESULT_SUCCESS`.  
- Error: `LPU237_DLL_RESULT_ERROR`.

Disables card reading.

### LPU237_cancel_wait_swipe

**Prototype:**  
```c
DWORD WINAPI LPU237_cancel_wait_swipe(HANDLE hDev)
```

**Parameters:**  
- `hDev`: [in] Open handle.

**Return:**  
- Success: `LPU237_DLL_RESULT_SUCCESS`.  
- Error: `LPU237_DLL_RESULT_ERROR`.

Cancels waiting for swipe.

### LPU237_wait_swipe_with_waits

**Prototype:**  
```c
DWORD WINAPI LPU237_wait_swipe_with_waits(HANDLE hDev)
```

**Parameters:**  
- `hDev`: [in] Open handle.

**Return:**  
- Success: Buffer index.  
- Error: `LPU237_DLL_RESULT_ERROR`.

Synchronous wait for swipe.(deprecated, Windows ONly)

### LPU237_wait_swipe_with_callback

**Prototype:**  
```c
DWORD WINAPI LPU237_wait_swipe_with_callback(HANDLE hDev, type_callback pFun, void *pParameter)
```

**Parameters:**  
- `hDev`: [in] Open handle.  
- `pFun`: [in] Callback pointer.  
- `pParameter`: [in] Callback parameter.

**Return:**  
- Success: Buffer index.  
- Error: `LPU237_DLL_RESULT_ERROR`.

Asynchronous wait with callback.

### LPU237_wait_swipe_with_message

**Prototype:**  
```c
DWORD WINAPI LPU237_wait_swipe_with_message(HANDLE hDev, HWND hWnd, UINT nMsg)
```

**Parameters:**  
- `hDev`: [in] Open handle.  
- `hWnd`: [in] Window handle.  
- `nMsg`: [in] User message.

**Return:**  
- Success: Buffer index.  
- Error: `LPU237_DLL_RESULT_ERROR`.

Asynchronous wait with message.(deprecated, Windows ONly)

### LPU237_get_data

**Prototype:**  
```c
DWORD WINAPI LPU237_get_data(DWORD dwBufferIndex, DWORD dwIsoTrack, BYTE *sTrackData)
```

**Parameters:**  
- `dwBufferIndex`: [in] Buffer index.  
- `dwIsoTrack`: [in] Track number (1,2,3).  
- `sTrackData`: [in/out] Track data buffer (can be NULL).

**Return:**  
- Success: Number of track data bytes.  
- Error: `LPU237_DLL_RESULT_ERROR` or `LPU237_DLL_RESULT_ERROR_MSR`.  
- Cancel: `LPU237_DLL_RESULT_CANCEL`.

### LPU237_get_id

**Prototype:**  
```c
DWORD WINAPI LPU237_get_id(HANDLE hDev, BYTE *sId)
```

**Parameters:**  
- `hDev`: [in] Open handle.  
- `sId`: [in/out] ID buffer (can be NULL).

**Return:**  
- Success: ID size (16 bytes).  
- Error: `LPU237_DLL_RESULT_ERROR`.

