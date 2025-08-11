# LPU237 Setting Guide

This document describes the rules for creating a settings file for the LPU237 using an XML document, which can be loaded into the mapper setting program to configure the LPU237.

All element names, attributes, and values must be in lowercase English letters. If an attribute is omitted, the existing value remains unchanged. The settings file follows the XML format with UTF-8 character encoding.

For Mapper versions 1.43.0.4 and 1.44.0.4, the file extension must be `.txt`. For version 1.45.0.4 and later, the extension must be `.xml`.

The first line of the file must be:

```xml
<?xml version="1.0" encoding="UTF-8" ?>
```

The root element is `<lpu237>`, which contains the following sub-elements: `<common>`, `<global>`, `<iso1>`, `<iso2>`, `<iso3>`, and `<ibutton>`.

**Example:**

```xml
<?xml version="1.0" encoding="UTF-8" ?>
<lpu237>
    <common ... />
    <global ... />
    <iso1 ... />
    <iso2 ... />
    <iso3 ... />
    <ibutton ... />
</lpu237>
```

## Common Element Attributes

### interface
Determines the communication method of the LPU237. Possible values:
- `usb_kb`: Keyboard emulation mode (LPU237 only).
- `usb_hid`: USB communication using manufacturer-defined protocol.
- `rs232`: Asynchronous serial communication.
- `usb_vcom`: Virtual COM port (LPU238 only).

### buzzer
Controls the buzzer sound. Possible values:
- `on`: Buzzer enabled.
- `off`: Buzzer disabled.

### language
Sets the keyboard map language. Possible values:
- `usa_english`, `spanish`, `danish`, `french`, `german`, `italian`, `norwegian`, `swedish`, `hebrew`, `turkiye` (or `turkey`).

### iso1, iso2, iso3
Determines whether track data is transmitted. Possible values:
- `enable`: Transmits track data if read.
- `disable`: Does not transmit track data.

### condition
Determines when global prefix/postfix is sent. Possible values:
- `and`: Sends global prefix/postfix only if all tracks are valid.
- `or`: Sends global prefix/postfix if any track is valid.

### indication (Mapper v1.45.0.4 and later)
Configures LED and buzzer behavior. Possible values:
- `and`: Indicates normal processing when all tracks are valid.
- `or`: Indicates normal processing when any track is valid.

### ignore1 (Mapper v1.45.0.4 and later)
Special function for ISO1 and ISO2 data. Possible values:
- `enable`: Does not transmit ISO1 data if it matches ISO2 data.
- `disable`: Transmits ISO1 data even if it matches ISO2.

### ignore3 (Mapper v1.45.0.4 and later)
Special function for ISO3 and ISO2 data. Possible values:
- `enable`: Does not transmit ISO3 data if it matches ISO2 data.
- `disable`: Transmits ISO3 data even if it matches ISO2.

### rm_colon (Mapper v1.45.0.4 and later)
Special function for colon handling. Possible values:
- `enable`: Does not transmit colon if the ETX value is `0xe0` and the first data character is a colon when left-aligned.
- `disable`: Always transmits colon.

### ibutton
Determines the transmission method when an iButton is removed. Possible values:
- `zeros`: Sends keyboard `0` key 16 times.
- `f12`: Sends keyboard `F12` key.
- `zeros7`: Sends keyboard `0` key 7 times.
- `addimat`: Transmits according to Addimat code stick method.
- `none`: Sends the key defined in the `remove` attribute of the `<ibutton>` element (Mapper v1.45.0.4 and later).

### ibutton_start (Mapper v1.47.0.4 and later)
Sets the start offset for iButton key value transmission. Possible values:
- `0` to `15`: Offset value.

### ibutton_end (Mapper v1.47.0.4 and later)
Sets the end offset for iButton key value transmission. Possible values:
- `0` to `15`: Offset value.

