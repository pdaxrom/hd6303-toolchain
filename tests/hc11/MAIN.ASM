    extern buffer
    extern helper
    public start
    main
start
    ldy #buffer
    sty buffer
    cpd #buffer
    jsr helper
    int $38
    end start
