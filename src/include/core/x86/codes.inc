%if 0

FakeNES - A portable, Open Source NES emulator.

core.c: Implementation of the RP2A03G CPU emulation

Copyright (c) 2002, Charles Bilyue' and Randy McDowell.

This is free software.  See 'LICENSE' for details.
You must read and accept the license prior to use.

 This file contains opcode emulation functions for the Ricoh RP2A03G
CPU, as used in the Nintendo Famicom (Family Computer) and NES
(Nintendo Entertainment System).

%endif

%ifndef CODES_I
%define CODES_I


OPCODE_PROLOG(0x10) ; BPL * REL
    Insn_Branch_Flag_Clear(FLAG_N)
OPCODE_EPILOG

OPCODE_PROLOG(0x30) ; BMI * REL
    Insn_Branch_Flag_Set(FLAG_N)
OPCODE_EPILOG

OPCODE_PROLOG(0xD0) ; BNE * REL
    Insn_Branch_Flag_Clear(FLAG_Z)
OPCODE_EPILOG

OPCODE_PROLOG(0xF0) ; BEQ * REL
    Insn_Branch_Flag_Set(FLAG_Z)
OPCODE_EPILOG

OPCODE_PROLOG(0x90) ; BCC * REL
    Insn_Branch_Flag_Clear(FLAG_C)
OPCODE_EPILOG

OPCODE_PROLOG(0xB0) ; BCS * REL
    Insn_Branch_Flag_Set(FLAG_C)
OPCODE_EPILOG

OPCODE_PROLOG(0x50) ; BVC * REL
    Insn_Branch_Flag_Clear(FLAG_V)
OPCODE_EPILOG

OPCODE_PROLOG(0x70) ; BVS * REL
    Insn_Branch_Flag_Set(FLAG_V)
OPCODE_EPILOG


OPCODE_PROLOG(0x40) ; RTI
 xor ebx,ebx
 mov bl,B_S
 mov cl,B_I
 inc bl
 test cl,cl
 Read_Stack ebx,al
 jz .i_flag_not_cleared
 test al,FLAG_I
 jnz .i_flag_not_cleared
 mov edx,B_IRequest
 test edx,edx
 jz .i_flag_not_cleared
 mov edx,B_ICount
 mov B_AfterCLI,cl
 mov ecx,1
 mov B_IBackup,edx
 mov B_ICount,ecx
.i_flag_not_cleared:

 inc bl

 Unpack_Flags

 Read_Stack ebx,al
 inc bl
 mov B_S,bl
 Read_Stack ebx,ah

 mov si,ax
OPCODE_EPILOG

OPCODE_PROLOG(0x60) ; RTS
 xor ebx,ebx
 mov bl,B_S
 inc bl
 Read_Stack ebx,al
 inc bl
 mov B_S,bl
 Read_Stack ebx,ah
 inc ax
 mov esi,eax
OPCODE_EPILOG


OPCODE_PROLOG(0x20) ; JSR $ssss ABS
 Address_Absolute_JMP
 mov eax,esi
 mov esi,ebx
 xor ebx,ebx
 mov bl,B_S
 Write_Stack ebx,ah
 dec bl
 Write_Stack ebx,al
 dec bl
 mov B_S,bl
OPCODE_EPILOG

OPCODE_PROLOG(0x4C) ; JMP $ssss ABS
 Address_Absolute_JMP
 mov esi,ebx
OPCODE_EPILOG

OPCODE_PROLOG(0x6C) ; JMP ($ssss) ABSINDIR
 Address_Absolute_JMP
 Read ebx
 inc bl
 push eax
 Read ebx,0
 pop ebx
 mov bh,al
 mov esi,ebx
OPCODE_EPILOG


OPCODE_PROLOG(0x00) ; BRK
 lea eax,[esi + 2]
 xor ebx,ebx
 mov bl,B_S
 Write_Stack ebx,ah
 dec bl
 Write_Stack ebx,al
 dec bl
 Pack_Flags
 Write_Stack ebx,al
 dec bl
 mov B_S,bl

 STORE_FLAGS_I al

 mov ebx,0xFFFE
 Read ebx
 push eax
 Read ebx,1
 pop ebx
 mov bh,al
 mov esi,ebx