### direction (Mapper v1.44.0.4 and later)
Determines the magnetic card reading direction. Possible values:
- `bidirectional`: Allows reading in both directions.
- `forward`: Allows reading in the forward direction.
- `backward`: Allows reading in the backward direction.

### track_order (Mapper v1.45.0.4 and later)
Determines the order of magnetic card track transmission. Possible values:
- `123`: Transmits track1, track2, track3.
- `132`: Transmits track1, track3, track2.
- `213`: Transmits track2, track1, track3.
- `231`: Transmits track2, track3, track1.
- `312`: Transmits track3, track1, track2.
- `321`: Transmits track3, track2, track1.

### mmd1100_reset_interval (Mapper v1.45.0.4 and later)
Sets the MMD1100 decoder hardware reset interval. Possible values:
- `default` or `0`: Resets every ~3 minutes 22 seconds.
- `disable` or `240`: No reset.
- `16`: Resets every ~6 minutes 43 seconds.
- `32`: Resets every ~13 minutes 27 seconds.
- `48`: Resets every ~20 minutes 10 seconds.
- `64`: Resets every ~26 minutes 53 seconds.
- `80`: Resets every ~33 minutes 36 seconds.
- `96`: Resets every ~40 minutes 19 seconds.
- `112`: Resets every ~47 minutes 3 seconds.
- `128`: Resets every ~53 minutes 46 seconds.
- `144`: Resets every ~1 hour 29 seconds.
- `160`: Resets every ~1 hour 7 minutes 12 seconds.
- `176`: Resets every ~1 hour 13 minutes 55 seconds.
- `192`: Resets every ~1 hour 20 minutes 39 seconds.
- `208`: Resets every ~1 hour 27 minutes 22 seconds.
- `224`: Resets every ~1 hour 34 minutes 5 seconds.

### mmd1100_iso_mode (Mapper v1.48.0.4 and later)
Sets the MMD1100 decoder mode. Possible values:
- `enable`: Sets to ISO mode (cannot revert to binary mode once set).
- `disable`: Sets to binary mode (default).

## Global Element

### prefix
Defines the value sent before the private prefix of the first enabled ISO track. Transmission depends on the `condition` attribute.

### postfix
Defines the value sent after the postfix of the last enabled ISO track. Transmission depends on the `condition` attribute.

## ISO1, ISO2, ISO3 Elements

### combination (Mapper v1.45.0.4 and later)
Sets the number of supported formats for tracks 1–3. Possible values:
- `1`: Supports one format (attributes ending in `0` are active).
- `2`: Supports two formats (attributes ending in `0` or `1` are active).
- `3`: Supports three formats (attributes ending in `0`, `1`, or `2` are active).

### max_size0, max_size1, max_size2 (Mapper v1.45.0.4 and later)
Sets the maximum number of characters (excluding STX, ETX, LRC) for formats 0, 1, or 2 in tracks 1–3. Each character supports up to 8 bits (including parity), with a 97-byte buffer per track (including STX, ETX, LRC, and NULLs). The value must satisfy:  
`((M+5)*Cb)/8 <= 97`, where `Cb` is the number of bits per character (including parity), and `M` is the attribute value.  
Typical values:
- ISO1 track: `76`
- ISO2 track: `38`
- ISO3 track: `104`

### bit_size0, bit_size1, bit_size2 (Mapper v1.45.0.4 and later)
Sets the number of bits (including parity) per character for formats 0, 1, or 2 in tracks 1–3. Possible values:
- `1` to `8`.

### data_mask0, data_mask1, data_mask2 (Mapper v1.45.0.4 and later)
Defines the mask bit pattern (in hexadecimal) for formats 0, 1, or 2 in tracks 1–3 when left-aligned to 8 bits.  
Examples:
- If `bit_size0` is `7`, the mask is `0xfe`.
- If `bit_size0` is `5`, the mask is `0xf8`.

### use_parity0, use_parity1, use_parity2 (Mapper v1.45.0.4 and later)
Determines if a parity bit is included for formats 0, 1, or 2 in tracks 1–3. Possible values:
- `enable`: Includes parity bit.
- `disable`: Excludes parity bit.

