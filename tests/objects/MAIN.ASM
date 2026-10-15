    public start
    extern print,buf
start
    ldx #buf
    jsr print
    int $38
    end start
