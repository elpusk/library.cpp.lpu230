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
| LPU237_wait_swipe_with_waits | DWORD WINAPI LPU237_wait_swipe_with_waits(HANDLE hDev) | Waits for card swipe (synchronous). |
| LPU237_wait_swipe_with_callback | DWORD WINAPI LPU237_wait_swipe_with_callback(HANDLE hDev, type_callback pFun, void *pParameter) | Waits for card swipe (asynchronous with callback). |
| LPU237_wait_swipe_with_message | DWORD WINAPI LPU237_wait_swipe_with_message(HANDLE hDev, HWND hWnd, UINT nMsg) | Waits for card swipe (asynchronous with message). |
| LPU237_get_data | DWORD WINAPI LPU237_get_data(DWORD dwBufferIndex, DWORD dwIsoTrack, BYTE *sTrackData) | Get the card data. |
| LPU237_apply | DWORD WINAPI LPU237_apply(HANDLE hDev) | Save changed parameters to flash memory. |
| LPU237_enter_config | DWORD WINAPI LPU237_enter_config(HANDLE hDev) | Enter CONFIG system status. |
| LPU237_leave_config | DWORD WINAPI LPU237_leave_config(HANDLE hDev) | Leave CONFIG system status. |

### Security Functions (from v5.0, Himalaya system v2.0 or later)

| Exported Name | Prototype | Description |
|---------------|-----------|-------------|
| LPU237_SECURE_get_data | DWORD WINAPI LPU237_SECURE_get_data(DWORD dwBufferIndex, BYTE* sTrackData, DWORD dwTrackData) | Get secure card data. |
| LPU237_SECURE_get_ksn | DWORD WINAPI LPU237_SECURE_get_ksn(HANDLE hDev, BYTE* pKSN) | Get current KSN. |
| LPU237_SECURE_get_flash_counter | DWORD WINAPI LPU237_SECURE_get_flash_counter(HANDLE hDev, DWORD* pdwCounter) | Get flash written counter. |
| LPU237_SECURE_get_prePAN_position | DWORD WINAPI LPU237_SECURE_get_prePAN_position(HANDLE hDev) | Get pre-PAN exposed digits. |
| LPU237_SECURE_get_postPAN_position | DWORD WINAPI LPU237_SECURE_get_postPAN_position(HANDLE hDev) | Get post-PAN exposed digits. |
| LPU237_SECURE_get_challenge | DWORD WINAPI LPU237_SECURE_get_challenge(HANDLE hDev, BYTE* pEncryptRandom) | Get encrypted random number. |
| LPU237_SECURE_external_authenticate | DWORD WINAPI LPU237_SECURE_external_authenticate(HANDLE hDev, const BYTE* pDecryptRandom) | Verify administrator rights. |
| LPU237_SECURE_set_security_key | DWORD WINAPI LPU237_SECURE_set_security_key(HANDLE hDev, const BYTE* pEncryptNewSecurityKey, DWORD dwKeyPart) | Change current BDK. |
| LPU237_SECURE_set_ipek | DWORD WINAPI LPU237_SECURE_set_ipek(HANDLE hDev, const BYTE* pEncryptNewIpek, DWORD dwKeyPart) | Change current IPEK. |
| LPU237_SECURE_set_ksn | DWORD WINAPI LPU237_SECURE_set_ksn(HANDLE hDev, const BYTE* pKsn) | Change current KSN. |
| LPU237_SECURE_enable_card_holder_name | DWORD WINAPI LPU237_SECURE_enable_card_holder_name(HANDLE hDev, BOOL bDisplay) | Include card holder name in encrypted data. |
| LPU237_SECURE_enable_card_expiration_date | DWORD WINAPI LPU237_SECURE_enable_card_expiration_date(HANDLE hDev, BOOL bDisplay) | Include card expiration date in encrypted data. |
| LPU237_SECURE_set_prePAN_position | DWORD WINAPI LPU237_SECURE_set_prePAN_position(HANDLE hDev, DWORD dwPosition) | Set pre-PAN exposed digits. |
| LPU237_SECURE_set_postPAN_position | DWORD WINAPI LPU237_SECURE_set_postPAN_position(HANDLE hDev, DWORD dwPosition) | Set post-PAN exposed digits. |

