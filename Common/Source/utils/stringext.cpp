/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/
//______________________________________________________________________________

#include "stringext.h"
#include <math.h>
#include <string.h>
#include <algorithm>
#include "utf8/unchecked.h"

#include "utils/heapcheck.h"

//______________________________________________________________________________

/// maximum UTF-16 code convertable through utf16toAscii[] map
static const unsigned int maxUtf16toAscii = 0x024F;

/// UTF-16 (0000-024F ~ C0, C1, Latin Extended-A, Latin Extended-B) character
/// to US-ASCII conversion table
static const char utf16toAscii[maxUtf16toAscii + 1] =
{
  '?',  // 0000 - Null character NUL
  '?',  // 0001 - Start of Heading SOH
  '?',  // 0002 - Start of Text STX
  '?',  // 0003 - End-of-text character ETX
  '?',  // 0004 - End-of-transmission character EOT
  '?',  // 0005 - Enquiry character ENQ
  '?',  // 0006 - Acknowledge character ACK
  '?',  // 0007 - Bell character BEL
  '?',  // 0008 - Backspace BS
  '?',  // 0009 - Horizontal tab HT
  '?',  // 000A - Line feed LF
  '?',  // 000B - Vertical tab VT
  '?',  // 000C - Form feed FF
  '?',  // 000D - Carriage return CR
  '?',  // 000E - Shift Out SO
  '?',  // 000F - Shift In SI
  '?',  // 0010 - Data Link Escape DLE
  '?',  // 0011 - Device Control 1 DC1
  '?',  // 0012 - Device Control 2 DC2
  '?',  // 0013 - Device Control 3 DC3
  '?',  // 0014 - Device Control 4 DC4
  '?',  // 0015 - Negative-acknowledge character NAK
  '?',  // 0016 - Synchronous Idle SYN
  '?',  // 0017 - End of Transmission Block ETB
  '?',  // 0018 - Cancel character CAN
  '?',  // 0019 - End of Medium EM
  '?',  // 001A - Substitute character SUB
  '?',  // 001B - Escape character ESC
  '?',  // 001C - File Separator FS
  '?',  // 001D - Group Separator GS
  '?',  // 001E - Record Separator RS
  '?',  // 001F - Unit Separator US
  ' ',  // 0020   Space SP
  '!',  // 0021 ! Exclamation mark
  '"',  // 0022 " Quotation mark
  '#',  // 0023 # Number sign
  '$',  // 0024 $ Dollar sign
  '%',  // 0025 % Percent sign
  '&',  // 0026 & Ampersand
  '\'', // 0027 ' Apostrophe
  '(',  // 0028 ( Left parenthesis
  ')',  // 0029 ) Right parenthesis
  '*',  // 002A * Asterisk
  '+',  // 002B + Plus sign
  ',',  // 002C , Comma
  '-',  // 002D - Hyphen-minus
  '.',  // 002E . Full stop
  '/',  // 002F / Slash
  '0',  // 0030 0 Digit Zero
  '1',  // 0031 1 Digit One
  '2',  // 0032 2 Digit Two
  '3',  // 0033 3 Digit Three
  '4',  // 0034 4 Digit Four
  '5',  // 0035 5 Digit Five
  '6',  // 0036 6 Digit Six
  '7',  // 0037 7 Digit Seven
  '8',  // 0038 8 Digit Eight
  '9',  // 0039 9 Digit Nine
  ':',  // 003A : Colon
  ';',  // 003B ; Semicolon
  '<',  // 003C < Less-than sign
  '=',  // 003D = Equal sign
  '>',  // 003E > Greater-than sign
  '?',  // 003F ? Question mark
  '@',  // 0040 @ At sign
  'A',  // 0041 A Latin Capital letter A
  'B',  // 0042 B Latin Capital letter B
  'C',  // 0043 C Latin Capital letter C
  'D',  // 0044 D Latin Capital letter D
  'E',  // 0045 E Latin Capital letter E
  'F',  // 0046 F Latin Capital letter F
  'G',  // 0047 G Latin Capital letter G
  'H',  // 0048 H Latin Capital letter H
  'I',  // 0049 I Latin Capital letter I
  'J',  // 004A J Latin Capital letter J
  'K',  // 004B K Latin Capital letter K
  'L',  // 004C L Latin Capital letter L
  'M',  // 004D M Latin Capital letter M
  'N',  // 004E N Latin Capital letter N
  'O',  // 004F O Latin Capital letter O
  'P',  // 0050 P Latin Capital letter P
  'Q',  // 0051 Q Latin Capital letter Q
  'R',  // 0052 R Latin Capital letter R
  'S',  // 0053 S Latin Capital letter S
  'T',  // 0054 T Latin Capital letter T
  'U',  // 0055 U Latin Capital letter U
  'V',  // 0056 V Latin Capital letter V
  'W',  // 0057 W Latin Capital letter W
  'X',  // 0058 X Latin Capital letter X
  'Y',  // 0059 Y Latin Capital letter Y
  'Z',  // 005A Z Latin Capital letter Z
  '[',  // 005B [ Left Square Bracket
  '\\', // 005C \ Backslash
  ']',  // 005D ] Right Square Bracket
  '^',  // 005E ^ Circumflex accent
  '_',  // 005F _ Low line
  '`',  // 0060 ` Grave accent
  'a',  // 0061 a Latin Small Letter A
  'b',  // 0062 b Latin Small Letter B
  'c',  // 0063 c Latin Small Letter C
  'd',  // 0064 d Latin Small Letter D
  'e',  // 0065 e Latin Small Letter E
  'f',  // 0066 f Latin Small Letter F
  'g',  // 0067 g Latin Small Letter G
  'h',  // 0068 h Latin Small Letter H
  'i',  // 0069 i Latin Small Letter I
  'j',  // 006A j Latin Small Letter J
  'k',  // 006B k Latin Small Letter K
  'l',  // 006C l Latin Small Letter L
  'm',  // 006D m Latin Small Letter M
  'n',  // 006E n Latin Small Letter N
  'o',  // 006F o Latin Small Letter O
  'p',  // 0070 p Latin Small Letter P
  'q',  // 0071 q Latin Small Letter Q
  'r',  // 0072 r Latin Small Letter R
  's',  // 0073 s Latin Small Letter S
  't',  // 0074 t Latin Small Letter T
  'u',  // 0075 u Latin Small Letter U
  'v',  // 0076 v Latin Small Letter V
  'w',  // 0077 w Latin Small Letter W
  'x',  // 0078 x Latin Small Letter X
  'y',  // 0079 y Latin Small Letter Y
  'z',  // 007A z Latin Small Letter Z
  '{',  // 007B { Left Curly Bracket
  '|',  // 007C | Vertical bar
  '}',  // 007D } Right Curly Bracket
  '~',  // 007E ~ Tilde
  '?',  // 007F - Delete DEL
  '?',  // 0080 - Padding Character
  '?',  // 0081 - High Octet Preset
  '?',  // 0082 - Break Permitted Here
  '?',  // 0083 - No Break Here
  '?',  // 0084 - Index
  '?',  // 0085 - Next Line
  '?',  // 0086 - Start of Selected Area
  '?',  // 0087 - End of Selected Area
  '?',  // 0088 - Character Tabulation Set
  '?',  // 0089 - Character Tabulation with Justification
  '?',  // 008A - Line Tabulation Set
  '?',  // 008B - Partial Line Forward
  '?',  // 008C - Partial Line Backward
  '?',  // 008D - Reverse Line Feed
  '?',  // 008E - Single-Shift Two
  '?',  // 008F - Single-Shift Three
  '?',  // 0090 - Device Control String
  '?',  // 0091 - Private Use 1
  '?',  // 0092 - Private Use 2
  '?',  // 0093 - Set Transmit State
  '?',  // 0094 - Cancel character
  '?',  // 0095 - Message Waiting
  '?',  // 0096 - Start of Protected Area
  '?',  // 0097 - End of Protected Area
  '?',  // 0098 - Start of String
  '?',  // 0099 - Single Graphic Character Introducer
  '?',  // 009A - Single Character Intro Introducer
  '?',  // 009B - Control Sequence Introducer
  '?',  // 009C - String Terminator
  '?',  // 009D - Operating System Command
  '?',  // 009E - Private Message
  '?',  // 009F - Application Program Command
  '?',  // 00A0 -  Non-breaking space
  '!',  // 00A1 ! Inverted Exclamation Mark
  'c',  // 00A2 c Cent sign
  'L',  // 00A3 L Pound sign
  '*',  // 00A4 � Currency sign
  'Y',  // 00A5 Y Yen sign
  '!',  // 00A6 � Broken bar
  'S',  // 00A7 � Section sign
  '"',  // 00A8 � Diaeresis
  'c',  // 00A9 � Copyright sign
  'a',  // 00AA a Feminine Ordinal Indicator
  '<',  // 00AB � Left-pointing double angle quotation mark
  '-',  // 00AC � Not sign
  '-',  // 00AD � Soft hyphen
  '-',  // 00AE � Registered sign
  '-',  // 00AF � Macron
  'o',  // 00B0 � Degree symbol
  '+',  // 00B1 � Plus-minus sign
  '2',  // 00B2 2 Superscript two
  '3',  // 00B3 3 Superscript three
  '`',  // 00B4 � Acute accent
  'u',  // 00B5 � Micro sign
  '?',  // 00B6 � Pilcrow sign
  '.',  // 00B7 � Middle dot
  ',',  // 00B8 � Cedilla
  '1',  // 00B9 1 Superscript one
  'o',  // 00BA o Masculine ordinal indicator
  '>',  // 00BB � Right-pointing double-angle quotation mark
  '?',  // 00BC 1 Vulgar fraction one quarter
  '?',  // 00BD 1 Vulgar fraction one half
  '?',  // 00BE 3 Vulgar fraction three quarters
  '?',  // 00BF ? Inverted Question Mark
  'A',  // 00C0 A Latin Capital Letter A with grave
  'A',  // 00C1 � Latin Capital letter A with acute
  'A',  // 00C2 � Latin Capital letter A with circumflex
  'A',  // 00C3 A Latin Capital letter A with tilde
  'A',  // 00C4 � Latin Capital letter A with diaeresis
  'A',  // 00C5 A Latin Capital letter A with ring above
  'A',  // 00C6 A Latin Capital letter AE
  'C',  // 00C7 � Latin Capital letter C with cedilla
  'E',  // 00C8 E Latin Capital letter E with grave
  'E',  // 00C9 � Latin Capital letter E with acute
  'E',  // 00CA E Latin Capital letter E with circumflex
  'E',  // 00CB � Latin Capital letter E with diaeresis
  'I',  // 00CC I Latin Capital letter I with grave
  'I',  // 00CD � Latin Capital letter I with acute
  'I',  // 00CE � Latin Capital letter I with circumflex
  'I',  // 00CF I Latin Capital letter I with diaeresis
  '?',  // 00D0 ? Latin Capital letter Eth
  'N',  // 00D1 N Latin Capital letter N with tilde
  'O',  // 00D2 O Latin Capital letter O with grave
  'O',  // 00D3 � Latin Capital letter O with acute
  'O',  // 00D4 � Latin Capital letter O with circumflex
  'O',  // 00D5 O Latin Capital letter O with tilde
  'O',  // 00D6 � Latin Capital letter O with diaeresis
  'x',  // 00D7 � Multiplication sign
  'O',  // 00D8 O Latin Capital letter O with stroke
  'U',  // 00D9 U Latin Capital letter U with grave
  'U',  // 00DA � Latin Capital letter U with acute
  'U',  // 00DB U Latin Capital Letter U with circumflex
  'U',  // 00DC � Latin Capital Letter U with diaeresis
  'Y',  // 00DD � Latin Capital Letter Y with acute
  '?',  // 00DE ? Latin Capital Letter Thorn
  's',  // 00DF � Latin Small Letter sharp S
  'a',  // 00E0 a Latin Small Letter A with grave
  'a',  // 00E1 � Latin Small Letter A with acute
  'a',  // 00E2 � Latin Small Letter A with circumflex
  'a',  // 00E3 a Latin Small Letter A with tilde
  'a',  // 00E4 � Latin Small Letter A with diaeresis
  'a',  // 00E5 a Latin Small Letter A with ring above
  'a',  // 00E6 a Latin Small Letter AE
  'c',  // 00E7 � Latin Small Letter C with cedilla
  'e',  // 00E8 e Latin Small Letter E with grave
  'e',  // 00E9 � Latin Small Letter E with acute
  'e',  // 00EA e Latin Small Letter E with circumflex
  'e',  // 00EB � Latin Small Letter E with diaeresis
  'i',  // 00EC i Latin Small Letter I with grave
  'i',  // 00ED � Latin Small Letter I with acute
  'i',  // 00EE � Latin Small Letter I with circumflex
  'i',  // 00EF i Latin Small Letter I with diaeresis
  '?',  // 00F0 ? Latin Small Letter Eth
  'n',  // 00F1 n Latin Small Letter N with tilde
  'o',  // 00F2 o Latin Small Letter O with grave
  'o',  // 00F3 � Latin Small Letter O with acute
  'o',  // 00F4 � Latin Small Letter O with circumflex
  'o',  // 00F5 o Latin Small Letter O with tilde
  'o',  // 00F6 � Latin Small Letter O with diaeresis
  '-',  // 00F7 � Division sign
  'o',  // 00F8 o Latin Small Letter O with stroke
  'u',  // 00F9 u Latin Small Letter U with grave
  'u',  // 00FA � Latin Small Letter U with acute
  'u',  // 00FB u Latin Small Letter U with circumflex
  'u',  // 00FC � Latin Small Letter U with diaeresis
  'y',  // 00FD � Latin Small Letter Y with acute
  '?',  // 00FE ? Latin Small Letter Thorn
  'y',  // 00FF y Latin Small Letter Y with diaeresis
  'A',  // 0100 A Latin Capital Letter A with macron
  'a',  // 0101 a Latin Small Letter A with macron
  'A',  // 0102 � Latin Capital Letter A with breve
  'a',  // 0103 � Latin Small Letter A with breve
  'A',  // 0104 � Latin Capital Letter A with ogonek
  'a',  // 0105 � Latin Small Letter A with ogonek
  'C',  // 0106 � Latin Capital Letter C with acute
  'c',  // 0107 � Latin Small Letter C with acute
  'C',  // 0108 C Latin Capital Letter C with circumflex
  'c',  // 0109 c Latin Small Letter C with circumflex
  'C',  // 010A C Latin Capital Letter C with dot above
  'c',  // 010B c Latin Small Letter C with dot above
  'C',  // 010C � Latin Capital Letter C with caron
  'c',  // 010D � Latin Small Letter C with caron
  'D',  // 010E � Latin Capital Letter D with caron
  'd',  // 010F � Latin Small Letter D with caron
  'D',  // 0110 � Latin Capital Letter D with stroke
  'd',  // 0111 � Latin Small Letter D with stroke
  'E',  // 0112 E Latin Capital Letter E with macron
  'e',  // 0113 e Latin Small Letter E with macron
  'E',  // 0114 E Latin Capital Letter E with breve
  'e',  // 0115 e Latin Small Letter E with breve
  'E',  // 0116 E Latin Capital Letter E with dot above
  'e',  // 0117 e Latin Small Letter E with dot above
  'E',  // 0118 � Latin Capital Letter E with ogonek
  'e',  // 0119 � Latin Small Letter E with ogonek
  'E',  // 011A � Latin Capital Letter E with caron
  'e',  // 011B � Latin Small Letter E with caron
  'G',  // 011C G Latin Capital Letter G with circumflex
  'g',  // 011D g Latin Small Letter G with circumflex
  'G',  // 011E G Latin Capital Letter G with breve
  'g',  // 011F g Latin Small Letter G with breve
  'G',  // 0120 G Latin Capital Letter G with dot above
  'g',  // 0121 g Latin Small Letter G with dot above
  'G',  // 0122 G Latin Capital Letter G with cedilla
  'g',  // 0123 g Latin Small Letter G with cedilla
  'H',  // 0124 H Latin Capital Letter H with circumflex
  'h',  // 0125 h Latin Small Letter H with circumflex
  'H',  // 0126 H Latin Capital Letter H with stroke
  'h',  // 0127 h Latin Small Letter H with stroke
  'I',  // 0128 I Latin Capital Letter I with tilde
  'i',  // 0129 i Latin Small Letter I with tilde
  'I',  // 012A I Latin Capital Letter I with macron
  'i',  // 012B i Latin Small Letter I with macron
  'I',  // 012C I Latin Capital Letter I with breve
  'i',  // 012D i Latin Small Letter I with breve
  'I',  // 012E I Latin Capital Letter I with ogonek
  'i',  // 012F i Latin Small Letter I with ogonek
  'I',  // 0130 I Latin Capital Letter I with dot above
  'i',  // 0131 i Latin Small Letter dotless I
  '?',  // 0132 ? Latin Capital Ligature IJ
  '?',  // 0133 ? Latin Small Ligature IJ
  'J',  // 0134 J Latin Capital Letter J with circumflex
  'j',  // 0135 j Latin Small Letter J with circumflex
  'K',  // 0136 K Latin Capital Letter K with cedilla
  'k',  // 0137 k Latin Small Letter K with cedilla
  'K',  // 0138 ? Latin Small Letter Kra
  'L',  // 0139 � Latin Capital Letter L with acute
  'l',  // 013A � Latin Small Letter L with acute
  'L',  // 013B L Latin Capital Letter L with cedilla
  'l',  // 013C l Latin Small Letter L with cedilla
  'L',  // 013D � Latin Capital Letter L with caron
  'l',  // 013E � Latin Small Letter L with caron
  'L',  // 013F ? Latin Capital Letter L with middle dot
  'l',  // 0140 ? Latin Small Letter L with middle dot
  'L',  // 0141 � Latin Capital Letter L with stroke
  'l',  // 0142 � Latin Small Letter L with stroke
  'N',  // 0143 � Latin Capital Letter N with acute
  'n',  // 0144 � Latin Small Letter N with acute
  'N',  // 0145 N Latin Capital Letter N with cedilla
  'n',  // 0146 n Latin Small Letter N with cedilla
  'N',  // 0147 � Latin Capital Letter N with caron
  'n',  // 0148 � Latin Small Letter N with caron
  'n',  // 0149 ? Latin Small Letter N preceded by apostrophe
  'n',  // 014A ? Latin Capital Letter Eng
  'n',  // 014B ? Latin Small Letter Eng
  'O',  // 014C O Latin Capital Letter O with macron
  'o',  // 014D o Latin Small Letter O with macron
  'O',  // 014E O Latin Capital Letter O with breve
  'o',  // 014F o Latin Small Letter O with breve
  'O',  // 0150 � Latin Capital Letter O with double acute
  'o',  // 0151 � Latin Small Letter O with double acute
  'O',  // 0152 O Latin Capital Ligature OE
  'o',  // 0153 o Latin Small Ligature OE
  'R',  // 0154 � Latin Capital Letter R with acute
  'r',  // 0155 � Latin Small Letter R with acute
  'R',  // 0156 R Latin Capital Letter R with cedilla
  'r',  // 0157 r Latin Small Letter R with cedilla
  'R',  // 0158 � Latin Capital Letter R with caron
  'r',  // 0159 � Latin Small Letter R with caron
  'S',  // 015A � Latin Capital Letter S with acute
  's',  // 015B � Latin Small Letter S with acute
  'S',  // 015C S Latin Capital Letter S with circumflex
  's',  // 015D s Latin Small Letter S with circumflex
  'S',  // 015E � Latin Capital Letter S with cedilla
  's',  // 015F � Latin Small Letter S with cedilla
  'S',  // 0160 � Latin Capital Letter S with caron
  's',  // 0161 � Latin Small Letter S with caron
  'T',  // 0162 � Latin Capital Letter T with cedilla
  't',  // 0163 � Latin Small Letter T with cedilla
  'T',  // 0164 � Latin Capital Letter T with caron
  't',  // 0165 � Latin Small Letter T with caron
  'T',  // 0166 T Latin Capital Letter T with stroke
  't',  // 0167 t Latin Small Letter T with stroke
  'U',  // 0168 U Latin Capital Letter U with tilde
  'u',  // 0169 u Latin Small Letter U with tilde
  'U',  // 016A U Latin Capital Letter U with macron
  'u',  // 016B u Latin Small Letter U with macron
  'U',  // 016C U Latin Capital Letter U with breve
  'u',  // 016D u Latin Small Letter U with breve
  'U',  // 016E � Latin Capital Letter U with ring above
  'u',  // 016F � Latin Small Letter U with ring above
  'U',  // 0170 � Latin Capital Letter U with double acute
  'u',  // 0171 � Latin Small Letter U with double acute
  'U',  // 0172 U Latin Capital Letter U with ogonek
  'u',  // 0173 u Latin Small Letter U with ogonek
  'W',  // 0174 W Latin Capital Letter W with circumflex
  'w',  // 0175 w Latin Small Letter W with circumflex
  'Y',  // 0176 Y Latin Capital Letter Y with circumflex
  'y',  // 0177 y Latin Small Letter Y with circumflex
  'Y',  // 0178 Y Latin Capital Letter Y with diaeresis
  'Z',  // 0179 � Latin Capital Letter Z with acute
  'z',  // 017A � Latin Small Letter Z with acute
  'Z',  // 017B � Latin Capital Letter Z with dot above
  'z',  // 017C � Latin Small Letter Z with dot above
  'Z',  // 017D � Latin Capital Letter Z with caron
  'z',  // 017E � Latin Small Letter Z with caron
  's',  // 017F ? Latin Small Letter long S
  'b',  // 0180 b Latin Small Letter B with stroke
  'B',  // 0181 ? Latin Capital Letter B with hook
  'b',  // 0182 ? Latin Capital Letter B with top bar
  'b',  // 0183 ? Latin Small Letter B with top bar
  'b',  // 0184 ? Latin Capital Letter Tone Six
  'b',  // 0185 ? Latin Small Letter Tone Six
  'o',  // 0186 ? Latin Capital Letter Open O
  'C',  // 0187 ? Latin Capital Letter C with hook
  'c',  // 0188 ? Latin Small Letter C with hook
  'D',  // 0189 � Latin Capital Letter African D
  'D',  // 018A ? Latin Capital Letter D with hook
  'd',  // 018B ? Latin Capital Letter D with top bar
  'd',  // 018C ? Latin Small Letter D with top bar
  '?',  // 018D ? Latin Small Letter Turned Delta
  '3',  // 018E ? Latin Capital Letter Reversed E
  '?',  // 018F ? Latin Capital Letter Schwa
  'E',  // 0190 ? Latin Capital Letter Open E
  'F',  // 0191 F Latin Capital Letter F with hook
  'f',  // 0192 f Latin Small Letter F with hook
  'G',  // 0193 ? Latin Capital Letter G with hook
  'V',  // 0194 ? Latin Capital Letter Gamma
  'h',  // 0195 ? Latin Small Letter HV
  '?',  // 0196 ? Latin Capital Letter Iota
  'I',  // 0197 I Latin Capital Letter I with stroke
  'K',  // 0198 ? Latin Capital Letter K with hook
  'k',  // 0199 ? Latin Small Letter K with hook
  'l',  // 019A l Latin Small Letter L with bar
  'A',  // 019B ? Latin Small Letter Lambda with stroke
  'W',  // 019C ? Latin Capital Letter Turned M
  'N',  // 019D ? Latin Capital Letter N with left hook
  'n',  // 019E ? Latin Small Letter N with long right leg
  'O',  // 019F O Latin Capital Letter O with middle tilde
  'O',  // 01A0 O Latin Capital Letter O with horn
  'o',  // 01A1 o Latin Small Letter O with horn
  '?',  // 01A2 ? Latin Capital Letter OI (= Latin Capital Letter Gha)
  '?',  // 01A3 ? Latin Small Letter OI (= Latin Small Letter Gha)
  'P',  // 01A4 ? Latin Capital Letter P with hook
  'p',  // 01A5 ? Latin Small Letter P with hook
  'R',  // 01A6 ? Latin Letter YR
  '2',  // 01A7 ? Latin Capital Letter Tone Two
  '2',  // 01A8 ? Latin Small Letter Tone Two
  'E',  // 01A9 ? Latin Capital Letter Esh
  '?',  // 01AA ? Latin Letter Reversed Esh Loop
  't',  // 01AB t Latin Small Letter T with palatal hook
  'T',  // 01AC ? Latin Capital Letter T with hook
  't',  // 01AD ? Latin Small Letter T with hook
  'T',  // 01AE T Latin Capital Letter T with retroflex hook
  'U',  // 01AF U Latin Capital Letter U with horn
  'u',  // 01B0 u Latin Small Letter U with horn
  'U',  // 01B1 ? Latin Capital Letter Upsilon
  'V',  // 01B2 ? Latin Capital Letter V with hook
  'Y',  // 01B3 ? Latin Capital Letter Y with hook
  'y',  // 01B4 ? Latin Small Letter Y with hook
  'Z',  // 01B5 ? Latin Capital Letter Z with stroke
  'z',  // 01B6 z Latin Small Letter Z with stroke
  '?',  // 01B7 ? Latin Capital Letter Ezh
  '?',  // 01B8 ? Latin Capital Letter Ezh reversed
  '?',  // 01B9 ? Latin Small Letter Ezh reversed
  '?',  // 01BA ? Latin Small Letter Ezh with tail
  '2',  // 01BB ? Latin Letter Two with stroke
  '5',  // 01BC ? Latin Capital Letter Tone Five
  '5',  // 01BD ? Latin Small Letter Tone Five
  '?',  // 01BE ? Latin Letter Inverted Glottal Stop with stroke
  'p',  // 01BF ? Latin Letter Wynn
  '|',  // 01C0 | Latin Letter Dental Click
  '?',  // 01C1 ? Latin Letter Lateral Click
  '?',  // 01C2 ? Latin Letter Alveolar Click
  '!',  // 01C3 ! Latin Letter Retroflex Click
  '?',  // 01C4 ? Latin Capital Letter DZ with caron
  '?',  // 01C5 ? Latin Capital Letter D with Small Letter Z with caron
  '?',  // 01C6 ? Latin Small Letter DZ with caron
  '?',  // 01C7 ? Latin Capital Letter LJ
  '?',  // 01C8 ? Latin Capital Letter L with Small Letter J
  '?',  // 01C9 ? Latin Small Letter LJ
  '?',  // 01CA ? Latin Capital Letter NJ
  '?',  // 01CB ? Latin Capital Letter N with Small Letter J
  '?',  // 01CC ? Latin Small Letter NJ
  'A',  // 01CD A Latin Capital Letter A with caron
  'a',  // 01CE a Latin Small Letter A with caron
  'I',  // 01CF I Latin Capital Letter I with caron
  'i',  // 01D0 i Latin Small Letter I with caron
  'O',  // 01D1 O Latin Capital Letter O with caron
  'o',  // 01D2 o Latin Small Letter O with caron
  'U',  // 01D3 U Latin Capital Letter U with caron
  'u',  // 01D4 u Latin Small Letter U with caron
  'U',  // 01D5 U Latin Capital Letter U with diaeresis and macron
  'u',  // 01D6 u Latin Small Letter U with diaeresis and macron
  'U',  // 01D7 U Latin Capital Letter U with diaeresis and acute
  'u',  // 01D8 u Latin Small Letter U with diaeresis and acute
  'U',  // 01D9 U Latin Capital Letter U with diaeresis and caron
  'u',  // 01DA u Latin Small Letter U with diaeresis and caron
  'U',  // 01DB U Latin Capital Letter U with diaeresis and grave
  'u',  // 01DC u Latin Small Letter U with diaeresis and grave
  'e',  // 01DD ? Latin Small Letter Turned E
  'A',  // 01DE A Latin Capital Letter A with diaeresis and macron
  'a',  // 01DF a Latin Small Letter A with diaeresis and macron
  'A',  // 01E0 ? Latin Capital Letter A with dot above and macron
  'a',  // 01E1 ? Latin Small Letter A with dot above and macron
  'E',  // 01E2 ? Latin Capital Letter AE with macron
  'e',  // 01E3 ? Latin Small Letter AE with macron
  'G',  // 01E4 G Latin Capital Letter G with stroke
  'g',  // 01E5 g Latin Small Letter G with stroke
  'G',  // 01E6 G Latin Capital Letter G with caron
  'g',  // 01E7 g Latin Small Letter G with caron
  'K',  // 01E8 K Latin Capital Letter K with caron
  'k',  // 01E9 k Latin Small Letter K with caron
  'O',  // 01EA O Latin Capital Letter O with ogonek
  'o',  // 01EB o Latin Small Letter O with ogonek
  'O',  // 01EC O Latin Capital Letter O with ogonek and macron
  'o',  // 01ED o Latin Small Letter O with ogonek and macron
  '3',  // 01EE ? Latin Capital Letter Ezh with caron
  '3',  // 01EF ? Latin Small Letter Ezh with caron
  'j',  // 01F0 j Latin Small Letter J with caron
  '?',  // 01F1 ? Latin Capital Letter DZ
  '?',  // 01F2 ? Latin Capital Letter D with Small Letter Z
  '?',  // 01F3 ? Latin Small Letter DZ
  'G',  // 01F4 ? Latin Capital Letter G with acute
  'g',  // 01F5 ? Latin Small Letter G with acute
  'H',  // 01F6 ? Latin Capital Letter Hwair
  'P',  // 01F7 ? Latin Capital Letter Wynn
  'N',  // 01F8 ? Latin Capital Letter N with grave
  'n',  // 01F9 ? Latin Small Letter N with grave
  'A',  // 01FA ? Latin Capital Letter A with ring above and acute
  'a',  // 01FB ? Latin Small Letter A with ring above and acute
  'E',  // 01FC ? Latin Capital Letter AE with acute
  'e',  // 01FD ? Latin Small Letter AE with acute
  'O',  // 01FE ? Latin Capital Letter O with stroke and acute
  'o',  // 01FF ? Latin Small Letter O with stroke and acute
  'A',  // 0200 ? Latin Capital Letter A with double grave
  'a',  // 0201 ? Latin Small Letter A with double grave
  'A',  // 0202 ? Latin Capital Letter A with inverted breve
  'a',  // 0203 ? Latin Small Letter A with inverted breve
  'E',  // 0204 ? Latin Capital Letter E with double grave
  'e',  // 0205 ? Latin Small Letter E with double grave
  'E',  // 0206 ? Latin Capital Letter E with inverted breve
  'e',  // 0207 ? Latin Small Letter E with inverted breve
  'I',  // 0208 ? Latin Capital Letter I with double grave
  'i',  // 0209 ? Latin Small Letter I with double grave
  'I',  // 020A ? Latin Capital Letter I with inverted breve
  'i',  // 020B ? Latin Small Letter I with inverted breve
  'O',  // 020C ? Latin Capital Letter O with double grave
  'o',  // 020D ? Latin Small Letter O with double grave
  'O',  // 020E ? Latin Capital Letter O with inverted breve
  'o',  // 020F ? Latin Small Letter O with inverted breve
  'R',  // 0210 ? Latin Capital Letter R with double grave
  'r',  // 0211 ? Latin Small Letter R with double grave
  'R',  // 0212 ? Latin Capital Letter R with inverted breve
  'r',  // 0213 ? Latin Small Letter R with inverted breve
  'U',  // 0214 ? Latin Capital Letter U with double grave
  'u',  // 0215 ? Latin Small Letter U with double grave
  'U',  // 0216 ? Latin Capital Letter U with inverted breve
  'u',  // 0217 ? Latin Small Letter U with inverted breve
  'S',  // 0218 ? Latin Capital Letter S with comma below
  's',  // 0219 ? Latin Small Letter S with comma below
  'T',  // 021A ? Latin Capital Letter T with comma below
  't',  // 021B ? Latin Small Letter T with comma below
  '3',  // 021C ? Latin Capital Letter Yogh
  '3',  // 021D ? Latin Small Letter Yogh
  'H',  // 021E ? Latin Capital Letter H with caron
  'h',  // 021F ? Latin Small Letter H with caron
  'n',  // 0220 ? Latin Capital Letter N with long right leg
  'd',  // 0221 ? Latin Small Letter D with curl
  'O',  // 0222 ? Latin Capital Letter OU
  'o',  // 0223 ? Latin Small Letter OU
  'Z',  // 0224 ? Latin Capital Letter Z with hook
  'z',  // 0225 ? Latin Small Letter Z with hook
  'A',  // 0226 ? Latin Capital Letter A with dot above
  'a',  // 0227 ? Latin Small Letter A with dot above
  'E',  // 0228 ? Latin Capital Letter E with cedilla
  'e',  // 0229 ? Latin Small Letter E with cedilla
  'O',  // 022A ? Latin Capital Letter O with diaeresis and macron
  'o',  // 022B ? Latin Small Letter O with diaeresis and macron
  'O',  // 022C ? Latin Capital Letter O with tilde and macron
  'o',  // 022D ? Latin Small Letter O with tilde and macron
  'O',  // 022E ? Latin Capital Letter O with dot above
  'o',  // 022F ? Latin Small Letter O with dot above
  'O',  // 0230 ? Latin Capital Letter O with dot above and macron
  'o',  // 0231 ? Latin Small Letter O with dot above and macron
  'Y',  // 0232 ? Latin Capital Letter Y with macron
  'y',  // 0233 ? Latin Small Letter Y with macron
  'l',  // 0234 ? Latin Small Letter L with curl
  'n',  // 0235 ? Latin Small Letter N with curl
  't',  // 0236 ? Latin Small Letter T with curl
  'j',  // 0237 ? Latin Small Letter Dotless J
  '?',  // 0238 ? Latin Small Letter DB Diagraph
  '?',  // 0239 ? Latin Small Letter QP Diagraph
  'A',  // 023A ? Latin Capital Letter A with stroke
  'C',  // 023B ? Latin Capital Letter C with stroke
  'c',  // 023C ? Latin Small Letter C with stroke
  'L',  // 023D ? Latin Capital Letter L with bar
  'T',  // 023E ? Latin Capital Letter T with diagonal stroke
  's',  // 023F ? Latin Small Letter S with swash tail
  'z',  // 0240 ? Latin Small Letter Z with swash tail
  '?',  // 0241 ? Latin Capital Letter Glottal Stop
  '?',  // 0242 ? Latin Small Letter Glottal Stop
  'B',  // 0243 ? Latin Capital Letter B with stroke
  'U',  // 0244 ? Latin Capital Letter U bar
  'A',  // 0245 ? Latin Capital Letter Turned V
  'E',  // 0246 ? Latin Capital Letter E with stroke
  'e',  // 0247 ? Latin Small Letter E with stroke
  'J',  // 0248 ? Latin Capital Letter J with stroke
  'j',  // 0249 ? Latin Small Letter J with stroke
  'Q',  // 024A ? Latin Capital Letter Q with hook tail
  'q',  // 024B ? Latin Small Letter Q with hook tail
  'R',  // 024C ? Latin Capital Letter R with stroke
  'r',  // 024D ? Latin Small Letter R with stroke
  'Y',  // 024E ? Latin Capital Letter Y with stroke
  'y'   // 024F ? Latin Small Letter Y with stroke
}; // utf16toAscii[]


