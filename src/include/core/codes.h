/** M6502: portable 6502 emulator ****************************/
/**                                                         **/
/**                          Codes.h                        **/
/**                                                         **/
/** This file contains implementation for the main table of **/
/** 6502 commands. It is included from 6502.c.              **/
/**                                                         **/
/** Copyright (C) Marat Fayzullin 1996                      **/
/**               Alex Krasivsky  1996                      **/
/**     You are not allowed to distribute this software     **/
/**     commercially. Please, notify me, if you make any    **/
/**     changes to this file.                               **/
/*************************************************************/
/* 11.June     2002 stainless $f2 JAM/HLT opcode added.      */
/* 16.January  2002 TRAC      Added flag emulation to TSX.   */
/* 09.January  2002 TRAC      Added opcodes 04, 0F, 80.      */
/* 07.January  2002 TRAC      Altered method of flag         */
/*                            emulation.                     */
/* 10.December 2001 TRAC      Added opcodes 07 and 74.       */
/* 04.December 2001 stainless $ff INS word,X opcode added.   */
/* 04.December 2001 stainless $ef INS word opcode added.     */
/* 02.December 2001 stainless Opcode fallback trace.         */
/* 27.November 2001 stainless Fixed GCC compiler warning.    */
/* 27.November 2001 stainless Added N6502 $6c opcode bug.    */
/* 26.November 2001 stainless Integrated into FakeNES.       */
/*************************************************************/

OPCODE_PROLOG(0x10) /* BPL * REL */
    if (R->N&N_FLAG) R->PC.W++; else { M_JR; } OPCODE_EXIT
OPCODE_EPILOG

OPCODE_PROLOG(0x30) /* BMI * REL */
    if (R->N&N_FLAG) { M_JR; } else R->PC.W++; OPCODE_EXIT
OPCODE_EPILOG

OPCODE_PROLOG(0xD0) /* BNE * REL */
    if (!R->Z) R->PC.W++; else { M_JR; } OPCODE_EXIT
OPCODE_EPILOG

OPCODE_PROLOG(0xF0) /* BEQ * REL */
    if (!R->Z) { M_JR; } else R->PC.W++; OPCODE_EXIT
OPCODE_EPILOG

OPCODE_PROLOG(0x90) /* BCC * REL */
    if (R->C) R->PC.W++; else { M_JR; } OPCODE_EXIT
OPCODE_EPILOG

OPCODE_PROLOG(0xB0) /* BCS * REL */
    if (R->C) { M_JR; } else R->PC.W++; OPCODE_EXIT
OPCODE_EPILOG

OPCODE_PROLOG(0x50) /* BVC * REL */
    if (R->V) R->PC.W++; else { M_JR; } OPCODE_EXIT
OPCODE_EPILOG

OPCODE_PROLOG(0x70) /* BVS * REL */
    if (R->V) { M_JR; } else R->PC.W++; OPCODE_EXIT
OPCODE_EPILOG


OPCODE_PROLOG(0x40) /* RTI */
    M_POP(R->P);
    if((R->IRequest!=INT_NONE)&&(!(R->P&I_FLAG) && R->I))
    {
      R->AfterCLI=1;
      R->IBackup=R->ICount;
      R->ICount=1;
    }
    R->P|=R_FLAG;M_UNFIX_P();M_POP(R->PC.B.l);M_POP(R->PC.B.h);
OPCODE_EPILOG

OPCODE_PROLOG(0x60) /* RTS */
    M_POP(R->PC.B.l);M_POP(R->PC.B.h);R->PC.W++;
OPCODE_EPILOG


OPCODE_PROLOG(0x20) /* JSR $ssss ABS */
    K.B.l=Op6502(R->PC.W++);
    K.B.h=Op6502(R->PC.W);
    M_PUSH(R->PC.B.h);
    M_PUSH(R->PC.B.l);
    R->PC=K;
OPCODE_EPILOG

OPCODE_PROLOG(0x4C) /* JMP $ssss ABS */
    M_LDWORD(K);R->PC=K;
