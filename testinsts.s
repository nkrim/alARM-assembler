    NOP
    HALT
    MOV     r4      r2
    MOV     r7,     r3
    MOV     R0,     0x828
    MOV     r1      -34
loop_1:loop_2: loop_3:
    mov     r2,     flags
    mov     r3      flags
    MOV     FLAGS,  r4
    MOV     flAgs   r5
    LDR     r1,     [ r5 ]
    LDR     r1,     r5 
    LDR     r1      r5        
    LDR     r1,     [ r5,       r6 ]
    LDR     r1,     r5,       r6 
    LDR     r1      r5        r6 
    STR     r1,     [ r5 ]
    STR     r1,     r5 
    STR     r1      r5        
    STR     r1,     [ r5,       r6 ]
    STR     r1,     r5,       r6 
    STR     r1      r5        r6 
    ADD     r1,     r2,     r3
    ADD     r2,     r3      r4
    ADD     r3      r4,     r5
    ADD     r4      r5      r6
    SUB     r1,     r2,     r3
    SUB     r2,     r3      r4
    SUB     r3      r4,     r5
    SUB     r4      r5      r6
    MUL     r1,     r2,     r3
    MUL     r2,     r3      r4
    MUL     r3      r4,     r5
    MUL     r7      r7      r7
    MULU    r1,     r2,     r3
    MULU    r2,     r3      r4
    MULU    r3      r4,     r5
    MULU    r7      r7      r7
    DIV     r1,     r2,     r3
    DIV     r2,     r3      r4
    DIV     r3      r4,     r5
    DIV     r7      r7      r7
    MOD     r1,     r2,     r3
    MOD     r2,     r3      r4
    MOD     r3      r4,     r5
    MOD     r7      r7      r7
    AND     r1,     r2,     r3
    AND     r2,     r3      r4
    AND     r3      r4,     r5
    AND     r4      r5      r6
    OR      r1,     r2,     r3
    OR      r2,     r3      r4
    OR      r3      r4,     r5
    OR      r4      r5      r6
    EOR     r1,     r2,     r3
    EOR     r2,     r3      r4
    EOR     r3      r4,     r5
    EOR     r4      r5      r6
    NOT     r1,     r2
    NOT     r2,     r3  
    NOT     r3      r4
    NOT     r7      r7  
    LSL     r1,     r2,     r3
    LSL     r2,     r3      r4
    LSL     r3      r4,     r5
    LSL     r4      r5      r6
    LSR     r1,     r2,     r3
    LSR     r2,     r3      r4
    LSR     r3      r4,     r5
    LSR     r4      r5      r6
a:  ASR     r1,     r2,     r3
    ASR     r2,     r3      r4
    ASR     r3      r4,     r5
    ASR     r4      r5      r6
    ROL     r1,     r2,     r3
    ROL     r2,     r3      r4
    ROL     r3      r4,     r5
    ROL     r4      r5      r6
    ROR     r1,     r2,     r3
    ROR     r2,     r3      r4
    ROR     r3      r4,     r5
    ROR     r4      r5      r6
    CMP     r1,     r2
    CMP     r2,     r3  
    CMP     r3      r4
    CMP     r7      r7 
    B       0b110
    b       a
    BEQ     0x003
end:BNE eNd