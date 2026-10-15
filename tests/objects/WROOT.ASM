    extern target
start db target,/target
    dw target
    db local,/local
    dw local
local int $38
    end start
