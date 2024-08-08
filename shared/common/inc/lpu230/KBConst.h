#ifndef _KEYBOARD_CODE_CONST_H_
#define _KEYBOARD_CODE_CONST_H_

#define	FOR_CVT_MAX_ASCII_CODE		130		//covertable maximmum of ascii-code.

#define	SUPPORT_KB_MAP			11

#define	HIDKEY_MAP_NUMBER			SUPPORT_KB_MAP	//the number of map table
#define	PS2KEY_MAP_NUMBER			SUPPORT_KB_MAP	//the number of map table

/////////////////////////////////////////////////////
//definition of key code.......  for USB keyboard
//USA.......(default definition)

// HID MODIFIERS KEYS
#define HIDKEY_MOD__NONE			0x00//none modifier
#define HIDKEY_MOD_L_CTL			0x01//left control
#define HIDKEY_MOD_L_SFT			0x02//left shift
#define HIDKEY_MOD_L_ALT			0x04//left alt
#define HIDKEY_MOD_L_GUI			0x08//left windows key
#define HIDKEY_MOD_R_CTL			0x10//right control
#define HIDKEY_MOD_R_SFT			0x20//right shift
#define HIDKEY_MOD_R_ALT			0x40//right alt
#define HIDKEY_MOD_R_GUI			0x80//right windows key


#define HIDKEY____a____A			0x04// a A
#define HIDKEY____b____B			0x05// b B
#define HIDKEY____c____C			0x06// c C
#define HIDKEY____d____D			0x07// d D
#define HIDKEY____e____E			0x08// e E
#define HIDKEY____f____F			0x09// f F
#define HIDKEY____g____G			0x0a// g G
#define HIDKEY____h____H			0x0b// h H
#define HIDKEY____i____I			0x0c// i I
#define HIDKEY____j____J			0x0d// j J
#define HIDKEY____k____K			0x0e// k K
#define HIDKEY____l____L			0x0f// l L
#define HIDKEY____m____M			0x10// m M
#define HIDKEY____n____N			0x11// n N
#define HIDKEY____o____O			0x12// o O
#define HIDKEY____p____P			0x13// p P
#define HIDKEY____q____Q			0x14// q Q
#define HIDKEY____r____R			0x15// r R
#define HIDKEY____s____S			0x16// s S
#define HIDKEY____t____T			0x17// t T
#define HIDKEY____u____U			0x18// u U
#define HIDKEY____v____V			0x19// v V
#define HIDKEY____w____W			0x1a// w W
#define HIDKEY____x____X			0x1b// x X
#define HIDKEY____y____Y			0x1c// y Y
#define HIDKEY____z____Z			0x1d// z Z
#define HIDKEY____1_EXCL			0x1e// 1 ! Exclamation point
#define HIDKEY____2_QUOT			0x1f// 2 @ Quotation mark
#define HIDKEY____3_SHAR			0x20// 3 # Sharp
#define HIDKEY____4_DOLL			0x21// 4 $ Dollar sign
#define HIDKEY____5_PERC			0x22// 5 % Percent sign
#define HIDKEY____6_CIRC			0x23// 6 ^ Circumflex
#define HIDKEY____7_AMPE			0x24// 7 & ampersand
#define HIDKEY____8_ASTE			0x25// 8 * asterisk
#define HIDKEY____9_L_PA			0x26// 9 ( left parenthesis
#define HIDKEY____0_R_PA			0x27// 0 ) right parenthesis
#define HIDKEY____RETURN			0x28// Return
#define HIDKEY____ESCAPE			0x29// Escape
#define HIDKEY_BACKSPACE			0x2a// Backspace
#define HIDKEY_______TAB			0x2b// Tab
#define HIDKEY_____SPACE			0x2c// Space
#define HIDKEY_MIN_UNDER			0x2d// - _ underline
#define HIDKEY_EQU__PLUS			0x2e// = +


