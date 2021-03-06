#include "assyntax.h"


SEG_TEXT


ALIGNTEXT16
GLOBL GLNAME(gl_mmx_blend_transparency)

GLNAME( gl_mmx_blend_transparency ):
    PUSH_L    ( EBP )
    MOV_L     ( ESP, EBP )
    SUB_L     ( CONST(52), ESP )
    PUSH_L    ( EBX )
    MOV_L     ( CONST(16711680), REGOFF(-8, EBP) )
    MOV_L     ( CONST(16711680), REGOFF(-4, EBP) )
    MOV_L     ( CONST(0), REGOFF(-16, EBP) )
    MOV_L     ( CONST(-1), REGOFF(-12, EBP) )
    MOV_L     ( CONST(-1), REGOFF(-24, EBP) )
    MOV_L     ( CONST(0), REGOFF(-20, EBP) )
    MOV_L     ( REGOFF(24, EBP), EAX )
    ADD_L     ( CONST(4), EAX )
    MOV_L     ( EAX, EDX )
    AND_L     ( REGOFF(20, EBP), EDX )
    MOV_L     ( EDX, EAX )
    AND_L     ( CONST(4), EAX )
    CMP_L     ( CONST(8), EAX )
    JNE       ( LLBL(GMBT_2) )
    MOV_L     ( REGOFF(20, EBP), EAX )
    ADD_L     ( CONST(3), EAX )
    XOR_L     ( EDX, EDX )
    MOV_B     ( REGIND(EAX), DL )
    MOV_L     ( EDX, REGOFF(-32, EBP) )
    MOV_L     ( CONST(255), EAX )
    MOV_L     ( EAX, EBX )
    SUB_L     ( REGOFF(-32, EBP), EBX )
    MOV_L     ( EBX, REGOFF(-36, EBP) )
    MOV_L     ( REGOFF(20, EBP), EAX )
    XOR_L     ( EDX, EDX )
    MOV_B     ( REGIND(EAX), DL )
    MOV_L     ( EDX, EAX )
    IMUL_L    ( REGOFF(-32, EBP), EAX )
    MOV_L     ( REGOFF(24, EBP), EDX )
    XOR_L     ( ECX, ECX )
    MOV_B     ( REGIND(EDX), CL )
    MOV_L     ( ECX, EDX )
    IMUL_L    ( REGOFF(-36, EBP), EDX )
    ADD_L     ( EDX, EAX )
    MOV_L     ( EAX, EBX )
    SAR_L     ( CONST(8), EBX )
    MOV_L     ( EBX, REGOFF(-40, EBP) )
    MOV_L     ( REGOFF(20, EBP), EAX )
    INC_L     ( EAX )
    XOR_L     ( EDX, EDX )
    MOV_B     ( REGIND(EAX), DL )
    MOV_L     ( EDX, EAX )
    IMUL_L    ( REGOFF(-32, EBP), EAX )
    MOV_L     ( REGOFF(24, EBP), EDX )
    INC_L     ( EDX )
    XOR_L     ( ECX, ECX )
    MOV_B     ( REGIND(EDX), CL )
    MOV_L     ( ECX, EDX )
    IMUL_L    ( REGOFF(-36, EBP), EDX )
    ADD_L     ( EDX, EAX )
    MOV_L     ( EAX, EBX )
    SAR_L     ( CONST(8), EBX )
    MOV_L     ( EBX, REGOFF(-44, EBP) )
    MOV_L     ( REGOFF(20, EBP), EAX )
    ADD_L     ( CONST(2), EAX )
    XOR_L     ( EDX, EDX )
    MOV_B     ( REGIND(EAX), DL )
    MOV_L     ( EDX, EAX )
    IMUL_L    ( REGOFF(-32, EBP), EAX )
    MOV_L     ( REGOFF(24, EBP), EDX )
    ADD_L     ( CONST(2), EDX )
    XOR_L     ( ECX, ECX )
    MOV_B     ( REGIND(EDX), CL )
    MOV_L     ( ECX, EDX )
    IMUL_L    ( REGOFF(-36, EBP), EDX )
    ADD_L     ( EDX, EAX )
    MOV_L     ( EAX, EBX )
    SAR_L     ( CONST(8), EBX )
    MOV_L     ( EBX, REGOFF(-48, EBP) )
    MOV_L     ( REGOFF(20, EBP), EAX )
    ADD_L     ( CONST(3), EAX )
    XOR_L     ( EDX, EDX )
    MOV_B     ( REGIND(EAX), DL )
    MOV_L     ( EDX, EAX )
    IMUL_L    ( REGOFF(-32, EBP), EAX )
    MOV_L     ( REGOFF(24, EBP), EDX )
    ADD_L     ( CONST(3), EDX )
    XOR_L     ( ECX, ECX )
    MOV_B     ( REGIND(EDX), CL )
    MOV_L     ( ECX, EDX )
    IMUL_L    ( REGOFF(-36, EBP), EDX )
    ADD_L     ( EDX, EAX )
    MOV_L     ( EAX, EBX )
    SAR_L     ( CONST(8), EBX )
    MOV_L     ( EBX, REGOFF(-52, EBP) )
    MOV_L     ( REGOFF(20, EBP), EAX )
    MOV_B     ( REGOFF(-40, EBP), DL )
    MOV_B     ( DL, REGIND(EAX) )
    MOV_L     ( REGOFF(20, EBP), EAX )
    INC_L     ( EAX )
    MOV_B     ( REGOFF(-44, EBP), DL )
    MOV_B     ( DL, REGIND(EAX) )
    MOV_L     ( REGOFF(20, EBP), EAX )
    ADD_L     ( CONST(2), EAX )
    MOV_B     ( REGOFF(-48, EBP), DL )
    MOV_B     ( DL, REGIND(EAX) )
    MOV_L     ( REGOFF(20, EBP), EAX )
    ADD_L     ( CONST(3), EAX )
    MOV_B     ( REGOFF(-52, EBP), DL )
    MOV_B     ( DL, REGIND(EAX) )
    INC_L     ( REGOFF(16, EBP) )
    ADD_L     ( CONST(4), REGOFF(20, EBP) )
    ADD_L     ( CONST(4), REGOFF(24, EBP) )
    DEC_L     ( REGOFF(12, EBP) )