OPCODE_EPILOG

OPCODE_PROLOG(0x6C) /* JMP ($ssss) ABDINDIR */
    M_LDWORD(K);
    R->PC.B.l=Rd6502(K.W);
    K.B.l++;
    R->PC.B.h=Rd6502(K.W);
OPCODE_EPILOG


OPCODE_PROLOG(0x00) /* BRK */
  R->PC.W++;
  M_PUSH(R->PC.B.h);M_PUSH(R->PC.B.l);
  M_FIX_P();
  M_PUSH(R->P|B_FLAG);
  R->I=1;R->D=0;
  R->PC.B.l=Rd6502(0xFFFE);
  R->PC.B.h=Rd6502(0xFFFF);
OPCODE_EPILOG


OPCODE_PROLOG(0x58) /* CLI */
    if((R->IRequest!=INT_NONE)&&(R->I))
    {
      R->AfterCLI=1;
      R->IBackup=R->ICount;
      R->ICount=1;
    }
    R->I=0;
OPCODE_EPILOG

OPCODE_PROLOG(0x28) /* PLP */
    M_POP(R->P);
    if((R->IRequest!=INT_NONE)&&(!(R->P&I_FLAG) && R->I))
    {
      R->AfterCLI=1;
      R->IBackup=R->ICount;
      R->ICount=1;
    }
    R->P|=R_FLAG;
    M_UNFIX_P();
OPCODE_EPILOG

OPCODE_PROLOG(0x08) /* PHP */
    M_FIX_P(); M_PUSH(R->P);
OPCODE_EPILOG

OPCODE_PROLOG(0x18) /* CLC */
    R->C=0;
OPCODE_EPILOG

OPCODE_PROLOG(0xB8) /* CLV */
    R->V=0;
OPCODE_EPILOG

OPCODE_PROLOG(0xD8) /* CLD */
    R->D=0;
OPCODE_EPILOG

OPCODE_PROLOG(0x38) /* SEC */
    R->C=1;
OPCODE_EPILOG

OPCODE_PROLOG(0xF8) /* SED */
    R->D=1;
OPCODE_EPILOG

OPCODE_PROLOG(0x78) /* SEI */
    R->I=1;
OPCODE_EPILOG


OPCODE_PROLOG(0x48) /* PHA */
    M_PUSH(R->A);
OPCODE_EPILOG

OPCODE_PROLOG(0x68) /* PLA */
    M_POP(R->A); M_FL(R->A);
OPCODE_EPILOG

OPCODE_PROLOG(0x98) /* TYA */
    R->A=R->Y; M_FL(R->A);
OPCODE_EPILOG

OPCODE_PROLOG(0xA8) /* TAY */
    R->Y=R->A; M_FL(R->Y);
OPCODE_EPILOG

OPCODE_PROLOG(0xC8) /* INY */
    R->Y++; M_FL(R->Y);
OPCODE_EPILOG

OPCODE_PROLOG(0x88) /* DEY */
    R->Y--; M_FL(R->Y);
OPCODE_EPILOG

OPCODE_PROLOG(0x8A) /* TXA */
    R->A=R->X; M_FL(R->A);
OPCODE_EPILOG

OPCODE_PROLOG(0xAA) /* TAX */
    R->X=R->A; M_FL(R->X);
OPCODE_EPILOG

OPCODE_PROLOG(0xE8) /* INX */
    R->X++; M_FL(R->X);
OPCODE_EPILOG

OPCODE_PROLOG(0xCA) /* DEX */
    R->X--; M_FL(R->X);
OPCODE_EPILOG

OPCODE_PROLOG(0xEA) /* NOP */
OPCODE_EPILOG

OPCODE_PROLOG(0x9A) /* TXS */
    R->S=R->X;
OPCODE_EPILOG

OPCODE_PROLOG(0xBA) /* TSX */
    R->X=R->S; M_FL(R->X);
OPCODE_EPILOG


OPCODE_PROLOG(0x24) /* BIT $ss ZP */
    MR_Zp(I); M_BIT(I);