#define HIDKEY_LBT___LBR			0x2f// [ { left bracket,left brace
#define HIDKEY_RBT___RBR			0x30// ] } right bracket,right brace
#define HIDKEY_BSLA_VBAR			0x31// \ | back slash, vertical bar 
#define HIDKEY_SEMI__COL			0x33// ; : semicolon, colon 
#define HIDKEY_APOS_QUOT			0x34// ' " apostrophe, Double Quotation
#define HIDKEY_GRAV_TILD			0x35// ` ~ Grave, Tilde
#define HIDKEY_COMA___LT			0x36// , < comma, less then sign
#define HIDKEY_PERIOD_GT			0x37// . > period, greater then sign
#define HIDKEY_SLASH__QM			0x38// / ? slash
#define HIDKEY__CAPSLOCK			0x39//Caps Lock
#define HIDKEY________F1			0x3a//F1
#define HIDKEY________F2			0x3b//F2
#define HIDKEY________F3			0x3c//F3
#define HIDKEY________F4			0x3d//F4
#define HIDKEY________F5			0x3e//F5
#define HIDKEY________F6			0x3f//F6
#define HIDKEY________F7			0x40//F7
#define HIDKEY________F8			0x41//F8
#define HIDKEY________F9			0x42//F19
#define HIDKEY_______F10			0x43//F10
#define HIDKEY_______F11			0x44//F11
#define HIDKEY_______F12			0x45//F12
#define HIDKEY_PRINT_SCR			0x46//Print Screen
#define HIDKEY_SCROLLLOC			0x47//Scroll Lock
#define HIDKEY_____BREAK			0x48//Break (Ctrl-Pause)
#define HIDKEY____INSERT			0x49//Insert
#define HIDKEY______HOME			0x4a//Home
#define HIDKEY____PAGEUP			0x4b//Page Up
#define HIDKEY____DELETE			0x4c//Delete
#define HIDKEY_______END			0x4d//End
#define HIDKEY__PAGEDOWN			0x4e//Page Down

#define HIDKEY_ARROW___R			0x4f//Right Arrow
#define HIDKEY_ARROW___L			0x50//Left Arrow
#define HIDKEY_ARROW___D			0x51//Down Arrow
#define HIDKEY_ARROW___U			0x52//Up Arrow

#define HIDKEY_KPAD__DIV			0x54//Keypad /
#define HIDKEY_KPAD__MUL			0x55//Keypad *
#define HIDKEY_KPAD_MINU			0x56//Keypad -
#define HIDKEY_KPAD_PLUS			0x57//Keypad +
#define HIDKEY_KEYPAD_EN	        0x58//Keypad Enter

#define HIDKEY_KEYPAD__1            0x59//Keypad 1 End
#define HIDKEY_KEYPAD__2            0x5a//Keypad 2 Down
#define HIDKEY_KEYPAD__3            0x5b//Keypad 3 PageDn
#define HIDKEY_KEYPAD__4            0x5c//Keypad 4 Left
#define HIDKEY_KEYPAD__5            0x5d//Keypad 5
#define HIDKEY_KEYPAD__6            0x5e//Keypad 6 Right
#define HIDKEY_KEYPAD__7            0x5f//Keypad 7 Home
#define HIDKEY_KEYPAD__8            0x60//Keypad 8 Up
#define HIDKEY_KEYPAD__9            0x61//Keypad 9 PageUp
#define HIDKEY_KEYPAD__0            0x62//Keypad 0 Insert
#define HIDKEY_KPAD__DOT			0x63//Keypad . Delete



/////////////////////////////////////////////////////
//definition of key code.......  for PS/2 keyboard scancode set2
//make code only, break code is 0xf0,make-code
//USA.......(default definition)

#define	PS2_BREAK_PRFIXCODE			0xf0

//control key
#define	PS2KEY______NONE			0x00
#define	PS2KEY_____L_CTL			0x14
#define	PS2KEY_____L_SFT			0x12
#define	PS2KEY_____L_ALT			0x11

//#define	PS2KEY_____R_CTL			0xE0 0x14
#define	PS2KEY_____R_CTL			0xF4	//using you must parsing data to 0xE0 0x14
#define	PS2KEY_____R_SFT			0x59
#define	PS2KEY_____R_ALT			0xF1	//using you must parsing data to 0xE0 0x11
//#define	PS2KEY_____R_ALT			0xE0 0x11

