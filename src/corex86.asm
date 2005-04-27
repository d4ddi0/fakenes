%if 0

FakeNES - A portable, Open Source NES emulator.

core.c: Implementation of the RP2A03G CPU emulation

Copyright (c) 2005, Charles Bilyue' and Randy McDowell.

This is free software.  See 'LICENSE' for details.
You must read and accept the license prior to use.

 This file contains x86-specific emulation core functions for
the Ricoh RP2A03G CPU, as used in the Nintendo Famicom (Family
Computer) and NES (Nintendo Entertainment System).

%endif


;%define BINARY_DEBUG
%ifndef NO_ASM_CORE

;register allocation;
;assumes C code preserves ebx, ebp, esi, edi, trash eax, ecx, edx
;eax,ecx,edx = general work registers, trashed by memory map access
;ebx = address work register
;esi = PC
;edi = pointer to reg struct

section .text

                               ; Compilation options:
%define CYCLE_LENGTH 3         ; Number of cycles that one
                               ; CPU cycle uses.

; This code assumes the standard FakeNES memory map, and is equivalent
;to FN2A03 C core when INLINE_MEMORY_HANDLERS, FAST_STACK, and FAST_ZP
;are defined.

;%define ALT_DEBUG             ; Compile debugging version

%define FN2A03_INT_NONE     0  ; No interrupt required
%define FN2A03_INT_IRQ_NONE 0  ; No interrupt required
%define FN2A03_INT_NMI      1  ; Non-maskable interrupt

%define FN2A03_INT_IRQ_BASE         2
; Maskable IRQ, cleared after an IRQ is acknowledged
%define FN2A03_INT_IRQ_SINGLE_SHOT  (FN2A03_INT_IRQ_BASE)
; Maskable IRQs cleared via FN2A03_Clear_Interrupt()
%define FN2A03_INT_IRQ_SOURCE(x)    (FN2A03_INT_IRQ_BASE + 1 + (x))
%define FN2A03_INT_IRQ_SOURCE_MAX   (31 - 1)

                               ; 2A03 status flags:
%define C_FLAG    0x01         ; 1: Carry occured
%define Z_FLAG    0x02         ; 1: Result is zero
%define I_FLAG    0x04         ; 1: Interrupts disabled
%define D_FLAG    0x08         ; 1: Decimal mode
%define B_FLAG    0x10         ; Break [0 on stk after int]
%define R_FLAG    0x20         ; Always 1
%define V_FLAG    0x40         ; 1: Overflow occured
%define N_FLAG    0x80         ; 1: Result is negative

%if 0
 The following data types must be defined.
  UINT8     unsigned    sizeof(UINT8) == 1
  INT8      signed      sizeof(INT8) == 1
  UINT16    unsigned    sizeof(UINT16) == 2
  UINT32    unsigned    sizeof(UINT32) == 4
  PAIR      union       sizeof(PAIR) == 2
   { UINT16 word; struct { UINT8 low, high } bytes; }
%endif

;%include "misc.inc"

%ifndef C_LABELS_PREFIX
%define C_LABELS_PREFIX
%endif

%ifndef C_LABELS_SUFFIX
%define C_LABELS_SUFFIX
%endif

%ifnidn _ %+ C_LABELS_PREFIX %+ _,__

%ifnidn _ %+ C_LABELS_SUFFIX %+ _,__
%define C_LABEL(x) C_LABELS_PREFIX %+ x %+ C_LABELS_SUFFIX
%else
%define C_LABEL(x) C_LABELS_PREFIX %+ x
%endif

%else

%ifnidn _ %+ C_LABELS_SUFFIX %+ _,__
%define C_LABEL(x) x %+ C_LABELS_SUFFIX
%else
%define C_LABEL(x) x
%endif

%endif


%define EXTERN_C(x) extern C_LABEL(x)

%macro EXPORT 1-2+
global %1
%1:
%2
%endmacro

%macro EXPORT_C 1-2+
global C_LABEL(%1)
C_LABEL(%1):
%2
%endmacro

%define R_Base edi

%include "core/x86/offsets.inc"

EXTERN_C(cpu_block_2k_read_address)
EXTERN_C(cpu_block_2k_read_handler)
EXTERN_C(cpu_block_2k_write_address)
EXTERN_C(cpu_block_2k_write_handler)
EXTERN_C(cpu_patch_table)
EXTERN_C(Cycles)
EXTERN_C(FN2A03_Interrupt)
EXTERN_C(cpu_ram)
EXTERN_C(FN2A03_report_bad_opcode)
EXTERN_C(FN2A03_opcode_fallback_trace)