OPCODE_EPILOG

OPCODE_PROLOG(0x2C) /* BIT $ssss ABS */
    MR_Ab(I); M_BIT(I);
OPCODE_EPILOG


OPCODE_PROLOG(0x04) /* NOP $ss ZP */
    MR_Zp(I);
OPCODE_EPILOG

OPCODE_PROLOG(0x05) /* ORA $ss ZP */
    MR_Zp(I); M_ORA(I);
OPCODE_EPILOG

OPCODE_PROLOG(0x06) /* ASL $ss ZP */
    MM_Zp(M_ASL);
OPCODE_EPILOG

OPCODE_PROLOG(0x07) /* SLO $ss ZP */
    MM_Zp(M_SLO);
OPCODE_EPILOG

OPCODE_PROLOG(0x25) /* AND $ss ZP */
    MR_Zp(I); M_AND(I);
OPCODE_EPILOG

OPCODE_PROLOG(0x26) /* ROL $ss ZP */
    MM_Zp(M_ROL);
OPCODE_EPILOG

OPCODE_PROLOG(0x45) /* EOR $ss ZP */
    MR_Zp(I); M_EOR(I);
OPCODE_EPILOG

OPCODE_PROLOG(0x46) /* LSR $ss ZP */
    MM_Zp(M_LSR);
OPCODE_EPILOG

OPCODE_PROLOG(0x65) /* ADC $ss ZP */
    MR_Zp(I); M_ADC(I);
OPCODE_EPILOG

OPCODE_PROLOG(0x66) /* ROL $ss ZP */
    MM_Zp(M_ROR);
OPCODE_EPILOG

OPCODE_PROLOG(0x84) /* STY $ss ZP */
    MW_Zp(R->Y);
OPCODE_EPILOG

OPCODE_PROLOG(0x85) /* STA $ss ZP */
    MW_Zp(R->A);
OPCODE_EPILOG

OPCODE_PROLOG(0x86) /* STX $ss ZP */
    MW_Zp(R->X);
OPCODE_EPILOG

OPCODE_PROLOG(0xA4) /* LDY $ss ZP */
    MR_Zp(R->Y); M_FL(R->Y);
OPCODE_EPILOG

OPCODE_PROLOG(0xA5) /* LDA $ss ZP */
    MR_Zp(R->A); M_FL(R->A);
OPCODE_EPILOG

OPCODE_PROLOG(0xA6) /* LDX $ss ZP */
    MR_Zp(R->X); M_FL(R->X);
OPCODE_EPILOG

OPCODE_PROLOG(0xC4) /* CPY $ss ZP */
    MR_Zp(I); M_CMP(R->Y,I);
OPCODE_EPILOG

OPCODE_PROLOG(0xC5) /* CMP $ss ZP */
    MR_Zp(I); M_CMP(R->A,I);
OPCODE_EPILOG

OPCODE_PROLOG(0xC6) /* DEC $ss ZP */
    MM_Zp(M_DEC);
OPCODE_EPILOG

OPCODE_PROLOG(0xE4) /* CPX $ss ZP */
    MR_Zp(I); M_CMP(R->X,I);
OPCODE_EPILOG

OPCODE_PROLOG(0xE5) /* SBC $ss ZP */
    MR_Zp(I); M_SBC(I);
OPCODE_EPILOG

OPCODE_PROLOG(0xE6) /* INC $ss ZP */
    MM_Zp(M_INC);
OPCODE_EPILOG


OPCODE_PROLOG(0x0D) /* ORA $ssss ABS */
    MR_Ab(I); M_ORA(I);
OPCODE_EPILOG

OPCODE_PROLOG(0x0E) /* ASL $ssss ABS */
    MM_Ab(M_ASL);
OPCODE_EPILOG

OPCODE_PROLOG(0x0F) /* SLO $ssss ABS */
    MM_Ab(M_SLO);
OPCODE_EPILOG

OPCODE_PROLOG(0x2D) /* AND $ssss ABS */
    MR_Ab(I); M_AND(I);
