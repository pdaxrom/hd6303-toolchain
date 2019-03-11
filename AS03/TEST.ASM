    org 	$100

    include test.inc

max	= $FFFF

;
;
;
start
    psha
    pshb
    pshx
    ldaa	#44
    ldx		#$a55a
    lds		#$1000
    ldd		$04
    ldd		$e628
    bne	~exit
    xgdx
    ldx		#hello
    stx		$80
    sts		$4000
    ldaa	0,x
    ldab	$fa,x
    ldd		220,x
    std		x
    MUL
    nop
    ABA
    ABX
    int		$20
    int		$22
    ldd		#max
    daa
    PULX
    PULB
    PULA
    staa	VPU_DATA
    jsr		$e0
    jsr		$F006
    jmp		$F000
~exit	RTS

	ds	16

dat	db	$33
hello	text	"HELLO, WORLD!"
	db	0
	db	$00,$01,$02,$03
	dw	$F800 ,$F802,$F804,$F806,$E600,$0
	ds	$200-*,$FF
