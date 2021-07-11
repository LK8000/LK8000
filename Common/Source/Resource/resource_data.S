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

    binfile IDR_RASTER_EGM96S, Common/Data/Bitmaps/egm96s.dem