OPCODE_EPILOG

OPCODE_PROLOG(0x2E) /* ROL $ssss ABS */
    MM_Ab(M_ROL);
OPCODE_EPILOG

OPCODE_PROLOG(0x4D) /* EOR $ssss ABS */
    MR_Ab(I); M_EOR(I);
OPCODE_EPILOG

OPCODE_PROLOG(0x4E) /* LSR $ssss ABS */
    MM_Ab(M_LSR);
OPCODE_EPILOG

OPCODE_PROLOG(0x6D) /* ADC $ssss ABS */
    MR_Ab(I); M_ADC(I);
OPCODE_EPILOG

OPCODE_PROLOG(0x6E) /* ROR $ssss ABS */
    MM_Ab(M_ROR);
OPCODE_EPILOG

OPCODE_PROLOG(0x8C) /* STY $ssss ABS */
    MW_Ab(R->Y);
OPCODE_EPILOG

OPCODE_PROLOG(0x8D) /* STA $ssss ABS */
    MW_Ab(R->A);
OPCODE_EPILOG

OPCODE_PROLOG(0x8E) /* STX $ssss ABS */
    MW_Ab(R->X);
OPCODE_EPILOG

OPCODE_PROLOG(0xAC) /* LDY $ssss ABS */
    MR_Ab(R->Y); M_FL(R->Y);
OPCODE_EPILOG

OPCODE_PROLOG(0xAD) /* LDA $ssss ABS */
    MR_Ab(R->A); M_FL(R->A);
OPCODE_EPILOG

OPCODE_PROLOG(0xAE) /* LDX $ssss ABS */
    MR_Ab(R->X); M_FL(R->X);
OPCODE_EPILOG

OPCODE_PROLOG(0xCC) /* CPY $ssss ABS */
    MR_Ab(I); M_CMP(R->Y,I);
OPCODE_EPILOG

OPCODE_PROLOG(0xCD) /* CMP $ssss ABS */
    MR_Ab(I); M_CMP(R->A,I);
OPCODE_EPILOG

OPCODE_PROLOG(0xCE) /* DEC $ssss ABS */
    MM_Ab(M_DEC);
OPCODE_EPILOG

OPCODE_PROLOG(0xEC) /* CPX $ssss ABS */
    MR_Ab(I); M_CMP(R->X,I);
OPCODE_EPILOG

OPCODE_PROLOG(0xED) /* SBC $ssss ABS */
    MR_Ab(I); M_SBC(I);
OPCODE_EPILOG

OPCODE_PROLOG(0xEE) /* INC $ssss ABS */
    MM_Ab(M_INC);
OPCODE_EPILOG


OPCODE_PROLOG(0x09) /* ORA #$ss IMM */
    MR_Im(I); M_ORA(I);
OPCODE_EPILOG

OPCODE_PROLOG(0x29) /* AND #$ss IMM */
    MR_Im(I); M_AND(I);
OPCODE_EPILOG

OPCODE_PROLOG(0x49) /* EOR #$ss IMM */
    MR_Im(I); M_EOR(I);
OPCODE_EPILOG

OPCODE_PROLOG(0x69) /* ADC #$ss IMM */
    MR_Im(I); M_ADC(I);
OPCODE_EPILOG

OPCODE_PROLOG(0x80) /* NOP #$ss IMM */
    MR_Im(I);
OPCODE_EPILOG

OPCODE_PROLOG(0xA0) /* LDY #$ss IMM */
    MR_Im(R->Y); M_FL(R->Y);
OPCODE_EPILOG

OPCODE_PROLOG(0xA2) /* LDX #$ss IMM */
    MR_Im(R->X); M_FL(R->X);
OPCODE_EPILOG

OPCODE_PROLOG(0xA9) /* LDA #$ss IMM */
    MR_Im(R->A); M_FL(R->A);
OPCODE_EPILOG

OPCODE_PROLOG(0xC0) /* CPY #$ss IMM */
    MR_Im(I); M_CMP(R->Y,I);