%macro Read_ZP 2
 mov byte %2,[C_LABEL(cpu_ram) + %1]
%endmacro


%macro Write_ZP 2
 mov byte [C_LABEL(cpu_ram) + %1],%2
%endmacro


%macro Read_Stack 2
 mov byte %2,[C_LABEL(cpu_ram) + 0x100 + %1]
%endmacro


%macro Write_Stack 2
 mov byte [C_LABEL(cpu_ram) + 0x100 + %1],%2
%endmacro


;%1 = address register
;%2 = if nonzero, add 1 to address
;%3 = register to copy previous value of al to
%macro Read 1-3 0,al
%if %2
 lea edx,[%1 + 1]
%else
 mov edx,%1
%endif
 shr edx,11
%if %2
 inc %1
%endif

 mov ecx,[C_LABEL(cpu_block_2k_read_address) + edx * 4]
%ifnidni %3,al
 mov %3,al
%endif
 test ecx,ecx
 jnz %%read_direct

 push %1
 call [C_LABEL(cpu_block_2k_read_handler) + edx * 4]
 pop edx
 jmp %%read_complete

%%read_direct:
 mov dl,[C_LABEL(cpu_patch_table) + %1]
%ifnidni %3,al
 mov %3,al
%endif
 mov al,[ecx + %1]
 add al,dl

%%read_complete:
%endmacro


;%1 = address register
;Alternate macros for when the value read is thrown away
%macro Dummy_Read 1
 mov edx,%1
 shr edx,11

 mov ecx,[C_LABEL(cpu_block_2k_read_address) + edx * 4]
 test ecx,ecx
 jnz %%read_complete

 push %1
 call [C_LABEL(cpu_block_2k_read_handler) + edx * 4]
 pop edx

%%read_complete:
%endmacro

;%1 = address register
;%2 = if nonzero, add 1 to address
%macro Write 1-2 0
%if %2
 lea edx,[%1 + 1]
%else
 mov edx,%1
%endif
 shr edx,11
%if %2
 inc %1
%endif

 mov ecx,[C_LABEL(cpu_block_2k_write_address) + edx * 4]
 test ecx,ecx
 jnz %%write_direct

 push eax
 push %1
 call [C_LABEL(cpu_block_2k_write_handler) + edx * 4]
 add esp,byte 8
 jmp %%write_complete

%%write_direct:
 mov [ecx + %1],al

%%write_complete:
%endmacro

%macro Dummy_Read_ZP 2
%endmacro

%macro Dummy_Read_Stack 2
%endmacro


;%1 = address register
;Alternate macros for when the value written is re-written in the next cycle
;%1 = address register
%macro Dummy_Write 1
 mov edx,%1
 shr edx,11

 mov ecx,[C_LABEL(cpu_block_2k_write_address) + edx * 4]
 test ecx,ecx
 jnz %%write_complete

 push eax
 push %1
 call [C_LABEL(cpu_block_2k_write_handler) + edx * 4]
 add esp,byte 8

%%write_complete:
%endmacro

%macro Dummy_Write_ZP 2
%endmacro

%macro Dummy_Write_Stack 2
%endmacro


; Corrupts arg 2, returns value in arg 3 (default to cl, al)
;|N|V|1|B|D|I|Z|C|
;%1 = break flag, %2 = scratchpad, %3 = output
%macro Pack_Flags 0-3 1,cl,al
 mov byte %3,B_N
 shr byte %3,7

 mov byte %2,B_V
 add byte %2,-1
 adc byte %3,%3

 mov byte %2,B_D
 shl byte %3,byte 2
%if %1
 or byte %3,3
%else
 or byte %3,2
%endif

 add byte %2,-1
 adc byte %3,%3

 mov byte %2,B_I
 add byte %2,-1
 adc byte %3,%3

 mov byte %2,B_Z
 cmp byte %2,1
 adc byte %3,%3

 mov byte %2,B_C
 add byte %2,-1
 adc byte %3,%3
%endmacro


; Corrupts arg 1, uses value in arg 2 (default to cl, al)
;%1 = scratchpad, %2 = input
%macro Unpack_Flags 0-2 cl,al
 mov byte B_N,%2    ;negative
 shl byte %2,2  ;start next (overflow)

 sbb byte %1,%1
 shl byte %2,3  ;start next (decimal mode)
 mov byte B_V,%1

 sbb byte %1,%1
 add byte %2,%2 ;start next (interrupt disable)
 mov byte B_D,%1

 sbb byte %1,%1
 add byte %2,%2 ;start next (zero)
 mov byte B_I,%1

 sbb byte %1,%1
 xor byte %1,0xFF
 add byte %2,%2 ;start next (carry)
 mov byte B_Z,%1

 sbb byte %1,%1
 mov byte B_C,%1
