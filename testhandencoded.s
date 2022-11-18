; Written by Dominic
; Assembly:       Encoding:
MOV  R0, 0x4    ; 8004
MOV  R1, 0x50   ; 9050
MOV  R2, 0x600  ; A600
ADD  R2, R3, R1 ; 20D1
SUB  R2, R0, R1 ; 2211
MUL  R5, R0, R7 ; 242F
MULU R2, R5, R1 ; 2751
DIV  R2, R0, R1 ; 2811
MOD  R2, R0, R1 ; 2A11
AND  R2, R0, R1 ; 2C11
OR   R2, R0, R1 ; 2E11
EOR  R2, R0, R1 ; 3011
NOT  R2, R0     ; 3210
LSL  R2, R0, R1 ; 3411
LSR  R2, R0, R1 ; 3611
ASR  R2, R0, R1 ; 3811
ROL  R2, R0, R1 ; 3A11
ROR  R2, R0, R1 ; 3C11
CMP  R2, R4     ; 3F10
B    0xFFF      ; 4FFF
BEQ  0x40       ; 6040