### Encryption Data Analysis Functions (from v5.0)

| Exported Name | Prototype | Description |
|---------------|-----------|-------------|
| LPU237_ANALYSIS_this_is_cipher_text | DWORD WINAPI LPU237_ANALYSIS_this_is_cipher_text(const BYTE* pRawMsData, DWORD dwRawMsData) | Check if data is encrypted card data. |
| LPU237_ANALYSIS_get_cipher_flash_counter | DWORD WINAPI LPU237_ANALYSIS_get_cipher_flash_counter(const BYTE* pRawMsData, DWORD dwRawMsData, LPTSTR pFlash, DWORD dwFlash) | Get flash counter from encrypted data. |
| LPU237_ANALYSIS_get_cipher_masked_pan | DWORD WINAPI LPU237_ANALYSIS_get_cipher_masked_pan(const BYTE* pRawMsData, DWORD dwRawMsData, LPTSTR pMaskedPan, DWORD dwMaskedPan) | Get masked PAN from encrypted data. |
| LPU237_ANALYSIS_get_cipher_card_holder_name | DWORD WINAPI LPU237_ANALYSIS_get_cipher_card_holder_name(const BYTE* pRawMsData, DWORD dwRawMsData, LPTSTR pName, DWORD dwName) | Get card holder name from encrypted data. |
| LPU237_ANALYSIS_get_cipher_expiration_date | DWORD WINAPI LPU237_ANALYSIS_get_cipher_expiration_date(const BYTE* pRawMsData, DWORD dwRawMsData, LPTSTR pDate, DWORD dwDate) | Get expiration date from encrypted data. |
| LPU237_ANALYSIS_get_cipher_mac4 | DWORD WINAPI LPU237_ANALYSIS_get_cipher_mac4(const BYTE* pRawMsData, DWORD dwRawMsData, LPTSTR pMac4, DWORD dwMac4) | Get 4-byte MAC from encrypted data. |
| LPU237_ANALYSIS_get_cipher_ksn | DWORD WINAPI LPU237_ANALYSIS_get_cipher_ksn(const BYTE* pRawMsData, DWORD dwRawMsData, LPTSTR pKsn, DWORD dwKsn) | Get KSN from encrypted data. |
| LPU237_ANALYSIS_get_cipher_card_data | DWORD WINAPI LPU237_ANALYSIS_get_cipher_card_data(const BYTE* pRawMsData, DWORD dwRawMsData, DWORD dwIsoTrack, LPTSTR pCipher, DWORD dwCipher) | Get encrypted card data. |
| LPU237_ANALYSIS_get_cipher_raw_data_except_mac | DWORD WINAPI LPU237_ANALYSIS_get_cipher_raw_data_except_mac(const BYTE* pRawMsData, DWORD dwRawMsData, BYTE* pRaw, DWORD dwRaw) | Get raw data excluding MAC. |

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

### Encryption Activation

1. Call `LPU237_dll_on()`.
2. Call `LPU237_get_list()`.
3. Call `LPU237_open()`.
4. Call `LPU237_enter_config()`.
5. Call `LPU237_SECURE_get_challenge()`.
6. Call `LPU237_SECURE_external_authenticate()`.
7-8. Call `LPU237_SECURE_set_security_key()` for key parts.
9-10. Call `LPU237_SECURE_set_ipek()` for IPEK parts.
11. Call `LPU237_SECURE_set_ksn()`.
12-15. Optional: Set name, date, pre/post PAN positions.
16. Call `LPU237_apply()`.
17. Call `LPU237_leave_config()`.
18. Call `LPU237_dll_off()`.

Success changes security status to SECURE.