%endmacro


%ifdef DEBUG
section .data
opcode_count:dd 0
opcode_trace:times 10 db 0
section .text
%endif


%ifdef BINARY_DEBUG
EXTERN_C(ops_dmp)
EXTERN_C(fputc)
EXTERN_C(fwrite)
%macro fputc 1
 push dword [C_LABEL(ops_dmp)]
 mov al,%1
 push eax
 call C_LABEL(fputc)
 add esp,byte 8
%endmacro
%endif
%if 0
 FN2A03_Run()

  This function will execute RP2A03G code until the cycle
 counter expires.
%endif
EXPORT_C FN2A03_Run
;void FN2A03_Run(FN2A03 *R)
;{
; if (R->Jammed) return;
 mov edx,[esp+4]
 mov al,[edx + O_Jammed]
 test al,al
 jnz .jammed

 push edi
 mov edi,edx

 push esi
 xor esi,esi

 push ebp
 push ebx

; for(;;)
; {
;   PAIR PC;
;   PC.word=R->PC.word;
.begin_exec:
 mov si,B_PC

;   while ((R->ICount - R->Cycles) > 0)
;   {
;     UINT8 opcode, cycles;
.check_cycles:
 mov eax,B_ICount
 mov ecx,B_Cycles
 sub eax,ecx
 test eax,eax
 jle .check_cli_state

%ifdef ALT_DEBUG
;     /* Turn tracing on when reached trap address */
;     if(PC.word==R->Trap) R->Trace=1;
 cmp B_Trap,si
 jne .no_trace

 mov byte B_Trace,1
.no_trap:

;     /* Call single-step debugger, exit if requested */
;     if(R->Trace)
;     {
 mov cl,B_Trace
 test cl,cl
 jz .no_trace

;       R->PC.word=PC.word;
 mov B_PC,si
;       if(!FN2A03_Debug(R)) return;
 push edi
 call C_LABEL(FN2A03)
 pop ecx

 test eax,eax
 jz .return

.no_exit:
.no_trace:
;     }
%endif

;     opcode=Fetch(PC.word);
 Read esi
 and eax,0xFF

%ifdef BINARY_DEBUG
 mov B_PC,si
 push eax
; if (ops_dmp)
 mov eax,[C_LABEL(ops_dmp)]
 test eax,eax
 jz .no_handle
;  fputc(R->PC.bytes.high, ops_dmp);
 fputc [R_Base + O_PC + 1]
;  fputc(R->PC.bytes.low, ops_dmp);
 fputc B_PC
;  fputc(R->A, ops_dmp);
 fputc B_A
;  fputc(R->X, ops_dmp);
 fputc B_X
;  fputc(R->Y, ops_dmp);
 fputc B_Y
;  fputc(R->S, ops_dmp);
 fputc B_S
;  fputc(Pack_Flags(), ops_dmp);
 push dword [C_LABEL(ops_dmp)]
 Pack_Flags
 push eax
 call C_LABEL(fputc)
 add esp,byte 8
;  fputc(opcode, ops_dmp);
 fputc [esp+4]
;  fwrite(&R->Cycles, 1, 4, ops_dmp);
 push dword [C_LABEL(ops_dmp)]
 push dword 4
 push dword 1
 lea eax,[edi + O_Cycles]
 push eax
 call C_LABEL(fwrite)
 add esp,byte 16
;  fwrite(&R->ICount, 1, 4, ops_dmp);
 push dword [C_LABEL(ops_dmp)]
 push dword 4
 push dword 1
 lea eax,[edi + O_ICount]
 push eax
 call C_LABEL(fwrite)
 add esp,byte 16
; }
.no_handle:
 pop eax
%endif

;     cycles=Cycles[opcode];
;     R->Cycles+=cycles;
 xor edx,edx
 mov ecx,B_Cycles
 mov dl,[C_LABEL(Cycles) + eax]
 add ecx,edx
 mov B_Cycles,ecx

%ifdef DEBUG
;     opcode_trace[opcode_count++]=opcode;
;     if (opcode_count==10) opcode_count=0;
 mov ecx,[opcode_count]
 mov [opcode_trace + ecx],al
 inc ecx
 cmp ecx,byte 10
 je .no_wrap
 xor ecx,ecx
.no_wrap:
 mov [opcode_count],ecx
%endif

;     switch(opcode)
;     {
;       PAIR address, result;
;       UINT8 zero_page_address, data;
;%include "core/codes.h"
;     }
;   }
 jmp [OpTable + eax * 4]

.check_cli_state:
;   R->PC.word=PC.word;
 mov B_PC,si

;   /* cycle counter expired, or we wouldn't be here */
;   if (!R->AfterCLI) return;
 mov al,B_AfterCLI
 test al,al
 jnz .after_cli

.return:
 pop ebx
 pop ebp
 pop esi
 pop edi

; if (R->Jammed) return;
.jammed:
 ret

.after_cli:
;   /* If we have come after CLI, get FN2A03_INT_? from IRequest */
;   R->ICount = R->IBackup;   /* Restore the ICount        */
;   R->AfterCLI=0;            /* Done with AfterCLI state  */
 mov eax,B_IBackup
 xor ecx,ecx
 mov B_ICount,eax
 mov B_AfterCLI,cl

;   /* Process pending interrupts */
;   if (R->IRequest) FN2A03_Interrupt(R, FN2A03_INT_NONE);
; }
 mov eax,B_IRequest
 test eax,eax
 jz .begin_exec

 push byte FN2A03_INT_NONE
 push edi
 call C_LABEL(FN2A03_Interrupt)
 add esp,byte 8
 jmp .begin_exec


