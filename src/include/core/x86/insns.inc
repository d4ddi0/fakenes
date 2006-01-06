

%if 0

FakeNES - A portable, Open Source NES emulator.

Distributed under the Clarified Artistic License.

insns.inc: CPU instructions emulation macros.

Copyright (c) 2001-2006, Randy McDowell.
Copyright (c) 2001-2006, Charles Bilyue'.

This is free software.  See 'LICENSE' for details.
You must read and accept the license prior to use.

This file contains macros for emulating instruction
specific behavior, for the Ricoh RP2A03G CPU.

%endif


%ifndef INSNS_I
%define INSNS_I


; These macros cover instruction-specific behavior */
%macro Insn_Branch_Flag_Clear 1
 Read_Immediate
 JUMP_FLAG %1,C_LABEL(FN2A03_Run).check_cycles
 movsx eax,al
 mov ecx,B_Cycles
 mov ebx,esi
 add ecx,byte CYCLE_LENGTH
 add bl,al
 add esi,eax
 mov B_Cycles,ecx
 cmp esi,ebx
 je C_LABEL(FN2A03_Run).check_cycles
 Dummy_Read ebx
 mov ecx,B_Cycles
 add ecx,byte CYCLE_LENGTH
 mov B_Cycles,ecx
%endmacro

; These macros cover instruction-specific behavior */
%macro Insn_Branch_Flag_Set 1
 Read_Immediate
 JUMP_NOT_FLAG %1,C_LABEL(FN2A03_Run).check_cycles
 movsx eax,al
 mov ecx,B_Cycles
 mov ebx,esi
 add ecx,byte CYCLE_LENGTH
 add bl,al
 add esi,eax
 mov B_Cycles,ecx
 cmp esi,ebx
 je C_LABEL(FN2A03_Run).check_cycles
 Dummy_Read ebx
 mov ecx,B_Cycles
 add ecx,byte CYCLE_LENGTH
 mov B_Cycles,ecx
%endmacro

%macro Insn_ADC 1
 Read_%1
 mov cl,B_C
 mov dl,B_A
 add cl,255
 adc dl,al
 seto B_V
 sbb al,al
 mov B_A,dl
 STORE_FLAGS_NZC dl,al
%endmacro

; Warning! C_FLAG is inverted before SBC and after it */
%macro Insn_SBC 1
 Read_%1
 mov cl,B_C
 mov dl,B_A
 cmp cl,1
 sbb dl,al
 seto B_V
 sbb al,al
 mov B_A,dl
 xor al,0xFF
 STORE_FLAGS_NZC dl,al
%endmacro

%macro Insn_CMP 2
 Read_%1
 mov dl,%2
 sub dl,al
 sbb al,al
 xor al,0xFF
 STORE_FLAGS_NZC dl,al
%endmacro

%macro Insn_BIT 1
 Read_%1
 mov dl,B_A
 STORE_FLAGS_N al
 and dl,al
 and al,FLAG_V
 STORE_FLAGS_Z dl
 STORE_FLAGS_V al
%endmacro

%macro Insn_AND 1
 Read_%1
 mov dl,B_A
 and al,dl
 mov B_A,al
 STORE_FLAGS_NZ al
%endmacro

%macro Insn_ORA 1
 Read_%1
 mov dl,B_A
 or al,dl
 mov B_A,al
 STORE_FLAGS_NZ al
%endmacro

%macro Insn_EOR 1
 Read_%1
 mov dl,B_A
 xor al,dl
 mov B_A,al
 STORE_FLAGS_NZ al
%endmacro

%macro Insn_LD 2
 Read_%1
 mov %2,al
 STORE_FLAGS_NZ al
%endmacro

%macro Insn_ST 2
 Write_%1 %2
%endmacro

%macro Insn_INC 0-1 al
%ifnidni %1,al
 mov al,%1
%endif
 inc al
 STORE_FLAGS_NZ al
%ifnidni %1,al
 mov %1,al
%endif
%endmacro

%macro Insn_DEC 0-1 al
%ifnidni %1,al
 mov al,%1
%endif
 dec al
 STORE_FLAGS_NZ al
%ifnidni %1,al
 mov %1,al
%endif
%endmacro

%macro Insn_ASL 0-1 al
%ifnidni %1,al
 mov al,%1
%endif
 add al,al
 STORE_FLAGS_Z al
 sbb dl,dl
 STORE_FLAGS_N al
 STORE_FLAGS_C dl
%ifnidni %1,al
 mov %1,al
%endif
%endmacro

%macro Insn_LSR 0-1 al
%ifnidni %1,al
 mov al,%1
%endif
 shr al,byte 1
 STORE_FLAGS_Z al
 sbb dl,dl
 STORE_FLAGS_N al
 STORE_FLAGS_C dl
%ifnidni %1,al
 mov %1,al
%endif
%endmacro

%macro Insn_ROL 0-1 al
 mov cl,B_C
%ifnidni %1,al
 mov al,%1
%endif
 add cl,255
 adc al,al
 STORE_FLAGS_Z al
 sbb dl,dl
 STORE_FLAGS_N al
 STORE_FLAGS_C dl
%ifnidni %1,al
 mov %1,al
%endif
%endmacro

%macro Insn_ROR 0-1 al
 mov cl,B_C
%ifnidni %1,al
 mov al,%1
%endif
 add cl,255
 rcr al,byte 1
 STORE_FLAGS_Z al
 sbb dl,dl
 STORE_FLAGS_N al
 STORE_FLAGS_C dl
%ifnidni %1,al
 mov %1,al
%endif
%endmacro

%macro Insn_SLO 0
 add al,al
 mov cl,B_A
 sbb dl,dl
 or cl,al
 STORE_FLAGS_C dl
 STORE_FLAGS_NZ cl
%endmacro

%macro Insn_INS 0
 inc al
 mov cl,B_C
 mov dl,B_A
 cmp cl,1
 sbb dl,al
 seto B_V
 sbb cl,cl
 mov B_A,dl
 xor cl,0xFF
 STORE_FLAGS_NZC dl,cl
%endmacro

%endif ; !defined(INSNS_I)