### Encrypted Card Data (SECURE Status)

1. Call `LPU237_dll_on()`.
2. Call `LPU237_get_list()`.
3. Call `LPU237_open()`.
4. Call `LPU237_enable()`.
5. Call `LPU237_wait_swipe_with_x()`.
6. Swipe card.
7. Call `LPU237_get_data()` (returns error for tracks).
8. Call `LPU237_SECURE_get_data()` to get encrypted data (RAW_EN).
9-14. Use analysis functions to extract name, date, KSN, masked PAN, MAC4, etc.
15. Verify MAC.
16. Derive decryption key from KSN and IPEK.
17. Call `LPU237_ANALYSIS_get_cipher_card_data()` to get EN.
18. Decrypt EN.
19. Call `LPU237_disable()`.
20. Call `LPU237_dll_off()`.

Repeat steps 5-18 for multiple reads.

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

Synchronous wait for swipe.

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

Asynchronous wait with message.

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

### LPU237_apply

**Prototype:**  
```c
DWORD WINAPI LPU237_apply(HANDLE hDev)
```

**Parameters:**  
- `hDev`: [in] Open handle.

**Return:**  
- Success: `LPU237_DLL_RESULT_SUCCESS`.  
- Error: `LPU237_DLL_RESULT_ERROR`.

Saves parameters (CONFIG status).

### LPU237_enter_config

**Prototype:**  
```c
DWORD WINAPI LPU237_enter_config(HANDLE hDev)
```

**Parameters:**  
- `hDev`: [in] Open handle.

**Return:**  
- Success: `LPU237_DLL_RESULT_SUCCESS`.  
- Error: `LPU237_DLL_RESULT_ERROR`.

Enters CONFIG status.

### LPU237_leave_config

**Prototype:**  
```c
DWORD WINAPI LPU237_leave_config(HANDLE hDev)
```

**Parameters:**  
- `hDev`: [in] Open handle.

**Return:**  
- Success: `LPU237_DLL_RESULT_SUCCESS`.  
- Error: `LPU237_DLL_RESULT_ERROR`.

Leaves CONFIG status.

### LPU237_SECURE_get_data

**Prototype:**  
```c
DWORD WINAPI LPU237_SECURE_get_data(DWORD dwBufferIndex, BYTE* sTrackData, DWORD dwTrackData)
```

**Parameters:**  
- `dwBufferIndex`: [in] Buffer index.  
- `sTrackData`: [in/out] Encrypted data buffer (can be NULL).  
- `dwTrackData`: [in] Buffer size.

**Return:**  
- Success: Number of returned bytes.  
- Error: `LPU237_DLL_RESULT_ERROR`.  
- Cancel: `LPU237_DLL_RESULT_CANCEL`.

Response format detailed in document.

### LPU237_SECURE_get_ksn

**Prototype:**  
```c
DWORD WINAPI LPU237_SECURE_get_ksn(HANDLE hDev, BYTE* pKSN)
```

**Parameters:**  
- `hDev`: [in] Open handle.  
- `pKSN`: [in/out] KSN buffer (10 bytes).

**Return:**  
- Success: `LPU237_DLL_RESULT_SUCCESS`.  
- Error: `LPU237_DLL_RESULT_ERROR`.

(CONFIG status, not BLOCK).

### LPU237_SECURE_get_flash_counter

**Prototype:**  
```c
DWORD WINAPI LPU237_SECURE_get_flash_counter(HANDLE hDev, DWORD* pdwCounter)
```

**Parameters:**  
- `hDev`: [in] Open handle.  
- `pdwCounter`: [in/out] Counter buffer (4 bytes).

**Return:**  
- Success: `LPU237_DLL_RESULT_SUCCESS`.  
- Error: `LPU237_DLL_RESULT_ERROR`.

(CONFIG status, not BLOCK).

### LPU237_SECURE_get_prePAN_position

**Prototype:**  
```c
DWORD WINAPI LPU237_SECURE_get_prePAN_position(HANDLE hDev)
```