OPCODE_EPILOG


OPCODE_PROLOG(0x58) ; CLI
 mov cl,B_I
 inc si
 test cl,cl
 jz .i_flag_not_cleared
 mov edx,B_IRequest
 test edx,edx
 jz .i_flag_not_cleared
 mov edx,B_ICount
 mov B_AfterCLI,cl
 mov ecx,1
 mov B_IBackup,edx
 mov B_ICount,ecx
.i_flag_not_cleared:
 STORE_FLAGS_I ah
OPCODE_EPILOG

OPCODE_PROLOG(0x28) ; PLP
 xor ebx,ebx
 inc si
 mov bl,B_S
 mov cl,B_I
 inc bl
 test cl,cl
 Read_Stack ebx,al
 jz .i_flag_not_cleared
 test al,FLAG_I
 jnz .i_flag_not_cleared
 mov edx,B_IRequest
 test edx,edx
 jz .i_flag_not_cleared
 mov edx,B_ICount
 mov B_AfterCLI,cl
 mov ecx,1
 mov B_IBackup,edx
 mov B_ICount,ecx
.i_flag_not_cleared:

 mov B_S,bl

 Unpack_Flags
OPCODE_EPILOG

OPCODE_PROLOG(0x08) ; PHP
 xor ebx,ebx
 inc si
 mov bl,B_S

 Pack_Flags

 Write_Stack ebx,al
 dec bl
 mov B_S,bl
OPCODE_EPILOG

OPCODE_PROLOG(0x18) ; CLC
 STORE_FLAGS_C ah
 inc si
OPCODE_EPILOG

OPCODE_PROLOG(0xB8) ; CLV
 STORE_FLAGS_V ah
 inc si
OPCODE_EPILOG

OPCODE_PROLOG(0xD8) ; CLD
 STORE_FLAGS_D ah
 inc si
OPCODE_EPILOG

OPCODE_PROLOG(0x38) ; SEC
 STORE_FLAGS_C al
 inc si
OPCODE_EPILOG

OPCODE_PROLOG(0xF8) ; SED
 STORE_FLAGS_D al
 inc si
OPCODE_EPILOG

OPCODE_PROLOG(0x78) ; SEI
 STORE_FLAGS_I al
 inc si
OPCODE_EPILOG


OPCODE_PROLOG(0x48) ; PHA
 xor ebx,ebx
 inc si
 mov bl,B_S
 mov al,B_A

 Write_Stack ebx,al
 dec bl
 mov B_S,bl
OPCODE_EPILOG

OPCODE_PROLOG(0x68) ; PLA
 xor ebx,ebx
 mov bl,B_S
 inc bl
 inc si
 mov B_S,bl

 Read_Stack ebx,al
 mov B_A,al
 STORE_FLAGS_NZ al
OPCODE_EPILOG

OPCODE_PROLOG(0x98) ; TYA
 mov al,B_Y
 inc si
 mov B_A,al
 STORE_FLAGS_NZ al
OPCODE_EPILOG

OPCODE_PROLOG(0xA8) ; TAY
 mov al,B_A
 inc si
 mov B_Y,al
 STORE_FLAGS_NZ al
OPCODE_EPILOG

OPCODE_PROLOG(0xC8) ; INY
 inc si
 Insn_INC B_Y
OPCODE_EPILOG

OPCODE_PROLOG(0x88) ; DEY
 inc si
 Insn_DEC B_Y
OPCODE_EPILOG

OPCODE_PROLOG(0x8A) ; TXA
 mov al,B_X
 inc si
 mov B_A,al
 STORE_FLAGS_NZ al
OPCODE_EPILOG

OPCODE_PROLOG(0xAA) ; TAX
 mov al,B_A
 inc si
 mov B_X,al
 STORE_FLAGS_NZ al
OPCODE_EPILOG

OPCODE_PROLOG(0xE8) ; INX
 inc si
 Insn_INC B_X