OPCODE_EPILOG

OPCODE_PROLOG(0xC9) /* CMP #$ss IMM */
    MR_Im(I); M_CMP(R->A,I);
OPCODE_EPILOG

OPCODE_PROLOG(0xE0) /* CPX #$ss IMM */
    MR_Im(I); M_CMP(R->X,I);
OPCODE_EPILOG

OPCODE_PROLOG(0xE9) /* SBC #$ss IMM */
    MR_Im(I); M_SBC(I);
OPCODE_EPILOG


OPCODE_PROLOG(0x15) /* ORA $ss,x ZP,x */
    MR_Zx(I); M_ORA(I);
OPCODE_EPILOG

OPCODE_PROLOG(0x16) /* ASL $ss,x ZP,x */
    MM_Zx(M_ASL);
OPCODE_EPILOG

OPCODE_PROLOG(0x35) /* AND $ss,x ZP,x */
    MR_Zx(I); M_AND(I);
OPCODE_EPILOG

OPCODE_PROLOG(0x36) /* ROL $ss,x ZP,x */
    MM_Zx(M_ROL);
OPCODE_EPILOG

OPCODE_PROLOG(0x55) /* EOR $ss,x ZP,x */
    MR_Zx(I); M_EOR(I);
OPCODE_EPILOG

OPCODE_PROLOG(0x56) /* LSR $ss,x ZP,x */
    MM_Zx(M_LSR);
OPCODE_EPILOG

OPCODE_PROLOG(0x74) /* NOP $ss,x ZP,x */
    MR_Zx(I);
OPCODE_EPILOG

OPCODE_PROLOG(0x75) /* ADC $ss,x ZP,x */
    MR_Zx(I); M_ADC(I);
OPCODE_EPILOG

OPCODE_PROLOG(0x76) /* ROR $ss,x ZP,x */
    MM_Zx(M_ROR);
OPCODE_EPILOG

OPCODE_PROLOG(0x94) /* STY $ss,x ZP,x */
    MW_Zx(R->Y);
OPCODE_EPILOG

OPCODE_PROLOG(0x95) /* STA $ss,x ZP,x */
    MW_Zx(R->A);
OPCODE_EPILOG

OPCODE_PROLOG(0x96) /* STX $ss,y ZP,y */
    MW_Zy(R->X);
OPCODE_EPILOG

OPCODE_PROLOG(0xB4) /* LDY $ss,x ZP,x */
    MR_Zx(R->Y); M_FL(R->Y);
OPCODE_EPILOG

OPCODE_PROLOG(0xB5) /* LDA $ss,x ZP,x */
    MR_Zx(R->A); M_FL(R->A);
OPCODE_EPILOG

OPCODE_PROLOG(0xB6) /* LDX $ss,y ZP,y */
    MR_Zy(R->X); M_FL(R->X);
OPCODE_EPILOG

OPCODE_PROLOG(0xD5) /* CMP $ss,x ZP,x */
    MR_Zx(I); M_CMP(R->A,I);
OPCODE_EPILOG

OPCODE_PROLOG(0xD6) /* DEC $ss,x ZP,x */
    MM_Zx(M_DEC);
OPCODE_EPILOG

OPCODE_PROLOG(0xF5) /* SBC $ss,x ZP,x */
    MR_Zx(I); M_SBC(I);
OPCODE_EPILOG

OPCODE_PROLOG(0xF6) /* INC $ss,x ZP,x */
    MM_Zx(M_INC);
OPCODE_EPILOG


OPCODE_PROLOG(0x19) /* ORA $ssss,y ABS,y */
    MR_Ay(I); M_ORA(I);
OPCODE_EPILOG

OPCODE_PROLOG(0x1D) /* ORA $ssss,x ABS,x */
    MR_Ax(I); M_ORA(I);
OPCODE_EPILOG

OPCODE_PROLOG(0x1E) /* ASL $ssss,x ABS,x */
    MM_Ax(M_ASL);
