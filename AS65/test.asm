;    *=$1000
    *=$100
;
;
;
start
    pha
    pha
    pla
    lda	dat
    lda	#$10
    ldx #$FA
    lda	0,x
    bne	~hello
    jsr  ~hello
    pla
    tay
~hello
    rts
dat
    .byte	$33