LLBL(GMBT_2):

    CMP_L     ( CONST(0), REGOFF(12, EBP) )
    JE        ( LLBL(GMBT_3) )
    MOV_L     ( CONST(0), REGOFF(-28, EBP) )
ALIGNTEXT4
LLBL(GMBT_4):

    MOV_L     ( REGOFF(12, EBP), EDX )
    MOV_L     ( EDX, EAX )
    SHR_L     ( CONST(1), EAX )
    CMP_L     ( EAX, REGOFF(-28, EBP) )
    JB        ( LLBL(GMBT_7) )
    JMP       ( LLBL(GMBT_5) )
ALIGNTEXT16
LLBL(GMBT_7):

    MOV_L     ( REGOFF(-28, EBP), EAX )
    LEA_L     ( REGDIS(0,EAX,2), EDX )
    MOV_L     ( REGOFF(16, EBP), EAX )
    CMP_B     ( CONST(0), REGBI(EAX,EDX) )
    JE        ( LLBL(GMBT_6) )
    MOV_L     ( REGOFF(-28, EBP), EAX )
    MOV_L     ( EAX, EDX )
    LEA_L     ( REGDIS(0,EDX,8), ECX )
    MOV_L     ( ECX, EAX )
    ADD_L     ( REGOFF(20, EBP), EAX )
    MOV_L     ( REGOFF(-28, EBP), EDX )
    MOV_L     ( EDX, ECX )
    LEA_L     ( REGDIS(0,ECX,8), EDX )
    MOV_L     ( EDX, ECX )
    ADD_L     ( REGOFF(24, EBP), ECX )

    MOVQ      ( REGIND(EAX), MM4 )
    PXOR      ( MM5, MM5 )
    MOVQ      ( MM4, MM1 )
    MOVQ      ( REGIND(ECX), MM7 )
    PUNPCKLBW ( MM5, MM1 )
    MOVQ      ( MM7, MM6 )
    MOVQ      ( MM1, MM0 )
    PUNPCKLBW ( MM5, MM6 )
    MOVQ      ( MM1, MM2 )
    PSRLQ     ( CONST(48), MM0 )
    PUNPCKHBW ( MM5, MM4 )
    PACKSSDW  ( MM0, MM0 )
    MOVQ      ( MM0, MM3 )
    PUNPCKHBW ( MM5, MM7 )
    PSLLQ     ( CONST(16), MM3 )
    POR       ( REGOFF(-8, EBP), MM0 )
    PUNPCKLWD ( MM6, MM1 )
    PSUBW     ( MM3, MM0 )
    PUNPCKHWD ( MM6, MM2 )
    MOVQ      ( MM4, MM3 )
    PSRLQ     ( CONST(48), MM3 )
    PACKSSDW  ( MM3, MM3 )
    MOVQ      ( MM3, MM6 )
    POR       ( REGOFF(-8, EBP), MM3 )
    PSLLQ     ( CONST(16), MM6 )
    PSUBW     ( MM6, MM3 )
    MOVQ      ( MM4, MM5 )
    PUNPCKLWD ( MM7, MM4 )
    PUNPCKHWD ( MM7, MM5 )
    PMADDWD   ( MM0, MM1 )
    PMADDWD   ( MM3, MM4 )
    PMADDWD   ( MM0, MM2 )
    PMADDWD   ( MM3, MM5 )
    PSRLD     ( CONST(8), MM1 )
    PSRLD     ( CONST(8), MM2 )
    PSRLD     ( CONST(8), MM4 )
    PACKSSDW  ( MM2, MM1 )
    PSRLD     ( CONST(8), MM5 )
    PACKUSWB  ( MM1, MM1 )
    PACKSSDW  ( MM5, MM4 )
    PAND      ( REGOFF(-24, EBP), MM1 )
    PACKUSWB  ( MM4, MM4 )
    PAND      ( REGOFF(-16, EBP), MM4 )
    POR       ( MM1, MM4 )
    MOVQ      ( MM4, REGIND(EAX) )


