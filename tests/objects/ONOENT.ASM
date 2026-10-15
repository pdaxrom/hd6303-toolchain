    public print
print ldx #message
    db $3f,$23
    rts
message db 'CROSS PASS',10,0
noentry equ $ffff
    end noentry
