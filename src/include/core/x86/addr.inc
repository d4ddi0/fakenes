

%if 0

FakeNES - A portable, Open Source NES emulator.

Distributed under the Clarified Artistic License.

addr.inc: CPU addressing mode emulation macros.

Copyright (c) 2005, Randy McDowell.
Copyright (c) 2005, Charles Bilyue'.

This is free software.  See 'LICENSE' for details.
You must read and accept the license prior to use.

This file contains macros for emulating addressing mode
behavior, for the Ricoh RP2A03G CPU.

%endif


%ifndef ADDR_I
%define ADDR_I

; Addressing modes */
; These macros calculate effective addresses, store them in */
; an implied variable, and update the Program Counter register. */

; address set to 'zero_page_address' */
%macro Address_Zero_Page 0
 inc si
 Read esi
 xor ebx,ebx
 inc si
 mov bl,al
%endmacro

%macro Address_Zero_Page_Index_X 0
 inc si
 Read esi
 xor ebx,ebx
 inc si
 mov bl,B_X
 add bl,al
%endmacro

%macro Address_Zero_Page_Index_Y 0
 inc si
 Read esi
 xor ebx,ebx
 inc si
 mov bl,B_Y
 add bl,al
%endmacro


; address set to 'address' */
%macro Address_Zero_Page_Indirect 0
 Read_ZP ebx,al
 inc bl
 Read_ZP ebx,bh
 mov bl,al
%endmacro

%macro Address_Indexed_Indirect_X 0
 Address_Zero_Page_Index_X
 Address_Zero_Page_Indirect
%endmacro

%macro Address_Absolute 0
 inc si
 Read esi
 xor ebx,ebx
 inc si
 Read esi,0,bl
 inc si
 mov bh,al
%endmacro

%macro Address_Absolute_JMP 0
 inc si
 Read esi
 xor ebx,ebx
 inc si
 Read esi,0,bl
 mov bh,al
%endmacro

%macro Address_Absolute_Index_X 0
 Address_Absolute
 xor edx,edx
 mov dl,B_X
 add bx,dx
%endmacro

%macro Address_Absolute_Index_Y 0
 Address_Absolute
 xor edx,edx
 mov dl,B_Y
 add dx,bx
%endmacro

%macro Address_Indirect_Indexed_Y 0
 Address_Zero_Page
 xor edx,edx
 Address_Zero_Page_Indirect
 mov dl,B_Y
 add bx,dx
%endmacro


; These macros calculate and read from effective addresses. */

%macro Read_Immediate 0
 inc si
 Read esi
 inc si
%endmacro

%macro Read_Zero_Page 0
 Address_Zero_Page
 Read_ZP ebx,al
%endmacro

%macro Read_Zero_Page_Index_X 0
 Address_Zero_Page_Index_X
 Read_ZP ebx,al
%endmacro

%macro Read_Zero_Page_Index_Y 0
 Address_Zero_Page_Index_Y
 Read_ZP ebx,al
%endmacro

%macro Read_Indexed_Indirect_X 0
 Address_Indexed_Indirect_X
 Read ebx
%endmacro

%macro Read_Absolute 0
 Address_Absolute
 Read ebx
%endmacro

%macro Read_Absolute_Indexed 1
 mov dl,%1
 add bl,dl
 Read ebx
 mov dl,%1
 cmp bl,dl
 jnb %%read_complete
 mov ecx,B_Cycles
 inc bh
 add ecx,byte CYCLE_LENGTH
 mov B_Cycles,ecx
 Read ebx
%%read_complete:
%endmacro

%macro Read_Absolute_Index_X 0
 Address_Absolute
 Read_Absolute_Indexed B_X
%endmacro

%macro Read_Absolute_Index_Y 0
 Address_Absolute
 Read_Absolute_Indexed B_Y
%endmacro

%macro Read_Indirect_Indexed_Y 0
 Address_Zero_Page
 Address_Zero_Page_Indirect
 Read_Absolute_Indexed B_Y
%endmacro


; These macros calculate and write to effective addresses. */

%macro Write_Zero_Page 1
 Address_Zero_Page
 mov al,%1
 Write_ZP ebx,al
%endmacro

%macro Write_Zero_Page_Index_X 1
 Address_Zero_Page_Index_X
 mov al,%1
 Write_ZP ebx,al
%endmacro

%macro Write_Zero_Page_Index_Y 1
 Address_Zero_Page_Index_Y
 mov al,%1
 Write_ZP ebx,al
%endmacro

%macro Write_Indexed_Indirect_X 1
 Address_Indexed_Indirect_X
 mov al,%1
 Write ebx
%endmacro

%macro Write_Absolute 1
 Address_Absolute
 mov al,%1
 Write ebx
%endmacro

%macro Write_Absolute_Indexed 2
 mov dl,%1
 add bl,dl
 Dummy_Read ebx
 mov dl,%1
 cmp bl,dl
 jnb %%no_wrap
 inc bh
%%no_wrap:
 mov al,%2
 Write ebx
%endmacro

%macro Write_Absolute_Index_X 1
 Address_Absolute
 Write_Absolute_Indexed B_X,%1
%endmacro

%macro Write_Absolute_Index_Y 1
 Address_Absolute
 Write_Absolute_Indexed B_Y,%1
%endmacro

%macro Write_Indirect_Indexed_Y 1
 Address_Zero_Page
 Address_Zero_Page_Indirect
 Write_Absolute_Indexed B_Y,%1
%endmacro


; These macros calculate effective addresses and perform */
; read-modify-write operations to the addressed data. */

%macro RMW_Zero_Page 1
 Address_Zero_Page
 Read_ZP ebx,al
 Dummy_Write_ZP ebx,al
 %1
 Write_ZP ebx,al
%endmacro

%macro RMW_Zero_Page_Index_X 1
 Address_Zero_Page_Index_X
 Read_ZP ebx,al
 Dummy_Write_ZP ebx,al
 %1
 Write_ZP ebx,al
%endmacro

%macro RMW_Absolute 1
 Address_Absolute
 Read ebx
 push eax
 Dummy_Write ebx
 pop eax
 %1
 Write ebx
%endmacro

%macro RMW_Absolute_Index_X 1
 Address_Absolute
 mov dl,B_X
 add bl,dl
 Dummy_Read ebx
 mov dl,B_X
 cmp bl,dl
 jnb %%no_wrap
 inc bh
%%no_wrap:
 Read ebx
 push eax
 Dummy_Write ebx
 pop eax
 %1
 Write ebx
%endmacro

%endif ; !defined(ADDR_I)