LLBL(GMBT_8):

LLBL(GMBT_6):

    INC_L     ( REGOFF(-28, EBP) )
    JMP       ( LLBL(GMBT_4) )
ALIGNTEXT16
LLBL(GMBT_5):


    EMMS

LLBL(GMBT_3):

    MOV_L     ( REGOFF(12, EBP), EAX )
    AND_L     ( CONST(1), EAX )
    TEST_L    ( EAX, EAX )
    JE        ( LLBL(GMBT_9) )
    MOV_L     ( REGOFF(12, EBP), EAX )
    LEA_L     ( REGDIS(0,EAX,4), EDX )
    MOV_L     ( EDX, EAX )
    ADD_L     ( REGOFF(20, EBP), EAX )
    LEA_L     ( REGOFF(-1, EAX), EDX )
    XOR_L     ( EAX, EAX )
    MOV_B     ( REGIND(EDX), AL )
    MOV_L     ( EAX, REGOFF(-52, EBP) )
    MOV_L     ( CONST(255), EAX )
    MOV_L     ( EAX, EBX )
    SUB_L     ( REGOFF(-52, EBP), EBX )
    MOV_L     ( EBX, REGOFF(-48, EBP) )
    MOV_L     ( REGOFF(12, EBP), EAX )
    LEA_L     ( REGDIS(0,EAX,4), EDX )
    MOV_L     ( EDX, EAX )
    ADD_L     ( REGOFF(20, EBP), EAX )
    LEA_L     ( REGOFF(-4, EAX), EDX )
    XOR_L     ( ECX, ECX )
    MOV_B     ( REGIND(EDX), CL )
    MOV_L     ( ECX, EAX )
    IMUL_L    ( REGOFF(-52, EBP), EAX )
    MOV_L     ( REGOFF(12, EBP), EDX )
    LEA_L     ( REGDIS(0,EDX,4), ECX )
    MOV_L     ( ECX, EDX )
    ADD_L     ( REGOFF(24, EBP), EDX )
    LEA_L     ( REGOFF(-4, EDX), ECX )
    XOR_L     ( EDX, EDX )
    MOV_B     ( REGIND(ECX), DL )
    MOV_L     ( EDX, ECX )
    IMUL_L    ( REGOFF(-48, EBP), ECX )
    ADD_L     ( ECX, EAX )
    MOV_L     ( EAX, EBX )
    SAR_L     ( CONST(8), EBX )
    MOV_L     ( EBX, REGOFF(-44, EBP) )
    MOV_L     ( REGOFF(12, EBP), EAX )
    LEA_L     ( REGDIS(0,EAX,4), EDX )
    MOV_L     ( EDX, EAX )
    ADD_L     ( REGOFF(20, EBP), EAX )
    LEA_L     ( REGOFF(-3, EAX), EDX )
    XOR_L     ( ECX, ECX )
    MOV_B     ( REGIND(EDX), CL )
    MOV_L     ( ECX, EAX )
    IMUL_L    ( REGOFF(-52, EBP), EAX )
    MOV_L     ( REGOFF(12, EBP), EDX )
    LEA_L     ( REGDIS(0,EDX,4), ECX )
    MOV_L     ( ECX, EDX )
    ADD_L     ( REGOFF(24, EBP), EDX )
    LEA_L     ( REGOFF(-3, EDX), ECX )
    XOR_L     ( EDX, EDX )
    MOV_B     ( REGIND(ECX), DL )
    MOV_L     ( EDX, ECX )
    IMUL_L    ( REGOFF(-48, EBP), ECX )
    ADD_L     ( ECX, EAX )
    MOV_L     ( EAX, EBX )
    SAR_L     ( CONST(8), EBX )
    MOV_L     ( EBX, REGOFF(-40, EBP) )
    MOV_L     ( REGOFF(12, EBP), EAX )
    LEA_L     ( REGDIS(0,EAX,4), EDX )
    MOV_L     ( EDX, EAX )
    ADD_L     ( REGOFF(20, EBP), EAX )
    LEA_L     ( REGOFF(-2, EAX), EDX )
    XOR_L     ( ECX, ECX )
    MOV_B     ( REGIND(EDX), CL )
    MOV_L     ( ECX, EAX )
    IMUL_L    ( REGOFF(-52, EBP), EAX )
    MOV_L     ( REGOFF(12, EBP), EDX )
    LEA_L     ( REGDIS(0,EDX,4), ECX )
    MOV_L     ( ECX, EDX )
    ADD_L     ( REGOFF(24, EBP), EDX )
    LEA_L     ( REGOFF(-2, EDX), ECX )
    XOR_L     ( EDX, EDX )
    MOV_B     ( REGIND(ECX), DL )
    MOV_L     ( EDX, ECX )
    IMUL_L    ( REGOFF(-48, EBP), ECX )
    ADD_L     ( ECX, EAX )
    MOV_L     ( EAX, EBX )
    SAR_L     ( CONST(8), EBX )
    MOV_L     ( EBX, REGOFF(-36, EBP) )
    MOV_L     ( REGOFF(12, EBP), EAX )
    LEA_L     ( REGDIS(0,EAX,4), EDX )
    MOV_L     ( EDX, EAX )
    ADD_L     ( REGOFF(20, EBP), EAX )
    LEA_L     ( REGOFF(-1, EAX), EDX )
    XOR_L     ( ECX, ECX )
    MOV_B     ( REGIND(EDX), CL )
    MOV_L     ( ECX, EAX )
    IMUL_L    ( REGOFF(-52, EBP), EAX )
    MOV_L     ( REGOFF(12, EBP), EDX )
    LEA_L     ( REGDIS(0,EDX,4), ECX )
    MOV_L     ( ECX, EDX )
    ADD_L     ( REGOFF(24, EBP), EDX )
    LEA_L     ( REGOFF(-1, EDX), ECX )
    XOR_L     ( EDX, EDX )
    MOV_B     ( REGIND(ECX), DL )
    MOV_L     ( EDX, ECX )
    IMUL_L    ( REGOFF(-48, EBP), ECX )
    ADD_L     ( ECX, EAX )
    MOV_L     ( EAX, EBX )
    SAR_L     ( CONST(8), EBX )
    MOV_L     ( EBX, REGOFF(-32, EBP) )
    MOV_L     ( REGOFF(12, EBP), EAX )
    LEA_L     ( REGDIS(0,EAX,4), EDX )
    MOV_L     ( EDX, EAX )
    ADD_L     ( REGOFF(20, EBP), EAX )
    LEA_L     ( REGOFF(-4, EAX), EDX )
    MOV_B     ( REGOFF(-44, EBP), AL )
    MOV_B     ( AL, REGIND(EDX) )
    MOV_L     ( REGOFF(12, EBP), EAX )
    LEA_L     ( REGDIS(0,EAX,4), EDX )
    MOV_L     ( EDX, EAX )
    ADD_L     ( REGOFF(20, EBP), EAX )
    LEA_L     ( REGOFF(-3, EAX), EDX )
    MOV_B     ( REGOFF(-40, EBP), AL )
    MOV_B     ( AL, REGIND(EDX) )
    MOV_L     ( REGOFF(12, EBP), EAX )
    LEA_L     ( REGDIS(0,EAX,4), EDX )
    MOV_L     ( EDX, EAX )
    ADD_L     ( REGOFF(20, EBP), EAX )
    LEA_L     ( REGOFF(-2, EAX), EDX )
    MOV_B     ( REGOFF(-36, EBP), AL )
    MOV_B     ( AL, REGIND(EDX) )
    MOV_L     ( REGOFF(12, EBP), EAX )
    LEA_L     ( REGDIS(0,EAX,4), EDX )
    MOV_L     ( EDX, EAX )
    ADD_L     ( REGOFF(20, EBP), EAX )
    LEA_L     ( REGOFF(-1, EAX), EDX )
    MOV_B     ( REGOFF(-32, EBP), AL )
    MOV_B     ( AL, REGIND(EDX) )
LLBL(GMBT_9):

LLBL(GMBT_1):

    MOV_L     ( REGOFF(-56, EBP), EBX )
    MOV_L     ( EBP, ESP )
    POP_L     ( EBP )
    RET
