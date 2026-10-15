; INT EF services; query ABI remains 1/X=5A56, B=CPU (0/1).
CV_QUERY equ 0
CV_RUN equ 1
CV_CALL equ 2
; CV_CALL X points to ten conventional-RAM bytes, all words big endian.
CV_ENTRY equ 0
CV_BYTES equ 2
CV_ARGS equ 4
CV_VALUE equ 6
CV_STATUS equ 8
CV_SIZE equ 10
CV_MAXARGS equ 64
; Arguments are contiguous words in C source order, first argument first.
; CV_BYTES must be even, 0..64. A=0 success, A=FF runtime error,
; A=3 invalid descriptor address. CV_STATUS is the returned A, zero extended.
; On failure CV_VALUE is unspecified. No argc/argv is synthesized.
