; Native caller enters an arbitrary exported bytecode function via CV_CALL.
    include CVMABI.INC
    extern main,cadd,retneg,noargs,explode,cvcall,mixed
start
    ; A native root initializes C data before calling its functions.
    ldx #mixed
    clr 0,x
    clr 1,x
    ldx #main_desc
    jsr cvcall
    tsta
    bne failed
    ldx main_desc+CV_VALUE
    cpx #0
    bne failed
    ldx #add_desc
    jsr cvcall
    tsta
    bne failed
    ldx add_desc+CV_VALUE
    cpx #$ff9c
    bne failed
    ldx #negative
    jsr cvcall
    tsta
    bne failed
    ldx negative+CV_VALUE
    cpx #$fb2e
    bne failed
    ldx #zeroargs
    jsr cvcall
    tsta
    bne failed
    ldx zeroargs+CV_VALUE
    cpx #$1234
    bne failed
    ldx #divide
    jsr cvcall
    cmpa #$ff
    bne failed
    ldx divide+CV_STATUS
    cpx #$ff
    bne failed
    ldx #zeroargs
    jsr cvcall
    tsta
    bne failed
    ldx #ok
    int $23
    clra
    int $38
failed
    ldx #bad
    int $23
    ldaa #1
    int $38
main_desc dw main,0,0,0,0
add_desc dw cadd,4,arguments,0,0
negative dw retneg,0,0,0,0
zeroargs dw noargs,0,0,0,0
divide dw explode,2,zero,0,0
arguments dw -123,23
zero dw 0
ok db 'C ABI PASS',10,0
bad db 'C ABI FAIL',10,0
    end start