OPCODE_EPILOG

OPCODE_PROLOG(0x39) /* AND $ssss,y ABS,y */
    MR_Ay(I); M_AND(I);
OPCODE_EPILOG

OPCODE_PROLOG(0x3D) /* AND $ssss,x ABS,x */
    MR_Ax(I); M_AND(I);
OPCODE_EPILOG

OPCODE_PROLOG(0x3E) /* ROL $ssss,x ABS,x */
    MM_Ax(M_ROL);
OPCODE_EPILOG

OPCODE_PROLOG(0x59) /* EOR $ssss,y ABS,y */
    MR_Ay(I); M_EOR(I);
OPCODE_EPILOG

OPCODE_PROLOG(0x5D) /* EOR $ssss,x ABS,x */
    MR_Ax(I); M_EOR(I);
OPCODE_EPILOG

OPCODE_PROLOG(0x5E) /* LSR $ssss,x ABS,x */
    MM_Ax(M_LSR);
OPCODE_EPILOG

OPCODE_PROLOG(0x79) /* ADC $ssss,y ABS,y */
    MR_Ay(I); M_ADC(I);
OPCODE_EPILOG

OPCODE_PROLOG(0x7D) /* ADC $ssss,x ABS,x */
    MR_Ax(I); M_ADC(I);
OPCODE_EPILOG

OPCODE_PROLOG(0x7E) /* ROR $ssss,x ABS,x */
    MM_Ax(M_ROR);
OPCODE_EPILOG

OPCODE_PROLOG(0x99) /* STA $ssss,y ABS,y */
    MW_Ay(R->A);
OPCODE_EPILOG

OPCODE_PROLOG(0x9D) /* STA $ssss,x ABS,x */
    MW_Ax(R->A);
OPCODE_EPILOG

OPCODE_PROLOG(0xB9) /* LDA $ssss,y ABS,y */
    MR_Ay(R->A); M_FL(R->A);
OPCODE_EPILOG

OPCODE_PROLOG(0xBC) /* LDY $ssss,x ABS,x */
    MR_Ax(R->Y); M_FL(R->Y);
OPCODE_EPILOG

OPCODE_PROLOG(0xBD) /* LDA $ssss,x ABS,x */
    MR_Ax(R->A); M_FL(R->A);
OPCODE_EPILOG

OPCODE_PROLOG(0xBE) /* LDX $ssss,y ABS,y */
    MR_Ay(R->X); M_FL(R->X);
OPCODE_EPILOG

OPCODE_PROLOG(0xD9) /* CMP $ssss,y ABS,y */
    MR_Ay(I); M_CMP(R->A,I);
OPCODE_EPILOG

OPCODE_PROLOG(0xDD) /* CMP $ssss,x ABS,x */
    MR_Ax(I); M_CMP(R->A,I);
OPCODE_EPILOG

OPCODE_PROLOG(0xDE) /* DEC $ssss,x ABS,x */
    MM_Ax(M_DEC);
OPCODE_EPILOG

OPCODE_PROLOG(0xF9) /* SBC $ssss,y ABS,y */
    MR_Ay(I); M_SBC(I);
OPCODE_EPILOG

OPCODE_PROLOG(0xFD) /* SBC $ssss,x ABS,x */
    MR_Ax(I); M_SBC(I);
OPCODE_EPILOG

OPCODE_PROLOG(0xFE) /* INC $ssss,x ABS,x */
    MM_Ax(M_INC);
OPCODE_EPILOG


OPCODE_PROLOG(0x01) /* ORA ($ss,x) INDEXINDIR */
    MR_Ix(I); M_ORA(I);
OPCODE_EPILOG

OPCODE_PROLOG(0x11) /* ORA ($ss),y INDIRINDEX */
    MR_Iy(I); M_ORA(I);
OPCODE_EPILOG

OPCODE_PROLOG(0x21) /* AND ($ss,x) INDEXINDIR */
    MR_Ix(I); M_AND(I);
OPCODE_EPILOG