FLAG_C equ 1
FLAG_Z equ 2
FLAG_I equ 4
FLAG_D equ 8
FLAG_B equ 0x10
FLAG_1 equ 0x20
FLAG_V equ 0x40
FLAG_N equ 0x80

FLAG_B1 equ (FLAG_B | FLAG_1)
FLAG_NZ equ (FLAG_N | FLAG_Z)
FLAG_NZC equ (FLAG_NZ | FLAG_C)


;%1 = flag, %2 = wheretogo
%macro JUMP_FLAG 2
%if %1 == FLAG_N
 mov ch,B_N
 test ch,ch
 js %2
%elif %1 == FLAG_V
 mov ch,B_V
 test ch,ch
 jnz %2
%elif %1 == FLAG_D
 mov ch,B_D
 test ch,ch
 jnz %2
%elif %1 == FLAG_I
 mov ch,B_I
 test ch,ch
 jnz %2
%elif %1 == FLAG_Z
 mov ch,B_Z
 test ch,ch
 jz %2
%elif %1 == FLAG_C
 mov ch,B_C
 test ch,ch
 jnz %2
%else
%error Unhandled flag in JUMP_FLAG
%endif
%endmacro

;%1 = flag, %2 = wheretogo
%macro JUMP_NOT_FLAG 2
%if %1 == FLAG_N
 mov ch,B_N
 test ch,ch
 jns %2
%elif %1 == FLAG_V
 mov ch,B_V
 test ch,ch
 jz %2
%elif %1 == FLAG_D
 mov ch,B_D
 test ch,ch
 jz %2
%elif %1 == FLAG_I
 mov ch,B_I
 test ch,ch
 jz %2
%elif %1 == FLAG_Z
 mov ch,B_Z
 test ch,ch
 jnz %2
%elif %1 == FLAG_C
 mov ch,B_C
 test ch,ch
 jz %2
%else
%error Unhandled flag in JUMP_NOT_FLAG
%endif
%endmacro

%macro STORE_FLAGS_E 1
 mov byte B_E,%1
%endmacro

%macro STORE_FLAGS_N 1
 mov byte B_N,%1
%endmacro

%macro STORE_FLAGS_V 1
 mov byte B_V,%1
%endmacro

%macro STORE_FLAGS_D 1
 mov byte B_D,%1
%endmacro

%macro STORE_FLAGS_I 1
 mov byte B_I,%1
%endmacro

%macro STORE_FLAGS_Z 1
 mov byte B_Z,%1
%endmacro

%macro STORE_FLAGS_C 1
 mov byte B_C,%1
%endmacro

%macro STORE_FLAGS_NZ 1
 STORE_FLAGS_N %1
 STORE_FLAGS_Z %1
%endmacro

%macro STORE_FLAGS_NZC 2
 STORE_FLAGS_N %1
 STORE_FLAGS_Z %1
 STORE_FLAGS_C %2
%endmacro


%define OPCODE_PROLOG(x) Op %+ x:
%define OPCODE_PROLOG_DEFAULT OpNone:
%define OPCODE_EPILOG jmp C_LABEL(FN2A03_Run).check_cycles

%include "core/x86/addr.inc"
%include "core/x86/insns.inc"
%include "core/x86/codes.inc"

%endif  ;!defined(NO_ASM_CORE)
