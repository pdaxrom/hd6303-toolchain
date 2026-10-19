    org $100
    .ifdef __HC11__
    include COND1.INC
    .else
    db $aa
    .endif
    db $55
    end