OPCODE_EPILOG

OPCODE_PROLOG(0xCA) ; DEX
 inc si
 Insn_DEC B_X
OPCODE_EPILOG

OPCODE_PROLOG(0xEA) ; NOP
 inc si
OPCODE_EPILOG

OPCODE_PROLOG(0x9A) ; TXS
 mov al,B_X
 inc si
 mov B_S,al
OPCODE_EPILOG

OPCODE_PROLOG(0xBA) ; TSX
 mov al,B_S
 inc si
 mov B_X,al
 STORE_FLAGS_NZ al
OPCODE_EPILOG


OPCODE_PROLOG(0x24) ; BIT $ss ZP
 Insn_BIT Zero_Page
OPCODE_EPILOG

OPCODE_PROLOG(0x2C) ; BIT $ssss ABS
 Insn_BIT Absolute
OPCODE_EPILOG


OPCODE_PROLOG(0x04) ; NOP $ss ZP
 Read_Zero_Page
OPCODE_EPILOG

OPCODE_PROLOG(0x05) ; ORA $ss ZP
 Insn_ORA Zero_Page
OPCODE_EPILOG

OPCODE_PROLOG(0x06) ; ASL $ss ZP
 RMW_Zero_Page Insn_ASL
OPCODE_EPILOG

OPCODE_PROLOG(0x07) ; SLO $ss ZP
 RMW_Zero_Page Insn_SLO
OPCODE_EPILOG

OPCODE_PROLOG(0x25) ; AND $ss ZP
 Insn_AND Zero_Page
OPCODE_EPILOG

OPCODE_PROLOG(0x26) ; ROL $ss ZP
 RMW_Zero_Page Insn_ROL
OPCODE_EPILOG

OPCODE_PROLOG(0x45) ; EOR $ss ZP
 Insn_EOR Zero_Page
OPCODE_EPILOG

OPCODE_PROLOG(0x46) ; LSR $ss ZP
 RMW_Zero_Page Insn_LSR
OPCODE_EPILOG

OPCODE_PROLOG(0x65) ; ADC $ss ZP
 Insn_ADC Zero_Page
OPCODE_EPILOG

OPCODE_PROLOG(0x66) ; ROR $ss ZP
 RMW_Zero_Page Insn_ROR
OPCODE_EPILOG

OPCODE_PROLOG(0x84) ; STY $ss ZP
 Insn_ST Zero_Page,B_Y
OPCODE_EPILOG

OPCODE_PROLOG(0x85) ; STA $ss ZP
 Insn_ST Zero_Page,B_A
OPCODE_EPILOG

OPCODE_PROLOG(0x86) ; STX $ss ZP
 Insn_ST Zero_Page,B_X
OPCODE_EPILOG

OPCODE_PROLOG(0xA4) ; LDY $ss ZP
 Insn_LD Zero_Page,B_Y
OPCODE_EPILOG

OPCODE_PROLOG(0xA5) ; LDA $ss ZP
 Insn_LD Zero_Page,B_A
OPCODE_EPILOG

OPCODE_PROLOG(0xA6) ; LDX $ss ZP
 Insn_LD Zero_Page,B_X
OPCODE_EPILOG

OPCODE_PROLOG(0xC4) ; CPY $ss ZP
 Insn_CMP Zero_Page,B_Y
OPCODE_EPILOG

OPCODE_PROLOG(0xC5) ; CMP $ss ZP
 Insn_CMP Zero_Page,B_A
OPCODE_EPILOG

OPCODE_PROLOG(0xC6) ; DEC $ss ZP
 RMW_Zero_Page Insn_DEC
OPCODE_EPILOG

OPCODE_PROLOG(0xE4) ; CPX $ss ZP
 Insn_CMP Zero_Page,B_X
OPCODE_EPILOG

OPCODE_PROLOG(0xE5) ; SBC $ss ZP
 Insn_SBC Zero_Page
OPCODE_EPILOG

OPCODE_PROLOG(0xE6) ; INC $ss ZP
 RMW_Zero_Page Insn_INC
OPCODE_EPILOG