**Parameters:**  
- `hDev`: [in] Open handle.

**Return:**  
- Success: 0-6.  
- Error: `LPU237_DLL_RESULT_ERROR`.

(CONFIG, ADMIN status).

### LPU237_SECURE_get_postPAN_position

**Prototype:**  
```c
DWORD WINAPI LPU237_SECURE_get_postPAN_position(HANDLE hDev)
```

**Parameters:**  
- `hDev`: [in] Open handle.

**Return:**  
- Success: 0-6.  
- Error: `LPU237_DLL_RESULT_ERROR`.

(CONFIG, ADMIN status).

### LPU237_SECURE_get_challenge

**Prototype:**  
```c
DWORD WINAPI LPU237_SECURE_get_challenge(HANDLE hDev, BYTE* pEncryptRandom)
```

**Parameters:**  
- `hDev`: [in] Open handle.  
- `pEncryptRandom`: [in/out] Encrypted random buffer.

**Return:**  
- Success: `LPU237_DLL_RESULT_SUCCESS`.  
- Error: `LPU237_DLL_RESULT_ERROR`.

(CONFIG status, not BLOCK).

### LPU237_SECURE_external_authenticate

**Prototype:**  
```c
DWORD WINAPI LPU237_SECURE_external_authenticate(HANDLE hDev, const BYTE* pDecryptRandom)
```

**Parameters:**  
- `hDev`: [in] Open handle.  
- `pDecryptRandom`: [in] Decrypted random.

**Return:**  
- Success: `LPU237_DLL_RESULT_SUCCESS`.  
- Error: `LPU237_DLL_RESULT_ERROR`.

Enters ADMIN status (CONFIG, not BLOCK).

### LPU237_SECURE_set_security_key

**Prototype:**  
```c
DWORD WINAPI LPU237_SECURE_set_security_key(HANDLE hDev, const BYTE* pEncryptNewSecurityKey, DWORD dwKeyPart)
```

**Parameters:**  
- `hDev`: [in] Open handle.  
- `pEncryptNewSecurityKey`: [in] New key.  
- `dwKeyPart`: [in] Key part.

**Return:**  
- Success: `LPU237_DLL_RESULT_SUCCESS`.  
- Error: `LPU237_DLL_RESULT_ERROR`.

(CONFIG, ADMIN status).

### LPU237_SECURE_set_ipek

**Prototype:**  
```c
DWORD WINAPI LPU237_SECURE_set_ipek(HANDLE hDev, const BYTE* pEncryptNewIpek, DWORD dwKeyPart)
```

**Parameters:**  
- `hDev`: [in] Open handle.  
- `pEncryptNewIpek`: [in] New IPEK.  
- `dwKeyPart`: [in] Key part.

**Return:**  
- Success: `LPU237_DLL_RESULT_SUCCESS`.  
- Error: `LPU237_DLL_RESULT_ERROR`.

(CONFIG, ADMIN status).

### LPU237_SECURE_set_ksn

**Prototype:**  
```c
DWORD WINAPI LPU237_SECURE_set_ksn(HANDLE hDev, const BYTE* pKsn)
```

**Parameters:**  
- `hDev`: [in] Open handle.  
- `pKsn`: [in] New KSN.

**Return:**  
- Success: `LPU237_DLL_RESULT_SUCCESS`.  
- Error: `LPU237_DLL_RESULT_ERROR`.

(CONFIG, ADMIN status).

### LPU237_SECURE_enable_card_holder_name

**Prototype:**  
```c
DWORD WINAPI LPU237_SECURE_enable_card_holder_name(HANDLE hDev, BOOL bDisplay)
```

**Parameters:**  
- `hDev`: [in] Open handle.  
- `bDisplay`: [in] Enable flag.

**Return:**  
- Success: `LPU237_DLL_RESULT_SUCCESS`.  
- Error: `LPU237_DLL_RESULT_ERROR`.

