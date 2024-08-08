#pragma once

#include <lpu230/KBConst.h>
#include <lpu230/info_sys.h>


class ckey_map
{
public:
	static const unsigned char* get_ascii_to_hid_key_map(unsigned long n_language, unsigned long n_ascii = 0, unsigned n_item = 0)
	{
#ifdef	WIN32
#pragma pack(push,1)
#endif	//WIN32
		//maps ASCII to USB HID-key code.
		//gASCToHIDKeyMap[i][j][k] ....... index i language map index, index j is ascii code.. editing 200 item
		static const unsigned char sASCToHIDKeyMap[HIDKEY_MAP_NUMBER][FOR_CVT_MAX_ASCII_CODE][2] = {
			{	//xxx.English.Table......
				//{ Modified key,HID key }
				// 0 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ 0,0 },//2
				{ 0,0 },//3
				{ 0,0 },//4
				{ 0,0 },//5
				{ 0,0 },//6
				{ 0,0 },//7
				{ HIDKEY_MOD__NONE,HIDKEY_BACKSPACE },//8
				{ HIDKEY_MOD__NONE,HIDKEY_______TAB },//9
				// 1 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ 0,0 },//2
				{ HIDKEY_MOD__NONE,HIDKEY____RETURN },//3 ... CR
				{ 0,0 },//4
				{ 0,0 },//5
				{ 0,0 },//6
				{ 0,0 },//7
				{ 0,0 },//8
				{ 0,0 },//9
				// 2 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ 0,0 },//2
				{ 0,0 },//3
				{ 0,0 },//4
				{ 0,0 },//5
				{ 0,0 },//6
				{ HIDKEY_MOD__NONE,HIDKEY____ESCAPE },//7
				{ 0,0 },//8
				{ 0,0 },//9
				// 3 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ HIDKEY_MOD__NONE,HIDKEY_____SPACE },//2
				{ HIDKEY_MOD_L_SFT,HIDKEY____1_EXCL },//3 !
				{ HIDKEY_MOD_L_SFT,HIDKEY_APOS_QUOT },//4 "
				{ HIDKEY_MOD_L_SFT,HIDKEY____3_SHAR },//5 #
				{ HIDKEY_MOD_L_SFT,HIDKEY____4_DOLL },//6 $
				{ HIDKEY_MOD_L_SFT,HIDKEY____5_PERC },//7 %
				{ HIDKEY_MOD_L_SFT,HIDKEY____7_AMPE },//8 &
				{ HIDKEY_MOD__NONE,HIDKEY_APOS_QUOT },//9 '
				// 4 ==================
				{ HIDKEY_MOD_L_SFT,HIDKEY____9_L_PA },//0 (
				{ HIDKEY_MOD_L_SFT,HIDKEY____0_R_PA },//1 )
				{ HIDKEY_MOD_L_SFT,HIDKEY____8_ASTE },//2 *
				{ HIDKEY_MOD_L_SFT,HIDKEY_EQU__PLUS },//3 +
				{ HIDKEY_MOD__NONE,HIDKEY_COMA___LT },//4 ,
				{ HIDKEY_MOD__NONE,HIDKEY_MIN_UNDER },//5 -
				{ HIDKEY_MOD__NONE,HIDKEY_PERIOD_GT },//6 .
				{ HIDKEY_MOD__NONE,HIDKEY_SLASH__QM },//7 /
				{ HIDKEY_MOD__NONE,HIDKEY____0_R_PA },//8 0
				{ HIDKEY_MOD__NONE,HIDKEY____1_EXCL },//9 1
				// 5 ==================
				{ HIDKEY_MOD__NONE,HIDKEY____2_QUOT },//0 2
				{ HIDKEY_MOD__NONE,HIDKEY____3_SHAR },//1 3
				{ HIDKEY_MOD__NONE,HIDKEY____4_DOLL },//2 4
				{ HIDKEY_MOD__NONE,HIDKEY____5_PERC },//3 5
				{ HIDKEY_MOD__NONE,HIDKEY____6_CIRC },//4 6
				{ HIDKEY_MOD__NONE,HIDKEY____7_AMPE },//5 7
				{ HIDKEY_MOD__NONE,HIDKEY____8_ASTE },//6 8
				{ HIDKEY_MOD__NONE,HIDKEY____9_L_PA },//7 9
				{ HIDKEY_MOD_L_SFT,HIDKEY_SEMI__COL },//8 :
				{ HIDKEY_MOD__NONE,HIDKEY_SEMI__COL },//9 ;
				// 6 ==================
				{ HIDKEY_MOD_L_SFT,HIDKEY_COMA___LT },//0 <
				{ HIDKEY_MOD__NONE,HIDKEY_EQU__PLUS },//1 =
				{ HIDKEY_MOD_L_SFT,HIDKEY_PERIOD_GT },//2 >
				{ HIDKEY_MOD_L_SFT,HIDKEY_SLASH__QM },//3 ?
				{ HIDKEY_MOD_L_SFT,HIDKEY____2_QUOT },//4 @
				{ HIDKEY_MOD_L_SFT,HIDKEY____a____A },//5 A
				{ HIDKEY_MOD_L_SFT,HIDKEY____b____B },//6 B
				{ HIDKEY_MOD_L_SFT,HIDKEY____c____C },//7 C
				{ HIDKEY_MOD_L_SFT,HIDKEY____d____D },//8 D
				{ HIDKEY_MOD_L_SFT,HIDKEY____e____E },//9 E
				// 7 ==================
				{ HIDKEY_MOD_L_SFT,HIDKEY____f____F },//0 F
				{ HIDKEY_MOD_L_SFT,HIDKEY____g____G },//1 G
				{ HIDKEY_MOD_L_SFT,HIDKEY____h____H },//2 H
				{ HIDKEY_MOD_L_SFT,HIDKEY____i____I },//3 I
				{ HIDKEY_MOD_L_SFT,HIDKEY____j____J },//4 J
				{ HIDKEY_MOD_L_SFT,HIDKEY____k____K },//5 K
				{ HIDKEY_MOD_L_SFT,HIDKEY____l____L },//6 L
				{ HIDKEY_MOD_L_SFT,HIDKEY____m____M },//7 M
				{ HIDKEY_MOD_L_SFT,HIDKEY____n____N },//8 N
				{ HIDKEY_MOD_L_SFT,HIDKEY____o____O },//9 O
				// 8 ==================
				{ HIDKEY_MOD_L_SFT,HIDKEY____p____P },//0 P
				{ HIDKEY_MOD_L_SFT,HIDKEY____q____Q },//1 Q
				{ HIDKEY_MOD_L_SFT,HIDKEY____r____R },//2 R
				{ HIDKEY_MOD_L_SFT,HIDKEY____s____S },//3 S
				{ HIDKEY_MOD_L_SFT,HIDKEY____t____T },//4 T
				{ HIDKEY_MOD_L_SFT,HIDKEY____u____U },//5 U
				{ HIDKEY_MOD_L_SFT,HIDKEY____v____V },//6 V
				{ HIDKEY_MOD_L_SFT,HIDKEY____w____W },//7 W
				{ HIDKEY_MOD_L_SFT,HIDKEY____x____X },//8 X
				{ HIDKEY_MOD_L_SFT,HIDKEY____y____Y },//9 Y
				// 9 ==================
				{ HIDKEY_MOD_L_SFT,HIDKEY____z____Z },//0 Z
				{ HIDKEY_MOD__NONE,HIDKEY_LBT___LBR },//1 [
				{ HIDKEY_MOD__NONE,HIDKEY_BSLA_VBAR },//2
				{ HIDKEY_MOD__NONE,HIDKEY_RBT___RBR },//3 ]
				{ HIDKEY_MOD_L_SFT,HIDKEY____6_CIRC },//4 ^
				{ HIDKEY_MOD_L_SFT,HIDKEY_MIN_UNDER },//5 _
				{ HIDKEY_MOD__NONE,HIDKEY_GRAV_TILD },//6 `
				{ HIDKEY_MOD__NONE,HIDKEY____a____A },//7 a
				{ HIDKEY_MOD__NONE,HIDKEY____b____B },//8 b
				{ HIDKEY_MOD__NONE,HIDKEY____c____C },//9 c
				// 10 ==================
				{ HIDKEY_MOD__NONE,HIDKEY____d____D },//0 d
				{ HIDKEY_MOD__NONE,HIDKEY____e____E },//1 e
				{ HIDKEY_MOD__NONE,HIDKEY____f____F },//2 f
				{ HIDKEY_MOD__NONE,HIDKEY____g____G },//3 g
				{ HIDKEY_MOD__NONE,HIDKEY____h____H },//4 h
				{ HIDKEY_MOD__NONE,HIDKEY____i____I },//5 i
				{ HIDKEY_MOD__NONE,HIDKEY____j____J },//6 j
				{ HIDKEY_MOD__NONE,HIDKEY____k____K },//7 k
				{ HIDKEY_MOD__NONE,HIDKEY____l____L },//8 l
				{ HIDKEY_MOD__NONE,HIDKEY____m____M },//9 m
				// 11 ==================
				{ HIDKEY_MOD__NONE,HIDKEY____n____N },//0 n
				{ HIDKEY_MOD__NONE,HIDKEY____o____O },//1 o
				{ HIDKEY_MOD__NONE,HIDKEY____p____P },//2 p
				{ HIDKEY_MOD__NONE,HIDKEY____q____Q },//3 q
				{ HIDKEY_MOD__NONE,HIDKEY____r____R },//4 r
				{ HIDKEY_MOD__NONE,HIDKEY____s____S },//5 s
				{ HIDKEY_MOD__NONE,HIDKEY____t____T },//6 t
				{ HIDKEY_MOD__NONE,HIDKEY____u____U },//7 u
				{ HIDKEY_MOD__NONE,HIDKEY____v____V },//8 v
				{ HIDKEY_MOD__NONE,HIDKEY____w____W },//9 w
				// 12 ==================
				{ HIDKEY_MOD__NONE,HIDKEY____x____X },//0 x
				{ HIDKEY_MOD__NONE,HIDKEY____y____Y },//1 y
				{ HIDKEY_MOD__NONE,HIDKEY____z____Z },//2 z
				{ HIDKEY_MOD_L_SFT,HIDKEY_LBT___LBR },//3 {
				{ HIDKEY_MOD_L_SFT,HIDKEY_BSLA_VBAR },//4 |
				{ HIDKEY_MOD_L_SFT,HIDKEY_RBT___RBR },//5 }
				{ HIDKEY_MOD_L_SFT,HIDKEY_GRAV_TILD },//6 ~
				{ 0,0 },//7
				{ 0,0 },//8
				{ 0,0 }//9
			},

			{	//xxx.Spanish.Table......
				//{ Modified key,HID key }
				// 0 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ 0,0 },//2
				{ 0,0 },//3
				{ 0,0 },//4
				{ 0,0 },//5
				{ 0,0 },//6
				{ 0,0 },//7
				{ HIDKEY_MOD__NONE,HIDKEY_BACKSPACE },//8
				{ HIDKEY_MOD__NONE,HIDKEY_______TAB },//9
				// 1 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ 0,0 },//2
				{ HIDKEY_MOD__NONE,HIDKEY____RETURN },//3 ... CR
				{ 0,0 },//4
				{ 0,0 },//5
				{ 0,0 },//6
				{ HIDKEY_MOD_L_SFT,0x33 },//7 - Spanish
				{ HIDKEY_MOD_L_SFT,0x31 },//8 - Spanish
				{ HIDKEY_MOD_L_SFT,0x34 },//9 - Spanish
				// 2 ==================
				{ HIDKEY_MOD_L_SFT,0x35 },//0 - Spanish
				{ 0,0 },//1
				{ 0,0 },//2
				{ 0,0 },//3
				{ 0,0 },//4
				{ 0,0 },//5
				{ 0,0 },//6
				{ HIDKEY_MOD__NONE,HIDKEY____ESCAPE },//7
				{ 0,0 },//8
				{ 0,0 },//9
				// 3 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ HIDKEY_MOD__NONE,HIDKEY_____SPACE },//2
				{ HIDKEY_MOD_L_SFT,HIDKEY____1_EXCL },//3 !
				{ HIDKEY_MOD_L_SFT,HIDKEY____2_QUOT },//4 " - Spanish
				{ HIDKEY_MOD_R_ALT,HIDKEY____3_SHAR },//5 #
				{ HIDKEY_MOD_L_SFT,HIDKEY____4_DOLL },//6 $
				{ HIDKEY_MOD_L_SFT,HIDKEY____5_PERC },//7 %
				{ HIDKEY_MOD_L_SFT,HIDKEY____6_CIRC },//8 & - Spanish
				{ HIDKEY_MOD__NONE,HIDKEY_MIN_UNDER },//9 ' - Spanish
				// 4 ==================
				{ HIDKEY_MOD_L_SFT,HIDKEY____8_ASTE },//0 ( - Spanish
				{ HIDKEY_MOD_L_SFT,HIDKEY____9_L_PA },//1 ) - Spanish
				{ HIDKEY_MOD_L_SFT,HIDKEY_RBT___RBR },//2 * - Spanish
				{ HIDKEY_MOD__NONE,HIDKEY_RBT___RBR },//3 + - Spanish
				{ HIDKEY_MOD__NONE,HIDKEY_COMA___LT },//4 ,
				{ HIDKEY_MOD__NONE,HIDKEY_SLASH__QM },//5 - - Spanish
				{ HIDKEY_MOD__NONE,HIDKEY_PERIOD_GT },//6 .
				{ HIDKEY_MOD_L_SFT,HIDKEY____7_AMPE },//7 / - Spanish
				{ HIDKEY_MOD__NONE,HIDKEY____0_R_PA },//8 0
				{ HIDKEY_MOD__NONE,HIDKEY____1_EXCL },//9 1
				// 5 ==================
				{ HIDKEY_MOD__NONE,HIDKEY____2_QUOT },//0 2
				{ HIDKEY_MOD__NONE,HIDKEY____3_SHAR },//1 3
				{ HIDKEY_MOD__NONE,HIDKEY____4_DOLL },//2 4
				{ HIDKEY_MOD__NONE,HIDKEY____5_PERC },//3 5
				{ HIDKEY_MOD__NONE,HIDKEY____6_CIRC },//4 6
				{ HIDKEY_MOD__NONE,HIDKEY____7_AMPE },//5 7
				{ HIDKEY_MOD__NONE,HIDKEY____8_ASTE },//6 8
				{ HIDKEY_MOD__NONE,HIDKEY____9_L_PA },//7 9
				{ HIDKEY_MOD_L_SFT,HIDKEY_PERIOD_GT },//8 : - Spanish
				{ HIDKEY_MOD_L_SFT,HIDKEY_COMA___LT },//9 ; - Spanish
				// 6 ==================
				{ HIDKEY_MOD__NONE,0x64 },//0 < - Spanish
				{ HIDKEY_MOD_L_SFT,HIDKEY____0_R_PA },//1 = - Spanish
				{ HIDKEY_MOD_L_SFT,0x64 },//2 > - Spanish
				{ HIDKEY_MOD_L_SFT,HIDKEY_MIN_UNDER },//3 ? - Spanish
				{ HIDKEY_MOD_R_ALT,HIDKEY____2_QUOT },//4 @ - Spanish
				{ HIDKEY_MOD_L_SFT,HIDKEY____a____A },//5 A
				{ HIDKEY_MOD_L_SFT,HIDKEY____b____B },//6 B
				{ HIDKEY_MOD_L_SFT,HIDKEY____c____C },//7 C
				{ HIDKEY_MOD_L_SFT,HIDKEY____d____D },//8 D
				{ HIDKEY_MOD_L_SFT,HIDKEY____e____E },//9 E
				// 7 ==================
				{ HIDKEY_MOD_L_SFT,HIDKEY____f____F },//0 F
				{ HIDKEY_MOD_L_SFT,HIDKEY____g____G },//1 G
				{ HIDKEY_MOD_L_SFT,HIDKEY____h____H },//2 H
				{ HIDKEY_MOD_L_SFT,HIDKEY____i____I },//3 I
				{ HIDKEY_MOD_L_SFT,HIDKEY____j____J },//4 J
				{ HIDKEY_MOD_L_SFT,HIDKEY____k____K },//5 K
				{ HIDKEY_MOD_L_SFT,HIDKEY____l____L },//6 L
				{ HIDKEY_MOD_L_SFT,HIDKEY____m____M },//7 M
				{ HIDKEY_MOD_L_SFT,HIDKEY____n____N },//8 N
				{ HIDKEY_MOD_L_SFT,HIDKEY____o____O },//9 O
				// 8 ==================
				{ HIDKEY_MOD_L_SFT,HIDKEY____p____P },//0 P
				{ HIDKEY_MOD_L_SFT,HIDKEY____q____Q },//1 Q
				{ HIDKEY_MOD_L_SFT,HIDKEY____r____R },//2 R
				{ HIDKEY_MOD_L_SFT,HIDKEY____s____S },//3 S
				{ HIDKEY_MOD_L_SFT,HIDKEY____t____T },//4 T
				{ HIDKEY_MOD_L_SFT,HIDKEY____u____U },//5 U
				{ HIDKEY_MOD_L_SFT,HIDKEY____v____V },//6 V
				{ HIDKEY_MOD_L_SFT,HIDKEY____w____W },//7 W
				{ HIDKEY_MOD_L_SFT,HIDKEY____x____X },//8 X
				{ HIDKEY_MOD_L_SFT,HIDKEY____y____Y },//9 Y
				// 9 ==================
				{ HIDKEY_MOD_L_SFT,HIDKEY____z____Z },//0 Z
				{ HIDKEY_MOD_R_ALT,HIDKEY_LBT___LBR },//1 [ - Spanish
				{ HIDKEY_MOD_R_ALT,HIDKEY_GRAV_TILD },//2
				{ HIDKEY_MOD_R_ALT,HIDKEY_RBT___RBR },//3 ] - Spanish
				{ HIDKEY_MOD_L_SFT,HIDKEY_LBT___LBR },//4 ^ - Spanish
				{ HIDKEY_MOD_L_SFT,HIDKEY_SLASH__QM },//5 _ - Spanish
				{ HIDKEY_MOD__NONE,0x2f },//6 ` - Spanish
				{ HIDKEY_MOD__NONE,HIDKEY____a____A },//7 a
				{ HIDKEY_MOD__NONE,HIDKEY____b____B },//8 b
				{ HIDKEY_MOD__NONE,HIDKEY____c____C },//9 c
				// 10 ==================
				{ HIDKEY_MOD__NONE,HIDKEY____d____D },//0 d
				{ HIDKEY_MOD__NONE,HIDKEY____e____E },//1 e
				{ HIDKEY_MOD__NONE,HIDKEY____f____F },//2 f
				{ HIDKEY_MOD__NONE,HIDKEY____g____G },//3 g
				{ HIDKEY_MOD__NONE,HIDKEY____h____H },//4 h
				{ HIDKEY_MOD__NONE,HIDKEY____i____I },//5 i
				{ HIDKEY_MOD__NONE,HIDKEY____j____J },//6 j
				{ HIDKEY_MOD__NONE,HIDKEY____k____K },//7 k
				{ HIDKEY_MOD__NONE,HIDKEY____l____L },//8 l
				{ HIDKEY_MOD__NONE,HIDKEY____m____M },//9 m
				// 11 ==================
				{ HIDKEY_MOD__NONE,HIDKEY____n____N },//0 n
				{ HIDKEY_MOD__NONE,HIDKEY____o____O },//1 o
				{ HIDKEY_MOD__NONE,HIDKEY____p____P },//2 p
				{ HIDKEY_MOD__NONE,HIDKEY____q____Q },//3 q
				{ HIDKEY_MOD__NONE,HIDKEY____r____R },//4 r
				{ HIDKEY_MOD__NONE,HIDKEY____s____S },//5 s
				{ HIDKEY_MOD__NONE,HIDKEY____t____T },//6 t
				{ HIDKEY_MOD__NONE,HIDKEY____u____U },//7 u
				{ HIDKEY_MOD__NONE,HIDKEY____v____V },//8 v
				{ HIDKEY_MOD__NONE,HIDKEY____w____W },//9 w
				// 12 ==================
				{ HIDKEY_MOD__NONE,HIDKEY____x____X },//0 x
				{ HIDKEY_MOD__NONE,HIDKEY____y____Y },//1 y
				{ HIDKEY_MOD__NONE,HIDKEY____z____Z },//2 z
				{ HIDKEY_MOD_L_SFT,0x33 },//3 { - Spanish
				{ HIDKEY_MOD_L_SFT,HIDKEY_BSLA_VBAR },//4 |
				{ HIDKEY_MOD_L_SFT,0x34 },//5 } - Spanish
				{ HIDKEY_MOD_L_SFT,0x35 },//6 ~ - Spanish
				{ 0,0 },//7
				{ 0,0 },//8
				{ 0,0 }//9
			},
			{	//xxx.Danish.Table......
				//{ Modified key,HID key }
				// 0 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ 0,0 },//2
				{ 0,0 },//3
				{ 0,0 },//4
				{ 0,0 },//5
				{ 0,0 },//6
				{ 0,0 },//7
				{ HIDKEY_MOD__NONE,HIDKEY_BACKSPACE },//8
				{ HIDKEY_MOD__NONE,HIDKEY_______TAB },//9
				// 1 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ 0,0 },//2
				{ HIDKEY_MOD__NONE,HIDKEY____RETURN },//3 ... CR
				{ 0,0 },//4
				{ 0,0 },//5
				{ 0,0 },//6
				{ 0,0 },//7
				{ 0,0 },//8
				{ 0,0 },//9
				// 2 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ 0,0 },//2
				{ 0,0 },//3
				{ 0,0 },//4
				{ 0,0 },//5
				{ 0,0 },//6
				{ HIDKEY_MOD__NONE,HIDKEY____ESCAPE },//7
				{ 0,0 },//8
				{ 0,0 },//9
				// 3 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ HIDKEY_MOD__NONE,HIDKEY_____SPACE },//2
				{ HIDKEY_MOD_L_SFT,HIDKEY____1_EXCL },//3 !
				{ HIDKEY_MOD_L_SFT,HIDKEY____2_QUOT },//4 "
				{ HIDKEY_MOD_L_SFT,HIDKEY____3_SHAR },//5 #
				{ HIDKEY_MOD_R_ALT,HIDKEY____4_DOLL },//6 $
				{ HIDKEY_MOD_L_SFT,HIDKEY____5_PERC },//7 %
				{ HIDKEY_MOD_L_SFT,HIDKEY____6_CIRC },//8 &
				{ HIDKEY_MOD__NONE,0x32 },//9 '
				// 4 ==================
				{ HIDKEY_MOD_L_SFT,HIDKEY____8_ASTE },//0 (
				{ HIDKEY_MOD_L_SFT,HIDKEY____9_L_PA },//1 )
				{ HIDKEY_MOD_L_SFT,0x32 },//2 *
				{ HIDKEY_MOD__NONE,HIDKEY_MIN_UNDER },//3 +
				{ HIDKEY_MOD__NONE,HIDKEY_COMA___LT },//4 ,
				{ HIDKEY_MOD__NONE,HIDKEY_SLASH__QM },//5 -
				{ HIDKEY_MOD__NONE,HIDKEY_PERIOD_GT },//6 .
				{ HIDKEY_MOD_L_SFT,HIDKEY____7_AMPE },//7 /
				{ HIDKEY_MOD__NONE,HIDKEY____0_R_PA },//8 0
				{ HIDKEY_MOD__NONE,HIDKEY____1_EXCL },//9 1
				// 5 ==================
				{ HIDKEY_MOD__NONE,HIDKEY____2_QUOT },//0 2
				{ HIDKEY_MOD__NONE,HIDKEY____3_SHAR },//1 3
				{ HIDKEY_MOD__NONE,HIDKEY____4_DOLL },//2 4
				{ HIDKEY_MOD__NONE,HIDKEY____5_PERC },//3 5
				{ HIDKEY_MOD__NONE,HIDKEY____6_CIRC },//4 6
				{ HIDKEY_MOD__NONE,HIDKEY____7_AMPE },//5 7
				{ HIDKEY_MOD__NONE,HIDKEY____8_ASTE },//6 8
				{ HIDKEY_MOD__NONE,HIDKEY____9_L_PA },//7 9
				{ HIDKEY_MOD_L_SFT,HIDKEY_PERIOD_GT },//8 :
				{ HIDKEY_MOD_L_SFT,HIDKEY_COMA___LT },//9 ;
				// 6 ==================
				{ HIDKEY_MOD__NONE,0x64 },//0 <
				{ HIDKEY_MOD_L_SFT,HIDKEY____0_R_PA },//1 =
				{ HIDKEY_MOD_L_SFT,0x64 },//2 >
				{ HIDKEY_MOD_L_SFT,HIDKEY_MIN_UNDER },//3 ?
				{ HIDKEY_MOD_R_ALT,HIDKEY____2_QUOT },//4 @
				{ HIDKEY_MOD_L_SFT,HIDKEY____a____A },//5 A
				{ HIDKEY_MOD_L_SFT,HIDKEY____b____B },//6 B
				{ HIDKEY_MOD_L_SFT,HIDKEY____c____C },//7 C
				{ HIDKEY_MOD_L_SFT,HIDKEY____d____D },//8 D
				{ HIDKEY_MOD_L_SFT,HIDKEY____e____E },//9 E
				// 7 ==================
				{ HIDKEY_MOD_L_SFT,HIDKEY____f____F },//0 F
				{ HIDKEY_MOD_L_SFT,HIDKEY____g____G },//1 G
				{ HIDKEY_MOD_L_SFT,HIDKEY____h____H },//2 H
				{ HIDKEY_MOD_L_SFT,HIDKEY____i____I },//3 I
				{ HIDKEY_MOD_L_SFT,HIDKEY____j____J },//4 J
				{ HIDKEY_MOD_L_SFT,HIDKEY____k____K },//5 K
				{ HIDKEY_MOD_L_SFT,HIDKEY____l____L },//6 L
				{ HIDKEY_MOD_L_SFT,HIDKEY____m____M },//7 M
				{ HIDKEY_MOD_L_SFT,HIDKEY____n____N },//8 N
				{ HIDKEY_MOD_L_SFT,HIDKEY____o____O },//9 O
				// 8 ==================
				{ HIDKEY_MOD_L_SFT,HIDKEY____p____P },//0 P
				{ HIDKEY_MOD_L_SFT,HIDKEY____q____Q },//1 Q
				{ HIDKEY_MOD_L_SFT,HIDKEY____r____R },//2 R
				{ HIDKEY_MOD_L_SFT,HIDKEY____s____S },//3 S
				{ HIDKEY_MOD_L_SFT,HIDKEY____t____T },//4 T
				{ HIDKEY_MOD_L_SFT,HIDKEY____u____U },//5 U
				{ HIDKEY_MOD_L_SFT,HIDKEY____v____V },//6 V
				{ HIDKEY_MOD_L_SFT,HIDKEY____w____W },//7 W
				{ HIDKEY_MOD_L_SFT,HIDKEY____x____X },//8 X
				{ HIDKEY_MOD_L_SFT,HIDKEY____y____Y },//9 Y
				// 9 ==================
				{ HIDKEY_MOD_L_SFT,HIDKEY____z____Z },//0 Z
				{ HIDKEY_MOD_R_ALT,HIDKEY____8_ASTE },//1 [
				{ HIDKEY_MOD_R_ALT,0x64 },//2
				{ HIDKEY_MOD_R_ALT,HIDKEY____9_L_PA },//3 ]
				{ HIDKEY_MOD_L_SFT,HIDKEY_RBT___RBR },//4 ^
				{ HIDKEY_MOD_L_SFT,HIDKEY_SLASH__QM },//5 _
				{ HIDKEY_MOD__NONE,HIDKEY_GRAV_TILD },//6 `
				{ HIDKEY_MOD__NONE,HIDKEY____a____A },//7 a
				{ HIDKEY_MOD__NONE,HIDKEY____b____B },//8 b
				{ HIDKEY_MOD__NONE,HIDKEY____c____C },//9 c
				// 10 ==================
				{ HIDKEY_MOD__NONE,HIDKEY____d____D },//0 d
				{ HIDKEY_MOD__NONE,HIDKEY____e____E },//1 e
				{ HIDKEY_MOD__NONE,HIDKEY____f____F },//2 f
				{ HIDKEY_MOD__NONE,HIDKEY____g____G },//3 g
				{ HIDKEY_MOD__NONE,HIDKEY____h____H },//4 h
				{ HIDKEY_MOD__NONE,HIDKEY____i____I },//5 i
				{ HIDKEY_MOD__NONE,HIDKEY____j____J },//6 j
				{ HIDKEY_MOD__NONE,HIDKEY____k____K },//7 k
				{ HIDKEY_MOD__NONE,HIDKEY____l____L },//8 l
				{ HIDKEY_MOD__NONE,HIDKEY____m____M },//9 m
				// 11 ==================
				{ HIDKEY_MOD__NONE,HIDKEY____n____N },//0 n
				{ HIDKEY_MOD__NONE,HIDKEY____o____O },//1 o
				{ HIDKEY_MOD__NONE,HIDKEY____p____P },//2 p
				{ HIDKEY_MOD__NONE,HIDKEY____q____Q },//3 q
				{ HIDKEY_MOD__NONE,HIDKEY____r____R },//4 r
				{ HIDKEY_MOD__NONE,HIDKEY____s____S },//5 s
				{ HIDKEY_MOD__NONE,HIDKEY____t____T },//6 t
				{ HIDKEY_MOD__NONE,HIDKEY____u____U },//7 u
				{ HIDKEY_MOD__NONE,HIDKEY____v____V },//8 v
				{ HIDKEY_MOD__NONE,HIDKEY____w____W },//9 w
				// 12 ==================
				{ HIDKEY_MOD__NONE,HIDKEY____x____X },//0 x
				{ HIDKEY_MOD__NONE,HIDKEY____y____Y },//1 y
				{ HIDKEY_MOD__NONE,HIDKEY____z____Z },//2 z
				{ HIDKEY_MOD_L_SFT,HIDKEY_LBT___LBR },//3 {
				{ HIDKEY_MOD_L_SFT,HIDKEY_BSLA_VBAR },//4 |
				{ HIDKEY_MOD_L_SFT,HIDKEY_RBT___RBR },//5 }
				{ HIDKEY_MOD_L_SFT,HIDKEY_GRAV_TILD },//6 ~
				{ 0,0 },//7
				{ 0,0 },//8
				{ 0,0 }//9
			},
			{	//xxx.French Table......
				//{ Modified key,HID key }
				// 0 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ 0,0 },//2
				{ 0,0 },//3
				{ 0,0 },//4
				{ 0,0 },//5
				{ 0,0 },//6
				{ 0,0 },//7
				{ HIDKEY_MOD__NONE,HIDKEY_BACKSPACE },//8
				{ HIDKEY_MOD__NONE,HIDKEY_______TAB },//9
				// 1 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ 0,0 },//2
				{ HIDKEY_MOD__NONE,HIDKEY____RETURN },//3 ... CR
				{ 0,0 },//4
				{ 0,0 },//5
				{ 0,0 },//6
				{ 0,0 },//7
				{ 0,0 },//8
				{ 0,0 },//9
				// 2 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ 0,0 },//2
				{ 0,0 },//3
				{ 0,0 },//4
				{ 0,0 },//5
				{ 0,0 },//6
				{ HIDKEY_MOD__NONE,HIDKEY____ESCAPE },//7
				{ 0,0 },//8
				{ 0,0 },//9
				// 3 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ HIDKEY_MOD__NONE,HIDKEY_____SPACE },//2
				{ HIDKEY_MOD__NONE,HIDKEY_SLASH__QM },//3 !
				{ HIDKEY_MOD__NONE,HIDKEY____3_SHAR },//4 "
				{ HIDKEY_MOD_R_ALT,HIDKEY____3_SHAR },//5 #
				{ HIDKEY_MOD__NONE,HIDKEY_RBT___RBR },//6 $
				{ HIDKEY_MOD_L_SFT,HIDKEY_APOS_QUOT },//7 %
				{ HIDKEY_MOD__NONE,HIDKEY____1_EXCL },//8 &
				{ HIDKEY_MOD__NONE,HIDKEY____4_DOLL },//9 '
				// 4 ==================
				{ HIDKEY_MOD__NONE,HIDKEY____5_PERC },//0 (
				{ HIDKEY_MOD__NONE,HIDKEY_MIN_UNDER },//1 )
				{ HIDKEY_MOD__NONE,0x32 },//2 *
				{ HIDKEY_MOD_L_SFT,HIDKEY_EQU__PLUS },//3 +
				{ HIDKEY_MOD__NONE,HIDKEY____m____M },//4 ,
				{ HIDKEY_MOD__NONE,HIDKEY____6_CIRC },//5 -
				{ HIDKEY_MOD_L_SFT,HIDKEY_COMA___LT },//6 .
				{ HIDKEY_MOD_L_SFT,HIDKEY_PERIOD_GT },//7 /
				{ HIDKEY_MOD_L_SFT,HIDKEY____0_R_PA },//8 0
				{ HIDKEY_MOD_L_SFT,HIDKEY____1_EXCL },//9 1
				// 5 ==================
				{ HIDKEY_MOD_L_SFT,HIDKEY____2_QUOT },//0 2
				{ HIDKEY_MOD_L_SFT,HIDKEY____3_SHAR },//1 3
				{ HIDKEY_MOD_L_SFT,HIDKEY____4_DOLL },//2 4
				{ HIDKEY_MOD_L_SFT,HIDKEY____5_PERC },//3 5
				{ HIDKEY_MOD_L_SFT,HIDKEY____6_CIRC },//4 6
				{ HIDKEY_MOD_L_SFT,HIDKEY____7_AMPE },//5 7
				{ HIDKEY_MOD_L_SFT,HIDKEY____8_ASTE },//6 8
				{ HIDKEY_MOD_L_SFT,HIDKEY____9_L_PA },//7 9
				{ HIDKEY_MOD__NONE,HIDKEY_PERIOD_GT },//8 :
				{ HIDKEY_MOD__NONE,HIDKEY_COMA___LT },//9 ;
				// 6 ==================
				{ HIDKEY_MOD__NONE,0x64 },//0 <
				{ HIDKEY_MOD__NONE,HIDKEY_EQU__PLUS },//1 =
				{ HIDKEY_MOD_L_SFT,0x64 },//2 >
				{ HIDKEY_MOD_L_SFT,HIDKEY____m____M },//3 ?
				{ HIDKEY_MOD_R_ALT,HIDKEY____0_R_PA },//4 @
				{ HIDKEY_MOD_L_SFT,HIDKEY____q____Q },//5 A
				{ HIDKEY_MOD_L_SFT,HIDKEY____b____B },//6 B
				{ HIDKEY_MOD_L_SFT,HIDKEY____c____C },//7 C
				{ HIDKEY_MOD_L_SFT,HIDKEY____d____D },//8 D
				{ HIDKEY_MOD_L_SFT,HIDKEY____e____E },//9 E
				// 7 ==================
				{ HIDKEY_MOD_L_SFT,HIDKEY____f____F },//0 F
				{ HIDKEY_MOD_L_SFT,HIDKEY____g____G },//1 G
				{ HIDKEY_MOD_L_SFT,HIDKEY____h____H },//2 H
				{ HIDKEY_MOD_L_SFT,HIDKEY____i____I },//3 I
				{ HIDKEY_MOD_L_SFT,HIDKEY____j____J },//4 J
				{ HIDKEY_MOD_L_SFT,HIDKEY____k____K },//5 K
				{ HIDKEY_MOD_L_SFT,HIDKEY____l____L },//6 L
				{ HIDKEY_MOD_L_SFT,HIDKEY_SEMI__COL },//7 M
				{ HIDKEY_MOD_L_SFT,HIDKEY____n____N },//8 N
				{ HIDKEY_MOD_L_SFT,HIDKEY____o____O },//9 O
				// 8 ==================
				{ HIDKEY_MOD_L_SFT,HIDKEY____p____P },//0 P
				{ HIDKEY_MOD_L_SFT,HIDKEY____a____A },//1 Q
				{ HIDKEY_MOD_L_SFT,HIDKEY____r____R },//2 R
				{ HIDKEY_MOD_L_SFT,HIDKEY____s____S },//3 S
				{ HIDKEY_MOD_L_SFT,HIDKEY____t____T },//4 T
				{ HIDKEY_MOD_L_SFT,HIDKEY____u____U },//5 U
				{ HIDKEY_MOD_L_SFT,HIDKEY____v____V },//6 V
				{ HIDKEY_MOD_L_SFT,HIDKEY____z____Z },//7 W
				{ HIDKEY_MOD_L_SFT,HIDKEY____x____X },//8 X
				{ HIDKEY_MOD_L_SFT,HIDKEY____y____Y },//9 Y
				// 9 ==================
				{ HIDKEY_MOD_L_SFT,HIDKEY____w____W },//0 Z
				{ HIDKEY_MOD_R_ALT,HIDKEY____5_PERC },//1 [
				{ HIDKEY_MOD_R_ALT,HIDKEY____8_ASTE },//2
				{ HIDKEY_MOD_R_ALT,HIDKEY_MIN_UNDER },//3 ]
				{ HIDKEY_MOD_R_ALT,HIDKEY____9_L_PA },//4 ^
				{ HIDKEY_MOD__NONE,HIDKEY____8_ASTE },//5 _
				{ HIDKEY_MOD__NONE,HIDKEY____8_ASTE },//6 `
				{ HIDKEY_MOD__NONE,HIDKEY____a____A },//7 a
				{ HIDKEY_MOD__NONE,HIDKEY____b____B },//8 b
				{ HIDKEY_MOD__NONE,HIDKEY____c____C },//9 c
				// 10 ==================
				{ HIDKEY_MOD__NONE,HIDKEY____d____D },//0 d
				{ HIDKEY_MOD__NONE,HIDKEY____e____E },//1 e
				{ HIDKEY_MOD__NONE,HIDKEY____f____F },//2 f
				{ HIDKEY_MOD__NONE,HIDKEY____g____G },//3 g
				{ HIDKEY_MOD__NONE,HIDKEY____h____H },//4 h
				{ HIDKEY_MOD__NONE,HIDKEY____i____I },//5 i
				{ HIDKEY_MOD__NONE,HIDKEY____j____J },//6 j
				{ HIDKEY_MOD__NONE,HIDKEY____k____K },//7 k
				{ HIDKEY_MOD__NONE,HIDKEY____l____L },//8 l
				{ HIDKEY_MOD__NONE,HIDKEY____m____M },//9 m
				// 11 ==================
				{ HIDKEY_MOD__NONE,HIDKEY____n____N },//0 n
				{ HIDKEY_MOD__NONE,HIDKEY____o____O },//1 o
				{ HIDKEY_MOD__NONE,HIDKEY____p____P },//2 p
				{ HIDKEY_MOD__NONE,HIDKEY____q____Q },//3 q
				{ HIDKEY_MOD__NONE,HIDKEY____r____R },//4 r
				{ HIDKEY_MOD__NONE,HIDKEY____s____S },//5 s
				{ HIDKEY_MOD__NONE,HIDKEY____t____T },//6 t
				{ HIDKEY_MOD__NONE,HIDKEY____u____U },//7 u
				{ HIDKEY_MOD__NONE,HIDKEY____v____V },//8 v
				{ HIDKEY_MOD__NONE,HIDKEY____w____W },//9 w
				// 12 ==================
				{ HIDKEY_MOD__NONE,HIDKEY____x____X },//0 x
				{ HIDKEY_MOD__NONE,HIDKEY____y____Y },//1 y
				{ HIDKEY_MOD__NONE,HIDKEY____z____Z },//2 z
				{ HIDKEY_MOD_L_SFT,HIDKEY_LBT___LBR },//3 {
				{ HIDKEY_MOD_L_SFT,HIDKEY_BSLA_VBAR },//4 |
				{ HIDKEY_MOD_L_SFT,HIDKEY_RBT___RBR },//5 }
				{ HIDKEY_MOD_L_SFT,HIDKEY_GRAV_TILD },//6 ~
				{ 0,0 },//7
				{ 0,0 },//8
				{ 0,0 }//9
			},
			{	//xxx.German Table......
				//{ Modified key,HID key }
				// 0 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ 0,0 },//2
				{ 0,0 },//3
				{ 0,0 },//4
				{ 0,0 },//5
				{ 0,0 },//6
				{ 0,0 },//7
				{ HIDKEY_MOD__NONE,HIDKEY_BACKSPACE },//8
				{ HIDKEY_MOD__NONE,HIDKEY_______TAB },//9
				// 1 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ 0,0 },//2
				{ HIDKEY_MOD__NONE,HIDKEY____RETURN },//3 ... CR
				{ 0,0 },//4
				{ 0,0 },//5
				{ 0,0 },//6
				{ 0,0 },//7
				{ 0,0 },//8
				{ 0,0 },//9
				// 2 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ 0,0 },//2
				{ 0,0 },//3
				{ 0,0 },//4
				{ 0,0 },//5
				{ 0,0 },//6
				{ HIDKEY_MOD__NONE,HIDKEY____ESCAPE },//7
				{ 0,0 },//8
				{ 0,0 },//9
				// 3 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ HIDKEY_MOD__NONE,HIDKEY_____SPACE },//2
				{ HIDKEY_MOD_L_SFT,HIDKEY____1_EXCL },//3 !
				{ HIDKEY_MOD_L_SFT,HIDKEY____2_QUOT },//4 "
				{ HIDKEY_MOD__NONE,0x32 },//5 #
				{ HIDKEY_MOD_L_SFT,HIDKEY____4_DOLL },//6 $
				{ HIDKEY_MOD_L_SFT,HIDKEY____5_PERC },//7 %
				{ HIDKEY_MOD_L_SFT,HIDKEY____6_CIRC },//8 &
				{ HIDKEY_MOD_L_SFT,0x32 },//9 '
				// 4 ==================
				{ HIDKEY_MOD_L_SFT,HIDKEY____8_ASTE },//0 (
				{ HIDKEY_MOD_L_SFT,HIDKEY____9_L_PA },//1 )
				{ HIDKEY_MOD_L_SFT,HIDKEY_RBT___RBR },//2 *
				{ HIDKEY_MOD__NONE,HIDKEY_RBT___RBR },//3 +
				{ HIDKEY_MOD__NONE,HIDKEY_COMA___LT },//4 ,
				{ HIDKEY_MOD__NONE,HIDKEY_SLASH__QM },//5 -
				{ HIDKEY_MOD__NONE,HIDKEY_PERIOD_GT },//6 .
				{ HIDKEY_MOD_L_SFT,HIDKEY____7_AMPE },//7 /
				{ HIDKEY_MOD__NONE,HIDKEY____0_R_PA },//8 0
				{ HIDKEY_MOD__NONE,HIDKEY____1_EXCL },//9 1
				// 5 ==================
				{ HIDKEY_MOD__NONE,HIDKEY____2_QUOT },//0 2
				{ HIDKEY_MOD__NONE,HIDKEY____3_SHAR },//1 3
				{ HIDKEY_MOD__NONE,HIDKEY____4_DOLL },//2 4
				{ HIDKEY_MOD__NONE,HIDKEY____5_PERC },//3 5
				{ HIDKEY_MOD__NONE,HIDKEY____6_CIRC },//4 6
				{ HIDKEY_MOD__NONE,HIDKEY____7_AMPE },//5 7
				{ HIDKEY_MOD__NONE,HIDKEY____8_ASTE },//6 8
				{ HIDKEY_MOD__NONE,HIDKEY____9_L_PA },//7 9
				{ HIDKEY_MOD_L_SFT,HIDKEY_PERIOD_GT },//8 :
				{ HIDKEY_MOD_L_SFT,HIDKEY_COMA___LT },//9 ;
				// 6 ==================
				{ HIDKEY_MOD__NONE,0x64 },//0 <
				{ HIDKEY_MOD_L_SFT,HIDKEY____0_R_PA },//1 =
				{ HIDKEY_MOD_L_SFT,0x64 },//2 >
				{ HIDKEY_MOD_L_SFT,HIDKEY_MIN_UNDER },//3 ?
				{ HIDKEY_MOD_R_ALT,HIDKEY____q____Q },//4 @
				{ HIDKEY_MOD_L_SFT,HIDKEY____a____A },//5 A
				{ HIDKEY_MOD_L_SFT,HIDKEY____b____B },//6 B
				{ HIDKEY_MOD_L_SFT,HIDKEY____c____C },//7 C
				{ HIDKEY_MOD_L_SFT,HIDKEY____d____D },//8 D
				{ HIDKEY_MOD_L_SFT,HIDKEY____e____E },//9 E
				// 7 ==================
				{ HIDKEY_MOD_L_SFT,HIDKEY____f____F },//0 F
				{ HIDKEY_MOD_L_SFT,HIDKEY____g____G },//1 G
				{ HIDKEY_MOD_L_SFT,HIDKEY____h____H },//2 H
				{ HIDKEY_MOD_L_SFT,HIDKEY____i____I },//3 I
				{ HIDKEY_MOD_L_SFT,HIDKEY____j____J },//4 J
				{ HIDKEY_MOD_L_SFT,HIDKEY____k____K },//5 K
				{ HIDKEY_MOD_L_SFT,HIDKEY____l____L },//6 L
				{ HIDKEY_MOD_L_SFT,HIDKEY____m____M },//7 M
				{ HIDKEY_MOD_L_SFT,HIDKEY____n____N },//8 N
				{ HIDKEY_MOD_L_SFT,HIDKEY____o____O },//9 O
				// 8 ==================
				{ HIDKEY_MOD_L_SFT,HIDKEY____p____P },//0 P
				{ HIDKEY_MOD_L_SFT,HIDKEY____q____Q },//1 Q
				{ HIDKEY_MOD_L_SFT,HIDKEY____r____R },//2 R
				{ HIDKEY_MOD_L_SFT,HIDKEY____s____S },//3 S
				{ HIDKEY_MOD_L_SFT,HIDKEY____t____T },//4 T
				{ HIDKEY_MOD_L_SFT,HIDKEY____u____U },//5 U
				{ HIDKEY_MOD_L_SFT,HIDKEY____v____V },//6 V
				{ HIDKEY_MOD_L_SFT,HIDKEY____w____W },//7 W
				{ HIDKEY_MOD_L_SFT,HIDKEY____x____X },//8 X
				{ HIDKEY_MOD_L_SFT,HIDKEY____z____Z },//9 Y
				// 9 ==================
				{ HIDKEY_MOD_L_SFT,HIDKEY____y____Y },//0 Z
				{ HIDKEY_MOD_R_ALT,HIDKEY____8_ASTE },//1 [
				{ HIDKEY_MOD_R_ALT,HIDKEY_MIN_UNDER },//2
				{ HIDKEY_MOD_R_ALT,HIDKEY____9_L_PA },//3 ]
				{ HIDKEY_MOD__NONE,HIDKEY_GRAV_TILD },//4 ^
				{ HIDKEY_MOD_L_SFT,HIDKEY_SLASH__QM },//5 _
				{ HIDKEY_MOD__NONE,HIDKEY_GRAV_TILD },//6 `
				{ HIDKEY_MOD__NONE,HIDKEY____a____A },//7 a
				{ HIDKEY_MOD__NONE,HIDKEY____b____B },//8 b
				{ HIDKEY_MOD__NONE,HIDKEY____c____C },//9 c
				// 10 ==================
				{ HIDKEY_MOD__NONE,HIDKEY____d____D },//0 d
				{ HIDKEY_MOD__NONE,HIDKEY____e____E },//1 e
				{ HIDKEY_MOD__NONE,HIDKEY____f____F },//2 f
				{ HIDKEY_MOD__NONE,HIDKEY____g____G },//3 g
				{ HIDKEY_MOD__NONE,HIDKEY____h____H },//4 h
				{ HIDKEY_MOD__NONE,HIDKEY____i____I },//5 i
				{ HIDKEY_MOD__NONE,HIDKEY____j____J },//6 j
				{ HIDKEY_MOD__NONE,HIDKEY____k____K },//7 k
				{ HIDKEY_MOD__NONE,HIDKEY____l____L },//8 l
				{ HIDKEY_MOD__NONE,HIDKEY____m____M },//9 m
				// 11 ==================
				{ HIDKEY_MOD__NONE,HIDKEY____n____N },//0 n
				{ HIDKEY_MOD__NONE,HIDKEY____o____O },//1 o
				{ HIDKEY_MOD__NONE,HIDKEY____p____P },//2 p
				{ HIDKEY_MOD__NONE,HIDKEY____q____Q },//3 q
				{ HIDKEY_MOD__NONE,HIDKEY____r____R },//4 r
				{ HIDKEY_MOD__NONE,HIDKEY____s____S },//5 s
				{ HIDKEY_MOD__NONE,HIDKEY____t____T },//6 t
				{ HIDKEY_MOD__NONE,HIDKEY____u____U },//7 u
				{ HIDKEY_MOD__NONE,HIDKEY____v____V },//8 v
				{ HIDKEY_MOD__NONE,HIDKEY____w____W },//9 w
				// 12 ==================
				{ HIDKEY_MOD__NONE,HIDKEY____x____X },//0 x
				{ HIDKEY_MOD__NONE,HIDKEY____y____Y },//1 y
				{ HIDKEY_MOD__NONE,HIDKEY____z____Z },//2 z
				{ HIDKEY_MOD_L_SFT,HIDKEY_LBT___LBR },//3 {
				{ HIDKEY_MOD_L_SFT,HIDKEY_BSLA_VBAR },//4 |
				{ HIDKEY_MOD_L_SFT,HIDKEY_RBT___RBR },//5 }
				{ HIDKEY_MOD_L_SFT,HIDKEY_GRAV_TILD },//6 ~
				{ 0,0 },//7
				{ 0,0 },//8
				{ 0,0 }//9
			},
			{	//xxx.Italian Table......
				//{ Modified key,HID key }
				// 0 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ 0,0 },//2
				{ 0,0 },//3
				{ 0,0 },//4
				{ 0,0 },//5
				{ 0,0 },//6
				{ 0,0 },//7
				{ HIDKEY_MOD__NONE,HIDKEY_BACKSPACE },//8
				{ HIDKEY_MOD__NONE,HIDKEY_______TAB },//9
				// 1 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ 0,0 },//2
				{ HIDKEY_MOD__NONE,HIDKEY____RETURN },//3 ... CR
				{ 0,0 },//4
				{ 0,0 },//5
				{ 0,0 },//6
				{ 0,0 },//7
				{ 0,0 },//8
				{ 0,0 },//9
				// 2 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ 0,0 },//2
				{ 0,0 },//3
				{ 0,0 },//4
				{ 0,0 },//5
				{ 0,0 },//6
				{ HIDKEY_MOD__NONE,HIDKEY____ESCAPE },//7
				{ 0,0 },//8
				{ 0,0 },//9
				// 3 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ HIDKEY_MOD__NONE,HIDKEY_____SPACE },//2
				{ HIDKEY_MOD_L_SFT,HIDKEY____1_EXCL },//3 !
				{ HIDKEY_MOD_L_SFT,HIDKEY____2_QUOT },//4 "
				{ HIDKEY_MOD_R_ALT,HIDKEY_APOS_QUOT },//5 #
				{ HIDKEY_MOD_L_SFT,HIDKEY____4_DOLL },//6 $
				{ HIDKEY_MOD_L_SFT,HIDKEY____5_PERC },//7 %
				{ HIDKEY_MOD_L_SFT,HIDKEY____6_CIRC },//8 &
				{ HIDKEY_MOD__NONE,HIDKEY_MIN_UNDER },//9 '
				// 4 ==================
				{ HIDKEY_MOD_L_SFT,HIDKEY____8_ASTE },//0 (
				{ HIDKEY_MOD_L_SFT,HIDKEY____9_L_PA },//1 )
				{ HIDKEY_MOD_L_SFT,HIDKEY_RBT___RBR },//2 *
				{ HIDKEY_MOD__NONE,HIDKEY_RBT___RBR },//3 +
				{ HIDKEY_MOD__NONE,HIDKEY_COMA___LT },//4 ,
				{ HIDKEY_MOD__NONE,HIDKEY_SLASH__QM },//5 -
				{ HIDKEY_MOD__NONE,HIDKEY_PERIOD_GT },//6 .
				{ HIDKEY_MOD_L_SFT,HIDKEY____7_AMPE },//7 /
				{ HIDKEY_MOD__NONE,HIDKEY____0_R_PA },//8 0
				{ HIDKEY_MOD__NONE,HIDKEY____1_EXCL },//9 1
				// 5 ==================
				{ HIDKEY_MOD__NONE,HIDKEY____2_QUOT },//0 2
				{ HIDKEY_MOD__NONE,HIDKEY____3_SHAR },//1 3
				{ HIDKEY_MOD__NONE,HIDKEY____4_DOLL },//2 4
				{ HIDKEY_MOD__NONE,HIDKEY____5_PERC },//3 5
				{ HIDKEY_MOD__NONE,HIDKEY____6_CIRC },//4 6
				{ HIDKEY_MOD__NONE,HIDKEY____7_AMPE },//5 7
				{ HIDKEY_MOD__NONE,HIDKEY____8_ASTE },//6 8
				{ HIDKEY_MOD__NONE,HIDKEY____9_L_PA },//7 9
				{ HIDKEY_MOD_L_SFT,HIDKEY_PERIOD_GT },//8 :
				{ HIDKEY_MOD_L_SFT,HIDKEY_COMA___LT },//9 ;
				// 6 ==================
				{ HIDKEY_MOD__NONE,0x64 },//0 <
				{ HIDKEY_MOD_L_SFT,HIDKEY____0_R_PA },//1 =
				{ HIDKEY_MOD_L_SFT,0x64 },//2 >
				{ HIDKEY_MOD_L_SFT,HIDKEY_MIN_UNDER },//3 ?
				{ HIDKEY_MOD_R_ALT,HIDKEY_SEMI__COL },//4 @
				{ HIDKEY_MOD_L_SFT,HIDKEY____a____A },//5 A
				{ HIDKEY_MOD_L_SFT,HIDKEY____b____B },//6 B
				{ HIDKEY_MOD_L_SFT,HIDKEY____c____C },//7 C
				{ HIDKEY_MOD_L_SFT,HIDKEY____d____D },//8 D
				{ HIDKEY_MOD_L_SFT,HIDKEY____e____E },//9 E
				// 7 ==================
				{ HIDKEY_MOD_L_SFT,HIDKEY____f____F },//0 F
				{ HIDKEY_MOD_L_SFT,HIDKEY____g____G },//1 G
				{ HIDKEY_MOD_L_SFT,HIDKEY____h____H },//2 H
				{ HIDKEY_MOD_L_SFT,HIDKEY____i____I },//3 I
				{ HIDKEY_MOD_L_SFT,HIDKEY____j____J },//4 J
				{ HIDKEY_MOD_L_SFT,HIDKEY____k____K },//5 K
				{ HIDKEY_MOD_L_SFT,HIDKEY____l____L },//6 L
				{ HIDKEY_MOD_L_SFT,HIDKEY____m____M },//7 M
				{ HIDKEY_MOD_L_SFT,HIDKEY____n____N },//8 N
				{ HIDKEY_MOD_L_SFT,HIDKEY____o____O },//9 O
				// 8 ==================
				{ HIDKEY_MOD_L_SFT,HIDKEY____p____P },//0 P
				{ HIDKEY_MOD_L_SFT,HIDKEY____q____Q },//1 Q
				{ HIDKEY_MOD_L_SFT,HIDKEY____r____R },//2 R
				{ HIDKEY_MOD_L_SFT,HIDKEY____s____S },//3 S
				{ HIDKEY_MOD_L_SFT,HIDKEY____t____T },//4 T
				{ HIDKEY_MOD_L_SFT,HIDKEY____u____U },//5 U
				{ HIDKEY_MOD_L_SFT,HIDKEY____v____V },//6 V
				{ HIDKEY_MOD_L_SFT,HIDKEY____w____W },//7 W
				{ HIDKEY_MOD_L_SFT,HIDKEY____x____X },//8 X
				{ HIDKEY_MOD_L_SFT,HIDKEY____y____Y },//9 Y
				// 9 ==================
				{ HIDKEY_MOD_L_SFT,HIDKEY____z____Z },//0 Z
				{ HIDKEY_MOD_R_ALT,HIDKEY_LBT___LBR },//1 [
				{ HIDKEY_MOD__NONE,HIDKEY_GRAV_TILD },//2
				{ HIDKEY_MOD_R_ALT,HIDKEY_RBT___RBR },//3 ]
				{ HIDKEY_MOD_L_SFT,HIDKEY_EQU__PLUS },//4 ^
				{ HIDKEY_MOD_L_SFT,HIDKEY_SLASH__QM },//5 _
				{ HIDKEY_MOD__NONE,HIDKEY_GRAV_TILD },//6 `
				{ HIDKEY_MOD__NONE,HIDKEY____a____A },//7 a
				{ HIDKEY_MOD__NONE,HIDKEY____b____B },//8 b
				{ HIDKEY_MOD__NONE,HIDKEY____c____C },//9 c
				// 10 ==================
				{ HIDKEY_MOD__NONE,HIDKEY____d____D },//0 d
				{ HIDKEY_MOD__NONE,HIDKEY____e____E },//1 e
				{ HIDKEY_MOD__NONE,HIDKEY____f____F },//2 f
				{ HIDKEY_MOD__NONE,HIDKEY____g____G },//3 g
				{ HIDKEY_MOD__NONE,HIDKEY____h____H },//4 h
				{ HIDKEY_MOD__NONE,HIDKEY____i____I },//5 i
				{ HIDKEY_MOD__NONE,HIDKEY____j____J },//6 j
				{ HIDKEY_MOD__NONE,HIDKEY____k____K },//7 k
				{ HIDKEY_MOD__NONE,HIDKEY____l____L },//8 l
				{ HIDKEY_MOD__NONE,HIDKEY____m____M },//9 m
				// 11 ==================
				{ HIDKEY_MOD__NONE,HIDKEY____n____N },//0 n
				{ HIDKEY_MOD__NONE,HIDKEY____o____O },//1 o
				{ HIDKEY_MOD__NONE,HIDKEY____p____P },//2 p
				{ HIDKEY_MOD__NONE,HIDKEY____q____Q },//3 q
				{ HIDKEY_MOD__NONE,HIDKEY____r____R },//4 r
				{ HIDKEY_MOD__NONE,HIDKEY____s____S },//5 s
				{ HIDKEY_MOD__NONE,HIDKEY____t____T },//6 t
				{ HIDKEY_MOD__NONE,HIDKEY____u____U },//7 u
				{ HIDKEY_MOD__NONE,HIDKEY____v____V },//8 v
				{ HIDKEY_MOD__NONE,HIDKEY____w____W },//9 w
				// 12 ==================
				{ HIDKEY_MOD__NONE,HIDKEY____x____X },//0 x
				{ HIDKEY_MOD__NONE,HIDKEY____y____Y },//1 y
				{ HIDKEY_MOD__NONE,HIDKEY____z____Z },//2 z
				{ HIDKEY_MOD_L_SFT,HIDKEY_LBT___LBR },//3 {
				{ HIDKEY_MOD_L_SFT,HIDKEY_BSLA_VBAR },//4 |
				{ HIDKEY_MOD_L_SFT,HIDKEY_RBT___RBR },//5 }
				{ HIDKEY_MOD_L_SFT,HIDKEY_GRAV_TILD },//6 ~
				{ 0,0 },//7
				{ 0,0 },//8
				{ 0,0 }//9
			},
			{	//xxx.Norwegian	Table......
				//{ Modified key,HID key }
				// 0 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ 0,0 },//2
				{ 0,0 },//3
				{ 0,0 },//4
				{ 0,0 },//5
				{ 0,0 },//6
				{ 0,0 },//7
				{ HIDKEY_MOD__NONE,HIDKEY_BACKSPACE },//8
				{ HIDKEY_MOD__NONE,HIDKEY_______TAB },//9
				// 1 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ 0,0 },//2
				{ HIDKEY_MOD__NONE,HIDKEY____RETURN },//3 ... CR
				{ 0,0 },//4
				{ 0,0 },//5
				{ 0,0 },//6
				{ 0,0 },//7
				{ 0,0 },//8
				{ 0,0 },//9
				// 2 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ 0,0 },//2
				{ 0,0 },//3
				{ 0,0 },//4
				{ 0,0 },//5
				{ 0,0 },//6
				{ HIDKEY_MOD__NONE,HIDKEY____ESCAPE },//7
				{ 0,0 },//8
				{ 0,0 },//9
				// 3 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ HIDKEY_MOD__NONE,HIDKEY_____SPACE },//2
				{ HIDKEY_MOD_L_SFT,HIDKEY____1_EXCL },//3 !
				{ HIDKEY_MOD_L_SFT,HIDKEY____2_QUOT },//4 "
				{ HIDKEY_MOD_L_SFT,HIDKEY____3_SHAR },//5 #
				{ HIDKEY_MOD_R_ALT,HIDKEY____4_DOLL },//6 $
				{ HIDKEY_MOD_L_SFT,HIDKEY____5_PERC },//7 %
				{ HIDKEY_MOD_L_SFT,HIDKEY____6_CIRC },//8 &
				{ HIDKEY_MOD__NONE,0x32 },//9 '
				// 4 ==================
				{ HIDKEY_MOD_L_SFT,HIDKEY____8_ASTE },//0 (
				{ HIDKEY_MOD_L_SFT,HIDKEY____9_L_PA },//1 )
				{ HIDKEY_MOD_L_SFT,0x32 },//2 *
				{ HIDKEY_MOD__NONE,HIDKEY_MIN_UNDER },//3 +
				{ HIDKEY_MOD__NONE,HIDKEY_COMA___LT },//4 ,
				{ HIDKEY_MOD__NONE,HIDKEY_SLASH__QM },//5 -
				{ HIDKEY_MOD__NONE,HIDKEY_PERIOD_GT },//6 .
				{ HIDKEY_MOD_L_SFT,HIDKEY____7_AMPE },//7 /
				{ HIDKEY_MOD__NONE,HIDKEY____0_R_PA },//8 0
				{ HIDKEY_MOD__NONE,HIDKEY____1_EXCL },//9 1
				// 5 ==================
				{ HIDKEY_MOD__NONE,HIDKEY____2_QUOT },//0 2
				{ HIDKEY_MOD__NONE,HIDKEY____3_SHAR },//1 3
				{ HIDKEY_MOD__NONE,HIDKEY____4_DOLL },//2 4
				{ HIDKEY_MOD__NONE,HIDKEY____5_PERC },//3 5
				{ HIDKEY_MOD__NONE,HIDKEY____6_CIRC },//4 6
				{ HIDKEY_MOD__NONE,HIDKEY____7_AMPE },//5 7
				{ HIDKEY_MOD__NONE,HIDKEY____8_ASTE },//6 8
				{ HIDKEY_MOD__NONE,HIDKEY____9_L_PA },//7 9
				{ HIDKEY_MOD_L_SFT,HIDKEY_PERIOD_GT },//8 :
				{ HIDKEY_MOD_L_SFT,HIDKEY_COMA___LT },//9 ;
				// 6 ==================
				{ HIDKEY_MOD__NONE,0x64 },//0 <
				{ HIDKEY_MOD_L_SFT,HIDKEY____0_R_PA },//1 =
				{ HIDKEY_MOD_L_SFT,0x64 },//2 >
				{ HIDKEY_MOD_L_SFT,HIDKEY_MIN_UNDER },//3 ?
				{ HIDKEY_MOD_R_ALT,HIDKEY____2_QUOT },//4 @
				{ HIDKEY_MOD_L_SFT,HIDKEY____a____A },//5 A
				{ HIDKEY_MOD_L_SFT,HIDKEY____b____B },//6 B
				{ HIDKEY_MOD_L_SFT,HIDKEY____c____C },//7 C
				{ HIDKEY_MOD_L_SFT,HIDKEY____d____D },//8 D
				{ HIDKEY_MOD_L_SFT,HIDKEY____e____E },//9 E
				// 7 ==================
				{ HIDKEY_MOD_L_SFT,HIDKEY____f____F },//0 F
				{ HIDKEY_MOD_L_SFT,HIDKEY____g____G },//1 G
				{ HIDKEY_MOD_L_SFT,HIDKEY____h____H },//2 H
				{ HIDKEY_MOD_L_SFT,HIDKEY____i____I },//3 I
				{ HIDKEY_MOD_L_SFT,HIDKEY____j____J },//4 J
				{ HIDKEY_MOD_L_SFT,HIDKEY____k____K },//5 K
				{ HIDKEY_MOD_L_SFT,HIDKEY____l____L },//6 L
				{ HIDKEY_MOD_L_SFT,HIDKEY____m____M },//7 M
				{ HIDKEY_MOD_L_SFT,HIDKEY____n____N },//8 N
				{ HIDKEY_MOD_L_SFT,HIDKEY____o____O },//9 O
				// 8 ==================
				{ HIDKEY_MOD_L_SFT,HIDKEY____p____P },//0 P
				{ HIDKEY_MOD_L_SFT,HIDKEY____q____Q },//1 Q
				{ HIDKEY_MOD_L_SFT,HIDKEY____r____R },//2 R
				{ HIDKEY_MOD_L_SFT,HIDKEY____s____S },//3 S
				{ HIDKEY_MOD_L_SFT,HIDKEY____t____T },//4 T
				{ HIDKEY_MOD_L_SFT,HIDKEY____u____U },//5 U
				{ HIDKEY_MOD_L_SFT,HIDKEY____v____V },//6 V
				{ HIDKEY_MOD_L_SFT,HIDKEY____w____W },//7 W
				{ HIDKEY_MOD_L_SFT,HIDKEY____x____X },//8 X
				{ HIDKEY_MOD_L_SFT,HIDKEY____y____Y },//9 Y
				// 9 ==================
				{ HIDKEY_MOD_L_SFT,HIDKEY____z____Z },//0 Z
				{ HIDKEY_MOD_R_ALT,HIDKEY____8_ASTE },//1 [
				{ HIDKEY_MOD__NONE,HIDKEY_EQU__PLUS },//2
				{ HIDKEY_MOD_R_ALT,HIDKEY____9_L_PA },//3 ]
				{ HIDKEY_MOD_L_SFT,HIDKEY_RBT___RBR },//4 ^
				{ HIDKEY_MOD_L_SFT,HIDKEY_SLASH__QM },//5 _
				{ HIDKEY_MOD__NONE,HIDKEY_GRAV_TILD },//6 `
				{ HIDKEY_MOD__NONE,HIDKEY____a____A },//7 a
				{ HIDKEY_MOD__NONE,HIDKEY____b____B },//8 b
				{ HIDKEY_MOD__NONE,HIDKEY____c____C },//9 c
				// 10 ==================
				{ HIDKEY_MOD__NONE,HIDKEY____d____D },//0 d
				{ HIDKEY_MOD__NONE,HIDKEY____e____E },//1 e
				{ HIDKEY_MOD__NONE,HIDKEY____f____F },//2 f
				{ HIDKEY_MOD__NONE,HIDKEY____g____G },//3 g
				{ HIDKEY_MOD__NONE,HIDKEY____h____H },//4 h
				{ HIDKEY_MOD__NONE,HIDKEY____i____I },//5 i
				{ HIDKEY_MOD__NONE,HIDKEY____j____J },//6 j
				{ HIDKEY_MOD__NONE,HIDKEY____k____K },//7 k
				{ HIDKEY_MOD__NONE,HIDKEY____l____L },//8 l
				{ HIDKEY_MOD__NONE,HIDKEY____m____M },//9 m
				// 11 ==================
				{ HIDKEY_MOD__NONE,HIDKEY____n____N },//0 n
				{ HIDKEY_MOD__NONE,HIDKEY____o____O },//1 o
				{ HIDKEY_MOD__NONE,HIDKEY____p____P },//2 p
				{ HIDKEY_MOD__NONE,HIDKEY____q____Q },//3 q
				{ HIDKEY_MOD__NONE,HIDKEY____r____R },//4 r
				{ HIDKEY_MOD__NONE,HIDKEY____s____S },//5 s
				{ HIDKEY_MOD__NONE,HIDKEY____t____T },//6 t
				{ HIDKEY_MOD__NONE,HIDKEY____u____U },//7 u
				{ HIDKEY_MOD__NONE,HIDKEY____v____V },//8 v
				{ HIDKEY_MOD__NONE,HIDKEY____w____W },//9 w
				// 12 ==================
				{ HIDKEY_MOD__NONE,HIDKEY____x____X },//0 x
				{ HIDKEY_MOD__NONE,HIDKEY____y____Y },//1 y
				{ HIDKEY_MOD__NONE,HIDKEY____z____Z },//2 z
				{ HIDKEY_MOD_L_SFT,HIDKEY_LBT___LBR },//3 {
				{ HIDKEY_MOD_L_SFT,HIDKEY_BSLA_VBAR },//4 |
				{ HIDKEY_MOD_L_SFT,HIDKEY_RBT___RBR },//5 }
				{ HIDKEY_MOD_L_SFT,HIDKEY_GRAV_TILD },//6 ~
				{ 0,0 },//7
				{ 0,0 },//8
				{ 0,0 }//9
			},
			{	//xxx.Swedish Table......
				//{ Modified key,HID key }
				// 0 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ 0,0 },//2
				{ 0,0 },//3
				{ 0,0 },//4
				{ 0,0 },//5
				{ 0,0 },//6
				{ 0,0 },//7
				{ HIDKEY_MOD__NONE,HIDKEY_BACKSPACE },//8
				{ HIDKEY_MOD__NONE,HIDKEY_______TAB },//9
				// 1 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ 0,0 },//2
				{ HIDKEY_MOD__NONE,HIDKEY____RETURN },//3 ... CR
				{ 0,0 },//4
				{ 0,0 },//5
				{ 0,0 },//6
				{ 0,0 },//7
				{ 0,0 },//8
				{ 0,0 },//9
				// 2 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ 0,0 },//2
				{ 0,0 },//3
				{ 0,0 },//4
				{ 0,0 },//5
				{ 0,0 },//6
				{ HIDKEY_MOD__NONE,HIDKEY____ESCAPE },//7
				{ 0,0 },//8
				{ 0,0 },//9
				// 3 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ HIDKEY_MOD__NONE,HIDKEY_____SPACE },//2
				{ HIDKEY_MOD_L_SFT,HIDKEY____1_EXCL },//3 !
				{ HIDKEY_MOD_L_SFT,HIDKEY____2_QUOT },//4 "
				{ HIDKEY_MOD_L_SFT,HIDKEY____3_SHAR },//5 #
				{ HIDKEY_MOD_R_ALT,HIDKEY____4_DOLL },//6 $
				{ HIDKEY_MOD_L_SFT,HIDKEY____5_PERC },//7 %
				{ HIDKEY_MOD_L_SFT,HIDKEY____6_CIRC },//8 &
				{ HIDKEY_MOD__NONE,0x32 },//9 '
				// 4 ==================
				{ HIDKEY_MOD_L_SFT,HIDKEY____8_ASTE },//0 (
				{ HIDKEY_MOD_L_SFT,HIDKEY____9_L_PA },//1 )
				{ HIDKEY_MOD_L_SFT,0x32 },//2 *
				{ HIDKEY_MOD__NONE,HIDKEY_MIN_UNDER },//3 +
				{ HIDKEY_MOD__NONE,HIDKEY_COMA___LT },//4 ,
				{ HIDKEY_MOD__NONE,HIDKEY_SLASH__QM },//5 -
				{ HIDKEY_MOD__NONE,HIDKEY_PERIOD_GT },//6 .
				{ HIDKEY_MOD_L_SFT,HIDKEY____7_AMPE },//7 /
				{ HIDKEY_MOD__NONE,HIDKEY____0_R_PA },//8 0
				{ HIDKEY_MOD__NONE,HIDKEY____1_EXCL },//9 1
				// 5 ==================
				{ HIDKEY_MOD__NONE,HIDKEY____2_QUOT },//0 2
				{ HIDKEY_MOD__NONE,HIDKEY____3_SHAR },//1 3
				{ HIDKEY_MOD__NONE,HIDKEY____4_DOLL },//2 4
				{ HIDKEY_MOD__NONE,HIDKEY____5_PERC },//3 5
				{ HIDKEY_MOD__NONE,HIDKEY____6_CIRC },//4 6
				{ HIDKEY_MOD__NONE,HIDKEY____7_AMPE },//5 7
				{ HIDKEY_MOD__NONE,HIDKEY____8_ASTE },//6 8
				{ HIDKEY_MOD__NONE,HIDKEY____9_L_PA },//7 9
				{ HIDKEY_MOD_L_SFT,HIDKEY_PERIOD_GT },//8 :
				{ HIDKEY_MOD_L_SFT,HIDKEY_COMA___LT },//9 ;
				// 6 ==================
				{ HIDKEY_MOD__NONE,0x64 },//0 <
				{ HIDKEY_MOD_L_SFT,HIDKEY____0_R_PA },//1 =
				{ HIDKEY_MOD_L_SFT,0x64 },//2 >
				{ HIDKEY_MOD_L_SFT,HIDKEY_MIN_UNDER },//3 ?
				{ HIDKEY_MOD_R_ALT,HIDKEY____2_QUOT },//4 @
				{ HIDKEY_MOD_L_SFT,HIDKEY____a____A },//5 A
				{ HIDKEY_MOD_L_SFT,HIDKEY____b____B },//6 B
				{ HIDKEY_MOD_L_SFT,HIDKEY____c____C },//7 C
				{ HIDKEY_MOD_L_SFT,HIDKEY____d____D },//8 D
				{ HIDKEY_MOD_L_SFT,HIDKEY____e____E },//9 E
				// 7 ==================
				{ HIDKEY_MOD_L_SFT,HIDKEY____f____F },//0 F
				{ HIDKEY_MOD_L_SFT,HIDKEY____g____G },//1 G
				{ HIDKEY_MOD_L_SFT,HIDKEY____h____H },//2 H
				{ HIDKEY_MOD_L_SFT,HIDKEY____i____I },//3 I
				{ HIDKEY_MOD_L_SFT,HIDKEY____j____J },//4 J
				{ HIDKEY_MOD_L_SFT,HIDKEY____k____K },//5 K
				{ HIDKEY_MOD_L_SFT,HIDKEY____l____L },//6 L
				{ HIDKEY_MOD_L_SFT,HIDKEY____m____M },//7 M
				{ HIDKEY_MOD_L_SFT,HIDKEY____n____N },//8 N
				{ HIDKEY_MOD_L_SFT,HIDKEY____o____O },//9 O
				// 8 ==================
				{ HIDKEY_MOD_L_SFT,HIDKEY____p____P },//0 P
				{ HIDKEY_MOD_L_SFT,HIDKEY____q____Q },//1 Q
				{ HIDKEY_MOD_L_SFT,HIDKEY____r____R },//2 R
				{ HIDKEY_MOD_L_SFT,HIDKEY____s____S },//3 S
				{ HIDKEY_MOD_L_SFT,HIDKEY____t____T },//4 T
				{ HIDKEY_MOD_L_SFT,HIDKEY____u____U },//5 U
				{ HIDKEY_MOD_L_SFT,HIDKEY____v____V },//6 V
				{ HIDKEY_MOD_L_SFT,HIDKEY____w____W },//7 W
				{ HIDKEY_MOD_L_SFT,HIDKEY____x____X },//8 X
				{ HIDKEY_MOD_L_SFT,HIDKEY____y____Y },//9 Y
				// 9 ==================
				{ HIDKEY_MOD_L_SFT,HIDKEY____z____Z },//0 Z
				{ HIDKEY_MOD_R_ALT,HIDKEY____8_ASTE },//1 [
				{ HIDKEY_MOD_R_ALT,HIDKEY_MIN_UNDER },//2
				{ HIDKEY_MOD_R_ALT,HIDKEY____9_L_PA },//3 ]
				{ HIDKEY_MOD_L_SFT,HIDKEY_RBT___RBR },//4 ^
				{ HIDKEY_MOD_L_SFT,HIDKEY_SLASH__QM },//5 _
				{ HIDKEY_MOD__NONE,HIDKEY_GRAV_TILD },//6 `
				{ HIDKEY_MOD__NONE,HIDKEY____a____A },//7 a
				{ HIDKEY_MOD__NONE,HIDKEY____b____B },//8 b
				{ HIDKEY_MOD__NONE,HIDKEY____c____C },//9 c
				// 10 ==================
				{ HIDKEY_MOD__NONE,HIDKEY____d____D },//0 d
				{ HIDKEY_MOD__NONE,HIDKEY____e____E },//1 e
				{ HIDKEY_MOD__NONE,HIDKEY____f____F },//2 f
				{ HIDKEY_MOD__NONE,HIDKEY____g____G },//3 g
				{ HIDKEY_MOD__NONE,HIDKEY____h____H },//4 h
				{ HIDKEY_MOD__NONE,HIDKEY____i____I },//5 i
				{ HIDKEY_MOD__NONE,HIDKEY____j____J },//6 j
				{ HIDKEY_MOD__NONE,HIDKEY____k____K },//7 k
				{ HIDKEY_MOD__NONE,HIDKEY____l____L },//8 l
				{ HIDKEY_MOD__NONE,HIDKEY____m____M },//9 m
				// 11 ==================
				{ HIDKEY_MOD__NONE,HIDKEY____n____N },//0 n
				{ HIDKEY_MOD__NONE,HIDKEY____o____O },//1 o
				{ HIDKEY_MOD__NONE,HIDKEY____p____P },//2 p
				{ HIDKEY_MOD__NONE,HIDKEY____q____Q },//3 q
				{ HIDKEY_MOD__NONE,HIDKEY____r____R },//4 r
				{ HIDKEY_MOD__NONE,HIDKEY____s____S },//5 s
				{ HIDKEY_MOD__NONE,HIDKEY____t____T },//6 t
				{ HIDKEY_MOD__NONE,HIDKEY____u____U },//7 u
				{ HIDKEY_MOD__NONE,HIDKEY____v____V },//8 v
				{ HIDKEY_MOD__NONE,HIDKEY____w____W },//9 w
				// 12 ==================
				{ HIDKEY_MOD__NONE,HIDKEY____x____X },//0 x
				{ HIDKEY_MOD__NONE,HIDKEY____y____Y },//1 y
				{ HIDKEY_MOD__NONE,HIDKEY____z____Z },//2 z
				{ HIDKEY_MOD_L_SFT,HIDKEY_LBT___LBR },//3 {
				{ HIDKEY_MOD_L_SFT,HIDKEY_BSLA_VBAR },//4 |
				{ HIDKEY_MOD_L_SFT,HIDKEY_RBT___RBR },//5 }
				{ HIDKEY_MOD_L_SFT,HIDKEY_GRAV_TILD },//6 ~
				{ 0,0 },//7
				{ 0,0 },//8
				{ 0,0 }//9
			},
			{	//xxx.UK_English Table......
				//{ Modified key,HID key }
				// 0 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ 0,0 },//2
				{ 0,0 },//3
				{ 0,0 },//4
				{ 0,0 },//5
				{ 0,0 },//6
				{ 0,0 },//7
				{ HIDKEY_MOD__NONE,HIDKEY_BACKSPACE },//8
				{ HIDKEY_MOD__NONE,HIDKEY_______TAB },//9
				// 1 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ 0,0 },//2
				{ HIDKEY_MOD__NONE,HIDKEY____RETURN },//3 ... CR
				{ 0,0 },//4
				{ 0,0 },//5
				{ 0,0 },//6
				{ 0,0 },//7
				{ 0,0 },//8
				{ 0,0 },//9
				// 2 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ 0,0 },//2
				{ 0,0 },//3
				{ 0,0 },//4
				{ 0,0 },//5
				{ 0,0 },//6
				{ HIDKEY_MOD__NONE,HIDKEY____ESCAPE },//7
				{ 0,0 },//8
				{ 0,0 },//9
				// 3 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ HIDKEY_MOD__NONE,HIDKEY_____SPACE },//2
				{ HIDKEY_MOD_L_SFT,HIDKEY____1_EXCL },//3 !
				{ HIDKEY_MOD_L_SFT,HIDKEY____2_QUOT },//4 "
				{ HIDKEY_MOD__NONE,0x32 },//5 #
				{ HIDKEY_MOD_L_SFT,HIDKEY____4_DOLL },//6 $
				{ HIDKEY_MOD_L_SFT,HIDKEY____5_PERC },//7 %
				{ HIDKEY_MOD_L_SFT,HIDKEY____7_AMPE },//8 &
				{ HIDKEY_MOD__NONE,HIDKEY_APOS_QUOT },//9 '
				// 4 ==================
				{ HIDKEY_MOD_L_SFT,HIDKEY____9_L_PA },//0 (
				{ HIDKEY_MOD_L_SFT,HIDKEY____0_R_PA },//1 )
				{ HIDKEY_MOD_L_SFT,HIDKEY____8_ASTE },//2 *
				{ HIDKEY_MOD_L_SFT,HIDKEY_EQU__PLUS },//3 +
				{ HIDKEY_MOD__NONE,HIDKEY_COMA___LT },//4 ,
				{ HIDKEY_MOD__NONE,HIDKEY_MIN_UNDER },//5 -
				{ HIDKEY_MOD__NONE,HIDKEY_PERIOD_GT },//6 .
				{ HIDKEY_MOD__NONE,HIDKEY_SLASH__QM },//7 /
				{ HIDKEY_MOD__NONE,HIDKEY____0_R_PA },//8 0
				{ HIDKEY_MOD__NONE,HIDKEY____1_EXCL },//9 1
				// 5 ==================
				{ HIDKEY_MOD__NONE,HIDKEY____2_QUOT },//0 2
				{ HIDKEY_MOD__NONE,HIDKEY____3_SHAR },//1 3
				{ HIDKEY_MOD__NONE,HIDKEY____4_DOLL },//2 4
				{ HIDKEY_MOD__NONE,HIDKEY____5_PERC },//3 5
				{ HIDKEY_MOD__NONE,HIDKEY____6_CIRC },//4 6
				{ HIDKEY_MOD__NONE,HIDKEY____7_AMPE },//5 7
				{ HIDKEY_MOD__NONE,HIDKEY____8_ASTE },//6 8
				{ HIDKEY_MOD__NONE,HIDKEY____9_L_PA },//7 9
				{ HIDKEY_MOD_L_SFT,HIDKEY_SEMI__COL },//8 :
				{ HIDKEY_MOD__NONE,HIDKEY_SEMI__COL },//9 ;
				// 6 ==================
				{ HIDKEY_MOD_L_SFT,HIDKEY_COMA___LT },//0 <
				{ HIDKEY_MOD__NONE,HIDKEY_EQU__PLUS },//1 =
				{ HIDKEY_MOD_L_SFT,HIDKEY_PERIOD_GT },//2 >
				{ HIDKEY_MOD_L_SFT,HIDKEY_SLASH__QM },//3 ?
				{ HIDKEY_MOD_L_SFT,HIDKEY_APOS_QUOT },//4 @
				{ HIDKEY_MOD_L_SFT,HIDKEY____a____A },//5 A
				{ HIDKEY_MOD_L_SFT,HIDKEY____b____B },//6 B
				{ HIDKEY_MOD_L_SFT,HIDKEY____c____C },//7 C
				{ HIDKEY_MOD_L_SFT,HIDKEY____d____D },//8 D
				{ HIDKEY_MOD_L_SFT,HIDKEY____e____E },//9 E
				// 7 ==================
				{ HIDKEY_MOD_L_SFT,HIDKEY____f____F },//0 F
				{ HIDKEY_MOD_L_SFT,HIDKEY____g____G },//1 G
				{ HIDKEY_MOD_L_SFT,HIDKEY____h____H },//2 H
				{ HIDKEY_MOD_L_SFT,HIDKEY____i____I },//3 I
				{ HIDKEY_MOD_L_SFT,HIDKEY____j____J },//4 J
				{ HIDKEY_MOD_L_SFT,HIDKEY____k____K },//5 K
				{ HIDKEY_MOD_L_SFT,HIDKEY____l____L },//6 L
				{ HIDKEY_MOD_L_SFT,HIDKEY____m____M },//7 M
				{ HIDKEY_MOD_L_SFT,HIDKEY____n____N },//8 N
				{ HIDKEY_MOD_L_SFT,HIDKEY____o____O },//9 O
				// 8 ==================
				{ HIDKEY_MOD_L_SFT,HIDKEY____p____P },//0 P
				{ HIDKEY_MOD_L_SFT,HIDKEY____q____Q },//1 Q
				{ HIDKEY_MOD_L_SFT,HIDKEY____r____R },//2 R
				{ HIDKEY_MOD_L_SFT,HIDKEY____s____S },//3 S
				{ HIDKEY_MOD_L_SFT,HIDKEY____t____T },//4 T
				{ HIDKEY_MOD_L_SFT,HIDKEY____u____U },//5 U
				{ HIDKEY_MOD_L_SFT,HIDKEY____v____V },//6 V
				{ HIDKEY_MOD_L_SFT,HIDKEY____w____W },//7 W
				{ HIDKEY_MOD_L_SFT,HIDKEY____x____X },//8 X
				{ HIDKEY_MOD_L_SFT,HIDKEY____y____Y },//9 Y
				// 9 ==================
				{ HIDKEY_MOD_L_SFT,HIDKEY____z____Z },//0 Z
				{ HIDKEY_MOD__NONE,HIDKEY_LBT___LBR },//1 [
				{ HIDKEY_MOD__NONE,0x64 },//2
				{ HIDKEY_MOD__NONE,HIDKEY_RBT___RBR },//3 ]
				{ HIDKEY_MOD_L_SFT,HIDKEY____6_CIRC },//4 ^
				{ HIDKEY_MOD_L_SFT,HIDKEY_MIN_UNDER },//5 _
				{ HIDKEY_MOD__NONE,HIDKEY_GRAV_TILD },//6 `
				{ HIDKEY_MOD__NONE,HIDKEY____a____A },//7 a
				{ HIDKEY_MOD__NONE,HIDKEY____b____B },//8 b
				{ HIDKEY_MOD__NONE,HIDKEY____c____C },//9 c
				// 10 ==================
				{ HIDKEY_MOD__NONE,HIDKEY____d____D },//0 d
				{ HIDKEY_MOD__NONE,HIDKEY____e____E },//1 e
				{ HIDKEY_MOD__NONE,HIDKEY____f____F },//2 f
				{ HIDKEY_MOD__NONE,HIDKEY____g____G },//3 g
				{ HIDKEY_MOD__NONE,HIDKEY____h____H },//4 h
				{ HIDKEY_MOD__NONE,HIDKEY____i____I },//5 i
				{ HIDKEY_MOD__NONE,HIDKEY____j____J },//6 j
				{ HIDKEY_MOD__NONE,HIDKEY____k____K },//7 k
				{ HIDKEY_MOD__NONE,HIDKEY____l____L },//8 l
				{ HIDKEY_MOD__NONE,HIDKEY____m____M },//9 m
				// 11 ==================
				{ HIDKEY_MOD__NONE,HIDKEY____n____N },//0 n
				{ HIDKEY_MOD__NONE,HIDKEY____o____O },//1 o
				{ HIDKEY_MOD__NONE,HIDKEY____p____P },//2 p
				{ HIDKEY_MOD__NONE,HIDKEY____q____Q },//3 q
				{ HIDKEY_MOD__NONE,HIDKEY____r____R },//4 r
				{ HIDKEY_MOD__NONE,HIDKEY____s____S },//5 s
				{ HIDKEY_MOD__NONE,HIDKEY____t____T },//6 t
				{ HIDKEY_MOD__NONE,HIDKEY____u____U },//7 u
				{ HIDKEY_MOD__NONE,HIDKEY____v____V },//8 v
				{ HIDKEY_MOD__NONE,HIDKEY____w____W },//9 w
				// 12 ==================
				{ HIDKEY_MOD__NONE,HIDKEY____x____X },//0 x
				{ HIDKEY_MOD__NONE,HIDKEY____y____Y },//1 y
				{ HIDKEY_MOD__NONE,HIDKEY____z____Z },//2 z
				{ HIDKEY_MOD_L_SFT,HIDKEY_LBT___LBR },//3 {
				{ HIDKEY_MOD_L_SFT,HIDKEY_BSLA_VBAR },//4 |
				{ HIDKEY_MOD_L_SFT,HIDKEY_RBT___RBR },//5 }
				{ HIDKEY_MOD_L_SFT,HIDKEY_GRAV_TILD },//6 ~
				{ 0,0 },//7
				{ 0,0 },//8
				{ 0,0 }//9
			},
			{	//xxx.Israel Table......
				//{ Modified key,HID key }
				// 0 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ 0,0 },//2
				{ 0,0 },//3
				{ 0,0 },//4
				{ 0,0 },//5
				{ 0,0 },//6
				{ 0,0 },//7
				{ HIDKEY_MOD__NONE,HIDKEY_BACKSPACE },//8
				{ HIDKEY_MOD__NONE,HIDKEY_______TAB },//9
				// 1 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ 0,0 },//2
				{ HIDKEY_MOD__NONE,HIDKEY____RETURN },//3 ... CR
				{ 0,0 },//4
				{ 0,0 },//5
				{ 0,0 },//6
				{ 0,0 },//7
				{ 0,0 },//8
				{ 0,0 },//9
				// 2 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ 0,0 },//2
				{ 0,0 },//3
				{ 0,0 },//4
				{ 0,0 },//5
				{ 0,0 },//6
				{ HIDKEY_MOD__NONE,HIDKEY____ESCAPE },//7
				{ 0,0 },//8
				{ 0,0 },//9
				// 3 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ HIDKEY_MOD__NONE,HIDKEY_____SPACE },//2
				{ HIDKEY_MOD_L_SFT,HIDKEY____1_EXCL },//3 !
				{ HIDKEY_MOD_L_SFT,HIDKEY_APOS_QUOT },//4 "
				{ HIDKEY_MOD_L_SFT,HIDKEY____3_SHAR },//5 #
				{ HIDKEY_MOD_L_SFT,HIDKEY____4_DOLL },//6 $
				{ HIDKEY_MOD_L_SFT,HIDKEY____5_PERC },//7 %
				{ HIDKEY_MOD_L_SFT,HIDKEY____7_AMPE },//8 &
				{ HIDKEY_MOD__NONE,HIDKEY____w____W },//9 '
				// 4 ==================
				{ HIDKEY_MOD_L_SFT,HIDKEY____0_R_PA },//0 (
				{ HIDKEY_MOD_L_SFT,HIDKEY____9_L_PA },//1 )
				{ HIDKEY_MOD_L_SFT,HIDKEY____8_ASTE },//2 *
				{ HIDKEY_MOD_L_SFT,HIDKEY_EQU__PLUS },//3 +
				{ HIDKEY_MOD__NONE,HIDKEY_APOS_QUOT },//4 ,
				{ HIDKEY_MOD__NONE,HIDKEY_MIN_UNDER },//5 -
				{ HIDKEY_MOD__NONE,HIDKEY_SLASH__QM },//6 .
				{ HIDKEY_MOD__NONE,HIDKEY____q____Q },//7 /
				{ HIDKEY_MOD__NONE,HIDKEY____0_R_PA },//8 0
				{ HIDKEY_MOD__NONE,HIDKEY____1_EXCL },//9 1
				// 5 ==================
				{ HIDKEY_MOD__NONE,HIDKEY____2_QUOT },//0 2
				{ HIDKEY_MOD__NONE,HIDKEY____3_SHAR },//1 3
				{ HIDKEY_MOD__NONE,HIDKEY____4_DOLL },//2 4
				{ HIDKEY_MOD__NONE,HIDKEY____5_PERC },//3 5
				{ HIDKEY_MOD__NONE,HIDKEY____6_CIRC },//4 6
				{ HIDKEY_MOD__NONE,HIDKEY____7_AMPE },//5 7
				{ HIDKEY_MOD__NONE,HIDKEY____8_ASTE },//6 8
				{ HIDKEY_MOD__NONE,HIDKEY____9_L_PA },//7 9
				{ HIDKEY_MOD_L_SFT,HIDKEY_SEMI__COL },//8 :
				{ HIDKEY_MOD__NONE,HIDKEY_GRAV_TILD },//9 ;
				// 6 ==================
				{ HIDKEY_MOD_L_SFT,HIDKEY_PERIOD_GT },//0 <
				{ HIDKEY_MOD__NONE,HIDKEY_EQU__PLUS },//1 =
				{ HIDKEY_MOD_L_SFT,HIDKEY_COMA___LT },//2 >
				{ HIDKEY_MOD_L_SFT,HIDKEY_SLASH__QM },//3 ?
				{ HIDKEY_MOD_L_SFT,HIDKEY____2_QUOT },//4 @
				{ HIDKEY_MOD_L_SFT,HIDKEY____a____A },//5 A
				{ HIDKEY_MOD_L_SFT,HIDKEY____b____B },//6 B
				{ HIDKEY_MOD_L_SFT,HIDKEY____c____C },//7 C
				{ HIDKEY_MOD_L_SFT,HIDKEY____d____D },//8 D
				{ HIDKEY_MOD_L_SFT,HIDKEY____e____E },//9 E
				// 7 ==================
				{ HIDKEY_MOD_L_SFT,HIDKEY____f____F },//0 F
				{ HIDKEY_MOD_L_SFT,HIDKEY____g____G },//1 G
				{ HIDKEY_MOD_L_SFT,HIDKEY____h____H },//2 H
				{ HIDKEY_MOD_L_SFT,HIDKEY____i____I },//3 I
				{ HIDKEY_MOD_L_SFT,HIDKEY____j____J },//4 J
				{ HIDKEY_MOD_L_SFT,HIDKEY____k____K },//5 K
				{ HIDKEY_MOD_L_SFT,HIDKEY____l____L },//6 L
				{ HIDKEY_MOD_L_SFT,HIDKEY____m____M },//7 M
				{ HIDKEY_MOD_L_SFT,HIDKEY____n____N },//8 N
				{ HIDKEY_MOD_L_SFT,HIDKEY____o____O },//9 O
				// 8 ==================
				{ HIDKEY_MOD_L_SFT,HIDKEY____p____P },//0 P
				{ HIDKEY_MOD_L_SFT,HIDKEY____q____Q },//1 Q
				{ HIDKEY_MOD_L_SFT,HIDKEY____r____R },//2 R
				{ HIDKEY_MOD_L_SFT,HIDKEY____s____S },//3 S
				{ HIDKEY_MOD_L_SFT,HIDKEY____t____T },//4 T
				{ HIDKEY_MOD_L_SFT,HIDKEY____u____U },//5 U
				{ HIDKEY_MOD_L_SFT,HIDKEY____v____V },//6 V
				{ HIDKEY_MOD_L_SFT,HIDKEY____w____W },//7 W
				{ HIDKEY_MOD_L_SFT,HIDKEY____x____X },//8 X
				{ HIDKEY_MOD_L_SFT,HIDKEY____y____Y },//9 Y
				// 9 ==================
				{ HIDKEY_MOD_L_SFT,HIDKEY____z____Z },//0 Z
				{ HIDKEY_MOD__NONE,HIDKEY_RBT___RBR },//1 [
				{ HIDKEY_MOD__NONE,0x64 },//2
				{ HIDKEY_MOD__NONE,HIDKEY_LBT___LBR },//3 ]
				{ HIDKEY_MOD_L_SFT,HIDKEY____6_CIRC },//4 ^
				{ HIDKEY_MOD_L_SFT,HIDKEY_MIN_UNDER },//5 _
				{ HIDKEY_MOD__NONE,HIDKEY_GRAV_TILD },//6 `
				{ HIDKEY_MOD__NONE,HIDKEY____a____A },//7 a
				{ HIDKEY_MOD__NONE,HIDKEY____b____B },//8 b
				{ HIDKEY_MOD__NONE,HIDKEY____c____C },//9 c
				// 10 ==================
				{ HIDKEY_MOD__NONE,HIDKEY____d____D },//0 d
				{ HIDKEY_MOD__NONE,HIDKEY____e____E },//1 e
				{ HIDKEY_MOD__NONE,HIDKEY____f____F },//2 f
				{ HIDKEY_MOD__NONE,HIDKEY____g____G },//3 g
				{ HIDKEY_MOD__NONE,HIDKEY____h____H },//4 h
				{ HIDKEY_MOD__NONE,HIDKEY____i____I },//5 i
				{ HIDKEY_MOD__NONE,HIDKEY____j____J },//6 j
				{ HIDKEY_MOD__NONE,HIDKEY____k____K },//7 k
				{ HIDKEY_MOD__NONE,HIDKEY____l____L },//8 l
				{ HIDKEY_MOD__NONE,HIDKEY____m____M },//9 m
				// 11 ==================
				{ HIDKEY_MOD__NONE,HIDKEY____n____N },//0 n
				{ HIDKEY_MOD__NONE,HIDKEY____o____O },//1 o
				{ HIDKEY_MOD__NONE,HIDKEY____p____P },//2 p
				{ HIDKEY_MOD__NONE,HIDKEY____q____Q },//3 q
				{ HIDKEY_MOD__NONE,HIDKEY____r____R },//4 r
				{ HIDKEY_MOD__NONE,HIDKEY____s____S },//5 s
				{ HIDKEY_MOD__NONE,HIDKEY____t____T },//6 t
				{ HIDKEY_MOD__NONE,HIDKEY____u____U },//7 u
				{ HIDKEY_MOD__NONE,HIDKEY____v____V },//8 v
				{ HIDKEY_MOD__NONE,HIDKEY____w____W },//9 w
				// 12 ==================
				{ HIDKEY_MOD__NONE,HIDKEY____x____X },//0 x
				{ HIDKEY_MOD__NONE,HIDKEY____y____Y },//1 y
				{ HIDKEY_MOD__NONE,HIDKEY____z____Z },//2 z
				{ HIDKEY_MOD_L_SFT,HIDKEY_LBT___LBR },//3 {
				{ HIDKEY_MOD_L_SFT,HIDKEY_BSLA_VBAR },//4 |
				{ HIDKEY_MOD_L_SFT,HIDKEY_RBT___RBR },//5 }
				{ HIDKEY_MOD_L_SFT,HIDKEY_GRAV_TILD },//6 ~
				{ 0,0 },//7
				{ 0,0 },//8
				{ 0,0 }//9
			},
			{	//xxx.Turkey Table......
				//{ Modified key,HID key }
				// 0 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ 0,0 },//2
				{ 0,0 },//3
				{ 0,0 },//4
				{ 0,0 },//5
				{ 0,0 },//6
				{ 0,0 },//7
				{ HIDKEY_MOD__NONE,HIDKEY_BACKSPACE },//8
				{ HIDKEY_MOD__NONE,HIDKEY_______TAB },//9
				// 1 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ 0,0 },//2
				{ HIDKEY_MOD__NONE,HIDKEY____RETURN },//3 ... CR
				{ 0,0 },//4
				{ 0,0 },//5
				{ 0,0 },//6
				{ 0,0 },//7
				{ 0,0 },//8
				{ 0,0 },//9
				// 2 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ 0,0 },//2
				{ 0,0 },//3
				{ 0,0 },//4
				{ 0,0 },//5
				{ 0,0 },//6
				{ HIDKEY_MOD__NONE,HIDKEY____ESCAPE },//7
				{ 0,0 },//8
				{ 0,0 },//9
				// 3 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ HIDKEY_MOD__NONE,HIDKEY_____SPACE },//2
				{ HIDKEY_MOD_L_SFT,HIDKEY____1_EXCL },//3 !
				{ HIDKEY_MOD__NONE,HIDKEY_GRAV_TILD },//4 "
				{ HIDKEY_MOD_R_ALT,HIDKEY____3_SHAR },//5 #
				{ HIDKEY_MOD_R_ALT,HIDKEY____4_DOLL },//6 $
				{ HIDKEY_MOD_L_SFT,HIDKEY____5_PERC },//7 %
				{ HIDKEY_MOD_L_SFT,HIDKEY____6_CIRC },//8 &
				{ HIDKEY_MOD_L_SFT,HIDKEY____2_QUOT },//9 '
				// 4 ==================
				{ HIDKEY_MOD_L_SFT,HIDKEY____8_ASTE },//0 (
				{ HIDKEY_MOD_L_SFT,HIDKEY____9_L_PA },//1 )
				{ HIDKEY_MOD__NONE,HIDKEY_MIN_UNDER },//2 *
				{ HIDKEY_MOD_L_SFT,HIDKEY____4_DOLL },//3 +
				{ HIDKEY_MOD__NONE,0x32 },//4 ,
				{ HIDKEY_MOD__NONE,HIDKEY_EQU__PLUS },//5 -
				{ HIDKEY_MOD__NONE,HIDKEY_SLASH__QM },//6 .
				{ HIDKEY_MOD_L_SFT,HIDKEY____7_AMPE },//7 /
				{ HIDKEY_MOD__NONE,HIDKEY____0_R_PA },//8 0
				{ HIDKEY_MOD__NONE,HIDKEY____1_EXCL },//9 1
				// 5 ==================
				{ HIDKEY_MOD__NONE,HIDKEY____2_QUOT },//0 2
				{ HIDKEY_MOD__NONE,HIDKEY____3_SHAR },//1 3
				{ HIDKEY_MOD__NONE,HIDKEY____4_DOLL },//2 4
				{ HIDKEY_MOD__NONE,HIDKEY____5_PERC },//3 5
				{ HIDKEY_MOD__NONE,HIDKEY____6_CIRC },//4 6
				{ HIDKEY_MOD__NONE,HIDKEY____7_AMPE },//5 7
				{ HIDKEY_MOD__NONE,HIDKEY____8_ASTE },//6 8
				{ HIDKEY_MOD__NONE,HIDKEY____9_L_PA },//7 9
				{ HIDKEY_MOD_L_SFT,HIDKEY_SLASH__QM },//8 :
				{ HIDKEY_MOD_L_SFT,0x32 },//9 ;
				// 6 ==================
				{ HIDKEY_MOD__NONE,0x64 },//0 <
				{ HIDKEY_MOD_L_SFT,HIDKEY____0_R_PA },//1 =
				{ HIDKEY_MOD_L_SFT,0x64 },//2 >
				{ HIDKEY_MOD_L_SFT,HIDKEY_MIN_UNDER },//3 ?
				{ HIDKEY_MOD_R_ALT,HIDKEY____q____Q },//4 @
				{ HIDKEY_MOD_L_SFT,HIDKEY____a____A },//5 A
				{ HIDKEY_MOD_L_SFT,HIDKEY____b____B },//6 B
				{ HIDKEY_MOD_L_SFT,HIDKEY____c____C },//7 C
				{ HIDKEY_MOD_L_SFT,HIDKEY____d____D },//8 D
				{ HIDKEY_MOD_L_SFT,HIDKEY____e____E },//9 E
				// 7 ==================
				{ HIDKEY_MOD_L_SFT,HIDKEY____f____F },//0 F
				{ HIDKEY_MOD_L_SFT,HIDKEY____g____G },//1 G
				{ HIDKEY_MOD_L_SFT,HIDKEY____h____H },//2 H
				{ HIDKEY_MOD_L_SFT,HIDKEY____i____I },//3 I
				{ HIDKEY_MOD_L_SFT,HIDKEY____j____J },//4 J
				{ HIDKEY_MOD_L_SFT,HIDKEY____k____K },//5 K
				{ HIDKEY_MOD_L_SFT,HIDKEY____l____L },//6 L
				{ HIDKEY_MOD_L_SFT,HIDKEY____m____M },//7 M
				{ HIDKEY_MOD_L_SFT,HIDKEY____n____N },//8 N
				{ HIDKEY_MOD_L_SFT,HIDKEY____o____O },//9 O
				// 8 ==================
				{ HIDKEY_MOD_L_SFT,HIDKEY____p____P },//0 P
				{ HIDKEY_MOD_L_SFT,HIDKEY____q____Q },//1 Q
				{ HIDKEY_MOD_L_SFT,HIDKEY____r____R },//2 R
				{ HIDKEY_MOD_L_SFT,HIDKEY____s____S },//3 S
				{ HIDKEY_MOD_L_SFT,HIDKEY____t____T },//4 T
				{ HIDKEY_MOD_L_SFT,HIDKEY____u____U },//5 U
				{ HIDKEY_MOD_L_SFT,HIDKEY____v____V },//6 V
				{ HIDKEY_MOD_L_SFT,HIDKEY____w____W },//7 W
				{ HIDKEY_MOD_L_SFT,HIDKEY____x____X },//8 X
				{ HIDKEY_MOD_L_SFT,HIDKEY____y____Y },//9 Y
				// 9 ==================
				{ HIDKEY_MOD_L_SFT,HIDKEY____z____Z },//0 Z
				{ HIDKEY_MOD_R_ALT,HIDKEY____8_ASTE },//1 [
				{ HIDKEY_MOD_R_ALT,HIDKEY_MIN_UNDER },//2
				{ HIDKEY_MOD_R_ALT,HIDKEY____9_L_PA },//3 ]
				{ HIDKEY_MOD_L_SFT,HIDKEY____3_SHAR },//4 ^
				{ HIDKEY_MOD_L_SFT,HIDKEY_EQU__PLUS },//5 _
				{ HIDKEY_MOD__NONE,HIDKEY_GRAV_TILD },//6 `
				{ HIDKEY_MOD__NONE,HIDKEY____a____A },//7 a
				{ HIDKEY_MOD__NONE,HIDKEY____b____B },//8 b
				{ HIDKEY_MOD__NONE,HIDKEY____c____C },//9 c
				// 10 ==================
				{ HIDKEY_MOD__NONE,HIDKEY____d____D },//0 d
				{ HIDKEY_MOD__NONE,HIDKEY____e____E },//1 e
				{ HIDKEY_MOD__NONE,HIDKEY____f____F },//2 f
				{ HIDKEY_MOD__NONE,HIDKEY____g____G },//3 g
				{ HIDKEY_MOD__NONE,HIDKEY____h____H },//4 h
				{ HIDKEY_MOD__NONE,HIDKEY____i____I },//5 i
				{ HIDKEY_MOD__NONE,HIDKEY____j____J },//6 j
				{ HIDKEY_MOD__NONE,HIDKEY____k____K },//7 k
				{ HIDKEY_MOD__NONE,HIDKEY____l____L },//8 l
				{ HIDKEY_MOD__NONE,HIDKEY____m____M },//9 m
				// 11 ==================
				{ HIDKEY_MOD__NONE,HIDKEY____n____N },//0 n
				{ HIDKEY_MOD__NONE,HIDKEY____o____O },//1 o
				{ HIDKEY_MOD__NONE,HIDKEY____p____P },//2 p
				{ HIDKEY_MOD__NONE,HIDKEY____q____Q },//3 q
				{ HIDKEY_MOD__NONE,HIDKEY____r____R },//4 r
				{ HIDKEY_MOD__NONE,HIDKEY____s____S },//5 s
				{ HIDKEY_MOD__NONE,HIDKEY____t____T },//6 t
				{ HIDKEY_MOD__NONE,HIDKEY____u____U },//7 u
				{ HIDKEY_MOD__NONE,HIDKEY____v____V },//8 v
				{ HIDKEY_MOD__NONE,HIDKEY____w____W },//9 w
				// 12 ==================
				{ HIDKEY_MOD__NONE,HIDKEY____x____X },//0 x
				{ HIDKEY_MOD__NONE,HIDKEY____y____Y },//1 y
				{ HIDKEY_MOD__NONE,HIDKEY____z____Z },//2 z
				{ HIDKEY_MOD_L_SFT,HIDKEY_LBT___LBR },//3 {
				{ HIDKEY_MOD_L_SFT,HIDKEY_BSLA_VBAR },//4 |
				{ HIDKEY_MOD_L_SFT,HIDKEY_RBT___RBR },//5 }
				{ HIDKEY_MOD_L_SFT,HIDKEY_GRAV_TILD },//6 ~
				{ 0,0 },//7
				{ 0,0 },//8
				{ 0,0 }//9
			}
		};
#ifdef	WIN32
#pragma pack(pop)
#endif	//WIN32
		return &sASCToHIDKeyMap[n_language][n_ascii][n_item];
	}
	static const unsigned char* get_ascii_to_ps2_key_map(unsigned long n_language, unsigned long n_ascii = 0, unsigned n_item = 0)
	{
#ifdef	WIN32
#pragma pack(push,1)
#endif	//WIN32
		/////////////////////////////////////////////////////////////////////////////
		//maps ASCII to PS/2 set2 scan-code.
		//gAsciiToPS2KeyTable[i][j][k] ....... index i language map index, index j is ascii code.. editing 200 item
		static const unsigned char sASCToPS2KeyMap[PS2KEY_MAP_NUMBER][FOR_CVT_MAX_ASCII_CODE][2] = {
			{	// xxx.English PS2 keymap
				//{ control key key,general key }
				// 0 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ 0,0 },//2
				{ 0,0 },//3
				{ 0,0 },//4
				{ 0,0 },//5
				{ 0,0 },//6
				{ 0,0 },//7
				{ PS2KEY______NONE,PS2KEY_BACKSPACE },//8
				{ PS2KEY______NONE,PS2KEY_______TAB },//9
				// 1 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ 0,0 },//2
				{ PS2KEY______NONE,PS2KEY____RETURN },//3 ... CR
				{ 0,0 },//4
				{ 0,0 },//5
				{ 0,0 },//6
				{ 0,0 },//7
				{ 0,0 },//8
				{ 0,0 },//9
				// 2 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ 0,0 },//2
				{ 0,0 },//3
				{ 0,0 },//4
				{ 0,0 },//5
				{ 0,0 },//6
				{ PS2KEY______NONE,PS2KEY____ESCAPE },//7
				{ 0,0 },//8
				{ 0,0 },//9
				// 3 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ PS2KEY______NONE,PS2KEY_____SPACE },//2
				{ PS2KEY_____L_SFT,PS2KEY____1_EXCL },//3 !
				{ PS2KEY_____L_SFT,PS2KEY_APOS_QUOT },//4 "
				{ PS2KEY_____L_SFT,PS2KEY____3_SHAR },//5 #
				{ PS2KEY_____L_SFT,PS2KEY____4_DOLL },//6 $
				{ PS2KEY_____L_SFT,PS2KEY____5_PERC },//7 %
				{ PS2KEY_____L_SFT,PS2KEY____7_AMPE },//8 &
				{ PS2KEY______NONE,PS2KEY_APOS_QUOT },//9 '
				// 4 ==================
				{ PS2KEY_____L_SFT,PS2KEY____9_L_PA },//0 (
				{ PS2KEY_____L_SFT,PS2KEY____0_R_PA },//1 )
				{ PS2KEY_____L_SFT,PS2KEY____8_ASTE },//2 *
				{ PS2KEY_____L_SFT,PS2KEY_EQU__PLUS },//3 +
				{ PS2KEY______NONE,PS2KEY_COMA___LT },//4 ,
				{ PS2KEY______NONE,PS2KEY_MIN_UNDER },//5 -
				{ PS2KEY______NONE,PS2KEY_PERIOD_GT },//6 .
				{ PS2KEY______NONE,PS2KEY_SLASH__QM },//7 /
				{ PS2KEY______NONE,PS2KEY____0_R_PA },//8 0
				{ PS2KEY______NONE,PS2KEY____1_EXCL },//9 1
				// 5 ==================
				{ PS2KEY______NONE,PS2KEY____2_QUOT },//0 2
				{ PS2KEY______NONE,PS2KEY____3_SHAR },//1 3
				{ PS2KEY______NONE,PS2KEY____4_DOLL },//2 4
				{ PS2KEY______NONE,PS2KEY____5_PERC },//3 5
				{ PS2KEY______NONE,PS2KEY____6_CIRC },//4 6
				{ PS2KEY______NONE,PS2KEY____7_AMPE },//5 7
				{ PS2KEY______NONE,PS2KEY____8_ASTE },//6 8
				{ PS2KEY______NONE,PS2KEY____9_L_PA },//7 9
				{ PS2KEY_____L_SFT,PS2KEY_SEMI__COL },//8 :
				{ PS2KEY______NONE,PS2KEY_SEMI__COL },//9 ;
				// 6 ==================
				{ PS2KEY_____L_SFT,PS2KEY_COMA___LT },//0 <
				{ PS2KEY______NONE,PS2KEY_EQU__PLUS },//1 =
				{ PS2KEY_____L_SFT,PS2KEY_PERIOD_GT },//2 >
				{ PS2KEY_____L_SFT,PS2KEY_SLASH__QM },//3 ?
				{ PS2KEY_____L_SFT,PS2KEY____2_QUOT },//4 @
				{ PS2KEY_____L_SFT,PS2KEY____a____A },//5 A
				{ PS2KEY_____L_SFT,PS2KEY____b____B },//6 B
				{ PS2KEY_____L_SFT,PS2KEY____c____C },//7 C
				{ PS2KEY_____L_SFT,PS2KEY____d____D },//8 D
				{ PS2KEY_____L_SFT,PS2KEY____e____E },//9 E
				// 7 ==================
				{ PS2KEY_____L_SFT,PS2KEY____f____F },//0 F
				{ PS2KEY_____L_SFT,PS2KEY____g____G },//1 G
				{ PS2KEY_____L_SFT,PS2KEY____h____H },//2 H
				{ PS2KEY_____L_SFT,PS2KEY____i____I },//3 I
				{ PS2KEY_____L_SFT,PS2KEY____j____J },//4 J
				{ PS2KEY_____L_SFT,PS2KEY____k____K },//5 K
				{ PS2KEY_____L_SFT,PS2KEY____l____L },//6 L
				{ PS2KEY_____L_SFT,PS2KEY____m____M },//7 M
				{ PS2KEY_____L_SFT,PS2KEY____n____N },//8 N
				{ PS2KEY_____L_SFT,PS2KEY____o____O },//9 O
				// 8 ==================
				{ PS2KEY_____L_SFT,PS2KEY____p____P },//0 P
				{ PS2KEY_____L_SFT,PS2KEY____q____Q },//1 Q
				{ PS2KEY_____L_SFT,PS2KEY____r____R },//2 R
				{ PS2KEY_____L_SFT,PS2KEY____s____S },//3 S
				{ PS2KEY_____L_SFT,PS2KEY____t____T },//4 T
				{ PS2KEY_____L_SFT,PS2KEY____u____U },//5 U
				{ PS2KEY_____L_SFT,PS2KEY____v____V },//6 V
				{ PS2KEY_____L_SFT,PS2KEY____w____W },//7 W
				{ PS2KEY_____L_SFT,PS2KEY____x____X },//8 X
				{ PS2KEY_____L_SFT,PS2KEY____y____Y },//9 Y
				// 9 ==================
				{ PS2KEY_____L_SFT,PS2KEY____z____Z },//0 Z
				{ PS2KEY______NONE,PS2KEY_LBT___LBR },//1 [
				{ PS2KEY______NONE,PS2KEY_BSLA_VBAR },//2
				{ PS2KEY______NONE,PS2KEY_RBT___RBR },//3 ]
				{ PS2KEY_____L_SFT,PS2KEY____6_CIRC },//4 ^
				{ PS2KEY_____L_SFT,PS2KEY_MIN_UNDER },//5 _
				{ PS2KEY______NONE,PS2KEY_GRAV_TILD },//6 `
				{ PS2KEY______NONE,PS2KEY____a____A },//7 a
				{ PS2KEY______NONE,PS2KEY____b____B },//8 b
				{ PS2KEY______NONE,PS2KEY____c____C },//9 c
				// 10 ==================
				{ PS2KEY______NONE,PS2KEY____d____D },//0 d
				{ PS2KEY______NONE,PS2KEY____e____E },//1 e
				{ PS2KEY______NONE,PS2KEY____f____F },//2 f
				{ PS2KEY______NONE,PS2KEY____g____G },//3 g
				{ PS2KEY______NONE,PS2KEY____h____H },//4 h
				{ PS2KEY______NONE,PS2KEY____i____I },//5 i
				{ PS2KEY______NONE,PS2KEY____j____J },//6 j
				{ PS2KEY______NONE,PS2KEY____k____K },//7 k
				{ PS2KEY______NONE,PS2KEY____l____L },//8 l
				{ PS2KEY______NONE,PS2KEY____m____M },//9 m
				// 11 ==================
				{ PS2KEY______NONE,PS2KEY____n____N },//0 n
				{ PS2KEY______NONE,PS2KEY____o____O },//1 o
				{ PS2KEY______NONE,PS2KEY____p____P },//2 p
				{ PS2KEY______NONE,PS2KEY____q____Q },//3 q
				{ PS2KEY______NONE,PS2KEY____r____R },//4 r
				{ PS2KEY______NONE,PS2KEY____s____S },//5 s
				{ PS2KEY______NONE,PS2KEY____t____T },//6 t
				{ PS2KEY______NONE,PS2KEY____u____U },//7 u
				{ PS2KEY______NONE,PS2KEY____v____V },//8 v
				{ PS2KEY______NONE,PS2KEY____w____W },//9 w
				// 12 ==================
				{ PS2KEY______NONE,PS2KEY____x____X },//0 x
				{ PS2KEY______NONE,PS2KEY____y____Y },//1 y
				{ PS2KEY______NONE,PS2KEY____z____Z },//2 z
				{ PS2KEY_____L_SFT,PS2KEY_LBT___LBR },//3 {
				{ PS2KEY_____L_SFT,PS2KEY_BSLA_VBAR },//4 |
				{ PS2KEY_____L_SFT,PS2KEY_RBT___RBR },//5 }
				{ PS2KEY_____L_SFT,PS2KEY_GRAV_TILD },//6 ~
				{ 0,0 },//7
				{ 0,0 },//8
				{ 0,0 }//9
			},
			{	// xxx.Spanish PS2 keymap
				//{ control key key,general key }
				// 0 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ 0,0 },//2
				{ 0,0 },//3
				{ 0,0 },//4
				{ 0,0 },//5
				{ 0,0 },//6
				{ 0,0 },//7
				{ PS2KEY______NONE,PS2KEY_BACKSPACE },//8
				{ PS2KEY______NONE,PS2KEY_______TAB },//9
				// 1 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ 0,0 },//2
				{ PS2KEY______NONE,PS2KEY____RETURN },//3 ... CR
				{ 0,0 },//4
				{ 0,0 },//5
				{ 0,0 },//6
				{ PS2KEY_____L_SFT,0x4c },//7 - Spanish
				{ PS2KEY_____L_SFT,0x5d },//8 - Spanish
				{ PS2KEY_____L_SFT,0x52 },//9 - Spanish
				// 2 ==================
				{ PS2KEY_____L_SFT,0x0e },//0 - Spanish
				{ 0,0 },//1
				{ 0,0 },//2
				{ 0,0 },//3
				{ 0,0 },//4
				{ 0,0 },//5
				{ 0,0 },//6
				{ PS2KEY______NONE,PS2KEY____ESCAPE },//7
				{ 0,0 },//8
				{ 0,0 },//9
				// 3 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ PS2KEY______NONE,PS2KEY_____SPACE },//2
				{ PS2KEY_____L_SFT,PS2KEY____1_EXCL },//3 !
				{ PS2KEY_____L_SFT,PS2KEY____2_QUOT },//4 " - Spanish
				{ PS2KEY_____R_ALT,PS2KEY____3_SHAR },//5 #
				{ PS2KEY_____L_SFT,PS2KEY____4_DOLL },//6 $
				{ PS2KEY_____L_SFT,PS2KEY____5_PERC },//7 %
				{ PS2KEY_____L_SFT,PS2KEY____6_CIRC },//8 & - Spanish
				{ PS2KEY______NONE,PS2KEY_MIN_UNDER },//9 ' - Spanish
				// 4 ==================
				{ PS2KEY_____L_SFT,PS2KEY____8_ASTE },//0 ( - Spanish
				{ PS2KEY_____L_SFT,PS2KEY____9_L_PA },//1 ) - Spanish
				{ PS2KEY_____L_SFT,PS2KEY_RBT___RBR },//2 * - Spanish
				{ PS2KEY______NONE,PS2KEY_RBT___RBR },//3 + - Spanish
				{ PS2KEY______NONE,PS2KEY_COMA___LT },//4 ,
				{ PS2KEY______NONE,PS2KEY_SLASH__QM },//5 - - Spanish
				{ PS2KEY______NONE,PS2KEY_PERIOD_GT },//6 .
				{ PS2KEY_____L_SFT,PS2KEY____7_AMPE },//7 / - Spanish
				{ PS2KEY______NONE,PS2KEY____0_R_PA },//8 0
				{ PS2KEY______NONE,PS2KEY____1_EXCL },//9 1
				// 5 ==================
				{ PS2KEY______NONE,PS2KEY____2_QUOT },//0 2
				{ PS2KEY______NONE,PS2KEY____3_SHAR },//1 3
				{ PS2KEY______NONE,PS2KEY____4_DOLL },//2 4
				{ PS2KEY______NONE,PS2KEY____5_PERC },//3 5
				{ PS2KEY______NONE,PS2KEY____6_CIRC },//4 6
				{ PS2KEY______NONE,PS2KEY____7_AMPE },//5 7
				{ PS2KEY______NONE,PS2KEY____8_ASTE },//6 8
				{ PS2KEY______NONE,PS2KEY____9_L_PA },//7 9
				{ PS2KEY_____L_SFT,PS2KEY_PERIOD_GT },//8 : - Spanish
				{ PS2KEY_____L_SFT,PS2KEY_COMA___LT },//9 ; - Spanish
				// 6 ==================
				{ PS2KEY______NONE,0x61 },//0 < - Spanish
				{ PS2KEY_____L_SFT,PS2KEY____0_R_PA },//1 = - Spanish
				{ PS2KEY______NONE,0x61 },//2 > - Spanish
				{ PS2KEY_____L_SFT,PS2KEY_MIN_UNDER },//3 ? - Spanish
				{ PS2KEY_____R_ALT,PS2KEY____2_QUOT },//4 @ - Spanish
				{ PS2KEY_____L_SFT,PS2KEY____a____A },//5 A
				{ PS2KEY_____L_SFT,PS2KEY____b____B },//6 B
				{ PS2KEY_____L_SFT,PS2KEY____c____C },//7 C
				{ PS2KEY_____L_SFT,PS2KEY____d____D },//8 D
				{ PS2KEY_____L_SFT,PS2KEY____e____E },//9 E
				// 7 ==================
				{ PS2KEY_____L_SFT,PS2KEY____f____F },//0 F
				{ PS2KEY_____L_SFT,PS2KEY____g____G },//1 G
				{ PS2KEY_____L_SFT,PS2KEY____h____H },//2 H
				{ PS2KEY_____L_SFT,PS2KEY____i____I },//3 I
				{ PS2KEY_____L_SFT,PS2KEY____j____J },//4 J
				{ PS2KEY_____L_SFT,PS2KEY____k____K },//5 K
				{ PS2KEY_____L_SFT,PS2KEY____l____L },//6 L
				{ PS2KEY_____L_SFT,PS2KEY____m____M },//7 M
				{ PS2KEY_____L_SFT,PS2KEY____n____N },//8 N
				{ PS2KEY_____L_SFT,PS2KEY____o____O },//9 O
				// 8 ==================
				{ PS2KEY_____L_SFT,PS2KEY____p____P },//0 P
				{ PS2KEY_____L_SFT,PS2KEY____q____Q },//1 Q
				{ PS2KEY_____L_SFT,PS2KEY____r____R },//2 R
				{ PS2KEY_____L_SFT,PS2KEY____s____S },//3 S
				{ PS2KEY_____L_SFT,PS2KEY____t____T },//4 T
				{ PS2KEY_____L_SFT,PS2KEY____u____U },//5 U
				{ PS2KEY_____L_SFT,PS2KEY____v____V },//6 V
				{ PS2KEY_____L_SFT,PS2KEY____w____W },//7 W
				{ PS2KEY_____L_SFT,PS2KEY____x____X },//8 X
				{ PS2KEY_____L_SFT,PS2KEY____y____Y },//9 Y
				// 9 ==================
				{ PS2KEY_____L_SFT,PS2KEY____z____Z },//0 Z
				{ PS2KEY_____R_ALT,PS2KEY_LBT___LBR },//1 [ - Spanish
				{ PS2KEY_____R_ALT,PS2KEY_GRAV_TILD },//2
				{ PS2KEY_____R_ALT,PS2KEY_RBT___RBR },//3 ] - Spanish
				{ PS2KEY_____L_SFT,PS2KEY_LBT___LBR },//4 ^ - Spanish
				{ PS2KEY_____L_SFT,PS2KEY_SLASH__QM },//5 _ - Spanish
				{ PS2KEY______NONE,0x54 },//6 ` - Spanish
				{ PS2KEY______NONE,PS2KEY____a____A },//7 a
				{ PS2KEY______NONE,PS2KEY____b____B },//8 b
				{ PS2KEY______NONE,PS2KEY____c____C },//9 c
				// 10 ==================
				{ PS2KEY______NONE,PS2KEY____d____D },//0 d
				{ PS2KEY______NONE,PS2KEY____e____E },//1 e
				{ PS2KEY______NONE,PS2KEY____f____F },//2 f
				{ PS2KEY______NONE,PS2KEY____g____G },//3 g
				{ PS2KEY______NONE,PS2KEY____h____H },//4 h
				{ PS2KEY______NONE,PS2KEY____i____I },//5 i
				{ PS2KEY______NONE,PS2KEY____j____J },//6 j
				{ PS2KEY______NONE,PS2KEY____k____K },//7 k
				{ PS2KEY______NONE,PS2KEY____l____L },//8 l
				{ PS2KEY______NONE,PS2KEY____m____M },//9 m
				// 11 ==================
				{ PS2KEY______NONE,PS2KEY____n____N },//0 n
				{ PS2KEY______NONE,PS2KEY____o____O },//1 o
				{ PS2KEY______NONE,PS2KEY____p____P },//2 p
				{ PS2KEY______NONE,PS2KEY____q____Q },//3 q
				{ PS2KEY______NONE,PS2KEY____r____R },//4 r
				{ PS2KEY______NONE,PS2KEY____s____S },//5 s
				{ PS2KEY______NONE,PS2KEY____t____T },//6 t
				{ PS2KEY______NONE,PS2KEY____u____U },//7 u
				{ PS2KEY______NONE,PS2KEY____v____V },//8 v
				{ PS2KEY______NONE,PS2KEY____w____W },//9 w
				// 12 ==================
				{ PS2KEY______NONE,PS2KEY____x____X },//0 x
				{ PS2KEY______NONE,PS2KEY____y____Y },//1 y
				{ PS2KEY______NONE,PS2KEY____z____Z },//2 z
				{ PS2KEY_____L_SFT,0x4c },//3 { - Spanish
				{ PS2KEY_____L_SFT,PS2KEY_BSLA_VBAR },//4 |
				{ PS2KEY_____L_SFT,0x52 },//5 } - Spanish
				{ PS2KEY_____L_SFT,0x0e },//6 ~ - Spanish
				{ 0,0 },//7
				{ 0,0 },//8
				{ 0,0 }//9
			},
			{	// xxx.Danish PS2 keymap		//{ control key key,general key }
				// 0 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ 0,0 },//2
				{ 0,0 },//3
				{ 0,0 },//4
				{ 0,0 },//5
				{ 0,0 },//6
				{ 0,0 },//7
				{ PS2KEY______NONE,PS2KEY_BACKSPACE },//8
				{ PS2KEY______NONE,PS2KEY_______TAB },//9
				// 1 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ 0,0 },//2
				{ PS2KEY______NONE,PS2KEY____RETURN },//3 ... CR
				{ 0,0 },//4
				{ 0,0 },//5
				{ 0,0 },//6
				{ 0,0 },//7
				{ 0,0 },//8
				{ 0,0 },//9
				// 2 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ 0,0 },//2
				{ 0,0 },//3
				{ 0,0 },//4
				{ 0,0 },//5
				{ 0,0 },//6
				{ PS2KEY______NONE,PS2KEY____ESCAPE },//7
				{ 0,0 },//8
				{ 0,0 },//9
				// 3 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ PS2KEY______NONE,PS2KEY_____SPACE },//2
				{ PS2KEY_____L_SFT,PS2KEY____1_EXCL },//3 !
				{ PS2KEY_____L_SFT,PS2KEY____2_QUOT },//4 "
				{ PS2KEY_____L_SFT,PS2KEY____3_SHAR },//5 #
				{ PS2KEY_____L_SFT,PS2KEY____4_DOLL },//6 $
				{ PS2KEY_____L_SFT,PS2KEY____5_PERC },//7 %
				{ PS2KEY_____L_SFT,PS2KEY____6_CIRC },//8 &
				{ PS2KEY______NONE,0x5D },//9 '
				// 4 ==================
				{ PS2KEY_____L_SFT,PS2KEY____8_ASTE },//0 (
				{ PS2KEY_____L_SFT,PS2KEY____9_L_PA },//1 )
				{ PS2KEY_____L_SFT,0x5D },//2 *
				{ PS2KEY______NONE,PS2KEY_MIN_UNDER },//3 +
				{ PS2KEY______NONE,PS2KEY_COMA___LT },//4 ,
				{ PS2KEY______NONE,PS2KEY_SLASH__QM },//5 -
				{ PS2KEY______NONE,PS2KEY_PERIOD_GT },//6 .
				{ PS2KEY_____L_SFT,PS2KEY____7_AMPE },//7 /
				{ PS2KEY______NONE,PS2KEY____0_R_PA },//8 0
				{ PS2KEY______NONE,PS2KEY____1_EXCL },//9 1
				// 5 ==================
				{ PS2KEY______NONE,PS2KEY____2_QUOT },//0 2
				{ PS2KEY______NONE,PS2KEY____3_SHAR },//1 3
				{ PS2KEY______NONE,PS2KEY____4_DOLL },//2 4
				{ PS2KEY______NONE,PS2KEY____5_PERC },//3 5
				{ PS2KEY______NONE,PS2KEY____6_CIRC },//4 6
				{ PS2KEY______NONE,PS2KEY____7_AMPE },//5 7
				{ PS2KEY______NONE,PS2KEY____8_ASTE },//6 8
				{ PS2KEY______NONE,PS2KEY____9_L_PA },//7 9
				{ PS2KEY_____L_SFT,PS2KEY_PERIOD_GT },//8 :
				{ PS2KEY_____L_SFT,PS2KEY_COMA___LT },//9 ;
				// 6 ==================
				{ PS2KEY______NONE,0x61 },//0 <
				{ PS2KEY_____L_SFT,PS2KEY____0_R_PA },//1 =
				{ PS2KEY_____L_SFT,0x61 },//2 >
				{ PS2KEY_____L_SFT,PS2KEY_MIN_UNDER },//3 ?
				{ PS2KEY_____R_ALT,PS2KEY____2_QUOT },//4 @
				{ PS2KEY_____L_SFT,PS2KEY____a____A },//5 A
				{ PS2KEY_____L_SFT,PS2KEY____b____B },//6 B
				{ PS2KEY_____L_SFT,PS2KEY____c____C },//7 C
				{ PS2KEY_____L_SFT,PS2KEY____d____D },//8 D
				{ PS2KEY_____L_SFT,PS2KEY____e____E },//9 E
				// 7 ==================
				{ PS2KEY_____L_SFT,PS2KEY____f____F },//0 F
				{ PS2KEY_____L_SFT,PS2KEY____g____G },//1 G
				{ PS2KEY_____L_SFT,PS2KEY____h____H },//2 H
				{ PS2KEY_____L_SFT,PS2KEY____i____I },//3 I
				{ PS2KEY_____L_SFT,PS2KEY____j____J },//4 J
				{ PS2KEY_____L_SFT,PS2KEY____k____K },//5 K
				{ PS2KEY_____L_SFT,PS2KEY____l____L },//6 L
				{ PS2KEY_____L_SFT,PS2KEY____m____M },//7 M
				{ PS2KEY_____L_SFT,PS2KEY____n____N },//8 N
				{ PS2KEY_____L_SFT,PS2KEY____o____O },//9 O
				// 8 ==================
				{ PS2KEY_____L_SFT,PS2KEY____p____P },//0 P
				{ PS2KEY_____L_SFT,PS2KEY____q____Q },//1 Q
				{ PS2KEY_____L_SFT,PS2KEY____r____R },//2 R
				{ PS2KEY_____L_SFT,PS2KEY____s____S },//3 S
				{ PS2KEY_____L_SFT,PS2KEY____t____T },//4 T
				{ PS2KEY_____L_SFT,PS2KEY____u____U },//5 U
				{ PS2KEY_____L_SFT,PS2KEY____v____V },//6 V
				{ PS2KEY_____L_SFT,PS2KEY____w____W },//7 W
				{ PS2KEY_____L_SFT,PS2KEY____x____X },//8 X
				{ PS2KEY_____L_SFT,PS2KEY____y____Y },//9 Y
				// 9 ==================
				{ PS2KEY_____L_SFT,PS2KEY____z____Z },//0 Z
				{ PS2KEY_____R_ALT,PS2KEY____8_ASTE },//1 [
				{ PS2KEY_____R_ALT,0x61 },//2
				{ PS2KEY_____R_ALT,PS2KEY____9_L_PA },//3 ]
				{ PS2KEY_____L_SFT,PS2KEY_RBT___RBR },//4 ^
				{ PS2KEY_____L_SFT,PS2KEY_SLASH__QM },//5 _
				{ PS2KEY______NONE,PS2KEY_GRAV_TILD },//6 `
				{ PS2KEY______NONE,PS2KEY____a____A },//7 a
				{ PS2KEY______NONE,PS2KEY____b____B },//8 b
				{ PS2KEY______NONE,PS2KEY____c____C },//9 c
				// 10 ==================
				{ PS2KEY______NONE,PS2KEY____d____D },//0 d
				{ PS2KEY______NONE,PS2KEY____e____E },//1 e
				{ PS2KEY______NONE,PS2KEY____f____F },//2 f
				{ PS2KEY______NONE,PS2KEY____g____G },//3 g
				{ PS2KEY______NONE,PS2KEY____h____H },//4 h
				{ PS2KEY______NONE,PS2KEY____i____I },//5 i
				{ PS2KEY______NONE,PS2KEY____j____J },//6 j
				{ PS2KEY______NONE,PS2KEY____k____K },//7 k
				{ PS2KEY______NONE,PS2KEY____l____L },//8 l
				{ PS2KEY______NONE,PS2KEY____m____M },//9 m
				// 11 ==================
				{ PS2KEY______NONE,PS2KEY____n____N },//0 n
				{ PS2KEY______NONE,PS2KEY____o____O },//1 o
				{ PS2KEY______NONE,PS2KEY____p____P },//2 p
				{ PS2KEY______NONE,PS2KEY____q____Q },//3 q
				{ PS2KEY______NONE,PS2KEY____r____R },//4 r
				{ PS2KEY______NONE,PS2KEY____s____S },//5 s
				{ PS2KEY______NONE,PS2KEY____t____T },//6 t
				{ PS2KEY______NONE,PS2KEY____u____U },//7 u
				{ PS2KEY______NONE,PS2KEY____v____V },//8 v
				{ PS2KEY______NONE,PS2KEY____w____W },//9 w
				// 12 ==================
				{ PS2KEY______NONE,PS2KEY____x____X },//0 x
				{ PS2KEY______NONE,PS2KEY____y____Y },//1 y
				{ PS2KEY______NONE,PS2KEY____z____Z },//2 z
				{ PS2KEY_____L_SFT,PS2KEY_LBT___LBR },//3 {
				{ PS2KEY_____L_SFT,PS2KEY_BSLA_VBAR },//4 |
				{ PS2KEY_____L_SFT,PS2KEY_RBT___RBR },//5 }
				{ PS2KEY_____L_SFT,PS2KEY_GRAV_TILD },//6 ~
				{ 0,0 },//7
				{ 0,0 },//8
				{ 0,0 }//9
			},
			{	// xxx.French PS2 keymap		//{ control key key,general key }
				// 0 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ 0,0 },//2
				{ 0,0 },//3
				{ 0,0 },//4
				{ 0,0 },//5
				{ 0,0 },//6
				{ 0,0 },//7
				{ PS2KEY______NONE,PS2KEY_BACKSPACE },//8
				{ PS2KEY______NONE,PS2KEY_______TAB },//9
				// 1 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ 0,0 },//2
				{ PS2KEY______NONE,PS2KEY____RETURN },//3 ... CR
				{ 0,0 },//4
				{ 0,0 },//5
				{ 0,0 },//6
				{ 0,0 },//7
				{ 0,0 },//8
				{ 0,0 },//9
				// 2 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ 0,0 },//2
				{ 0,0 },//3
				{ 0,0 },//4
				{ 0,0 },//5
				{ 0,0 },//6
				{ PS2KEY______NONE,PS2KEY____ESCAPE },//7
				{ 0,0 },//8
				{ 0,0 },//9
				// 3 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ PS2KEY______NONE,PS2KEY_____SPACE },//2
				{ PS2KEY______NONE,PS2KEY_SLASH__QM },//3 !
				{ PS2KEY______NONE,PS2KEY____3_SHAR },//4 "
				{ PS2KEY_____R_ALT,PS2KEY____3_SHAR },//5 #
				{ PS2KEY______NONE,PS2KEY_RBT___RBR },//6 $
				{ PS2KEY_____L_SFT,PS2KEY_APOS_QUOT },//7 %
				{ PS2KEY______NONE,PS2KEY____1_EXCL },//8 &
				{ PS2KEY______NONE,PS2KEY____4_DOLL },//9 '
				// 4 ==================
				{ PS2KEY______NONE,PS2KEY____5_PERC },//0 (
				{ PS2KEY______NONE,PS2KEY_MIN_UNDER },//1 )
				{ PS2KEY______NONE,0x5D },//2 *
				{ PS2KEY_____L_SFT,PS2KEY_EQU__PLUS },//3 +
				{ PS2KEY______NONE,PS2KEY____m____M },//4 ,
				{ PS2KEY______NONE,PS2KEY____6_CIRC },//5 -
				{ PS2KEY______NONE,PS2KEY_COMA___LT },//6 .
				{ PS2KEY______NONE,PS2KEY_PERIOD_GT },//7 /
				{ PS2KEY______NONE,PS2KEY____0_R_PA },//8 0
				{ PS2KEY______NONE,PS2KEY____1_EXCL },//9 1
				// 5 ==================
				{ PS2KEY______NONE,PS2KEY____2_QUOT },//0 2
				{ PS2KEY______NONE,PS2KEY____3_SHAR },//1 3
				{ PS2KEY______NONE,PS2KEY____4_DOLL },//2 4
				{ PS2KEY______NONE,PS2KEY____5_PERC },//3 5
				{ PS2KEY______NONE,PS2KEY____6_CIRC },//4 6
				{ PS2KEY______NONE,PS2KEY____7_AMPE },//5 7
				{ PS2KEY______NONE,PS2KEY____8_ASTE },//6 8
				{ PS2KEY______NONE,PS2KEY____9_L_PA },//7 9
				{ PS2KEY______NONE,PS2KEY_PERIOD_GT },//8 :
				{ PS2KEY______NONE,PS2KEY_COMA___LT },//9 ;
				// 6 ==================
				{ PS2KEY______NONE,0x61 },//0 <
				{ PS2KEY______NONE,PS2KEY_EQU__PLUS },//1 =
				{ PS2KEY_____L_SFT,0x61 },//2 >
				{ PS2KEY_____L_SFT,PS2KEY____m____M },//3 ?
				{ PS2KEY_____R_ALT,PS2KEY____0_R_PA },//4 @
				{ PS2KEY_____L_SFT,PS2KEY____q____Q },//5 A
				{ PS2KEY_____L_SFT,PS2KEY____b____B },//6 B
				{ PS2KEY_____L_SFT,PS2KEY____c____C },//7 C
				{ PS2KEY_____L_SFT,PS2KEY____d____D },//8 D
				{ PS2KEY_____L_SFT,PS2KEY____e____E },//9 E
				// 7 ==================
				{ PS2KEY_____L_SFT,PS2KEY____f____F },//0 F
				{ PS2KEY_____L_SFT,PS2KEY____g____G },//1 G
				{ PS2KEY_____L_SFT,PS2KEY____h____H },//2 H
				{ PS2KEY_____L_SFT,PS2KEY____i____I },//3 I
				{ PS2KEY_____L_SFT,PS2KEY____j____J },//4 J
				{ PS2KEY_____L_SFT,PS2KEY____k____K },//5 K
				{ PS2KEY_____L_SFT,PS2KEY____l____L },//6 L
				{ PS2KEY_____L_SFT,PS2KEY_SEMI__COL },//7 M
				{ PS2KEY_____L_SFT,PS2KEY____n____N },//8 N
				{ PS2KEY_____L_SFT,PS2KEY____o____O },//9 O
				// 8 ==================
				{ PS2KEY_____L_SFT,PS2KEY____p____P },//0 P
				{ PS2KEY_____L_SFT,PS2KEY____a____A },//1 Q
				{ PS2KEY_____L_SFT,PS2KEY____r____R },//2 R
				{ PS2KEY_____L_SFT,PS2KEY____s____S },//3 S
				{ PS2KEY_____L_SFT,PS2KEY____t____T },//4 T
				{ PS2KEY_____L_SFT,PS2KEY____u____U },//5 U
				{ PS2KEY_____L_SFT,PS2KEY____v____V },//6 V
				{ PS2KEY_____L_SFT,PS2KEY____z____Z },//7 W
				{ PS2KEY_____L_SFT,PS2KEY____x____X },//8 X
				{ PS2KEY_____L_SFT,PS2KEY____y____Y },//9 Y
				// 9 ==================
				{ PS2KEY_____L_SFT,PS2KEY____w____W },//0 Z
				{ PS2KEY_____R_ALT,PS2KEY____5_PERC },//1 [
				{ PS2KEY_____R_ALT,PS2KEY____8_ASTE },//2
				{ PS2KEY_____R_ALT,PS2KEY_MIN_UNDER },//3 ]
				{ PS2KEY_____R_ALT,PS2KEY____9_L_PA },//4 ^
				{ PS2KEY______NONE,PS2KEY____8_ASTE },//5 _
				{ PS2KEY______NONE,PS2KEY_GRAV_TILD },//6 `
				{ PS2KEY______NONE,PS2KEY____a____A },//7 a
				{ PS2KEY______NONE,PS2KEY____b____B },//8 b
				{ PS2KEY______NONE,PS2KEY____c____C },//9 c
				// 10 ==================
				{ PS2KEY______NONE,PS2KEY____d____D },//0 d
				{ PS2KEY______NONE,PS2KEY____e____E },//1 e
				{ PS2KEY______NONE,PS2KEY____f____F },//2 f
				{ PS2KEY______NONE,PS2KEY____g____G },//3 g
				{ PS2KEY______NONE,PS2KEY____h____H },//4 h
				{ PS2KEY______NONE,PS2KEY____i____I },//5 i
				{ PS2KEY______NONE,PS2KEY____j____J },//6 j
				{ PS2KEY______NONE,PS2KEY____k____K },//7 k
				{ PS2KEY______NONE,PS2KEY____l____L },//8 l
				{ PS2KEY______NONE,PS2KEY____m____M },//9 m
				// 11 ==================
				{ PS2KEY______NONE,PS2KEY____n____N },//0 n
				{ PS2KEY______NONE,PS2KEY____o____O },//1 o
				{ PS2KEY______NONE,PS2KEY____p____P },//2 p
				{ PS2KEY______NONE,PS2KEY____q____Q },//3 q
				{ PS2KEY______NONE,PS2KEY____r____R },//4 r
				{ PS2KEY______NONE,PS2KEY____s____S },//5 s
				{ PS2KEY______NONE,PS2KEY____t____T },//6 t
				{ PS2KEY______NONE,PS2KEY____u____U },//7 u
				{ PS2KEY______NONE,PS2KEY____v____V },//8 v
				{ PS2KEY______NONE,PS2KEY____w____W },//9 w
				// 12 ==================
				{ PS2KEY______NONE,PS2KEY____x____X },//0 x
				{ PS2KEY______NONE,PS2KEY____y____Y },//1 y
				{ PS2KEY______NONE,PS2KEY____z____Z },//2 z
				{ PS2KEY_____L_SFT,PS2KEY_LBT___LBR },//3 {
				{ PS2KEY_____L_SFT,PS2KEY_BSLA_VBAR },//4 |
				{ PS2KEY_____L_SFT,PS2KEY_RBT___RBR },//5 }
				{ PS2KEY_____L_SFT,PS2KEY_GRAV_TILD },//6 ~
				{ 0,0 },//7
				{ 0,0 },//8
				{ 0,0 }//9
			},
			{	// xxx.German PS2 keymap		//{ control key key,general key }
				// 0 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ 0,0 },//2
				{ 0,0 },//3
				{ 0,0 },//4
				{ 0,0 },//5
				{ 0,0 },//6
				{ 0,0 },//7
				{ PS2KEY______NONE,PS2KEY_BACKSPACE },//8
				{ PS2KEY______NONE,PS2KEY_______TAB },//9
				// 1 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ 0,0 },//2
				{ PS2KEY______NONE,PS2KEY____RETURN },//3 ... CR
				{ 0,0 },//4
				{ 0,0 },//5
				{ 0,0 },//6
				{ 0,0 },//7
				{ 0,0 },//8
				{ 0,0 },//9
				// 2 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ 0,0 },//2
				{ 0,0 },//3
				{ 0,0 },//4
				{ 0,0 },//5
				{ 0,0 },//6
				{ PS2KEY______NONE,PS2KEY____ESCAPE },//7
				{ 0,0 },//8
				{ 0,0 },//9
				// 3 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ PS2KEY______NONE,PS2KEY_____SPACE },//2
				{ PS2KEY_____L_SFT,PS2KEY____1_EXCL },//3 !
				{ PS2KEY_____L_SFT,PS2KEY____2_QUOT },//4 "
				{ PS2KEY______NONE,0x5D },//5 #
				{ PS2KEY_____L_SFT,PS2KEY____4_DOLL },//6 $
				{ PS2KEY_____L_SFT,PS2KEY____5_PERC },//7 %
				{ PS2KEY_____L_SFT,PS2KEY____6_CIRC },//8 &
				{ PS2KEY_____L_SFT,0x5D },//9 '
				// 4 ==================
				{ PS2KEY_____L_SFT,PS2KEY____8_ASTE },//0 (
				{ PS2KEY_____L_SFT,PS2KEY____9_L_PA },//1 )
				{ PS2KEY_____L_SFT,PS2KEY_RBT___RBR },//2 *
				{ PS2KEY______NONE,PS2KEY_RBT___RBR },//3 +
				{ PS2KEY______NONE,PS2KEY_COMA___LT },//4 ,
				{ PS2KEY______NONE,PS2KEY_SLASH__QM },//5 -
				{ PS2KEY______NONE,PS2KEY_PERIOD_GT },//6 .
				{ PS2KEY_____L_SFT,PS2KEY____7_AMPE },//7 /
				{ PS2KEY______NONE,PS2KEY____0_R_PA },//8 0
				{ PS2KEY______NONE,PS2KEY____1_EXCL },//9 1
				// 5 ==================
				{ PS2KEY______NONE,PS2KEY____2_QUOT },//0 2
				{ PS2KEY______NONE,PS2KEY____3_SHAR },//1 3
				{ PS2KEY______NONE,PS2KEY____4_DOLL },//2 4
				{ PS2KEY______NONE,PS2KEY____5_PERC },//3 5
				{ PS2KEY______NONE,PS2KEY____6_CIRC },//4 6
				{ PS2KEY______NONE,PS2KEY____7_AMPE },//5 7
				{ PS2KEY______NONE,PS2KEY____8_ASTE },//6 8
				{ PS2KEY______NONE,PS2KEY____9_L_PA },//7 9
				{ PS2KEY_____L_SFT,PS2KEY_PERIOD_GT },//8 :
				{ PS2KEY_____L_SFT,PS2KEY_COMA___LT },//9 ;
				// 6 ==================
				{ PS2KEY______NONE,0x61 },//0 <
				{ PS2KEY_____L_SFT,PS2KEY____0_R_PA },//1 =
				{ PS2KEY_____L_SFT,0x61 },//2 >
				{ PS2KEY_____L_SFT,PS2KEY_MIN_UNDER },//3 ?
				{ PS2KEY_____R_ALT,PS2KEY____q____Q },//4 @
				{ PS2KEY_____L_SFT,PS2KEY____a____A },//5 A
				{ PS2KEY_____L_SFT,PS2KEY____b____B },//6 B
				{ PS2KEY_____L_SFT,PS2KEY____c____C },//7 C
				{ PS2KEY_____L_SFT,PS2KEY____d____D },//8 D
				{ PS2KEY_____L_SFT,PS2KEY____e____E },//9 E
				// 7 ==================
				{ PS2KEY_____L_SFT,PS2KEY____f____F },//0 F
				{ PS2KEY_____L_SFT,PS2KEY____g____G },//1 G
				{ PS2KEY_____L_SFT,PS2KEY____h____H },//2 H
				{ PS2KEY_____L_SFT,PS2KEY____i____I },//3 I
				{ PS2KEY_____L_SFT,PS2KEY____j____J },//4 J
				{ PS2KEY_____L_SFT,PS2KEY____k____K },//5 K
				{ PS2KEY_____L_SFT,PS2KEY____l____L },//6 L
				{ PS2KEY_____L_SFT,PS2KEY____m____M },//7 M
				{ PS2KEY_____L_SFT,PS2KEY____n____N },//8 N
				{ PS2KEY_____L_SFT,PS2KEY____o____O },//9 O
				// 8 ==================
				{ PS2KEY_____L_SFT,PS2KEY____p____P },//0 P
				{ PS2KEY_____L_SFT,PS2KEY____q____Q },//1 Q
				{ PS2KEY_____L_SFT,PS2KEY____r____R },//2 R
				{ PS2KEY_____L_SFT,PS2KEY____s____S },//3 S
				{ PS2KEY_____L_SFT,PS2KEY____t____T },//4 T
				{ PS2KEY_____L_SFT,PS2KEY____u____U },//5 U
				{ PS2KEY_____L_SFT,PS2KEY____v____V },//6 V
				{ PS2KEY_____L_SFT,PS2KEY____w____W },//7 W
				{ PS2KEY_____L_SFT,PS2KEY____x____X },//8 X
				{ PS2KEY_____L_SFT,PS2KEY____z____Z },//9 Y
				// 9 ==================
				{ PS2KEY_____L_SFT,PS2KEY____y____Y },//0 Z
				{ PS2KEY_____R_ALT,PS2KEY____8_ASTE },//1 [
				{ PS2KEY_____R_ALT,PS2KEY_MIN_UNDER },//2
				{ PS2KEY_____R_ALT,PS2KEY____9_L_PA },//3 ]
				{ PS2KEY_____L_SFT,PS2KEY_GRAV_TILD },//4 ^
				{ PS2KEY______NONE,PS2KEY_SLASH__QM },//5 _
				{ PS2KEY______NONE,PS2KEY_GRAV_TILD },//6 `
				{ PS2KEY______NONE,PS2KEY____a____A },//7 a
				{ PS2KEY______NONE,PS2KEY____b____B },//8 b
				{ PS2KEY______NONE,PS2KEY____c____C },//9 c
				// 10 ==================
				{ PS2KEY______NONE,PS2KEY____d____D },//0 d
				{ PS2KEY______NONE,PS2KEY____e____E },//1 e
				{ PS2KEY______NONE,PS2KEY____f____F },//2 f
				{ PS2KEY______NONE,PS2KEY____g____G },//3 g
				{ PS2KEY______NONE,PS2KEY____h____H },//4 h
				{ PS2KEY______NONE,PS2KEY____i____I },//5 i
				{ PS2KEY______NONE,PS2KEY____j____J },//6 j
				{ PS2KEY______NONE,PS2KEY____k____K },//7 k
				{ PS2KEY______NONE,PS2KEY____l____L },//8 l
				{ PS2KEY______NONE,PS2KEY____m____M },//9 m
				// 11 ==================
				{ PS2KEY______NONE,PS2KEY____n____N },//0 n
				{ PS2KEY______NONE,PS2KEY____o____O },//1 o
				{ PS2KEY______NONE,PS2KEY____p____P },//2 p
				{ PS2KEY______NONE,PS2KEY____q____Q },//3 q
				{ PS2KEY______NONE,PS2KEY____r____R },//4 r
				{ PS2KEY______NONE,PS2KEY____s____S },//5 s
				{ PS2KEY______NONE,PS2KEY____t____T },//6 t
				{ PS2KEY______NONE,PS2KEY____u____U },//7 u
				{ PS2KEY______NONE,PS2KEY____v____V },//8 v
				{ PS2KEY______NONE,PS2KEY____w____W },//9 w
				// 12 ==================
				{ PS2KEY______NONE,PS2KEY____x____X },//0 x
				{ PS2KEY______NONE,PS2KEY____y____Y },//1 y
				{ PS2KEY______NONE,PS2KEY____z____Z },//2 z
				{ PS2KEY_____L_SFT,PS2KEY_LBT___LBR },//3 {
				{ PS2KEY_____L_SFT,PS2KEY_BSLA_VBAR },//4 |
				{ PS2KEY_____L_SFT,PS2KEY_RBT___RBR },//5 }
				{ PS2KEY_____L_SFT,PS2KEY_GRAV_TILD },//6 ~
				{ 0,0 },//7
				{ 0,0 },//8
				{ 0,0 }//9
			},
			{	// xxx.Italian PS2 keymap		//{ control key key,general key }
				// 0 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ 0,0 },//2
				{ 0,0 },//3
				{ 0,0 },//4
				{ 0,0 },//5
				{ 0,0 },//6
				{ 0,0 },//7
				{ PS2KEY______NONE,PS2KEY_BACKSPACE },//8
				{ PS2KEY______NONE,PS2KEY_______TAB },//9
				// 1 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ 0,0 },//2
				{ PS2KEY______NONE,PS2KEY____RETURN },//3 ... CR
				{ 0,0 },//4
				{ 0,0 },//5
				{ 0,0 },//6
				{ 0,0 },//7
				{ 0,0 },//8
				{ 0,0 },//9
				// 2 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ 0,0 },//2
				{ 0,0 },//3
				{ 0,0 },//4
				{ 0,0 },//5
				{ 0,0 },//6
				{ PS2KEY______NONE,PS2KEY____ESCAPE },//7
				{ 0,0 },//8
				{ 0,0 },//9
				// 3 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ PS2KEY______NONE,PS2KEY_____SPACE },//2
				{ PS2KEY_____L_SFT,PS2KEY____1_EXCL },//3 !
				{ PS2KEY_____L_SFT,PS2KEY____2_QUOT },//4 "
				{ PS2KEY_____R_ALT,PS2KEY_APOS_QUOT },//5 #
				{ PS2KEY_____L_SFT,PS2KEY____4_DOLL },//6 $
				{ PS2KEY_____L_SFT,PS2KEY____5_PERC },//7 %
				{ PS2KEY_____L_SFT,PS2KEY____6_CIRC },//8 &
				{ PS2KEY______NONE,PS2KEY_MIN_UNDER },//9 '
				// 4 ==================
				{ PS2KEY_____L_SFT,PS2KEY____8_ASTE },//0 (
				{ PS2KEY_____L_SFT,PS2KEY____9_L_PA },//1 )
				{ PS2KEY_____L_SFT,PS2KEY_RBT___RBR },//2 *
				{ PS2KEY______NONE,PS2KEY_RBT___RBR },//3 +
				{ PS2KEY______NONE,PS2KEY_COMA___LT },//4 ,
				{ PS2KEY______NONE,PS2KEY_SLASH__QM },//5 -
				{ PS2KEY______NONE,PS2KEY_PERIOD_GT },//6 .
				{ PS2KEY_____L_SFT,PS2KEY____7_AMPE },//7 /
				{ PS2KEY______NONE,PS2KEY____0_R_PA },//8 0
				{ PS2KEY______NONE,PS2KEY____1_EXCL },//9 1
				// 5 ==================
				{ PS2KEY______NONE,PS2KEY____2_QUOT },//0 2
				{ PS2KEY______NONE,PS2KEY____3_SHAR },//1 3
				{ PS2KEY______NONE,PS2KEY____4_DOLL },//2 4
				{ PS2KEY______NONE,PS2KEY____5_PERC },//3 5
				{ PS2KEY______NONE,PS2KEY____6_CIRC },//4 6
				{ PS2KEY______NONE,PS2KEY____7_AMPE },//5 7
				{ PS2KEY______NONE,PS2KEY____8_ASTE },//6 8
				{ PS2KEY______NONE,PS2KEY____9_L_PA },//7 9
				{ PS2KEY_____L_SFT,PS2KEY_PERIOD_GT },//8 :
				{ PS2KEY_____L_SFT,PS2KEY_COMA___LT },//9 ;
				// 6 ==================
				{ PS2KEY______NONE,0x61 },//0 <
				{ PS2KEY_____L_SFT,PS2KEY____0_R_PA },//1 =
				{ PS2KEY_____L_SFT,0x61 },//2 >
				{ PS2KEY_____L_SFT,PS2KEY_MIN_UNDER },//3 ?
				{ PS2KEY_____R_ALT,PS2KEY_SEMI__COL },//4 @
				{ PS2KEY_____L_SFT,PS2KEY____a____A },//5 A
				{ PS2KEY_____L_SFT,PS2KEY____b____B },//6 B
				{ PS2KEY_____L_SFT,PS2KEY____c____C },//7 C
				{ PS2KEY_____L_SFT,PS2KEY____d____D },//8 D
				{ PS2KEY_____L_SFT,PS2KEY____e____E },//9 E
				// 7 ==================
				{ PS2KEY_____L_SFT,PS2KEY____f____F },//0 F
				{ PS2KEY_____L_SFT,PS2KEY____g____G },//1 G
				{ PS2KEY_____L_SFT,PS2KEY____h____H },//2 H
				{ PS2KEY_____L_SFT,PS2KEY____i____I },//3 I
				{ PS2KEY_____L_SFT,PS2KEY____j____J },//4 J
				{ PS2KEY_____L_SFT,PS2KEY____k____K },//5 K
				{ PS2KEY_____L_SFT,PS2KEY____l____L },//6 L
				{ PS2KEY_____L_SFT,PS2KEY____m____M },//7 M
				{ PS2KEY_____L_SFT,PS2KEY____n____N },//8 N
				{ PS2KEY_____L_SFT,PS2KEY____o____O },//9 O
				// 8 ==================
				{ PS2KEY_____L_SFT,PS2KEY____p____P },//0 P
				{ PS2KEY_____L_SFT,PS2KEY____q____Q },//1 Q
				{ PS2KEY_____L_SFT,PS2KEY____r____R },//2 R
				{ PS2KEY_____L_SFT,PS2KEY____s____S },//3 S
				{ PS2KEY_____L_SFT,PS2KEY____t____T },//4 T
				{ PS2KEY_____L_SFT,PS2KEY____u____U },//5 U
				{ PS2KEY_____L_SFT,PS2KEY____v____V },//6 V
				{ PS2KEY_____L_SFT,PS2KEY____w____W },//7 W
				{ PS2KEY_____L_SFT,PS2KEY____x____X },//8 X
				{ PS2KEY_____L_SFT,PS2KEY____y____Y },//9 Y
				// 9 ==================
				{ PS2KEY_____L_SFT,PS2KEY____z____Z },//0 Z
				{ PS2KEY_____R_ALT,PS2KEY_LBT___LBR },//1 [
				{ PS2KEY______NONE,PS2KEY_GRAV_TILD },//2
				{ PS2KEY_____R_ALT,PS2KEY_RBT___RBR },//3 ]
				{ PS2KEY_____L_SFT,PS2KEY_EQU__PLUS },//4 ^
				{ PS2KEY_____L_SFT,PS2KEY_SLASH__QM },//5 _
				{ PS2KEY______NONE,PS2KEY_GRAV_TILD },//6 `
				{ PS2KEY______NONE,PS2KEY____a____A },//7 a
				{ PS2KEY______NONE,PS2KEY____b____B },//8 b
				{ PS2KEY______NONE,PS2KEY____c____C },//9 c
				// 10 ==================
				{ PS2KEY______NONE,PS2KEY____d____D },//0 d
				{ PS2KEY______NONE,PS2KEY____e____E },//1 e
				{ PS2KEY______NONE,PS2KEY____f____F },//2 f
				{ PS2KEY______NONE,PS2KEY____g____G },//3 g
				{ PS2KEY______NONE,PS2KEY____h____H },//4 h
				{ PS2KEY______NONE,PS2KEY____i____I },//5 i
				{ PS2KEY______NONE,PS2KEY____j____J },//6 j
				{ PS2KEY______NONE,PS2KEY____k____K },//7 k
				{ PS2KEY______NONE,PS2KEY____l____L },//8 l
				{ PS2KEY______NONE,PS2KEY____m____M },//9 m
				// 11 ==================
				{ PS2KEY______NONE,PS2KEY____n____N },//0 n
				{ PS2KEY______NONE,PS2KEY____o____O },//1 o
				{ PS2KEY______NONE,PS2KEY____p____P },//2 p
				{ PS2KEY______NONE,PS2KEY____q____Q },//3 q
				{ PS2KEY______NONE,PS2KEY____r____R },//4 r
				{ PS2KEY______NONE,PS2KEY____s____S },//5 s
				{ PS2KEY______NONE,PS2KEY____t____T },//6 t
				{ PS2KEY______NONE,PS2KEY____u____U },//7 u
				{ PS2KEY______NONE,PS2KEY____v____V },//8 v
				{ PS2KEY______NONE,PS2KEY____w____W },//9 w
				// 12 ==================
				{ PS2KEY______NONE,PS2KEY____x____X },//0 x
				{ PS2KEY______NONE,PS2KEY____y____Y },//1 y
				{ PS2KEY______NONE,PS2KEY____z____Z },//2 z
				{ PS2KEY_____L_SFT,PS2KEY_LBT___LBR },//3 {
				{ PS2KEY_____L_SFT,PS2KEY_BSLA_VBAR },//4 |
				{ PS2KEY_____L_SFT,PS2KEY_RBT___RBR },//5 }
				{ PS2KEY_____L_SFT,PS2KEY_GRAV_TILD },//6 ~
				{ 0,0 },//7
				{ 0,0 },//8
				{ 0,0 }//9
			},
			{	// xxx.Norwegian PS2 keymap		//{ control key key,general key }
				// 0 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ 0,0 },//2
				{ 0,0 },//3
				{ 0,0 },//4
				{ 0,0 },//5
				{ 0,0 },//6
				{ 0,0 },//7
				{ PS2KEY______NONE,PS2KEY_BACKSPACE },//8
				{ PS2KEY______NONE,PS2KEY_______TAB },//9
				// 1 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ 0,0 },//2
				{ PS2KEY______NONE,PS2KEY____RETURN },//3 ... CR
				{ 0,0 },//4
				{ 0,0 },//5
				{ 0,0 },//6
				{ 0,0 },//7
				{ 0,0 },//8
				{ 0,0 },//9
				// 2 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ 0,0 },//2
				{ 0,0 },//3
				{ 0,0 },//4
				{ 0,0 },//5
				{ 0,0 },//6
				{ PS2KEY______NONE,PS2KEY____ESCAPE },//7
				{ 0,0 },//8
				{ 0,0 },//9
				// 3 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ PS2KEY______NONE,PS2KEY_____SPACE },//2
				{ PS2KEY_____L_SFT,PS2KEY____1_EXCL },//3 !
				{ PS2KEY_____L_SFT,PS2KEY____2_QUOT },//4 "
				{ PS2KEY_____L_SFT,PS2KEY____3_SHAR },//5 #
				{ PS2KEY_____R_ALT,PS2KEY____4_DOLL },//6 $
				{ PS2KEY_____L_SFT,PS2KEY____5_PERC },//7 %
				{ PS2KEY_____L_SFT,PS2KEY____6_CIRC },//8 &
				{ PS2KEY______NONE,0x5D },//9 '
				// 4 ==================
				{ PS2KEY_____L_SFT,PS2KEY____8_ASTE },//0 (
				{ PS2KEY_____L_SFT,PS2KEY____9_L_PA },//1 )
				{ PS2KEY_____L_SFT,0x5D },//2 *
				{ PS2KEY______NONE,PS2KEY_MIN_UNDER },//3 +
				{ PS2KEY______NONE,PS2KEY_COMA___LT },//4 ,
				{ PS2KEY______NONE,PS2KEY_SLASH__QM },//5 -
				{ PS2KEY______NONE,PS2KEY_PERIOD_GT },//6 .
				{ PS2KEY_____L_SFT,PS2KEY____7_AMPE },//7 /
				{ PS2KEY______NONE,PS2KEY____0_R_PA },//8 0
				{ PS2KEY______NONE,PS2KEY____1_EXCL },//9 1
				// 5 ==================
				{ PS2KEY______NONE,PS2KEY____2_QUOT },//0 2
				{ PS2KEY______NONE,PS2KEY____3_SHAR },//1 3
				{ PS2KEY______NONE,PS2KEY____4_DOLL },//2 4
				{ PS2KEY______NONE,PS2KEY____5_PERC },//3 5
				{ PS2KEY______NONE,PS2KEY____6_CIRC },//4 6
				{ PS2KEY______NONE,PS2KEY____7_AMPE },//5 7
				{ PS2KEY______NONE,PS2KEY____8_ASTE },//6 8
				{ PS2KEY______NONE,PS2KEY____9_L_PA },//7 9
				{ PS2KEY_____L_SFT,PS2KEY_PERIOD_GT },//8 :
				{ PS2KEY_____L_SFT,PS2KEY_COMA___LT },//9 ;
				// 6 ==================
				{ PS2KEY______NONE,0x61 },//0 <
				{ PS2KEY_____L_SFT,PS2KEY____0_R_PA },//1 =
				{ PS2KEY_____L_SFT,0x61 },//2 >
				{ PS2KEY_____L_SFT,PS2KEY_MIN_UNDER },//3 ?
				{ PS2KEY_____R_ALT,PS2KEY____2_QUOT },//4 @
				{ PS2KEY_____L_SFT,PS2KEY____a____A },//5 A
				{ PS2KEY_____L_SFT,PS2KEY____b____B },//6 B
				{ PS2KEY_____L_SFT,PS2KEY____c____C },//7 C
				{ PS2KEY_____L_SFT,PS2KEY____d____D },//8 D
				{ PS2KEY_____L_SFT,PS2KEY____e____E },//9 E
				// 7 ==================
				{ PS2KEY_____L_SFT,PS2KEY____f____F },//0 F
				{ PS2KEY_____L_SFT,PS2KEY____g____G },//1 G
				{ PS2KEY_____L_SFT,PS2KEY____h____H },//2 H
				{ PS2KEY_____L_SFT,PS2KEY____i____I },//3 I
				{ PS2KEY_____L_SFT,PS2KEY____j____J },//4 J
				{ PS2KEY_____L_SFT,PS2KEY____k____K },//5 K
				{ PS2KEY_____L_SFT,PS2KEY____l____L },//6 L
				{ PS2KEY_____L_SFT,PS2KEY____m____M },//7 M
				{ PS2KEY_____L_SFT,PS2KEY____n____N },//8 N
				{ PS2KEY_____L_SFT,PS2KEY____o____O },//9 O
				// 8 ==================
				{ PS2KEY_____L_SFT,PS2KEY____p____P },//0 P
				{ PS2KEY_____L_SFT,PS2KEY____q____Q },//1 Q
				{ PS2KEY_____L_SFT,PS2KEY____r____R },//2 R
				{ PS2KEY_____L_SFT,PS2KEY____s____S },//3 S
				{ PS2KEY_____L_SFT,PS2KEY____t____T },//4 T
				{ PS2KEY_____L_SFT,PS2KEY____u____U },//5 U
				{ PS2KEY_____L_SFT,PS2KEY____v____V },//6 V
				{ PS2KEY_____L_SFT,PS2KEY____w____W },//7 W
				{ PS2KEY_____L_SFT,PS2KEY____x____X },//8 X
				{ PS2KEY_____L_SFT,PS2KEY____y____Y },//9 Y
				// 9 ==================
				{ PS2KEY_____L_SFT,PS2KEY____z____Z },//0 Z
				{ PS2KEY_____R_ALT,PS2KEY____8_ASTE },//1 [
				{ PS2KEY______NONE,PS2KEY_EQU__PLUS },//2
				{ PS2KEY_____R_ALT,PS2KEY____9_L_PA },//3 ]
				{ PS2KEY_____L_SFT,PS2KEY_RBT___RBR },//4 ^
				{ PS2KEY_____L_SFT,PS2KEY_SLASH__QM },//5 _
				{ PS2KEY______NONE,PS2KEY_GRAV_TILD },//6 `
				{ PS2KEY______NONE,PS2KEY____a____A },//7 a
				{ PS2KEY______NONE,PS2KEY____b____B },//8 b
				{ PS2KEY______NONE,PS2KEY____c____C },//9 c
				// 10 ==================
				{ PS2KEY______NONE,PS2KEY____d____D },//0 d
				{ PS2KEY______NONE,PS2KEY____e____E },//1 e
				{ PS2KEY______NONE,PS2KEY____f____F },//2 f
				{ PS2KEY______NONE,PS2KEY____g____G },//3 g
				{ PS2KEY______NONE,PS2KEY____h____H },//4 h
				{ PS2KEY______NONE,PS2KEY____i____I },//5 i
				{ PS2KEY______NONE,PS2KEY____j____J },//6 j
				{ PS2KEY______NONE,PS2KEY____k____K },//7 k
				{ PS2KEY______NONE,PS2KEY____l____L },//8 l
				{ PS2KEY______NONE,PS2KEY____m____M },//9 m
				// 11 ==================
				{ PS2KEY______NONE,PS2KEY____n____N },//0 n
				{ PS2KEY______NONE,PS2KEY____o____O },//1 o
				{ PS2KEY______NONE,PS2KEY____p____P },//2 p
				{ PS2KEY______NONE,PS2KEY____q____Q },//3 q
				{ PS2KEY______NONE,PS2KEY____r____R },//4 r
				{ PS2KEY______NONE,PS2KEY____s____S },//5 s
				{ PS2KEY______NONE,PS2KEY____t____T },//6 t
				{ PS2KEY______NONE,PS2KEY____u____U },//7 u
				{ PS2KEY______NONE,PS2KEY____v____V },//8 v
				{ PS2KEY______NONE,PS2KEY____w____W },//9 w
				// 12 ==================
				{ PS2KEY______NONE,PS2KEY____x____X },//0 x
				{ PS2KEY______NONE,PS2KEY____y____Y },//1 y
				{ PS2KEY______NONE,PS2KEY____z____Z },//2 z
				{ PS2KEY_____L_SFT,PS2KEY_LBT___LBR },//3 {
				{ PS2KEY_____L_SFT,PS2KEY_BSLA_VBAR },//4 |
				{ PS2KEY_____L_SFT,PS2KEY_RBT___RBR },//5 }
				{ PS2KEY_____L_SFT,PS2KEY_GRAV_TILD },//6 ~
				{ 0,0 },//7
				{ 0,0 },//8
				{ 0,0 }//9
			},
			{	// xxx.Swedish PS2 keymap		//{ control key key,general key }
				// 0 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ 0,0 },//2
				{ 0,0 },//3
				{ 0,0 },//4
				{ 0,0 },//5
				{ 0,0 },//6
				{ 0,0 },//7
				{ PS2KEY______NONE,PS2KEY_BACKSPACE },//8
				{ PS2KEY______NONE,PS2KEY_______TAB },//9
				// 1 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ 0,0 },//2
				{ PS2KEY______NONE,PS2KEY____RETURN },//3 ... CR
				{ 0,0 },//4
				{ 0,0 },//5
				{ 0,0 },//6
				{ 0,0 },//7
				{ 0,0 },//8
				{ 0,0 },//9
				// 2 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ 0,0 },//2
				{ 0,0 },//3
				{ 0,0 },//4
				{ 0,0 },//5
				{ 0,0 },//6
				{ PS2KEY______NONE,PS2KEY____ESCAPE },//7
				{ 0,0 },//8
				{ 0,0 },//9
				// 3 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ PS2KEY______NONE,PS2KEY_____SPACE },//2
				{ PS2KEY_____L_SFT,PS2KEY____1_EXCL },//3 !
				{ PS2KEY_____L_SFT,PS2KEY____2_QUOT },//4 "
				{ PS2KEY_____L_SFT,PS2KEY____3_SHAR },//5 #
				{ PS2KEY_____R_ALT,PS2KEY____4_DOLL },//6 $
				{ PS2KEY_____L_SFT,PS2KEY____5_PERC },//7 %
				{ PS2KEY_____L_SFT,PS2KEY____6_CIRC },//8 &
				{ PS2KEY______NONE,0x5D },//9 '
				// 4 ==================
				{ PS2KEY_____L_SFT,PS2KEY____8_ASTE },//0 (
				{ PS2KEY_____L_SFT,PS2KEY____9_L_PA },//1 )
				{ PS2KEY_____L_SFT,0x5D },//2 *
				{ PS2KEY______NONE,PS2KEY_MIN_UNDER },//3 +
				{ PS2KEY______NONE,PS2KEY_COMA___LT },//4 ,
				{ PS2KEY______NONE,PS2KEY_SLASH__QM },//5 -
				{ PS2KEY______NONE,PS2KEY_PERIOD_GT },//6 .
				{ PS2KEY_____L_SFT,PS2KEY____7_AMPE },//7 /
				{ PS2KEY______NONE,PS2KEY____0_R_PA },//8 0
				{ PS2KEY______NONE,PS2KEY____1_EXCL },//9 1
				// 5 ==================
				{ PS2KEY______NONE,PS2KEY____2_QUOT },//0 2
				{ PS2KEY______NONE,PS2KEY____3_SHAR },//1 3
				{ PS2KEY______NONE,PS2KEY____4_DOLL },//2 4
				{ PS2KEY______NONE,PS2KEY____5_PERC },//3 5
				{ PS2KEY______NONE,PS2KEY____6_CIRC },//4 6
				{ PS2KEY______NONE,PS2KEY____7_AMPE },//5 7
				{ PS2KEY______NONE,PS2KEY____8_ASTE },//6 8
				{ PS2KEY______NONE,PS2KEY____9_L_PA },//7 9
				{ PS2KEY_____L_SFT,PS2KEY_PERIOD_GT },//8 :
				{ PS2KEY_____L_SFT,PS2KEY_COMA___LT },//9 ;
				// 6 ==================
				{ PS2KEY______NONE,0x61 },//0 <
				{ PS2KEY______NONE,PS2KEY_EQU__PLUS },//1 =
				{ PS2KEY_____L_SFT,0x61 },//2 >
				{ PS2KEY_____L_SFT,PS2KEY_MIN_UNDER },//3 ?
				{ PS2KEY_____R_ALT,PS2KEY____2_QUOT },//4 @
				{ PS2KEY_____L_SFT,PS2KEY____a____A },//5 A
				{ PS2KEY_____L_SFT,PS2KEY____b____B },//6 B
				{ PS2KEY_____L_SFT,PS2KEY____c____C },//7 C
				{ PS2KEY_____L_SFT,PS2KEY____d____D },//8 D
				{ PS2KEY_____L_SFT,PS2KEY____e____E },//9 E
				// 7 ==================
				{ PS2KEY_____L_SFT,PS2KEY____f____F },//0 F
				{ PS2KEY_____L_SFT,PS2KEY____g____G },//1 G
				{ PS2KEY_____L_SFT,PS2KEY____h____H },//2 H
				{ PS2KEY_____L_SFT,PS2KEY____i____I },//3 I
				{ PS2KEY_____L_SFT,PS2KEY____j____J },//4 J
				{ PS2KEY_____L_SFT,PS2KEY____k____K },//5 K
				{ PS2KEY_____L_SFT,PS2KEY____l____L },//6 L
				{ PS2KEY_____L_SFT,PS2KEY____m____M },//7 M
				{ PS2KEY_____L_SFT,PS2KEY____n____N },//8 N
				{ PS2KEY_____L_SFT,PS2KEY____o____O },//9 O
				// 8 ==================
				{ PS2KEY_____L_SFT,PS2KEY____p____P },//0 P
				{ PS2KEY_____L_SFT,PS2KEY____q____Q },//1 Q
				{ PS2KEY_____L_SFT,PS2KEY____r____R },//2 R
				{ PS2KEY_____L_SFT,PS2KEY____s____S },//3 S
				{ PS2KEY_____L_SFT,PS2KEY____t____T },//4 T
				{ PS2KEY_____L_SFT,PS2KEY____u____U },//5 U
				{ PS2KEY_____L_SFT,PS2KEY____v____V },//6 V
				{ PS2KEY_____L_SFT,PS2KEY____w____W },//7 W
				{ PS2KEY_____L_SFT,PS2KEY____x____X },//8 X
				{ PS2KEY_____L_SFT,PS2KEY____y____Y },//9 Y
				// 9 ==================
				{ PS2KEY_____L_SFT,PS2KEY____z____Z },//0 Z
				{ PS2KEY_____R_ALT,PS2KEY____8_ASTE },//1 [
				{ PS2KEY_____R_ALT,PS2KEY_MIN_UNDER },//2
				{ PS2KEY_____R_ALT,PS2KEY____9_L_PA },//3 ]
				{ PS2KEY_____L_SFT,PS2KEY_RBT___RBR },//4 ^
				{ PS2KEY_____L_SFT,PS2KEY_SLASH__QM },//5 _
				{ PS2KEY______NONE,PS2KEY_GRAV_TILD },//6 `
				{ PS2KEY______NONE,PS2KEY____a____A },//7 a
				{ PS2KEY______NONE,PS2KEY____b____B },//8 b
				{ PS2KEY______NONE,PS2KEY____c____C },//9 c
				// 10 ==================
				{ PS2KEY______NONE,PS2KEY____d____D },//0 d
				{ PS2KEY______NONE,PS2KEY____e____E },//1 e
				{ PS2KEY______NONE,PS2KEY____f____F },//2 f
				{ PS2KEY______NONE,PS2KEY____g____G },//3 g
				{ PS2KEY______NONE,PS2KEY____h____H },//4 h
				{ PS2KEY______NONE,PS2KEY____i____I },//5 i
				{ PS2KEY______NONE,PS2KEY____j____J },//6 j
				{ PS2KEY______NONE,PS2KEY____k____K },//7 k
				{ PS2KEY______NONE,PS2KEY____l____L },//8 l
				{ PS2KEY______NONE,PS2KEY____m____M },//9 m
				// 11 ==================
				{ PS2KEY______NONE,PS2KEY____n____N },//0 n
				{ PS2KEY______NONE,PS2KEY____o____O },//1 o
				{ PS2KEY______NONE,PS2KEY____p____P },//2 p
				{ PS2KEY______NONE,PS2KEY____q____Q },//3 q
				{ PS2KEY______NONE,PS2KEY____r____R },//4 r
				{ PS2KEY______NONE,PS2KEY____s____S },//5 s
				{ PS2KEY______NONE,PS2KEY____t____T },//6 t
				{ PS2KEY______NONE,PS2KEY____u____U },//7 u
				{ PS2KEY______NONE,PS2KEY____v____V },//8 v
				{ PS2KEY______NONE,PS2KEY____w____W },//9 w
				// 12 ==================
				{ PS2KEY______NONE,PS2KEY____x____X },//0 x
				{ PS2KEY______NONE,PS2KEY____y____Y },//1 y
				{ PS2KEY______NONE,PS2KEY____z____Z },//2 z
				{ PS2KEY_____L_SFT,PS2KEY_LBT___LBR },//3 {
				{ PS2KEY_____L_SFT,PS2KEY_BSLA_VBAR },//4 |
				{ PS2KEY_____L_SFT,PS2KEY_RBT___RBR },//5 }
				{ PS2KEY_____L_SFT,PS2KEY_GRAV_TILD },//6 ~
				{ 0,0 },//7
				{ 0,0 },//8
				{ 0,0 }//9
			},
			{	// xxx.UK_English PS2 keymap		//{ control key key,general key }
				// 0 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ 0,0 },//2
				{ 0,0 },//3
				{ 0,0 },//4
				{ 0,0 },//5
				{ 0,0 },//6
				{ 0,0 },//7
				{ PS2KEY______NONE,PS2KEY_BACKSPACE },//8
				{ PS2KEY______NONE,PS2KEY_______TAB },//9
				// 1 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ 0,0 },//2
				{ PS2KEY______NONE,PS2KEY____RETURN },//3 ... CR
				{ 0,0 },//4
				{ 0,0 },//5
				{ 0,0 },//6
				{ 0,0 },//7
				{ 0,0 },//8
				{ 0,0 },//9
				// 2 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ 0,0 },//2
				{ 0,0 },//3
				{ 0,0 },//4
				{ 0,0 },//5
				{ 0,0 },//6
				{ PS2KEY______NONE,PS2KEY____ESCAPE },//7
				{ 0,0 },//8
				{ 0,0 },//9
				// 3 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ PS2KEY______NONE,PS2KEY_____SPACE },//2
				{ PS2KEY_____L_SFT,PS2KEY____1_EXCL },//3 !
				{ PS2KEY_____L_SFT,PS2KEY____2_QUOT },//4 "
				{ PS2KEY______NONE,0x5D },//5 #
				{ PS2KEY_____L_SFT,PS2KEY____4_DOLL },//6 $
				{ PS2KEY_____L_SFT,PS2KEY____5_PERC },//7 %
				{ PS2KEY_____L_SFT,PS2KEY____7_AMPE },//8 &
				{ PS2KEY______NONE,PS2KEY_APOS_QUOT },//9 '
				// 4 ==================
				{ PS2KEY_____L_SFT,PS2KEY____9_L_PA },//0 (
				{ PS2KEY_____L_SFT,PS2KEY____0_R_PA },//1 )
				{ PS2KEY_____L_SFT,PS2KEY____8_ASTE },//2 *
				{ PS2KEY_____L_SFT,PS2KEY_EQU__PLUS },//3 +
				{ PS2KEY______NONE,PS2KEY_COMA___LT },//4 ,
				{ PS2KEY______NONE,PS2KEY_MIN_UNDER },//5 -
				{ PS2KEY______NONE,PS2KEY_PERIOD_GT },//6 .
				{ PS2KEY______NONE,PS2KEY_SLASH__QM },//7 /
				{ PS2KEY______NONE,PS2KEY____0_R_PA },//8 0
				{ PS2KEY______NONE,PS2KEY____1_EXCL },//9 1
				// 5 ==================
				{ PS2KEY______NONE,PS2KEY____2_QUOT },//0 2
				{ PS2KEY______NONE,PS2KEY____3_SHAR },//1 3
				{ PS2KEY______NONE,PS2KEY____4_DOLL },//2 4
				{ PS2KEY______NONE,PS2KEY____5_PERC },//3 5
				{ PS2KEY______NONE,PS2KEY____6_CIRC },//4 6
				{ PS2KEY______NONE,PS2KEY____7_AMPE },//5 7
				{ PS2KEY______NONE,PS2KEY____8_ASTE },//6 8
				{ PS2KEY______NONE,PS2KEY____9_L_PA },//7 9
				{ PS2KEY_____L_SFT,PS2KEY_SEMI__COL },//8 :
				{ PS2KEY______NONE,PS2KEY_SEMI__COL },//9 ;
				// 6 ==================
				{ PS2KEY_____L_SFT,PS2KEY_COMA___LT },//0 <
				{ PS2KEY______NONE,PS2KEY_EQU__PLUS },//1 =
				{ PS2KEY_____L_SFT,PS2KEY_PERIOD_GT },//2 >
				{ PS2KEY_____L_SFT,PS2KEY_SLASH__QM },//3 ?
				{ PS2KEY_____L_SFT,PS2KEY_APOS_QUOT },//4 @
				{ PS2KEY_____L_SFT,PS2KEY____a____A },//5 A
				{ PS2KEY_____L_SFT,PS2KEY____b____B },//6 B
				{ PS2KEY_____L_SFT,PS2KEY____c____C },//7 C
				{ PS2KEY_____L_SFT,PS2KEY____d____D },//8 D
				{ PS2KEY_____L_SFT,PS2KEY____e____E },//9 E
				// 7 ==================
				{ PS2KEY_____L_SFT,PS2KEY____f____F },//0 F
				{ PS2KEY_____L_SFT,PS2KEY____g____G },//1 G
				{ PS2KEY_____L_SFT,PS2KEY____h____H },//2 H
				{ PS2KEY_____L_SFT,PS2KEY____i____I },//3 I
				{ PS2KEY_____L_SFT,PS2KEY____j____J },//4 J
				{ PS2KEY_____L_SFT,PS2KEY____k____K },//5 K
				{ PS2KEY_____L_SFT,PS2KEY____l____L },//6 L
				{ PS2KEY_____L_SFT,PS2KEY____m____M },//7 M
				{ PS2KEY_____L_SFT,PS2KEY____n____N },//8 N
				{ PS2KEY_____L_SFT,PS2KEY____o____O },//9 O
				// 8 ==================
				{ PS2KEY_____L_SFT,PS2KEY____p____P },//0 P
				{ PS2KEY_____L_SFT,PS2KEY____q____Q },//1 Q
				{ PS2KEY_____L_SFT,PS2KEY____r____R },//2 R
				{ PS2KEY_____L_SFT,PS2KEY____s____S },//3 S
				{ PS2KEY_____L_SFT,PS2KEY____t____T },//4 T
				{ PS2KEY_____L_SFT,PS2KEY____u____U },//5 U
				{ PS2KEY_____L_SFT,PS2KEY____v____V },//6 V
				{ PS2KEY_____L_SFT,PS2KEY____w____W },//7 W
				{ PS2KEY_____L_SFT,PS2KEY____x____X },//8 X
				{ PS2KEY_____L_SFT,PS2KEY____y____Y },//9 Y
				// 9 ==================
				{ PS2KEY_____L_SFT,PS2KEY____z____Z },//0 Z
				{ PS2KEY______NONE,PS2KEY_LBT___LBR },//1 [
				{ PS2KEY______NONE,0x61 },//2
				{ PS2KEY______NONE,PS2KEY_RBT___RBR },//3 ]
				{ PS2KEY_____L_SFT,PS2KEY____6_CIRC },//4 ^
				{ PS2KEY_____L_SFT,PS2KEY_MIN_UNDER },//5 _
				{ PS2KEY______NONE,PS2KEY_GRAV_TILD },//6 `
				{ PS2KEY______NONE,PS2KEY____a____A },//7 a
				{ PS2KEY______NONE,PS2KEY____b____B },//8 b
				{ PS2KEY______NONE,PS2KEY____c____C },//9 c
				// 10 ==================
				{ PS2KEY______NONE,PS2KEY____d____D },//0 d
				{ PS2KEY______NONE,PS2KEY____e____E },//1 e
				{ PS2KEY______NONE,PS2KEY____f____F },//2 f
				{ PS2KEY______NONE,PS2KEY____g____G },//3 g
				{ PS2KEY______NONE,PS2KEY____h____H },//4 h
				{ PS2KEY______NONE,PS2KEY____i____I },//5 i
				{ PS2KEY______NONE,PS2KEY____j____J },//6 j
				{ PS2KEY______NONE,PS2KEY____k____K },//7 k
				{ PS2KEY______NONE,PS2KEY____l____L },//8 l
				{ PS2KEY______NONE,PS2KEY____m____M },//9 m
				// 11 ==================
				{ PS2KEY______NONE,PS2KEY____n____N },//0 n
				{ PS2KEY______NONE,PS2KEY____o____O },//1 o
				{ PS2KEY______NONE,PS2KEY____p____P },//2 p
				{ PS2KEY______NONE,PS2KEY____q____Q },//3 q
				{ PS2KEY______NONE,PS2KEY____r____R },//4 r
				{ PS2KEY______NONE,PS2KEY____s____S },//5 s
				{ PS2KEY______NONE,PS2KEY____t____T },//6 t
				{ PS2KEY______NONE,PS2KEY____u____U },//7 u
				{ PS2KEY______NONE,PS2KEY____v____V },//8 v
				{ PS2KEY______NONE,PS2KEY____w____W },//9 w
				// 12 ==================
				{ PS2KEY______NONE,PS2KEY____x____X },//0 x
				{ PS2KEY______NONE,PS2KEY____y____Y },//1 y
				{ PS2KEY______NONE,PS2KEY____z____Z },//2 z
				{ PS2KEY_____L_SFT,PS2KEY_LBT___LBR },//3 {
				{ PS2KEY_____L_SFT,PS2KEY_BSLA_VBAR },//4 |
				{ PS2KEY_____L_SFT,PS2KEY_RBT___RBR },//5 }
				{ PS2KEY_____L_SFT,PS2KEY_GRAV_TILD },//6 ~
				{ 0,0 },//7
				{ 0,0 },//8
				{ 0,0 }//9
			},
			{	// xxx.Israel PS2 keymap		//{ control key key,general key }
				// 0 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ 0,0 },//2
				{ 0,0 },//3
				{ 0,0 },//4
				{ 0,0 },//5
				{ 0,0 },//6
				{ 0,0 },//7
				{ PS2KEY______NONE,PS2KEY_BACKSPACE },//8
				{ PS2KEY______NONE,PS2KEY_______TAB },//9
				// 1 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ 0,0 },//2
				{ PS2KEY______NONE,PS2KEY____RETURN },//3 ... CR
				{ 0,0 },//4
				{ 0,0 },//5
				{ 0,0 },//6
				{ 0,0 },//7
				{ 0,0 },//8
				{ 0,0 },//9
				// 2 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ 0,0 },//2
				{ 0,0 },//3
				{ 0,0 },//4
				{ 0,0 },//5
				{ 0,0 },//6
				{ PS2KEY______NONE,PS2KEY____ESCAPE },//7
				{ 0,0 },//8
				{ 0,0 },//9
				// 3 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ PS2KEY______NONE,PS2KEY_____SPACE },//2
				{ PS2KEY_____L_SFT,PS2KEY____1_EXCL },//3 !
				{ PS2KEY_____L_SFT,PS2KEY_APOS_QUOT },//4 "
				{ PS2KEY_____L_SFT,PS2KEY____3_SHAR },//5 #
				{ PS2KEY_____L_SFT,PS2KEY____4_DOLL },//6 $
				{ PS2KEY_____L_SFT,PS2KEY____5_PERC },//7 %
				{ PS2KEY_____L_SFT,PS2KEY____7_AMPE },//8 &
				{ PS2KEY______NONE,PS2KEY____w____W },//9 '
				// 4 ==================
				{ PS2KEY_____L_SFT,PS2KEY____0_R_PA },//0 (
				{ PS2KEY_____L_SFT,PS2KEY____9_L_PA },//1 )
				{ PS2KEY_____L_SFT,PS2KEY____8_ASTE },//2 *
				{ PS2KEY_____L_SFT,PS2KEY_EQU__PLUS },//3 +
				{ PS2KEY______NONE,PS2KEY_APOS_QUOT },//4 ,
				{ PS2KEY______NONE,PS2KEY_MIN_UNDER },//5 -
				{ PS2KEY______NONE,PS2KEY_SLASH__QM },//6 .
				{ PS2KEY______NONE,PS2KEY____q____Q },//7 /
				{ PS2KEY______NONE,PS2KEY____0_R_PA },//8 0
				{ PS2KEY______NONE,PS2KEY____1_EXCL },//9 1
				// 5 ==================
				{ PS2KEY______NONE,PS2KEY____2_QUOT },//0 2
				{ PS2KEY______NONE,PS2KEY____3_SHAR },//1 3
				{ PS2KEY______NONE,PS2KEY____4_DOLL },//2 4
				{ PS2KEY______NONE,PS2KEY____5_PERC },//3 5
				{ PS2KEY______NONE,PS2KEY____6_CIRC },//4 6
				{ PS2KEY______NONE,PS2KEY____7_AMPE },//5 7
				{ PS2KEY______NONE,PS2KEY____8_ASTE },//6 8
				{ PS2KEY______NONE,PS2KEY____9_L_PA },//7 9
				{ PS2KEY_____L_SFT,PS2KEY_SEMI__COL },//8 :
				{ PS2KEY______NONE,PS2KEY_GRAV_TILD },//9 ;
				// 6 ==================
				{ PS2KEY_____L_SFT,PS2KEY_PERIOD_GT },//0 <
				{ PS2KEY______NONE,PS2KEY_EQU__PLUS },//1 =
				{ PS2KEY_____L_SFT,PS2KEY_COMA___LT },//2 >
				{ PS2KEY_____L_SFT,PS2KEY_SLASH__QM },//3 ?
				{ PS2KEY_____L_SFT,PS2KEY____2_QUOT },//4 @
				{ PS2KEY_____L_SFT,PS2KEY____a____A },//5 A
				{ PS2KEY_____L_SFT,PS2KEY____b____B },//6 B
				{ PS2KEY_____L_SFT,PS2KEY____c____C },//7 C
				{ PS2KEY_____L_SFT,PS2KEY____d____D },//8 D
				{ PS2KEY_____L_SFT,PS2KEY____e____E },//9 E
				// 7 ==================
				{ PS2KEY_____L_SFT,PS2KEY____f____F },//0 F
				{ PS2KEY_____L_SFT,PS2KEY____g____G },//1 G
				{ PS2KEY_____L_SFT,PS2KEY____h____H },//2 H
				{ PS2KEY_____L_SFT,PS2KEY____i____I },//3 I
				{ PS2KEY_____L_SFT,PS2KEY____j____J },//4 J
				{ PS2KEY_____L_SFT,PS2KEY____k____K },//5 K
				{ PS2KEY_____L_SFT,PS2KEY____l____L },//6 L
				{ PS2KEY_____L_SFT,PS2KEY____m____M },//7 M
				{ PS2KEY_____L_SFT,PS2KEY____n____N },//8 N
				{ PS2KEY_____L_SFT,PS2KEY____o____O },//9 O
				// 8 ==================
				{ PS2KEY_____L_SFT,PS2KEY____p____P },//0 P
				{ PS2KEY_____L_SFT,PS2KEY____q____Q },//1 Q
				{ PS2KEY_____L_SFT,PS2KEY____r____R },//2 R
				{ PS2KEY_____L_SFT,PS2KEY____s____S },//3 S
				{ PS2KEY_____L_SFT,PS2KEY____t____T },//4 T
				{ PS2KEY_____L_SFT,PS2KEY____u____U },//5 U
				{ PS2KEY_____L_SFT,PS2KEY____v____V },//6 V
				{ PS2KEY_____L_SFT,PS2KEY____w____W },//7 W
				{ PS2KEY_____L_SFT,PS2KEY____x____X },//8 X
				{ PS2KEY_____L_SFT,PS2KEY____y____Y },//9 Y
				// 9 ==================
				{ PS2KEY_____L_SFT,PS2KEY____z____Z },//0 Z
				{ PS2KEY______NONE,PS2KEY_RBT___RBR },//1 [
				{ PS2KEY______NONE,0x61 },//2
				{ PS2KEY______NONE,PS2KEY_LBT___LBR },//3 ]
				{ PS2KEY_____L_SFT,PS2KEY____6_CIRC },//4 ^
				{ PS2KEY_____L_SFT,PS2KEY_MIN_UNDER },//5 _
				{ PS2KEY______NONE,PS2KEY_GRAV_TILD },//6 `
				{ PS2KEY______NONE,PS2KEY____a____A },//7 a
				{ PS2KEY______NONE,PS2KEY____b____B },//8 b
				{ PS2KEY______NONE,PS2KEY____c____C },//9 c
				// 10 ==================
				{ PS2KEY______NONE,PS2KEY____d____D },//0 d
				{ PS2KEY______NONE,PS2KEY____e____E },//1 e
				{ PS2KEY______NONE,PS2KEY____f____F },//2 f
				{ PS2KEY______NONE,PS2KEY____g____G },//3 g
				{ PS2KEY______NONE,PS2KEY____h____H },//4 h
				{ PS2KEY______NONE,PS2KEY____i____I },//5 i
				{ PS2KEY______NONE,PS2KEY____j____J },//6 j
				{ PS2KEY______NONE,PS2KEY____k____K },//7 k
				{ PS2KEY______NONE,PS2KEY____l____L },//8 l
				{ PS2KEY______NONE,PS2KEY____m____M },//9 m
				// 11 ==================
				{ PS2KEY______NONE,PS2KEY____n____N },//0 n
				{ PS2KEY______NONE,PS2KEY____o____O },//1 o
				{ PS2KEY______NONE,PS2KEY____p____P },//2 p
				{ PS2KEY______NONE,PS2KEY____q____Q },//3 q
				{ PS2KEY______NONE,PS2KEY____r____R },//4 r
				{ PS2KEY______NONE,PS2KEY____s____S },//5 s
				{ PS2KEY______NONE,PS2KEY____t____T },//6 t
				{ PS2KEY______NONE,PS2KEY____u____U },//7 u
				{ PS2KEY______NONE,PS2KEY____v____V },//8 v
				{ PS2KEY______NONE,PS2KEY____w____W },//9 w
				// 12 ==================
				{ PS2KEY______NONE,PS2KEY____x____X },//0 x
				{ PS2KEY______NONE,PS2KEY____y____Y },//1 y
				{ PS2KEY______NONE,PS2KEY____z____Z },//2 z
				{ PS2KEY_____L_SFT,PS2KEY_LBT___LBR },//3 {
				{ PS2KEY_____L_SFT,PS2KEY_BSLA_VBAR },//4 |
				{ PS2KEY_____L_SFT,PS2KEY_RBT___RBR },//5 }
				{ PS2KEY_____L_SFT,PS2KEY_GRAV_TILD },//6 ~
				{ 0,0 },//7
				{ 0,0 },//8
				{ 0,0 }//9
			},
			{	// xxx.Turkey PS2 keymap		//{ control key key,general key }
				// 0 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ 0,0 },//2
				{ 0,0 },//3
				{ 0,0 },//4
				{ 0,0 },//5
				{ 0,0 },//6
				{ 0,0 },//7
				{ PS2KEY______NONE,PS2KEY_BACKSPACE },//8
				{ PS2KEY______NONE,PS2KEY_______TAB },//9
				// 1 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ 0,0 },//2
				{ PS2KEY______NONE,PS2KEY____RETURN },//3 ... CR
				{ 0,0 },//4
				{ 0,0 },//5
				{ 0,0 },//6
				{ 0,0 },//7
				{ 0,0 },//8
				{ 0,0 },//9
				// 2 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ 0,0 },//2
				{ 0,0 },//3
				{ 0,0 },//4
				{ 0,0 },//5
				{ 0,0 },//6
				{ PS2KEY______NONE,PS2KEY____ESCAPE },//7
				{ 0,0 },//8
				{ 0,0 },//9
				// 3 ==================
				{ 0,0 },//0
				{ 0,0 },//1
				{ PS2KEY______NONE,PS2KEY_____SPACE },//2
				{ PS2KEY_____L_SFT,PS2KEY____1_EXCL },//3 !
				{ PS2KEY______NONE,PS2KEY_GRAV_TILD },//4 "
				{ PS2KEY_____R_ALT,PS2KEY____3_SHAR },//5 #
				{ PS2KEY_____R_ALT,PS2KEY____4_DOLL },//6 $
				{ PS2KEY_____L_SFT,PS2KEY____5_PERC },//7 %
				{ PS2KEY_____L_SFT,PS2KEY____6_CIRC },//8 &
				{ PS2KEY_____L_SFT,PS2KEY____2_QUOT },//9 '
				// 4 ==================
				{ PS2KEY_____L_SFT,PS2KEY____8_ASTE },//0 (
				{ PS2KEY_____L_SFT,PS2KEY____9_L_PA },//1 )
				{ PS2KEY______NONE,PS2KEY_MIN_UNDER },//2 *
				{ PS2KEY_____L_SFT,PS2KEY____4_DOLL },//3 +
				{ PS2KEY______NONE,0x5D },//4 ,
				{ PS2KEY______NONE,PS2KEY_EQU__PLUS },//5 -
				{ PS2KEY______NONE,PS2KEY_SLASH__QM },//6 .
				{ PS2KEY_____L_SFT,PS2KEY____7_AMPE },//7 /
				{ PS2KEY______NONE,PS2KEY____0_R_PA },//8 0
				{ PS2KEY______NONE,PS2KEY____1_EXCL },//9 1
				// 5 ==================
				{ PS2KEY______NONE,PS2KEY____2_QUOT },//0 2
				{ PS2KEY______NONE,PS2KEY____3_SHAR },//1 3
				{ PS2KEY______NONE,PS2KEY____4_DOLL },//2 4
				{ PS2KEY______NONE,PS2KEY____5_PERC },//3 5
				{ PS2KEY______NONE,PS2KEY____6_CIRC },//4 6
				{ PS2KEY______NONE,PS2KEY____7_AMPE },//5 7
				{ PS2KEY______NONE,PS2KEY____8_ASTE },//6 8
				{ PS2KEY______NONE,PS2KEY____9_L_PA },//7 9
				{ PS2KEY_____L_SFT,PS2KEY_SLASH__QM },//8 :
				{ PS2KEY_____L_SFT,0x5D },//9 ;
				// 6 ==================
				{ PS2KEY______NONE,0x61 },//0 <
				{ PS2KEY_____L_SFT,PS2KEY____0_R_PA },//1 =
				{ PS2KEY_____L_SFT,0x61 },//2 >
				{ PS2KEY_____L_SFT,PS2KEY_MIN_UNDER },//3 ?
				{ PS2KEY_____R_ALT,PS2KEY____q____Q },//4 @
				{ PS2KEY_____L_SFT,PS2KEY____a____A },//5 A
				{ PS2KEY_____L_SFT,PS2KEY____b____B },//6 B
				{ PS2KEY_____L_SFT,PS2KEY____c____C },//7 C
				{ PS2KEY_____L_SFT,PS2KEY____d____D },//8 D
				{ PS2KEY_____L_SFT,PS2KEY____e____E },//9 E
				// 7 ==================
				{ PS2KEY_____L_SFT,PS2KEY____f____F },//0 F
				{ PS2KEY_____L_SFT,PS2KEY____g____G },//1 G
				{ PS2KEY_____L_SFT,PS2KEY____h____H },//2 H
				{ PS2KEY_____L_SFT,PS2KEY____i____I },//3 I
				{ PS2KEY_____L_SFT,PS2KEY____j____J },//4 J
				{ PS2KEY_____L_SFT,PS2KEY____k____K },//5 K
				{ PS2KEY_____L_SFT,PS2KEY____l____L },//6 L
				{ PS2KEY_____L_SFT,PS2KEY____m____M },//7 M
				{ PS2KEY_____L_SFT,PS2KEY____n____N },//8 N
				{ PS2KEY_____L_SFT,PS2KEY____o____O },//9 O
				// 8 ==================
				{ PS2KEY_____L_SFT,PS2KEY____p____P },//0 P
				{ PS2KEY_____L_SFT,PS2KEY____q____Q },//1 Q
				{ PS2KEY_____L_SFT,PS2KEY____r____R },//2 R
				{ PS2KEY_____L_SFT,PS2KEY____s____S },//3 S
				{ PS2KEY_____L_SFT,PS2KEY____t____T },//4 T
				{ PS2KEY_____L_SFT,PS2KEY____u____U },//5 U
				{ PS2KEY_____L_SFT,PS2KEY____v____V },//6 V
				{ PS2KEY_____L_SFT,PS2KEY____w____W },//7 W
				{ PS2KEY_____L_SFT,PS2KEY____x____X },//8 X
				{ PS2KEY_____L_SFT,PS2KEY____y____Y },//9 Y
				// 9 ==================
				{ PS2KEY_____L_SFT,PS2KEY____z____Z },//0 Z
				{ PS2KEY_____R_ALT,PS2KEY____8_ASTE },//1 [
				{ PS2KEY_____R_ALT,PS2KEY_MIN_UNDER },//2
				{ PS2KEY_____R_ALT,PS2KEY____9_L_PA },//3 ]
				{ PS2KEY_____L_SFT,PS2KEY____3_SHAR },//4 ^
				{ PS2KEY_____L_SFT,PS2KEY_EQU__PLUS },//5 _
				{ PS2KEY______NONE,PS2KEY_GRAV_TILD },//6 `
				{ PS2KEY______NONE,PS2KEY____a____A },//7 a
				{ PS2KEY______NONE,PS2KEY____b____B },//8 b
				{ PS2KEY______NONE,PS2KEY____c____C },//9 c
				// 10 ==================
				{ PS2KEY______NONE,PS2KEY____d____D },//0 d
				{ PS2KEY______NONE,PS2KEY____e____E },//1 e
				{ PS2KEY______NONE,PS2KEY____f____F },//2 f
				{ PS2KEY______NONE,PS2KEY____g____G },//3 g
				{ PS2KEY______NONE,PS2KEY____h____H },//4 h
				{ PS2KEY______NONE,PS2KEY____i____I },//5 i
				{ PS2KEY______NONE,PS2KEY____j____J },//6 j
				{ PS2KEY______NONE,PS2KEY____k____K },//7 k
				{ PS2KEY______NONE,PS2KEY____l____L },//8 l
				{ PS2KEY______NONE,PS2KEY____m____M },//9 m
				// 11 ==================
				{ PS2KEY______NONE,PS2KEY____n____N },//0 n
				{ PS2KEY______NONE,PS2KEY____o____O },//1 o
				{ PS2KEY______NONE,PS2KEY____p____P },//2 p
				{ PS2KEY______NONE,PS2KEY____q____Q },//3 q
				{ PS2KEY______NONE,PS2KEY____r____R },//4 r
				{ PS2KEY______NONE,PS2KEY____s____S },//5 s
				{ PS2KEY______NONE,PS2KEY____t____T },//6 t
				{ PS2KEY______NONE,PS2KEY____u____U },//7 u
				{ PS2KEY______NONE,PS2KEY____v____V },//8 v
				{ PS2KEY______NONE,PS2KEY____w____W },//9 w
				// 12 ==================
				{ PS2KEY______NONE,PS2KEY____x____X },//0 x
				{ PS2KEY______NONE,PS2KEY____y____Y },//1 y
				{ PS2KEY______NONE,PS2KEY____z____Z },//2 z
				{ PS2KEY_____L_SFT,PS2KEY_LBT___LBR },//3 {
				{ PS2KEY_____L_SFT,PS2KEY_BSLA_VBAR },//4 |
				{ PS2KEY_____L_SFT,PS2KEY_RBT___RBR },//5 }
				{ PS2KEY_____L_SFT,PS2KEY_GRAV_TILD },//6 ~
				{ 0,0 },//7
				{ 0,0 },//8
				{ 0,0 }//9
			}
		};
#ifdef	WIN32
#pragma pack(pop)
#endif	//WIN32
		return &sASCToPS2KeyMap[n_language][n_ascii][n_item];
	}

public:
	~ckey_map() {}
private:
	ckey_map();
	ckey_map(const ckey_map&);
	ckey_map& operator=(const ckey_map&);
};