### parity_type0, parity_type1, parity_type2 (Mapper v1.45.0.4 and later)
Sets the parity type for formats 0, 1, or 2 in tracks 1–3 when parity is enabled. Possible values:
- `odd`: Odd parity (total number of 1 bits, including parity, is odd).
- `even`: Even parity.

### stxl0, stxl1, stxl2 (Mapper v1.45.0.4 and later)
Defines the start-of-text character (in hexadecimal, left-aligned, including parity) for formats 0, 1, or 2 in tracks 1–3.

### etxl0, etxl1, etxl2 (Mapper v1.45.0.4 and later)
Defines the end-of-text character (in hexadecimal, left-aligned, including parity) for formats 0, 1, or 2 in tracks 1–3.

### use_error_correct0, use_error_correct1, use_error_correct2 (Mapper v1.45.0.4 and later)
Determines if an error detection character is used for formats 0, 1, or 2 in tracks 1–3. Possible values:
- `enable`: Includes error detection character.
- `disable`: Excludes error detection character.

### error_correct_type0, error_correct_type1, error_correct_type2 (Mapper v1.45.0.4 and later)
Sets the error detection character type for formats 0, 1, or 2 in tracks 1–3 when enabled. Possible values:
- `lrc`: Longitudinal Redundancy Check.
- `invlrc`: Inverted LRC.

### add_value0, add_value1, add_value2 (Mapper v1.45.0.4 and later)
Defines the value (in hexadecimal) to add to a character to convert it to ASCII for formats 0, 1, or 2 in tracks 1–3. Possible values:
- `0x00` to `0xFF`.

### prefix, prefix0, prefix1, prefix2 (Mapper v1.45.0.4 and later for prefix0–2)
Defines the value sent before track data for formats 0, 1, or 2 in tracks 1–3. The `prefix` and `prefix0` attributes are identical; use `prefix0` for Mapper v1.45.0.4 and later.

### postfix, postfix0, postfix1, postfix2 (Mapper v1.45.0.4 and later for postfix0–2)
Defines the value sent after track data for formats 0, 1, or 2 in tracks 1–3. The `postfix` and `postfix0` attributes are identical; use `postfix0` for Mapper v1.45.0.4 and later.

## iButton Element

### prefix
Defines the value sent before iButton data.

### postfix
Defines the value sent after iButton data.

### remove
Defines the key(s) sent when the iButton is removed (up to 20 keys, referred to as “remove-indicator”).

### prefix_remove
Defines the value sent before the “remove-indicator”.

### postfix_remove
Defines the value sent after the “remove-indicator”.

## Prefix and Postfix Value Format

The values for `prefix`, `prefix0–2`, `postfix`, and `postfix0–2` consist of up to 7 pairs of modifier and key values, each enclosed in square brackets `[]`. For example, to send `?1:`, use: `[s][1][][1][s][;]`.

An empty string (`""`), `[]`, or `[][]` indicates no prefix/postfix.

### Modifiers
Possible modifier values:
- `s`: Left Shift key pressed.
- `c`: Left Control key pressed.
- `a`: Left Alt key pressed.

Modifiers can be combined. An empty modifier `[]` indicates no Shift, Control, or Alt keys are pressed. For example, `Ctrl+M` is represented as `[cs][m]`.

### Keys
Possible key values include:
- Function keys: `f1` to `f12`
- Letters: `a` to `z`
- Numbers: `0` to `9`
- Symbols: `-`, `=`, `;`, `:`, `,`, `.`, `/`, `\`, `[`, `]`, `'`, `` ` ``
- Special keys: `enter`, `esc`, `space`, `tab`, `bs` (backspace), `del`
- USB HID key codes (e.g., `[0x59]` for Keypad 1).

For a full list of USB HID key codes, refer to the USB HID Usage Tables (Hut1_12v2.pdf).