OPCODE_PROLOG(0x0D) ; ORA $ssss ABS
 Insn_ORA Absolute
OPCODE_EPILOG

OPCODE_PROLOG(0x0E) ; ASL $ssss ABS
 RMW_Absolute Insn_ASL
OPCODE_EPILOG

OPCODE_PROLOG(0x0F) ; SLO $ssss ABS
 RMW_Absolute Insn_SLO
OPCODE_EPILOG

OPCODE_PROLOG(0x2D) ; AND $ssss ABS
 Insn_AND Absolute
OPCODE_EPILOG

OPCODE_PROLOG(0x2E) ; ROL $ssss ABS
 RMW_Absolute Insn_ROL
OPCODE_EPILOG

OPCODE_PROLOG(0x4D) ; EOR $ssss ABS
 Insn_EOR Absolute
OPCODE_EPILOG

OPCODE_PROLOG(0x4E) ; LSR $ssss ABS
 RMW_Absolute Insn_LSR
OPCODE_EPILOG

OPCODE_PROLOG(0x6D) ; ADC $ssss ABS
 Insn_ADC Absolute
OPCODE_EPILOG

OPCODE_PROLOG(0x6E) ; ROR $ssss ABS
 RMW_Absolute Insn_ROR
OPCODE_EPILOG

OPCODE_PROLOG(0x8C) ; STY $ssss ABS
 Insn_ST Absolute,B_Y
OPCODE_EPILOG

OPCODE_PROLOG(0x8D) ; STA $ssss ABS
 Insn_ST Absolute,B_A
OPCODE_EPILOG

OPCODE_PROLOG(0x8E) ; STX $ssss ABS
 Insn_ST Absolute,B_X
OPCODE_EPILOG

OPCODE_PROLOG(0xAC) ; LDY $ssss ABS
 Insn_LD Absolute,B_Y
OPCODE_EPILOG

OPCODE_PROLOG(0xAD) ; LDA $ssss ABS
 Insn_LD Absolute,B_A
OPCODE_EPILOG

OPCODE_PROLOG(0xAE) ; LDX $ssss ABS
 Insn_LD Absolute,B_X
OPCODE_EPILOG

OPCODE_PROLOG(0xCC) ; CPY $ssss ABS
 Insn_CMP Absolute,B_Y
OPCODE_EPILOG

OPCODE_PROLOG(0xCD) ; CMP $ssss ABS
 Insn_CMP Absolute,B_A
OPCODE_EPILOG

OPCODE_PROLOG(0xCE) ; DEC $ssss ABS
 RMW_Absolute Insn_DEC
OPCODE_EPILOG

OPCODE_PROLOG(0xEC) ; CPX $ssss ABS
 Insn_CMP Absolute, B_X
OPCODE_EPILOG

OPCODE_PROLOG(0xED) ; SBC $ssss ABS
 Insn_SBC Absolute
OPCODE_EPILOG

OPCODE_PROLOG(0xEE) ; INC $ssss ABS
 RMW_Absolute Insn_INC
OPCODE_EPILOG

OPCODE_PROLOG(0xEF) ; INS $ssss ABS
 RMW_Absolute Insn_INS
OPCODE_EPILOG


OPCODE_PROLOG(0x09) ; ORA #$ss IMM
 Insn_ORA Immediate
OPCODE_EPILOG

OPCODE_PROLOG(0x29) ; AND #$ss IMM
 Insn_AND Immediate
OPCODE_EPILOG

OPCODE_PROLOG(0x49) ; EOR #$ss IMM
 Insn_EOR Immediate
OPCODE_EPILOG

OPCODE_PROLOG(0x69) ; ADC #$ss IMM
 Insn_ADC Immediate
OPCODE_EPILOG

OPCODE_PROLOG(0x80) ; NOP #$ss IMM
 Read_Immediate
OPCODE_EPILOG

OPCODE_PROLOG(0xA0) ; LDY #$ss IMM
 Insn_LD Immediate,B_Y
OPCODE_EPILOG