(CONFIG, ADMIN status).

### LPU237_SECURE_enable_card_expiration_date

**Prototype:**  
```c
DWORD WINAPI LPU237_SECURE_enable_card_expiration_date(HANDLE hDev, BOOL bDisplay)
```

**Parameters:**  
- `hDev`: [in] Open handle.  
- `bDisplay`: [in] Enable flag.

**Return:**  
- Success: `LPU237_DLL_RESULT_SUCCESS`.  
- Error: `LPU237_DLL_RESULT_ERROR`.

(CONFIG, ADMIN status).

### LPU237_SECURE_set_prePAN_position

**Prototype:**  
```c
DWORD WINAPI LPU237_SECURE_set_prePAN_position(HANDLE hDev, DWORD dwPosition)
```

**Parameters:**  
- `hDev`: [in] Open handle.  
- `dwPosition`: [in] Position (0-6).

**Return:**  
- Success: `LPU237_DLL_RESULT_SUCCESS`.  
- Error: `LPU237_DLL_RESULT_ERROR`.

(CONFIG, ADMIN status).

### LPU237_SECURE_set_postPAN_position

**Prototype:**  
```c
DWORD WINAPI LPU237_SECURE_set_postPAN_position(HANDLE hDev, DWORD dwPosition)
```

**Parameters:**  
- `hDev`: [in] Open handle.  
- `dwPosition`: [in] Position (0-6).

**Return:**  
- Success: `LPU237_DLL_RESULT_SUCCESS`.  
- Error: `LPU237_DLL_RESULT_ERROR`.

(CONFIG, ADMIN status).

### LPU237_ANALYSIS_this_is_cipher_text

**Prototype:**  
```c
DWORD WINAPI LPU237_ANALYSIS_this_is_cipher_text(const BYTE* pRawMsData, DWORD dwRawMsData)
```

**Parameters:**  
- `pRawMsData`: [in] Raw data.  
- `dwRawMsData`: [in] Data size.

**Return:**  
- Success: `LPU237_DLL_RESULT_SUCCESS` (if cipher text).  
- Error: `LPU237_DLL_RESULT_ERROR`.

### LPU237_ANALYSIS_get_cipher_flash_counter

**Prototype:**  
```c
DWORD WINAPI LPU237_ANALYSIS_get_cipher_flash_counter(const BYTE* pRawMsData, DWORD dwRawMsData, LPTSTR pFlash, DWORD dwFlash)
```

**Parameters:**  
- `pRawMsData`: [in] Raw data.  
- `dwRawMsData`: [in] Data size.  
- `pFlash`: [out] Flash counter buffer.  
- `dwFlash`: [in] Buffer size.

**Return:**  
- Success: Length of counter.  
- Error: `LPU237_DLL_RESULT_ERROR`.

### LPU237_ANALYSIS_get_cipher_masked_pan

**Prototype:**  
```c
DWORD WINAPI LPU237_ANALYSIS_get_cipher_masked_pan(const BYTE* pRawMsData, DWORD dwRawMsData, LPTSTR pMaskedPan, DWORD dwMaskedPan)
```

**Parameters:**  
- `pRawMsData`: [in] Raw data.  
- `dwRawMsData`: [in] Data size.  
- `pMaskedPan`: [out] Masked PAN buffer.  
- `dwMaskedPan`: [in] Buffer size.

**Return:**  
- Success: Length of PAN.  
- Error: `LPU237_DLL_RESULT_ERROR`.

### LPU237_ANALYSIS_get_cipher_card_holder_name

**Prototype:**  
```c
DWORD WINAPI LPU237_ANALYSIS_get_cipher_card_holder_name(const BYTE* pRawMsData, DWORD dwRawMsData, LPTSTR pName, DWORD dwName)
```

**Parameters:**  
- `pRawMsData`: [in] Raw data.  
- `dwRawMsData`: [in] Data size.  
- `pName`: [out] Name buffer.  
- `dwName`: [in] Buffer size.

