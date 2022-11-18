    NOP
    HALT
    MOV     r7,     r3
    MOV     R0,     0x828
loop_1:loop_2: loop_3:
    mov     r2,     flags
    MOV     FLAGS,  r4
    LDR     r1,     [ r5 ]     
    LDR     r1,     [ r5,       r6 ]
    ldr r1,[r5]
    ldr r1,[r5,r6]
    STR     r1,     [ r5 ]     
    STR     r1,     [ r5,       r6 ]
    str r1,[r5]
    str r1,[r5,r6]
    ADD     r1,     r2,     r3
    add r5,r6,r7
    SUB     r1,     r2,     r3
    MUL     r1,     r2,     r3
    MULU    r1,     r2,     r3
    DIV     r1,     r2,     r3
    MOD     r1,     r2,     r3
    AND     r1,     r2,     r3
    OR      r1,     r2,     r3
    EOR     r1,     r2,     r3
    NOT     r1,     r2
    not r6,r7  
    LSL     r1,     r2,     r3
    LSR     r1,     r2,     r3
a:  ASR     r1,     r2,     r3
    ROL     r1,     r2,     r3
    ROR     r1,     r2,     r3
    CMP     r1,     r2
    B       0b110
    b       a
    BEQ     0x003
    b       0x800
    b       0b100000000000
    b 1
end:BNE eNd