OPCODE_PROLOG(0xA2) ; LDX #$ss IMM
 Insn_LD Immediate,B_X
OPCODE_EPILOG

OPCODE_PROLOG(0xA9) ; LDA #$ss IMM
 Insn_LD Immediate,B_A
OPCODE_EPILOG

OPCODE_PROLOG(0xC0) ; CPY #$ss IMM
 Insn_CMP Immediate,B_Y
OPCODE_EPILOG

OPCODE_PROLOG(0xC9) ; CMP #$ss IMM
 Insn_CMP Immediate,B_A
OPCODE_EPILOG

OPCODE_PROLOG(0xE0) ; CPX #$ss IMM
 Insn_CMP Immediate,B_X
OPCODE_EPILOG

OPCODE_PROLOG(0xE9) ; SBC #$ss IMM
 Insn_SBC Immediate
OPCODE_EPILOG


OPCODE_PROLOG(0x15) ; ORA $ss,x ZP,x
 Insn_ORA Zero_Page_Index_X
OPCODE_EPILOG

OPCODE_PROLOG(0x16) ; ASL $ss,x ZP,x
 RMW_Zero_Page_Index_X Insn_ASL
OPCODE_EPILOG

OPCODE_PROLOG(0x35) ; AND $ss,x ZP,x
 Insn_AND Zero_Page_Index_X
OPCODE_EPILOG

OPCODE_PROLOG(0x36) ; ROL $ss,x ZP,x
 RMW_Zero_Page_Index_X Insn_ROL
OPCODE_EPILOG

OPCODE_PROLOG(0x55) ; EOR $ss,x ZP,x
 Insn_EOR Zero_Page_Index_X
OPCODE_EPILOG

OPCODE_PROLOG(0x56) ; LSR $ss,x ZP,x
 RMW_Zero_Page_Index_X Insn_LSR
OPCODE_EPILOG

OPCODE_PROLOG(0x74) ; NOP $ss,x ZP,x
 Read_Zero_Page_Index_X
OPCODE_EPILOG

OPCODE_PROLOG(0x75) ; ADC $ss,x ZP,x
 Insn_ADC Zero_Page_Index_X
OPCODE_EPILOG

OPCODE_PROLOG(0x76) ; ROR $ss,x ZP,x
 RMW_Zero_Page_Index_X Insn_ROR
OPCODE_EPILOG

OPCODE_PROLOG(0x94) ; STY $ss,x ZP,x
 Insn_ST Zero_Page_Index_X,B_Y
OPCODE_EPILOG

OPCODE_PROLOG(0x95) ; STA $ss,x ZP,x
 Insn_ST Zero_Page_Index_X,B_A
OPCODE_EPILOG

OPCODE_PROLOG(0x96) ; STX $ss,y ZP,y
 Insn_ST Zero_Page_Index_Y,B_X
OPCODE_EPILOG

OPCODE_PROLOG(0xB4) ; LDY $ss,x ZP,x
 Insn_LD Zero_Page_Index_X,B_Y
OPCODE_EPILOG

OPCODE_PROLOG(0xB5) ; LDA $ss,x ZP,x
 Insn_LD Zero_Page_Index_X,B_A
OPCODE_EPILOG

OPCODE_PROLOG(0xB6) ; LDX $ss,y ZP,y
 Insn_LD Zero_Page_Index_Y,B_X
OPCODE_EPILOG

OPCODE_PROLOG(0xD5) ; CMP $ss,x ZP,x
 Insn_CMP Zero_Page_Index_X,B_A
OPCODE_EPILOG

OPCODE_PROLOG(0xD6) ; DEC $ss,x ZP,x
 RMW_Zero_Page_Index_X Insn_DEC
OPCODE_EPILOG

OPCODE_PROLOG(0xF5) ; SBC $ss,x ZP,x
 Insn_SBC Zero_Page_Index_X
OPCODE_EPILOG

OPCODE_PROLOG(0xF6) ; INC $ss,x ZP,x
 RMW_Zero_Page_Index_X Insn_INC
OPCODE_EPILOG