//general key
#define PS2KEY____a____A			0x1c// a A
#define PS2KEY____b____B			0x32// b B
#define PS2KEY____c____C			0x21// c C
#define PS2KEY____d____D			0x23// d D
#define PS2KEY____e____E			0x24// e E
#define PS2KEY____f____F			0x2b// f F
#define PS2KEY____g____G			0x34// g G
#define PS2KEY____h____H			0x33// h H
#define PS2KEY____i____I			0x43// i I
#define PS2KEY____j____J			0x3b// j J
#define PS2KEY____k____K			0x42// k K
#define PS2KEY____l____L			0x4b// l L
#define PS2KEY____m____M			0x3a// m M
#define PS2KEY____n____N			0x31// n N
#define PS2KEY____o____O			0x44// o O
#define PS2KEY____p____P			0x4d// p P
#define PS2KEY____q____Q			0x15// q Q
#define PS2KEY____r____R			0x2d// r R
#define PS2KEY____s____S			0x1b// s S
#define PS2KEY____t____T			0x2c// t T
#define PS2KEY____u____U			0x3c// u U
#define PS2KEY____v____V			0x2a// v V
#define PS2KEY____w____W			0x1d// w W
#define PS2KEY____x____X			0x22// x X
#define PS2KEY____y____Y			0x35// y Y
#define PS2KEY____z____Z			0x1a// z Z
#define PS2KEY____1_EXCL			0x16// 1 ! Exclamation point
#define PS2KEY____2_QUOT			0x1e// 2 @ Quotation mark
#define PS2KEY____3_SHAR			0x26// 3 # Sharp
#define PS2KEY____4_DOLL			0x25// 4 $ Dollar sign
#define PS2KEY____5_PERC			0x2e// 5 % Percent sign
#define PS2KEY____6_CIRC			0x36// 6 ^ Circumflex
#define PS2KEY____7_AMPE			0x3d// 7 & ampersand
#define PS2KEY____8_ASTE			0x3e// 8 * asterisk
#define PS2KEY____9_L_PA			0x46// 9 ( left parenthesis
#define PS2KEY____0_R_PA			0x45// 0 ) right parenthesis
#define PS2KEY____RETURN			0x5a// Return
#define PS2KEY____ESCAPE			0x76// Escape
#define PS2KEY_BACKSPACE			0x66// Backspace
#define PS2KEY_______TAB			0x0d// Tab
#define PS2KEY_____SPACE			0x29// Space
#define PS2KEY_MIN_UNDER			0x4e// - _ underline
#define PS2KEY_EQU__PLUS			0x55// = +


#define PS2KEY_LBT___LBR			0x54// [ { left bracket,left brace
#define PS2KEY_RBT___RBR			0x5b// ] } right bracket,right brace
#define PS2KEY_BSLA_VBAR			0x5d// \ | back slash, vertical bar 
#define PS2KEY_SEMI__COL			0x4c// ; : semicolon, colon 
#define PS2KEY_APOS_QUOT			0x52// ' " apostrophe, Quotation mark
#define PS2KEY_GRAV_TILD			0x0e// ` ~ Grave, Tilde
#define PS2KEY_COMA___LT			0x41// , < comma, less then sign
#define PS2KEY_PERIOD_GT			0x49// . > period, greater then sign
#define PS2KEY_SLASH__QM			0x4a// / ? slash
#define PS2KEY__CAPSLOCK			0x58//Caps Lock
#define PS2KEY________F1			0x05//F1
#define PS2KEY________F2			0x06//F2
#define PS2KEY________F3			0x04//F3
#define PS2KEY________F4			0x0c//F4
#define PS2KEY________F5			0x03//F5
#define PS2KEY________F6			0x0b//F6
#define PS2KEY________F7			0x83//F7
#define PS2KEY________F8			0x0a//F8
#define PS2KEY________F9			0x01//F9
#define PS2KEY_______F10			0x09//F10
#define PS2KEY_______F11			0x78//F11
#define PS2KEY_______F12			0x07//F12
//#define PS2KEY_PRINT_SCR			//Print Screen
#define PS2KEY_SCROLLLOC			0x7e//Scroll Lock
//#define PS2KEY_____BREAK			//Break (Ctrl-Pause)
//#define PS2KEY____INSERT			//Insert
//#define PS2KEY______HOME			//Home
//#define PS2KEY____PAGEUP			//Page Up
//#define PS2KEY____DELETE			//Delete
//#define PS2KEY_______END			//End
//#define PS2KEY__PAGEDOWN			//Page Down