//______________________________________________________________________________

#ifndef SYS_UTF8_CONV
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Iterator for constant size arrays (helper for utf8::utf16to8 etc).
template <class ItemType> class array_back_insert_iterator
: public std::iterator<std::output_iterator_tag, void, void, void, void>
{
  protected:
    ItemType* array;
    int       size;
    int       elements;
    bool      overflow;

  public:
    typedef ItemType container_type;

    explicit array_back_insert_iterator(ItemType* _array, int _size)
    : array(_array),
      size(_size),
      elements(0),
      overflow(false)
    {}

    int length() const
    { return elements; }

    bool overflowed() const
    { return overflow; }

    array_back_insert_iterator<ItemType>& operator= (ItemType value)
      {
        if (elements >= size)
          overflow = true;
        else
          array[elements++] = value;
        return *this;
      }

    array_back_insert_iterator<ItemType>& operator* ()
      { return *this; }

    array_back_insert_iterator<ItemType>& operator++ ()
      { return *this; }

    array_back_insert_iterator<ItemType>& operator++ (int)
      { return *this; }
}; // array_back_insert_iterator
#endif

//______________________________________________________________________________


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Converts ASCII string encoded in system code page into Unicode string.
/// \return Unicode string length, -1 on conversion error
int ascii2unicode(const char* ascii, wchar_t* unicode, int maxChars) {
    // The conversion from ASCII to Unicode and vice versa are quite trivial. By design, the first 128 Unicode
    // values are the same as ASCII (in fact, the first 256 are equal to ISO-8859-1).

    wchar_t* end = std::copy_n(ascii, std::max(maxChars-1, (int)strlen(ascii)), unicode);
    *end = L'\0';
    return std::distance(unicode, end);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Converts Unicode string into ASCII encoded in system code page.
/// \return ASCII string length, -1 on conversion error (insufficient buffer e.g.)
int unicode2ascii(const wchar_t* unicode, char* ascii, int maxChars) {
    // The conversion from ASCII to Unicode and vice versa are quite trivial. By design, the first 128 Unicode
    // values are the same as ASCII (in fact, the first 256 are equal to ISO-8859-1).

    char* end = std::copy_n(unicode, std::max(maxChars-1, (int)wcslen(unicode)), ascii);
    *end = '\0';
    return std::distance(ascii, end);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Converts Unicode string into UTF-8 encoded string.
/// \return UTF8 string size [octets], -1 on conversion error (insufficient buffer e.g.)
int unicode2utf(const wchar_t* unicode, char* utf, int maxChars)
{
  #ifndef SYS_UTF8_CONV

  // we will use our own UTF16->UTF8 conversion (WideCharToMultiByte(CP_UTF8)
  // is not working on some Win CE systems)
  size_t len = wcslen(unicode);

  array_back_insert_iterator<char> iter = array_back_insert_iterator<char>(utf, maxChars - 1);

  iter = utf8::unchecked::utf16to8(unicode, unicode + len, iter);

  if (!iter.overflowed()) {
    utf[iter.length()] = '\0';
    return(iter.length());
  }

  // for safety reasons, return empty string
  if (maxChars >= 1)
    utf[0] = '\0';
  return(-1);

  #else /* just for fallback return to old code, to be removed after tests */

  int res = WideCharToMultiByte(CP_UTF8, 0, unicode, -1, utf, maxChars, NULL, NULL);

  if (res > 0)
    return(res - 1);

  // for safety reasons, return empty string
  if (maxChars >= 1)
    utf[0] = '\0';
  return(-1);

  #endif
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Converts UTF-8 encoded string into Unicode encoded string.
/// \return Unicode string size [TCHARs], -1 on conversion error (insufficient buffer e.g.)
int utf2unicode(const char* utf, wchar_t* unicode, int maxChars)
{
  #ifndef SYS_UTF8_CONV

  // we will use our own UTF16->UTF8 conversion (MultiByteToWideChar(CP_UTF8)
  // is not working on some Win CE systems)
  size_t len = strlen(utf);

  // first check if UTF8 is correct (utf8to16() may not be called on invalid string)
  if (utf8::find_invalid(utf, utf + len) == (utf + len)) {
    array_back_insert_iterator<wchar_t> iter = array_back_insert_iterator<wchar_t>(unicode, maxChars - 1);

    iter = utf8::unchecked::utf8to16(utf, utf + len, iter);

    if (!iter.overflowed()) {
      unicode[iter.length()] = '\0';
      return(iter.length());
    }
  }

  // for safety reasons, return empty string
  if (maxChars >= 1)
    unicode[0] = '\0';
  return(-1);

  #else /* just for fallback return to old code, to be removed after tests */

  int res = MultiByteToWideChar(CP_UTF8, 0, utf, -1, unicode, maxChars);

  if (res > 0)
    return(res - 1);

  // for safety reasons, return empty string
  if (maxChars >= 1)
    unicode[0] = '\0';
  return(-1);

  #endif
}

#ifdef _UNICODE
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Converts Unicode string into US-ASCII string (writing as much as possible
/// characters into @p ascii). Output string will always be terminated by '\0'.
///
/// Characters are converted into their most similar representation
/// in US-ASCII. Nonconvertable characters are replaced by '?'.
///
/// Output string will always be terminated by '\0'.
///
/// @param unicode    input string (must be terminated with '\0')
/// @param ascii      output buffer
/// @param maxChars   output buffer size
///
/// @retval  1  all characters copied
/// @retval -1  some characters could not be copied due to buffer size
///
int unicode2usascii(const wchar_t* unicode, char* ascii, int outSize)
{
  wchar_t uc;

  if (outSize == 0)
    return(false);

  // decrement indeces to use more efficient pre-increments
  unicode--;
  ascii--;
  while ((uc = *++unicode) != 0 && --outSize > 0)
  {
    if (uc <= maxUtf16toAscii)
      *++ascii = utf16toAscii[uc];
    else if (uc < 0xDC00 || uc > 0xDFFF)
      *++ascii = '?';
    else // skipping surrogate pair low item
      ++outSize;
  }

  *++ascii = '\0';

  return((uc == 0) ? 1 : -1);
} // Wide2Ascii()
#endif


int ascii2TCHAR(const char* ascii, TCHAR* unicode, int maxChars) {
#if defined(_UNICODE)
    return  ascii2unicode(ascii, unicode, maxChars);
#else
    size_t len = std::min(_tcslen(ascii), (size_t)maxChars);
    _tcsncpy(unicode, ascii, len+1);
    return len;
#endif 
}

int TCHAR2ascii(const TCHAR* unicode, char* ascii, int maxChars) {
#if defined(_UNICODE)
    return  unicode2ascii(unicode, ascii, maxChars);
#else
    size_t len = std::min(_tcslen(ascii), (size_t)maxChars);
    _tcsncpy(ascii, unicode, len);
    return len;
#endif 
}

int TCHAR2utf(const TCHAR* unicode, char* utf, int maxChars) {
#if defined(_UNICODE)
    return  unicode2utf(unicode, utf, maxChars);
#elif defined(_MBCS)
    wchar_t temp[maxChars];
    size_t len = mbstowcs(temp, unicode, maxChars);
    if(len!=(size_t)-1) {
        return unicode2utf(temp, utf, maxChars);
    } 
    // if error, return simple copy
    len = std::min(_tcslen(unicode), (size_t)maxChars);
    _tcsncpy(utf, unicode, maxChars);
    return len;
#else
    size_t len = std::min(_tcslen(unicode), (size_t)maxChars);
    _tcsncpy(utf, unicode, maxChars);
    return len;   
#endif 
}

int utf2TCHAR(const char* utf, TCHAR* unicode, int maxChars){
#if defined(_UNICODE)
    return  utf2unicode(utf, unicode, maxChars);
#elif defined(_MBCS)
    wchar_t temp[maxChars];
    memset(unicode, 0, maxChars*sizeof(TCHAR));
    utf2unicode(utf, temp, maxChars);
    return wcstombs(unicode, temp, maxChars);
#else
    size_t len = std::min(_tcslen(utf), (size_t)maxChars);
    _tcsncpy(unicode, utf, len+1);
    return len;
#endif 
}
  
int TCHAR2usascii(const TCHAR* unicode, char* ascii, int outSize) {
#if defined(_UNICODE)
    return  unicode2usascii(unicode, ascii, outSize);
#elif defined(_MBCS)
    wchar_t temp[outSize];
    size_t len = mbstowcs(temp, unicode, outSize);
    if(len!=(size_t)-1) {
        return unicode2usascii(temp, ascii, outSize);
    } 
    // if error, return simple copy
    len = std::min(_tcslen(unicode), (size_t)outSize);
    _tcsncpy(ascii, unicode, len);
    return len;
#else
    size_t len = std::min(_tcslen(unicode), (size_t)outSize);
    _tcsncpy(ascii, unicode, len+1);
    return len;
#endif 
}