OPCODE_PROLOG(0x19) ; ORA $ssss,y ABS,y
 Insn_ORA Absolute_Index_Y
OPCODE_EPILOG

OPCODE_PROLOG(0x1D) ; ORA $ssss,x ABS,x
 Insn_ORA Absolute_Index_X
OPCODE_EPILOG

OPCODE_PROLOG(0x1E) ; ASL $ssss,x ABS,x
 RMW_Absolute_Index_X Insn_ASL
OPCODE_EPILOG

OPCODE_PROLOG(0x39) ; AND $ssss,y ABS,y
 Insn_AND Absolute_Index_Y
OPCODE_EPILOG

OPCODE_PROLOG(0x3D) ; AND $ssss,x ABS,x
 Insn_AND Absolute_Index_X
OPCODE_EPILOG

OPCODE_PROLOG(0x3E) ; ROL $ssss,x ABS,x
 RMW_Absolute_Index_X Insn_ROL
OPCODE_EPILOG

OPCODE_PROLOG(0x59) ; EOR $ssss,y ABS,y
 Insn_EOR Absolute_Index_Y
OPCODE_EPILOG

OPCODE_PROLOG(0x5D) ; EOR $ssss,x ABS,x
 Insn_EOR Absolute_Index_X
OPCODE_EPILOG

OPCODE_PROLOG(0x5E) ; LSR $ssss,x ABS,x
 RMW_Absolute_Index_X Insn_LSR
OPCODE_EPILOG

OPCODE_PROLOG(0x79) ; ADC $ssss,y ABS,y
 Insn_ADC Absolute_Index_Y
OPCODE_EPILOG

OPCODE_PROLOG(0x7D) ; ADC $ssss,x ABS,x
 Insn_ADC Absolute_Index_X
OPCODE_EPILOG

OPCODE_PROLOG(0x7E) ; ROR $ssss,x ABS,x
 RMW_Absolute_Index_X Insn_ROR
OPCODE_EPILOG

OPCODE_PROLOG(0x99) ; STA $ssss,y ABS,y
 Insn_ST Absolute_Index_Y,B_A
OPCODE_EPILOG

OPCODE_PROLOG(0x9D) ; STA $ssss,x ABS,x
 Insn_ST Absolute_Index_X,B_A
OPCODE_EPILOG

OPCODE_PROLOG(0xB9) ; LDA $ssss,y ABS,y
 Insn_LD Absolute_Index_Y,B_A
OPCODE_EPILOG

OPCODE_PROLOG(0xBC) ; LDY $ssss,x ABS,x
 Insn_LD Absolute_Index_X,B_Y
OPCODE_EPILOG

OPCODE_PROLOG(0xBD) ; LDA $ssss,x ABS,x
 Insn_LD Absolute_Index_X,B_A
OPCODE_EPILOG

OPCODE_PROLOG(0xBE) ; LDX $ssss,y ABS,y
 Insn_LD Absolute_Index_Y,B_X
OPCODE_EPILOG

OPCODE_PROLOG(0xD9) ; CMP $ssss,y ABS,y
 Insn_CMP Absolute_Index_Y,B_A
OPCODE_EPILOG

OPCODE_PROLOG(0xDD) ; CMP $ssss,x ABS,x
 Insn_CMP Absolute_Index_X,B_A
OPCODE_EPILOG

OPCODE_PROLOG(0xDE) ; DEC $ssss,x ABS,x
 RMW_Absolute_Index_X Insn_DEC
OPCODE_EPILOG

OPCODE_PROLOG(0xF9) ; SBC $ssss,y ABS,y
 Insn_SBC Absolute_Index_Y
OPCODE_EPILOG

OPCODE_PROLOG(0xFD) ; SBC $ssss,x ABS,x
 Insn_SBC Absolute_Index_X
OPCODE_EPILOG

OPCODE_PROLOG(0xFE) ; INC $ssss,x ABS,x
 RMW_Absolute_Index_X Insn_INC
OPCODE_EPILOG

OPCODE_PROLOG(0xFF) ; INS $ssss,X ABS,x
 RMW_Absolute_Index_X Insn_INS