OPCODE_PROLOG(0x31) /* AND ($ss),y INDIRINDEX */
    MR_Iy(I); M_AND(I);
OPCODE_EPILOG

OPCODE_PROLOG(0x41) /* EOR ($ss,x) INDEXINDIR */
    MR_Ix(I); M_EOR(I);
OPCODE_EPILOG

OPCODE_PROLOG(0x51) /* EOR ($ss),y INDIRINDEX */
    MR_Iy(I); M_EOR(I);
OPCODE_EPILOG

OPCODE_PROLOG(0x61) /* ADC ($ss,x) INDEXINDIR */
    MR_Ix(I); M_ADC(I);
OPCODE_EPILOG

OPCODE_PROLOG(0x71) /* ADC ($ss),y INDIRINDEX */
    MR_Iy(I); M_ADC(I);
OPCODE_EPILOG

OPCODE_PROLOG(0x81) /* STA ($ss,x) INDEXINDIR */
    MW_Ix(R->A);
OPCODE_EPILOG

OPCODE_PROLOG(0x91) /* STA ($ss),y INDIRINDEX */
    MW_Iy(R->A);
OPCODE_EPILOG

OPCODE_PROLOG(0xA1) /* LDA ($ss,x) INDEXINDIR */
    MR_Ix(R->A); M_FL(R->A);
OPCODE_EPILOG

OPCODE_PROLOG(0xB1) /* LDA ($ss),y INDIRINDEX */
    MR_Iy(R->A); M_FL(R->A);
OPCODE_EPILOG

OPCODE_PROLOG(0xC1) /* CMP ($ss,x) INDEXINDIR */
    MR_Ix(I); M_CMP(R->A,I);
OPCODE_EPILOG

OPCODE_PROLOG(0xD1) /* CMP ($ss),y INDIRINDEX */
    MR_Iy(I); M_CMP(R->A,I);
OPCODE_EPILOG

OPCODE_PROLOG(0xE1) /* SBC ($ss,x) INDEXINDIR */
    MR_Ix(I); M_SBC(I);
OPCODE_EPILOG

OPCODE_PROLOG(0xF1) /* SBC ($ss),y INDIRINDEX */
    MR_Iy(I); M_SBC(I);
OPCODE_EPILOG


OPCODE_PROLOG(0x0A) /* ASL a ACC */
    M_ASL(R->A);
OPCODE_EPILOG

OPCODE_PROLOG(0x2A) /* ROL a ACC */
    M_ROL(R->A);
OPCODE_EPILOG

OPCODE_PROLOG(0x4A) /* LSR a ACC */
    M_LSR(R->A);
OPCODE_EPILOG

OPCODE_PROLOG(0x6A) /* ROR a ACC */
    M_ROR(R->A);
OPCODE_EPILOG


OPCODE_PROLOG(0xEF) /* INS abcd */
    M_LDWORD(K);
    I=Rd6502(K.W);
    Wr6502(K.W,++I);
    M_SBC(I);
OPCODE_EPILOG

OPCODE_PROLOG(0xFF) /* INS abcd,X */
    M_LDWORD(K);
    K.W+=R->X;
    I=Rd6502(K.W);
    Wr6502(K.W,++I);
    M_SBC(I);
OPCODE_EPILOG

OPCODE_PROLOG(0xF2) /* JAM */
    R->Jammed=1;
OPCODE_EPILOG

OPCODE_PROLOG_DEFAULT
    if(R->TrapBadOps)
        printf
        (
            "[M6502 %lX] Unrecognized instruction: $%02X at PC=$%04X\n",
            ((UINT32)R->User),Op6502(R->PC.W-1),(word)(R->PC.W-1)
        );
#ifdef DEBUG
        printf("\nOpcode fallback trace:\n\n");
        for (opcode_count=0;opcode_count<10;opcode_count++) {
            printf("$%02X ",opcode_trace[opcode_count]);
            opcode_trace[opcode_count]=0;
        }
        printf("\n\n");
        opcode_count=0;
#endif
OPCODE_EPILOG