**Return:**  
- Success: Length of name.  
- Error: `LPU237_DLL_RESULT_ERROR`.

### LPU237_ANALYSIS_get_cipher_expiration_date

**Prototype:**  
```c
DWORD WINAPI LPU237_ANALYSIS_get_cipher_expiration_date(const BYTE* pRawMsData, DWORD dwRawMsData, LPTSTR pDate, DWORD dwDate)
```

**Parameters:**  
- `pRawMsData`: [in] Raw data.  
- `dwRawMsData`: [in] Data size.  
- `pDate`: [out] Date buffer.  
- `dwDate`: [in] Buffer size.

**Return:**  
- Success: Length of date.  
- Error: `LPU237_DLL_RESULT_ERROR`.

### LPU237_ANALYSIS_get_cipher_mac4

**Prototype:**  
```c
DWORD WINAPI LPU237_ANALYSIS_get_cipher_mac4(const BYTE* pRawMsData, DWORD dwRawMsData, LPTSTR pMac4, DWORD dwMac4)
```

**Parameters:**  
- `pRawMsData`: [in] Raw data.  
- `dwRawMsData`: [in] Data size.  
- `pMac4`: [out] MAC4 buffer.  
- `dwMac4`: [in] Buffer size.

**Return:**  
- Success: Length of MAC4.  
- Error: `LPU237_DLL_RESULT_ERROR`.

### LPU237_ANALYSIS_get_cipher_ksn

**Prototype:**  
```c
DWORD WINAPI LPU237_ANALYSIS_get_cipher_ksn(const BYTE* pRawMsData, DWORD dwRawMsData, LPTSTR pKsn, DWORD dwKsn)
```

**Parameters:**  
- `pRawMsData`: [in] Raw data.  
- `dwRawMsData`: [in] Data size.  
- `pKsn`: [out] KSN buffer.  
- `dwKsn`: [in] Buffer size.

**Return:**  
- Success: Length of KSN.  
- Error: `LPU237_DLL_RESULT_ERROR`.

### LPU237_ANALYSIS_get_cipher_card_data

**Prototype:**  
```c
DWORD WINAPI LPU237_ANALYSIS_get_cipher_card_data(const BYTE* pRawMsData, DWORD dwRawMsData, DWORD dwIsoTrack, LPTSTR pCipher, DWORD dwCipher)
```

**Parameters:**  
- `pRawMsData`: [in] Raw data.  
- `dwRawMsData`: [in] Data size.  
- `dwIsoTrack`: [in] Track number.  
- `pCipher`: [out] Cipher data buffer.  
- `dwCipher`: [in] Buffer size.

**Return:**  
- Success: Length of cipher data.  
- Error: `LPU237_DLL_RESULT_ERROR`.

### LPU237_ANALYSIS_get_cipher_raw_data_except_mac

**Prototype:**  
```c
DWORD WINAPI LPU237_ANALYSIS_get_cipher_raw_data_except_mac(const BYTE* pRawMsData, DWORD dwRawMsData, BYTE* pRaw, DWORD dwRaw)
```

**Parameters:**  
- `pRawMsData`: [in] Raw data.  
- `dwRawMsData`: [in] Data size.  
- `pRaw`: [out] Raw data buffer (excluding MAC).  
- `dwRaw`: [in] Buffer size.

**Return:**  
- Success: Length of raw data.  
- Error: `LPU237_DLL_RESULT_ERROR`.

## Detail Programming Sequence

### Using LPU237_wait_swipe_with_waits()

Synchronous method for waiting swipe.

### Using LPU237_wait_swipe_with_callback()

Asynchronous with callback.

### Using LPU237_wait_swipe_with_message()

Asynchronous with window message.

## Ini File

Details on "lpu230_api.ini" for mode selection.

## Dual Mode

### NDM Mode

Next Device Manager mode (version 3.x required).

### Direct Mode

From version 4.0.

## History

Document history.