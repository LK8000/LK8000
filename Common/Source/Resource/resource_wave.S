/*
 * LK8000 Tactical Flight Computer - WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 * 
 * File: resource_xml.S
 * Author: Bruno de Lacheisserie
 * 
 * Created on 18 septembre 2014, 13:34
 */

.macro binfile p q
    .globl \p\()_begin
    .globl \p\()_end
    .globl \p\()_size
\p\()_begin:
    .incbin "\q"
\p\()_end:
    /* 
    Put a ".byte 0" here if you know  your data is text
    and you wish to use \p&_begin as a C string. It 
    doesn't hurt to leave it here even for binary data
    since it is not counted in \p_&size
    */
    .byte 0
    .byte 0
\p\()_size:
    .int (\p\()_end - \p\()_begin)
    /* add alias for "_" decorated symname for avoid link error. */
    .globl _\p\()_begin
    .set _\p\()_begin, \p\()_begin
    .globl _\p\()_end
    .set _\p\()_end, \p\()_end
    .globl _\p\()_size
    .set _\p\()_size, \p\()_size

.endm	

    .section .rodata

    binfile IDR_WAV_MM0, Common/Data/Sounds/MM0.WAV
    binfile IDR_WAV_MM1, Common/Data/Sounds/MM1.WAV
    binfile IDR_WAV_MM2, Common/Data/Sounds/MM2.WAV
    binfile IDR_WAV_MM3, Common/Data/Sounds/MM3.WAV
    binfile IDR_WAV_MM4, Common/Data/Sounds/MM4.WAV
    binfile IDR_WAV_MM5, Common/Data/Sounds/MM5.WAV
    binfile IDR_WAV_MM6, Common/Data/Sounds/MM6.WAV
    
    binfile IDR_WAV_DRIP, Common/Data/Sounds/LKbeep-drip.wav
    binfile IDR_WAV_CLICK, Common/Data/Sounds/LK_SHORTERCLICKM.WAV
    binfile IDR_WAV_HIGHCLICK, Common/Data/Sounds/LK_CLICKH.WAV
    
    binfile IDR_WAV_TONE1, Common/Data/Sounds/LK_T1.WAV
    binfile IDR_WAV_TONE2, Common/Data/Sounds/LK_T2.WAV
    binfile IDR_WAV_TONE3, Common/Data/Sounds/LK_T3.WAV
    binfile IDR_WAV_TONE4, Common/Data/Sounds/LK_T4.WAV
    binfile IDR_WAV_TONE7, Common/Data/Sounds/LK_T8.WAV
    
    binfile IDR_WAV_BTONE2, Common/Data/Sounds/LK_B2b.wav
    binfile IDR_WAV_BTONE4, Common/Data/Sounds/LK_B4.wav
    binfile IDR_WAV_BTONE5, Common/Data/Sounds/LK_B5.wav
    binfile IDR_WAV_BTONE6, Common/Data/Sounds/LK_B5b.wav
    binfile IDR_WAV_BTONE7, Common/Data/Sounds/LK_B8.wav

    binfile IDR_WAV_OVERTONE0, Common/Data/Sounds/LK_S0.WAV
    binfile IDR_WAV_OVERTONE1, Common/Data/Sounds/LK_S1.WAV
    binfile IDR_WAV_OVERTONE2, Common/Data/Sounds/LK_S2.WAV
    binfile IDR_WAV_OVERTONE3, Common/Data/Sounds/LK_S3.WAV
    binfile IDR_WAV_OVERTONE4, Common/Data/Sounds/LK_S4.WAV
    binfile IDR_WAV_OVERTONE5, Common/Data/Sounds/LK_S5.WAV
    binfile IDR_WAV_OVERTONE6, Common/Data/Sounds/LK_S6.WAV
    binfile IDR_WAV_OVERTONE7, Common/Data/Sounds/LK_S6b.WAV
