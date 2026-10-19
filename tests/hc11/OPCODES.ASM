; Golden bytes transcribed from Motorola M68HC11ERG instruction tables.
; The test runner reads only the expected-byte comments and runs UniAS.
    org $1000
    ldaa #__HC11__       ; 86 01
    aby                  ; 18 3a
    dey                  ; 18 09
    iny                  ; 18 08
    pshy                 ; 18 3c
    puly                 ; 18 38
    tsy                  ; 18 30
    tys                  ; 18 35
    xgdy                 ; 18 8f
    xgdx                 ; 8f
    idiv                 ; 02
    fdiv                 ; 03
    stop                 ; cf
    ldy #$1234           ; 18 ce 12 34
    ldy $80              ; 18 de 80
    ldy $8123            ; 18 fe 81 23
    ldy 0,x              ; 1a ee 00
    ldy 255,y            ; 18 ee ff
    sty $80              ; 18 df 80
    sty $8123            ; 18 ff 81 23
    sty 255,x            ; 1a ef ff
    sty 0,y              ; 18 ef 00
    cpy #$1234           ; 18 8c 12 34
    cpy $80              ; 18 9c 80
    cpy $8123            ; 18 bc 81 23
    cpy 255,x            ; 1a ac ff
    cpy 0,y              ; 18 ac 00
    cpd #$1234           ; 1a 83 12 34
    cpd $80              ; 1a 93 80
    cpd $8123            ; 1a b3 81 23
    cpd 0,x              ; 1a a3 00
    cpd 255,y            ; cd a3 ff
    cmpd #$1234          ; 1a 83 12 34
    cmpy $80             ; 18 9c 80
    cmpx #$1234          ; 8c 12 34
    cpx 255,y            ; cd ac ff
    ldx 0,y              ; cd ee 00
    stx 255,y            ; cd ef ff
    ldaa 0,y             ; 18 a6 00
    ldab 255,y           ; 18 e6 ff
    staa 0,y             ; 18 a7 00
    stab 255,y           ; 18 e7 ff
    ldd 0,y              ; 18 ec 00
    std 255,y            ; 18 ed ff
    lds 0,y              ; 18 ae 00
    sts 255,y            ; 18 af ff
    adda 0,y             ; 18 ab 00
    addb 255,y           ; 18 eb ff
    adca 0,y             ; 18 a9 00
    adcb 255,y           ; 18 e9 ff
    suba 0,y             ; 18 a0 00
    subb 255,y           ; 18 e0 ff
    sbca 0,y             ; 18 a2 00
    sbcb 255,y           ; 18 e2 ff
    addd 0,y             ; 18 e3 00
    subd 255,y           ; 18 a3 ff
    cmpa 0,y             ; 18 a1 00
    cmpb 255,y           ; 18 e1 ff
    anda 0,y             ; 18 a4 00
    andb 255,y           ; 18 e4 ff
    oraa 0,y             ; 18 aa 00
    orab 255,y           ; 18 ea ff
    eora 0,y             ; 18 a8 00
    eorb 255,y           ; 18 e8 ff
    bita 0,y             ; 18 a5 00
    bitb 255,y           ; 18 e5 ff
    asl 0,y              ; 18 68 00
    asr 255,y            ; 18 67 ff
    lsr 0,y              ; 18 64 00
    rol 255,y            ; 18 69 ff
    ror 0,y              ; 18 66 00
    inc 255,y            ; 18 6c ff
    dec 0,y              ; 18 6a 00
    neg 255,y            ; 18 60 ff
    com 0,y              ; 18 63 00
    clr 255,y            ; 18 6f ff
    tst 0,y              ; 18 6d 00
    jmp 255,y            ; 18 6e ff
    jsr 0,y              ; 18 ad 00
    jsr $80              ; 9d 80
    bset $80,#$40        ; 14 80 40
    bclr $80,$40         ; 15 80 40
    bset 255,x,#1        ; 1c ff 01
    bclr 0,x,#$ff        ; 1d 00 ff
    bset 0,y,#$80        ; 18 1c 00 80
    bclr 255,y,#$55      ; 18 1d ff 55
    brset $80,#1,*+4     ; 12 80 01 00
    brclr $ff,#$80,*     ; 13 ff 80 fc
    brset 0,x,#1,*+4     ; 1e 00 01 00
    brclr 5,x,#2,*       ; 1f 05 02 fc
    brset 255,y,#$80,*+5 ; 18 1e ff 80 00
    brclr 0,y,#$ff,*     ; 18 1f 00 ff fb
    brset $80,#1,*+131   ; 12 80 01 7f
    brclr 0,y,#1,*-123   ; 18 1f 00 01 80
    ldaa y               ; 18 a6 00
    ldaa y,255           ; 18 a6 ff
    ldy x,255            ; 1a ee ff
    ldx y,255            ; cd ee ff
    bra *+2              ; 20 00
    brn *                ; 21 fe
    int $38              ; 3f 38
    end