OPCODE_EPILOG


OPCODE_PROLOG(0x01) ; ORA ($ss,x)
 Insn_ORA Indexed_Indirect_X
OPCODE_EPILOG

OPCODE_PROLOG(0x11) ; ORA ($ss),y
 Insn_ORA Indirect_Indexed_Y
OPCODE_EPILOG

OPCODE_PROLOG(0x21) ; AND ($ss,x)
 Insn_AND Indexed_Indirect_X
OPCODE_EPILOG

OPCODE_PROLOG(0x31) ; AND ($ss),y
 Insn_AND Indirect_Indexed_Y
OPCODE_EPILOG

OPCODE_PROLOG(0x41) ; EOR ($ss,x)
 Insn_EOR Indexed_Indirect_X
OPCODE_EPILOG

OPCODE_PROLOG(0x51) ; EOR ($ss),y
 Insn_EOR Indirect_Indexed_Y
OPCODE_EPILOG

OPCODE_PROLOG(0x61) ; ADC ($ss,x)
 Insn_ADC Indexed_Indirect_X
OPCODE_EPILOG

OPCODE_PROLOG(0x71) ; ADC ($ss),y
 Insn_ADC Indirect_Indexed_Y
OPCODE_EPILOG

OPCODE_PROLOG(0x81) ; STA ($ss,x)
 Insn_ST Indexed_Indirect_X,B_A
OPCODE_EPILOG

OPCODE_PROLOG(0x91) ; STA ($ss),y
 Insn_ST Indirect_Indexed_Y,B_A
OPCODE_EPILOG

OPCODE_PROLOG(0xA1) ; LDA ($ss,x)
 Insn_LD Indexed_Indirect_X,B_A
OPCODE_EPILOG

OPCODE_PROLOG(0xB1) ; LDA ($ss),y
 Insn_LD Indirect_Indexed_Y,B_A
OPCODE_EPILOG

OPCODE_PROLOG(0xC1) ; CMP ($ss,x)
 Insn_CMP Indexed_Indirect_X,B_A
OPCODE_EPILOG

OPCODE_PROLOG(0xD1) ; CMP ($ss),y
 Insn_CMP Indirect_Indexed_Y,B_A
OPCODE_EPILOG

OPCODE_PROLOG(0xE1) ; SBC ($ss,x)
 Insn_SBC Indexed_Indirect_X
OPCODE_EPILOG

OPCODE_PROLOG(0xF1) ; SBC ($ss),y
 Insn_SBC Indirect_Indexed_Y
OPCODE_EPILOG


OPCODE_PROLOG(0x0A) ; ASL a ACC
 inc si
 Insn_ASL B_A
OPCODE_EPILOG

OPCODE_PROLOG(0x2A) ; ROL a ACC
 inc si
 Insn_ROL B_A
OPCODE_EPILOG

OPCODE_PROLOG(0x4A) ; LSR a ACC
 inc si
 Insn_LSR B_A
OPCODE_EPILOG

OPCODE_PROLOG(0x6A) ; ROR a ACC
 inc si
 Insn_ROR B_A
OPCODE_EPILOG


OPCODE_PROLOG(0xF2) ; JAM
 mov B_Jammed,al
OPCODE_EPILOG

section .data
badop_str:db "Unrecognized instruction: $%02X at PC=$%04X",10,0

section .text

OPCODE_PROLOG_DEFAULT
;   if(R->TrapBadOps)
;   {
 mov cl,B_TrapBadOps
 test cl,cl
 jz .no_trap

;       FN2A03_report_bad_opcode
;       (
;           Fetch(PC.word),(UINT16)(PC.word)
;       );
 push esi
 push eax
 call C_LABEL(FN2A03_report_bad_opcode)
 add esp,byte 8

.no_trap:
;   }

%ifdef DEBUG
;       FN2A03_opcode_fallback_trace();
 call C_LABEL(FN2A03_opcode_fallback_trace)
%endif
OPCODE_EPILOG

