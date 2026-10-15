    public print,buf
print
    ldx #message
    int $23
    rts
message db 'OBJECT PASS',10,0
    section
buf ds 16
    ends
    end
