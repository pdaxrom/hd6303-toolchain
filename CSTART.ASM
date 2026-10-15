; Native entry and bytecode main bridge for linked Small-C programs.
    include CVMDEF.INC
    public __cbeg
    extern __cend,main
start
    jmp cinit
    db 'CVM',1
    dw cboot
cinit
    ldx #__cbeg
czero
    clr 0,x
    inx
    cpx #__cend
    bne czero
    .ifdef PYLDIN601
    ldaa #$ef
    .else
    ldaa #$e8
    .endif
    int $2e
    cpx #0
    beq missing
    .ifdef PYLDIN601
    clra
    int $ef
    cmpa #1
    bne missing
    cpx #$5a56
    bne missing
    ldx #cboot
    ldaa #1
    int $ef
    rts
    .else
    ldx #cboot
    int $e8
    rts
    .endif
missing
    ldx #errmsg
    int $23
    ldaa #1
    int $38
errmsg db 'ZCodeVM unavailable.',10,0
cboot
    db 28
    dw main
    db 20,0
    dw 2
    db 28
    dw exit
    db 86
    int $38
    section
__cbeg ds 1
    ends
    end start
