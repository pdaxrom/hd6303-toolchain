; VM ABI entries are bytecode addresses, even for native implementations.
    include CVMDEF.INC
    include CVMABI.INC
    public nativefn,backcall,badcall
    extern cvcall,mixed
nativefn
    db 86
    ldx #mixed
    inc 1,x
    bne native_sum
    inc 0,x
native_sum
    tsx
    ldaa 4,x
    ldab 5,x
    addb 3,x
    adca 2,x
    jmp RTSC
backcall
    db 86
    ldaa #2
    bra callback
badcall
    db 86
    ldaa #1
callback
    ; Ten descriptor bytes are private to this invocation, on the stack.
    des
    des
    des
    des
    des
    des
    des
    des
    des
    des
    tsx
    staa CV_BYTES+1,x
    clr CV_BYTES,x
    ldaa 14,x
    ldab 15,x
    staa CV_ENTRY,x
    stab CV_ENTRY+1,x
    stx CV_ARGS,x
    ldaa CV_ARGS,x
    ldab CV_ARGS+1,x
    addb #12
    adca #0
    staa CV_ARGS,x
    stab CV_ARGS+1,x
    jsr cvcall
    tsta
    bne call_failed
    ldaa CV_VALUE,x
    ldab CV_VALUE+1,x
    bra call_return
call_failed
    tab
    clra
call_return
    ins
    ins
    ins
    ins
    ins
    ins
    ins
    ins
    ins
    ins
    jmp RTSC
    end
