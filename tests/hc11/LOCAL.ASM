    main
start
    ldy #buffer
    sty buffer
    cpd #buffer
    jsr helper
    int $38
helper
    aby
    iny
    rts
    section
buffer ds 8
    ends
    end start
