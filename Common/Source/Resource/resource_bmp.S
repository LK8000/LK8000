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
    and you wish to use \p\()_begin as a C string. It 
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

    binfile IDB_EMPTY,              Common/Data/Bitmaps/Empty.bmp
    binfile IDB_TOWN                Common/Data/Bitmaps/town.bmp
    binfile IDB_LKSMALLTOWN         Common/Data/Bitmaps/smalltown.bmp
    binfile IDB_LKVERYSMALLTOWN     Common/Data/Bitmaps/verysmalltown.bmp

#if defined(__linux__) && defined(__ELF__)
    .section .note.GNU-stack,"",%progbits
#endif