section .data
OpTable:
 dd Op0x00, Op0x01, OpNone, OpNone, Op0x04, Op0x05, Op0x06, Op0x07
 dd Op0x08, Op0x09, Op0x0A, OpNone, OpNone, Op0x0D, Op0x0E, Op0x0F
 dd Op0x10, Op0x11, OpNone, OpNone, OpNone, Op0x15, Op0x16, OpNone
 dd Op0x18, Op0x19, OpNone, OpNone, OpNone, Op0x1D, Op0x1E, OpNone
 dd Op0x20, Op0x21, OpNone, OpNone, Op0x24, Op0x25, Op0x26, OpNone
 dd Op0x28, Op0x29, Op0x2A, OpNone, Op0x2C, Op0x2D, Op0x2E, OpNone
 dd Op0x30, Op0x31, OpNone, OpNone, OpNone, Op0x35, Op0x36, OpNone
 dd Op0x38, Op0x39, OpNone, OpNone, OpNone, Op0x3D, Op0x3E, OpNone
 dd Op0x40, Op0x41, OpNone, OpNone, OpNone, Op0x45, Op0x46, OpNone
 dd Op0x48, Op0x49, Op0x4A, OpNone, Op0x4C, Op0x4D, Op0x4E, OpNone
 dd Op0x50, Op0x51, OpNone, OpNone, OpNone, Op0x55, Op0x56, OpNone
 dd Op0x58, Op0x59, OpNone, OpNone, OpNone, Op0x5D, Op0x5E, OpNone
 dd Op0x60, Op0x61, OpNone, OpNone, OpNone, Op0x65, Op0x66, OpNone
 dd Op0x68, Op0x69, Op0x6A, OpNone, Op0x6C, Op0x6D, Op0x6E, OpNone
 dd Op0x70, Op0x71, OpNone, OpNone, Op0x74, Op0x75, Op0x76, OpNone
 dd Op0x78, Op0x79, OpNone, OpNone, OpNone, Op0x7D, Op0x7E, OpNone
 dd Op0x80, Op0x81, OpNone, OpNone, Op0x84, Op0x85, Op0x86, OpNone
 dd Op0x88, OpNone, Op0x8A, OpNone, Op0x8C, Op0x8D, Op0x8E, OpNone
 dd Op0x90, Op0x91, OpNone, OpNone, Op0x94, Op0x95, Op0x96, OpNone
 dd Op0x98, Op0x99, Op0x9A, OpNone, OpNone, Op0x9D, OpNone, OpNone
 dd Op0xA0, Op0xA1, Op0xA2, OpNone, Op0xA4, Op0xA5, Op0xA6, OpNone
 dd Op0xA8, Op0xA9, Op0xAA, OpNone, Op0xAC, Op0xAD, Op0xAE, OpNone
 dd Op0xB0, Op0xB1, OpNone, OpNone, Op0xB4, Op0xB5, Op0xB6, OpNone
 dd Op0xB8, Op0xB9, Op0xBA, OpNone, Op0xBC, Op0xBD, Op0xBE, OpNone
 dd Op0xC0, Op0xC1, OpNone, OpNone, Op0xC4, Op0xC5, Op0xC6, OpNone
 dd Op0xC8, Op0xC9, Op0xCA, OpNone, Op0xCC, Op0xCD, Op0xCE, OpNone
 dd Op0xD0, Op0xD1, OpNone, OpNone, OpNone, Op0xD5, Op0xD6, OpNone
 dd Op0xD8, Op0xD9, OpNone, OpNone, OpNone, Op0xDD, Op0xDE, OpNone
 dd Op0xE0, Op0xE1, OpNone, OpNone, Op0xE4, Op0xE5, Op0xE6, OpNone
 dd Op0xE8, Op0xE9, Op0xEA, OpNone, Op0xEC, Op0xED, Op0xEE, Op0xEF
 dd Op0xF0, Op0xF1, Op0xF2, OpNone, OpNone, Op0xF5, Op0xF6, OpNone
 dd Op0xF8, Op0xF9, OpNone, OpNone, OpNone, Op0xFD, Op0xFE, Op0xFF

%endif ; !defined(CODES_I)

