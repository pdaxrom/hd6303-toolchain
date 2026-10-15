; Native JSR cvcall, X -> CVMABI.INC descriptor. A=status, result at +6.
    include CVMABI.INC
    public cvcall
cvcall
    .ifdef PYLDIN601
    ldaa #CV_CALL
    int $ef
    .else
    ; Original Lilith E8 entry has no arbitrary-function call service.
    ldaa #1
    .endif
    rts
    end
