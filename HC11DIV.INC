; Signed Small-C divide, after divisor normalization and zero check.
; Keep the existing quotient/remainder/sign ABI. Y addresses this invocation's
; own stack; interrupts and nested BIOS calls preserve Y in their HC11 frame.
    PSHB
    PSHA
    TSY
    LDD 4,Y
    BPL HC11_DPOS
    COM DFLAG
    JSR NEGATE
HC11_DPOS
    LDX 0,Y
    IDIV
    STX 4,Y
    INS
    INS
    RTS