//#define PS2KEY_ARROW___R			//Right Arrow
//#define PS2KEY_ARROW___L			//Left Arrow
//#define PS2KEY_ARROW___D			//Down Arrow
//#define PS2KEY_ARROW___U			//Up Arrow

//#define PS2KEY_KPAD__DIV			//Keypad /
#define PS2KEY_KPAD__MUL			0x7c//Keypad *
#define PS2KEY_KPAD_MINU			0x7b//Keypad -
#define PS2KEY_KPAD_PLUS			0x79//Keypad +
//#define PS2KEY_KEYPAD_EN	        //Keypad Enter

#define PS2KEY_KEYPAD__1            0x69//Keypad 1 End
#define PS2KEY_KEYPAD__2            0x72//Keypad 2 Down
#define PS2KEY_KEYPAD__3            0x7a//Keypad 3 PageDn
#define PS2KEY_KEYPAD__4            0x6b//Keypad 4 Left
#define PS2KEY_KEYPAD__5            0x73//Keypad 5
#define PS2KEY_KEYPAD__6            0x74//Keypad 6 Right
#define PS2KEY_KEYPAD__7            0x6c//Keypad 7 Home
#define PS2KEY_KEYPAD__8            0x75//Keypad 8 Up
#define PS2KEY_KEYPAD__9            0x7d//Keypad 9 PageUp
#define PS2KEY_KEYPAD__0            0x70//Keypad 0 Insert
#define PS2KEY_KPAD__DOT			0x71//Keypad . Delete

/*
;=============================
;PC to Keyboard Command Set
;=============================
*/
#define	PS2_CMD_RESET				0xff
#define	PS2_CMD_RESEND				0xfe
#define	PS2_CMD_SCANSET3_0			0xf7
#define	PS2_CMD_SCANSET3_1			0xf8
#define	PS2_CMD_SCANSET3_2			0xf9
#define	PS2_CMD_SCANSET3_3			0xfa
#define	PS2_CMD_SCANSET3_4			0xfb
#define	PS2_CMD_SCANSET3_5			0xfc
#define	PS2_CMD_SCANSET3_6			0xfd
#define	PS2_CMD_SETDEFAULT			0xf6
#define	PS2_CMD_DEFAULTDISABLE		0xf5
#define	PS2_CMD_ENABLE				0xf4
#define	PS2_CMD_SETTYPEMATICRATE	0xf3
#define	PS2_CMD_IDBYTEREQUEST		0xf2
#define	PS2_CMD_INVAILD				0xf1
#define	PS2_CMD_SETSCANSET			0xf0
#define	PS2_CMD_INVAILD0			0xef
#define	PS2_CMD_ECHO				0xee
#define	PS2_CMD_INDICATORCONTROL	0xed

/*
;=============================
;Keyboard to PC Command Set
;=============================
*/
#define	PS2_RSP_ACK					0xfa
#define	PS2_RSP_OVERRUN1			0xff	//Mark fifo as full for Scan Set1
#define	PS2_RSP_OVERRUN23			0xff	//Mark fifo as full for Scan Set2,3
#define	PS2_RSP_DIAGNOSTICFAILURE	0xfc
#define	PS2_RSP_BREAKCODEPREFIX		0xf0
#define	PS2_RSP_DIAGCOMPLETION		0xaa

#define	PS2_KEYBOARD_ID_XX			0xab
#define	PS2_KEYBOARD_ID_YY			0x83

#define	PS2_KEYBOARD_CUR_SCODE		0x02	//the currentm scan-code of keyboard

#define	PS2_SCAN2_BREAK_PREFIX		0xf0	//the prefix code of scancode-set2 


#define	FOR_CVT_MAX_ASCII_CODE		130		//covertable maximmum of ascii-code.

#endif//_KEYBOARD_CODE_CONST_H_
