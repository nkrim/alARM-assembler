; parser errors
; -------------
; r0: mov r0 r1
; bEq: mov r0 r1 
; flAgs: mov r0 r1
; 0loop: mov r0 r1
; beep:beep: mov r0 r1
; --- mov r0 r1
; 0x800
; foo r0 r1
; mov r0
; mov r0 r1 r2
; add r0 r1 
; add r0 r1 r2 r3
; add r0 r1 0x800
; mov r3 r8
; mov r3 0x1000
; mov 0x800 r0
; not rx r3
; not r0r1
; b1
; r1

; encoder errors
; --------------
; mov r3 loop
; mov r3 r-1
; mov r01 r3
; b   fakeaddr
; beq 2048
; mov r0 0x0800
; bne 0b1000000000000
; 