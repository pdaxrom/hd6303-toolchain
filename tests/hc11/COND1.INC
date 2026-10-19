    .ifdef __HC11__
    include COND2.INC
    .else
    db $bb
    .endif
    db $22
