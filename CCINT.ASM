	ORG	$100
; NAM SMALL-C INTERPRETER
; OPT NOP,NOG
;
;   LAST UPDATE   9-SEP-82
;

; LIB FLEXPTRS
	SECTION	$E8
PC	DS	2	; PSEUDO PROGRAM COUNTER
R1A	DS	1	; WORKING 16 BIT
R1B	DS	1	; --REGISTER
STEMP	DS	2	; TEMP STORAGE FOR STACK POINTER
X1TMP	DS	2	; TEMP STORAGE FOR X REG
X2TMP	DS	2	; ... DITTO ...
DFLAG	DS	1	; DIVIDE ROUTINE FLAG

ERRNO	DS	1
FTABLE	DS	4
FCHARIO	DS	2
	DS	2
FCHARBUF DS	1
	ENDS

;**************************************************

CURR_PCB EQU	$EC00
ARGC	EQU	$1D
ARGV	EQU	$20

	JMP	BEGIN ;START THE INTERPRETER

;  AN INDIRECT CALL TABLE
	NOP		;PUT ON A BOUNDARY OF 4

	DB	86
	JMP	fclose+1
	DB	86
	JMP	fopen+1
	DB	86
	JMP	getc+1
	DB	86
	JMP	getchar+1
	DB	86
	JMP	gets+1
	DB	86
	JMP	putc+1
	DB	86
	JMP	putchar+1
	DB	86
	JMP	puts+1
	NOP
	JMP	RTSC
	DB	86
	JMP	isalpha+1
	DB	86
	JMP	isdigit+1
	DB	86
	JMP	isalnum+1
	DB	86
	JMP	islower+1
	DB	86
	JMP	isupper+1
	DB	86
	JMP	isspace+1
	DB	86
	JMP	toupper+1
	DB	86
	JMP	tolower+1
	DB	86
	JMP	strclr+1
	DB	86
	JMP	strlen+1
	DB	86
	JMP	strcpy+1
	DB	86
	JMP	strcat+1
	DB	86
	JMP	strcmp+1
	DB	86
	JMP	sysexit+1
	DB	86
	JMP	fgets+1
	DB	86
	JMP	fputs+1
	DB	86
	JMP	fread+1
	DB	86
	JMP	fwrite+1
	DB	86
	JMP	feof+1
	DB	86
	JMP	fflush+1
	DB	86
	JMP	fseek+1
	DB	86
	JMP	ftell+1
	DB	86
	JMP	unlink+1
	DB	86
	JMP	system+1
	DB	86
	JMP	geterrno+1
	DB	86
	JMP	getstrerr+1
	DS	4*4,$FF	;ROOM FOR 4 MORE

BEGIN
	LDX	#FCHARBUF
	STX	FCHARIO
	LDX	#1
	STX	FCHARIO+2

	LDX	CURR_PCB
	LDAB	ARGC,X
	CLRA
	XGDX
	ADDD	#ARGV
	PSHX
	PSHB
	PSHA

EXECUTE	LDX	#$800
	BRA	NEXT2		; START THE INTERPRETATION

;*************************************************************
;
;  THE HEART OF THE INTERPRETER--- NEXT INSTRUCTION FETCHER.
;
BUMP2	LDX	PC		; GET PROG COUNTER
BUMP2A	INX			; INCR BY 2
	INX
	BRA	NEXT1		; FETCH NEXT INSTRUCTION

NEXT	LDX	PC
NEXT1	STD	R1A		; SAVE THE WORK REGISTER
NEXT2	LDAB	0,X		; GET THE PSEUDO-INSTRUCTION
	INX			;  (B CONTAINS A TABLE OFFSET)
	STX	PC		; SAVE NEW PC
	LDX	#JTABLE
	ABX
	LDX	0,X
	LDD	R1A		; RESTORE
	JMP	0,X		; GO EXECUTE THE PSEUDO-INSTR.

;*************************************************************
;                  THE JUMP TABLE                            *
;*************************************************************

	DS $200-*		;  MUST START ON A PAGE BOUNDARY

JTABLE
 DW LD1IM		; #0
 DW LD1SOFF		; #1
 DW LD1			; #2
 DW LDB1		; #3
 DW LD1R		; #4
 DW LDB1R		; #5
 DW ST1			; #6
 DW STB1		; #7
 DW ST1SP		; #8
 DW STB1SP		; #9
 DW PUSHR1		; #10
 DW EXG1		; #11
 DW JMPL		; #12
 DW BRZL		; #13
 DW JSRL		; #14
 DW JSRSP		; #15
 DW RTSC		; #16
 DW MODSP		; #17
 DW DBL1		; #18
 DW ADDS		; #19
 DW SUBFST		; #20
 DW MUL1		; #21
 DW DIV1		; #22
 DW MOD			; #23
 DW ORS			; #24
 DW XORS		; #25
 DW ANDS		; #26
 DW ASRS		; #27
 DW ASLS		; #28
 DW NEGR		; #29
 DW NOTR		; #30
 DW INCR		; #31
 DW DECR		; #32
 DW ZEQ			; #33
 DW ZNE			; #34
 DW ZLT			; #35
 DW ZLE			; #36
 DW ZGT			; #37
 DW ZGE			; #38
 DW ULT			; #39
 DW ULE			; #40
 DW UGT			; #41
 DW UGE			; #42
 DW ASMC		; #43

;************************************************************
;-------------------------
; #0 LOAD REG WITH IMMED. VALUE
LD1IM	LDX	PC
	LDD	0,X
	JMP	BUMP2A

;-------------------------
; #1 LOAD STACK ADDRESS + OFFSET INTO REG
LD1SOFF	STS	R1A	; SAVE STACK VALUE
	LDX	PC
	LDD	0,X	; GET OFFSET VALUE
	ADDD	R1A
	ADDD	#1	; ADD OFFSET + 1
	JMP	BUMP2A

;-------------------------
; #2  LOAD WORD @ ADDRESS
LD1	LDX	PC
	LDX	0,X	; GET ADDRESS
	LDD	0,X	; GET WORD
	JMP	BUMP2

;-------------------------
; #3  LOAD BYTE @ ADDRESS
LDB1	LDX	PC
	LDX	0,X	; GET ADDRESS
	CLRA
	LDAB	0,X	; GET BYTE
	BPL	LDB1A
	COMA		; SIGN EXTEND
LDB1A	JMP	BUMP2

;-------------------------
; #4  LOAD WORD INDIRECT (ADDR IN REG)
LD1R	LDX	R1A	; GET ADDRESS
	LDD	0,X	; GET WORD
	JMP	NEXT

;-------------------------
; #5  LOAD BYTE INDIRECT (ADDR IN REG)
LDB1R	LDX	R1A
	CLRA
	LDAB	0,X	; GET BYTE
	BPL	LDB1RA
	COMA
LDB1RA	JMP	NEXT

;-------------------------
; #6  STORE WORD @ ADDRESS
ST1	LDX	PC
	LDX	0,X	; GET ADDRESS
	STD	0,X	; STORE WORD
	JMP	BUMP2

;-------------------------
; #7  STORE BYTE @ ADDRESS
STB1	LDX	PC
	LDX	0,X	; GET ADDR
	STAB	0,X	; STORE BYTE
	JMP	BUMP2

;-------------------------
; #8  STORE WORD @ ADDRESS ON STACK
ST1SP	TSX		; STACK TO INDEX
	LDX	0,X	; GET ADDRESS
	STD	0,X	; STORE WORD
	INS
	INS		; POP STACK
	JMP	NEXT

;-------------------------
; #9  STORE BYTE @ ADDRESS ON STACK
STB1SP	TSX
	LDX	0,X
	STAB	0,X	; STORE BYTE
	INS		; POP ...
	INS
	JMP	NEXT

;-------------------------
; #10  PUSH WORD ON STACK
PUSHR1	PSHB
	PSHA
	LDX PC
	JMP NEXT2

;-------------------------
; #11  SWAP REG AND TOP OF STACK
EXG1	TSX
	LDX	0,X	; GET VALUE ON STACK
	STX	R1A	; SAVE
	INS
	INS
	PSHB
	PSHA		; REG ON STACK
	LDD	R1A	; NEW REG
	LDX	PC
	JMP	NEXT2

;-------------------------
; #12  JUMP TO LABEL
JMPL	LDX	PC
JMP1	LDX	0,X	; GET ADDRESS (NEW PC)
	JMP	NEXT2

;-------------------------
; #13  JUMP TO LABEL IF FALSE
BRZL	ORAA	R1B	; SET FLAGS
	BEQ	JMPL	; IF REG=0 -- JUMP
	JMP	BUMP2	; ELSE, PROCEED

;-------------------------
; #14  CALL TO LABEL
JSRL	LDX	PC
	INX		; ADJUST RETURN
	INX		; -- ADDRESS
	PSHX
	BRA	JMPL

;-------------------------
; #15  CALL TO TOP OF STACK
JSRSP	TSX		; POINT TO STACK
	LDX	0,X	; GET ADDRESS (NEW PC)
	INS		; POP
	INS
	LDD	PC
	PSHB
	PSHA
	JMP	NEXT2

;-------------------------
; #16  RETURN TO CALLER
RTSC	TSX
	LDX	0,X	; GET ADDRESS
	INS		; POP
	INS
	JMP	NEXT1

;-------------------------
; #17  MODIFY THE STACK POINTER
MODSP	LDX	PC
	LDD	0,X	; GET VALUE
	STS	STEMP
	ADDD	STEMP	; ADD STACK POINTER
	STD	STEMP
	LDS	STEMP	; NEW STACK POINTER
	LDD	R1A	; RESTORE REGISTER
	JMP	BUMP2A

;---------------------------
; #18  DOUBLE THE PRIMARY REGISTER
DBL1	ASLB
	ROLA
	JMP	NEXT

;---------------------------
; #19  ADD REG AND TOP OF STACK (THEN POP)
ADDS	TSX
	ADDD	0,X	; DO THE ADD
	JMP	POPS	; POP & RETURN

;---------------------------
; #20  SUBTRACT REG FROM TOP OF STACK
SUBFST	PULA		; GET VALUE OFF STACK
	PULB
	SUBD	R1A	; SUBTRACK REGISTER
	JMP	NEXT

;---------------------------
; #21  MULTIPLY TOP OF STACK BY REG (RESULT IN REG)
MUL1	PSHB
	PSHA		; REG ON STACK
	LDAA	#16
	PSHA		; SET COUNTER
	CLRA
	CLRB
	TSX		; POINT TO DATA

M2	ROR	3,X	; SHIFT MULTIPLIER
	ROR	4,X
	DEC	0,X	; DONE ?
	BMI	M4
	BCC	M3
	ADDD	1,X

M3	RORA
	RORB		; SHIFT RESULT
	BRA	M2	; AND LOOP

M4	INS		; CLEAN STACK
	INS
	INS
	PULA		; GET RESULT
	PULB
	JMP	NEXT

;-----------------------------
; #22  DIVIDE THE TOP OF STACK BY REG --- RESULT IN REG.
DIV1	BSR	BDIV	; DO THE BASIC DIVIDE
	LDAA	DFLAG	; GET SIGN FLAG
	ANDA	#1	; MASK OFF BIT ZERO
	PULA		; GET RESULT
	PULB
	BEQ	DIV1R

DIV1N	BSR	NEGATE	; NEGATE THE VALUE IN A,B

DIV1R	JMP	NEXT

;-----------------------------
; #23  DIVIDE TOP OF STACK BY REG --- REMAINDER IN REG
MOD	BSR	BDIV
	INS		; CLEAN STACK
	INS
	PSHA		; TEMP SAVE
	LDAA	DFLAG	; GET SIGN FLAG
	BPL	MOD1
	COMA

MOD1	ANDA	#1	; MASK OFF BIT 0
	PULA
	BNE	DIV1N	; IF BIT 0 SET, NEGATE

	JMP	NEXT

;****************************************************
;   BASIC 16 BIT DIVIDE ROUTINE
; ENTER WITH: DIVIDEND ON STACK
;             DIVISOR IN A,B
; EXIT WITH:  QUOTIENT ON STACK
;             REMAINDER IN A,B
;             SIGN FLAG IN DFLAG
;
BDIV	CLR	DFLAG
	TSTA			; CHECK DIVISOR SIGN
	BPL	BDIV1

	INC	DFLAG		; ADJUST SIGN FLAG
	BSR	NEGATE		; TAKE ABSOLUTE VALUE

BDIV1	PSHB			; FORCE ON STACK
	PSHA
	LDAA	#17		; BIT COUNTER
	PSHA
	TSX			; POINT TO DATA
	LDAA	5,X		; CHECK SIGN
	BPL	BDIV2		; -- OF DIVIDEND

	COM	DFLAG		; ADJUST FLAG
	LDAB	6,X
	BSR	NEGATE
	STD	5,X

BDIV2	CLRA
	CLRB

; MAIN DIVIDE LOOP (UNSIGNED)

UDIV1	CMPA	1,X
	BHI	UDIV3
	BCS	UDIV2
	CMPB	2,X
	BCC	UDIV3

UDIV2	CLC
	BRA	UDIV4

UDIV3	SUBD	1,X
	SEC

UDIV4	ROL	6,X
	ROL	5,X
	DEC	0,X
	BEQ	UDIV5

	ROLB
	ROLA
	BCC	UDIV1
	BRA	UDIV3

UDIV5	INS
	INS
	INS
	RTS

;----------------------------------------
; NEGATE THE VALUE IN A,B
NEGATE	COMA
	COMB
	ADDD	#1
	RTS

;----------------------------------
; #24  INCLUSIVE OR THE TOP OF STACK AND REG.
ORS	TSX
	ORAA	0,X
	ORAB	1,X
POPS	INS		; POP THE STACK
	INS
	JMP	NEXT

;----------------------------------
; #25  EXCLUSIVE OR ......
XORS	TSX
	EORA	0,X
	EORB	1,X
	BRA	POPS

;----------------------------------
; #26  AND .........
ANDS	TSX
	ANDA	0,X
	ANDB	1,X
	BRA	POPS

;----------------------------------
; #27  ARITH. SHIFT RIGHT THE TOP OF STACK
ASRS	TSX
	ANDB	#$1F	; MAX REASONABLE SHIFT
	BEQ	ASRS2

ASRS1	ASR	0,X
	ROR	1,X
	DECB
	BNE	ASRS1

ASRS2	PULA		; GET THE RESULT
	PULB
	JMP	NEXT

;--------------------------------
; #28  ARITH. SHIFT LEFT THE TOP OF STACK
ASLS	TSX
	ANDB	#$1F
	BEQ	ASRS2

ASLS1	ASL	1,X
	ROL	0,X
	DECB
	BNE	ASLS1
	BRA	ASRS2

;--------------------------------
; #29  NEGATE THE REGISTER
NEGR	BSR	NEGATE
	JMP	NEXT

;--------------------------------
; #30  COMPLEMENT THE REGISTER
NOTR	COMA
	COMB
	JMP	NEXT

;--------------------------------
; #31  ADD 1 TO REG
INCR	ADDD	#1
	JMP	NEXT

;--------------------------------
; #32 SUBTRACT 1 FROM REG
DECR	SUBD	#1
	JMP	NEXT

;****************************************************
;
;   BASIC COMPARE INSTRUCTION SUBROUTINE
;   Compare the top of Stack to Register and set Condition codes
;
;  Signed compare -- Carry reflects the sign of difference
;         (set means: top of stack < A,B )
;
SCMP	TSX
	LDD	2,X		; GET TOP OF STACK
	SUBD	R1A		; SET CONDITION FLAGS
	BPL	STCMP1		; SKIP IF PLUS

	STAB	R1B		; TEMP SAVE
	ORAA	R1B		; SET/RESET ZERO FLAG
	SEC			; AND SET CARRY
	RTS

STCMP1	STAB	R1B
	ORAA	R1B
	CLC			; CLEAR THE CARRY
	RTS
;
;  Unsigned compare, Carry set if top of stack < A,B
;
BCMP	TSX
	LDD	2,X		; GET TOP OF STACK
	CMPA	R1A		; CHECK TOP BYTE
	BNE	BCMP1
	CMPB	R1B
BCMP1	RTS

;-------------------------------
; #33  TEST FOR EQUALITY
ZEQ	BSR	BCMP
	BEQ	TRUE
	BRA	FALSE

;-------------------------------
; #34  TEST FOR NOT-EQUAL
ZNE	BSR	BCMP
	BNE	TRUE
	BRA	FALSE

;-------------------------------
; #35  TEST FOR LESS THAN
ZLT	BSR	SCMP
	BCS	TRUE
	BRA	FALSE

;-------------------------------
; #36  TEST FOR LESS THAN OR EQUAL
ZLE	BSR	SCMP
	BLS	TRUE
	BRA	FALSE

;-------------------------------
; #37  TEST FOR GREATER THAN
ZGT	BSR	SCMP
	BHI	TRUE
	BRA	FALSE

;-------------------------------
; #38  TEST FOR GREATER THAN OR EQUAL
ZGE	BSR	SCMP
	BCC	TRUE
	BRA	FALSE

;-------------------------------
; #39 TEST FOR LESS THAN (UNSIGNED)
ULT	BSR	BCMP
	BCS	TRUE
	BRA	FALSE

;-------------------------------
; #40  TEST FOR LESS THAN OR EQUAL (UNSIGNED)
ULE	BSR	BCMP
	BLS	TRUE
	BRA	FALSE

;-------------------------------
; #41  TEST FOR GREATER THAN (UNSIGNED)
UGT	BSR	BCMP
	BHI	TRUE
	BRA	FALSE

;------------------------------
; #42  TEST FOR GREATER THAN OR EQUAL (UNSIGNED)
UGE	BSR	BCMP
	BCC	TRUE

FALSE	CLRB		; RETURN FALSE
	BRA	TRUE1

TRUE	LDAB	#1	; RETURN TRUE

TRUE1	CLRA
	JMP	POPS	; POP STACK AND PROCEED

;-------------------------------------
; #43  SWITCH TO EXECUTABLE (ASSEMBLY) CODE
ASMC	LDX	PC	; POINT TO CODE
	JMP	0,X	; GO EXECUTE IT

;**********************************************************
;
;        RUN-TIME SUBROUTINE LIBRARY
;
;**********************************************************

;   fopen(file-name, "type")
;   Open a File..........
fopen	PROC
	GLOBAL	FEXIT
	DB	86		; SWITCH TO INLINE CODE
	LDAA	#$EB		; WRONG FILE OPEN MODE
	STAA	ERRNO
	TSX
	LDX	4,X		; POINTER TO FILE NAME
	STX	FTABLE
	LDX	#0
	STX	FTABLE+2
	TSX
	LDX	2,X		; POINTER TO MODE
	LDAB	0,X
	CMPB	#'w'
	BNE	FOPEN1
	LDAA	#2
	LDX	#FTABLE
	INT	$4A
	TSTA
	BNE	FCREAT
	PSHA
	PSHB
	TBA
	INT	$6C	; truncate
	PULB
	PULA
FWRRD	PSHB
	TSX
	LDX	2,X
	LDAB	1,X
	CMPB	#'+'
	PULB
	BNE	FEXIT
	TSTA
	BNE	FEXIT
	TBA
	INT	$4E
	BRA	FOPENRW
FCREAT	CLRA
	LDX	#FTABLE
	INT	$4B
	BRA	FWRRD

FOPEN1	CMPB	#'r'
	BNE	FOPERR
	LDAA	#1
	LDAB	1,X
	BEQ	FOPENR
	CMPB	#'+'
	BNE	FOPERR
FOPENRW	LDAA	#3
FOPENR	LDX	#FTABLE
	INT	$4A
FEXIT	STAA	ERRNO
	TSTA
	BEQ	FOPEXIT
FOPERR	LDD	#0	; FIXME STDIN
FOPEXIT JMP	RTSC
	ENDP

;-------------------------------------------------

;  fclose(unit)
;  CLOSE A FILE
fclose	PROC
	DB 86		; SWITCH TO IN-LINE
	TSX
	LDD	2,X		; FILE HANDLE
	TBA
	INT	$4E
	BRA	FEXIT
	ENDP

;--------------------------------------------------

;  getc(unit)  read a byte from file
;        return a char, else a -1 if EOF

getc	PROC
	GLOBAL	FRWERR
	DB	86
	TSX
	LDD	2,X
	TBA
	LDX	#FCHARIO
	INT	$4C
FRWERR	STAA	ERRNO
	TSTA
	BNE	GETCER
	CPX	#1
	BNE	GETCER
	LDAB	FCHARBUF
	CLRA
	BRA	GETCOK
GETCER	LDD	#-1
GETCOK	JMP	RTSC
	ENDP

;----------------------------------------------

;  putc(c,unit)   write to file

putc	PROC
	DB	86
	TSX
	LDAA	5,X
	STAA	FCHARBUF
	LDAA	3,X
	LDX	#FCHARIO
	INT	$4D
	BRA	FRWERR
	ENDP

;   getchar()    get a char from standard input

getchar	PROC
	DB	86
	INT	$20
	CMPA	#$1A		; COMPARE TO CNTRL-Z (EOF)
	BNE	GETCHOK		; NO
	LDD	#-1		; YES...
	BRA	GETCH1
GETCHOK	TAB			;  COPY CHAR IN A
	CLRA
GETCH1	JMP	RTSC
	ENDP

;-----------------------------------------------

;   putchar(c)   write a char to standard output

putchar	PROC
	DB	86
	TSX
	LDAA	3,X		; GET THE CHAR
	INT	$22
	TAB
	CLRA
	JMP	RTSC
	ENDP

;----------------------------------------------

;   gets(buffer)  get a char string into buffer

gets	PROC
	DB	86
	TSX
	LDX	2,X GET START OF BUFFER
	LDAB	#80		; max buffer size
	INT	$21
	CLRA
	TSX
	LDD	2,X
	JMP	RTSC
	ENDP

;----------------------------------------------
;  puts(string)  print a string on the terminal
puts	PROC
	DB	86
	TSX
	LDX	2,X		; GET STRING ADDRESS
	INT	$23
	LDAA	#10
	INT	$22
	LDD	#1
	JMP	RTSC
	ENDP

;----------------------------------------------
;
;
;       Test if given char is alpha     *
; isalpha(c)
;       char c;
; {     c=c&127;
;       return(((c>='a')&(c<='z'))|
;               ((c>='A')&(c<='Z'))|
;               (c='_'));
; }
;
isalpha	PROC
	DB	86	; switch to assembly
	TSX
	LDAB	3,X	; get char
	BSR	alPHA
	JMP	RTSC
	ENDP
;
;---------------------------------------------
;       Test if given char is numeric   *
;
; isdigit(c)
;       char c;
; {     c=c&127;
;       return((c>='0')&(c<='9'));
; }
;
isdigit	PROC
	DB	86
	TSX
	LDAB	3,X
	BSR	nuMERIC
	JMP	RTSC
	ENDP
;
;----------------------------------------------
;       Test if given char is alphanumeric      *
;
; isalnum(c)
;       char c;
; {     return((alpha(c)|(numeric(c)));
; }
;
isalnum	PROC
	GLOBAL	alPHA, nuMERIC
	DB	86
	TSX
	LDAB	3,X	;get char
	BSR	alPHA	;check if alpha
	TSX
	PSHB		;save result
	LDAB	3,X	;get char again
	BSR	nuMERIC	;check if decimal
	TSX
	ORAB	0,X	;fix flag
	INS		;clean stack
	JMP	RTSC
;
;
alPHA	CLRA
	ANDB	#$7F
	CMPB	#'a'
	BLT	alPHA1
	CMPB	#'z'
	BLE	alYES
alPHA1	CMPB	#'A'
	BLT	alPHA2
	CMPB	#'Z'
	BLE	alYES
alPHA2	CMPB	#'_'
	BEQ	alYES
;
alNO	CLRB
	RTS
;
alYES	LDAB	#1
	RTS
;
;
nuMERIC	CLRA
	ANDB	#$7F
	CMPB	#'0'
	BLT	alNO
	CMPB	#'9'
	BLE	alYES
	BRA	alNO
	ENDP
;
;-----------------------------------------------
;  islower(c)
;       char c;        returns TRUE if c is lower case alpha,
;                        FALSE otherwise.
;
islower	PROC
	GLOBAL	ISNO, ISYES
	DB	86	; SWITCH TO IN-LINE
	TSX
	LDAB	3,X	; GET CHAR
	ANDB	#$7F
	CLRA
	CMPB	#'a'
	BLT	ISNO
	CMPB	#'z'
	BLE	ISYES
;
ISNO	CLRB
	JMP	RTSC	; RETURN FALSE
;
ISYES	LDAB	#1
	JMP	RTSC	; RETURN TRUE
	ENDP
;
;----------------------------------------------
;  isupper(c)
;      char c;      return TRUE if c is upper case alpha.
;
isupper	PROC
	DB	86
	TSX
	LDAB	3,X
	ANDB	#$7F
	CLRA
	CMPB	#'A'
	BLT	ISNO
	CMPB	#'Z'
	BLE	ISYES
	BRA	ISNO
	ENDP
;
;-----------------------------------------------
;  isspace(c)
;      char c;       return TRUE if a "white space" char
;
isspace	PROC
	DB	86
	TSX
	LDAB	3,X	; GET CHAR
	ANDB	#$7F
	CLRA
	CMPB	#' '	;  SPACE ?
	BEQ	ISYES
	CMPB	#$0D	;  CR ???
	BEQ	ISYES
	CMPB	#$0A	;  LF ???
	BEQ	ISYES
	CMPB	#$09	;  HOR TAB ???
	BEQ	ISYES
	BRA	ISNO
	ENDP
;
;----------------------------------------------
;  toupper(c)
;      char c;   make c an upper case char if lower
;                          case alpha
;
toupper	PROC
	DB	86
	TSX
	LDD	2,X
	TSTA
	BNE	TOUPP1	;  SKIP IF MSB'S NOT ZERO
	CMPB	#'a'
	BLT	TOUPP1
	CMPB	#'z'
	BGT	TOUPP1
	SUBB	#$20	;  CONVERT TO UPPER CASE
TOUPP1	JMP	RTSC
	ENDP
;
;---------------------------------------------
;  tolower(c)
;      char c;   convert to lower case if upper case alpha.
;
tolower	PROC
	DB	86
	TSX
	LDD	2,X
	TSTA
	BNE	TOLOW1
	CMPB	#'A'
	BLT	TOLOW1
	CMPB	#'Z'
	BGT	TOLOW1
	ADDB	#$20	; CONVERT TO LOWER CASE
TOLOW1	JMP	RTSC
	ENDP
;
;---------------------------------------------
;  strclr(s,n)
;      char *s;  int n;  clear a string of n bytes.
;
strclr	PROC
	DB	86
	TSX
	LDD	2,X	; GET LENGTH OF STRING
	LDX	4,X	; POINT TO STRING
SCLR1	CLR	0,X
	INX
	SUBD	#1
	BNE	SCLR1
SCLR3	JMP	RTSC
	ENDP
;
;-----------------------------------------------
;  return the length of a string
;
; strlen(s)
;       char *s;
; {     char *t;
;       t=s;
;       while (*s) s++;
;       return (s-t);
; }
;
strlen	PROC
	DB	86
	TSX
	LDX	2,X	; point to string
	CLRA		; preset counter
	CLRB
loop	TST	0,X	; look for NULL
	BEQ	exit	; found !!
	INX
	ADDD	#1	; bump counter
	BRA	loop
exit	JMP	RTSC
	ENDP
;
;------------------------------------------------
;  strcpy(s1,s2)
;      char *s1, *s2;    copy s2 into s1.
;
strcpy	PROC
	DB	86
	TSX
	LDD	4,X		; POINT TO S1
	STD	X2TMP		; SAVE POINTER
	LDD	2,X		; POINT TO S2
	STD	X1TMP		;
	XGDX
	CLRA
	CLRB
loop	TST	0,X
	BEQ	copy
	ADDD	#1
	INX
	BRA	loop
copy	ADDD	#1		; zero - end of string
	PSHB			; Pyldin use BA instead of AB :(
	TAB
	PULA
	LDX	#X1TMP
	INT	$2D		; memmove
	LDD	X2TMP
	JMP	RTSC
	ENDP
;
;------------------------------------------------
;  strcat(s1,s2)
;      char *s1, *s2;   s2 is concatenated onto s1.
;
strcat	PROC
	DB	86
	TSX
	LDX	2,X
	STX	X2TMP	; SAVE POINTER TO S2
	TSX
	LDX	4,X	; POINT TO S1

SCAT1	TST	0,X	; LOOK FOR END OF STRING
	BEQ	SCAT2
	INX
	BRA	SCAT1

SCAT2	STX	X1TMP	; SAVE POINTER
	LDX	X2TMP
	LDAB	0,X
	INX
	STX	X2TMP
	LDX	X1TMP
	STAB	0,X
	BEQ	SCAT3
	INX
	BRA	SCAT2

SCAT3	JMP	RTSC
	ENDP
;
;--------------------------------------------
;  strcmp(s1,s2)
;        char *s1, *s2;       returns:   0  if s1 = s2
;                                       <0  if s1 < s2
;                                       >0  if s1 > s2
;
;strcmp(s1, s2)
;char *s1, *s2;
;{
;  do
;    {
;      c1 = (unsigned char) *s1++;
;      c2 = (unsigned char) *s2++;
;      if (c1 == '\0')
;        return c1 - c2;
;    }
;  while (c1 == c2);
;  return c1 - c2;
;}
;

strcmp	PROC
	DB	86
	TSX
	LDD	4,X	; POINT TO S1
	STD	X1TMP
	LDX	2,X	; POINT TO S2

loop	LDAB	0,X	; GET S1 CHAR
	INX
	STX	X2TMP
	LDX	X1TMP
	LDAA	0,X
	BEQ	s1exit
	SBA		; COMPARE BY SUBTRACTING
	BNE	exit
	INX
	STX	X1TMP
	LDX	X2TMP
	BRA	loop

s1exit	SBA
	BEQ	exit1
exit	TAB
	CLRA
	TSTB
	BPL	exit1
	COMA
exit1	JMP	RTSC
	ENDP

;
;--------------------------------------------
;  exit()
sysexit	PROC
	DB	86
	INT	$38
	ENDP

;
;--------------------------------------------
;  fgets(s, size, stream)
fgets	PROC
	DB	86
	TSX
	LDAB	5,X	; POINT TO size
	LDX	6,X	; POINT to s
loop	PSHB
	PSHX
	TSX
	LDAA	6,X	; POINT TO stream
	LDX	#FCHARIO
	INT	$4C
	STAA	ERRNO
	CPX	#1
	PULX
	PULB
	BNE	error
	TST	ERRNO
	BNE	error
	LDAA	FCHARBUF
	STAA	0,X
	INX
	CMPA	#10
	BEQ	endstr
	DECB
	BNE	loop
endstr	CLR	0,X
	BRA	ok
error	CLRB
	CLRA
exit	JMP	RTSC
ok	TSX
	LDD	6,X
	BRA	exit
	ENDP

;
;--------------------------------------------
;  fputs(s, stream)
fputs	PROC
	DB	86
	TSX
	LDX	4,X
	LDAA	3,X
	CLRB
loop	TST	0,X
	BEQ	write
	INCB
	INX
	BRA	loop
write	TSX
	LDX	4,X
	PSHB		;
	CLRB
	PSHB		;
	PSHX		;
	INT	$4D
	STAA	ERRNO
	INS
	INS
	INS
	INS
	TSTA
	BEQ	ok
	LDD	#-1
exit	JMP	RTSC
ok	XGDX
	BRA	exit
	ENDP

;
;--------------------------------------------
;  fread(s, size, n, stream)
fread	PROC
	GLOBAL	freader
	DB	86
	TSX
	LDD	4,X	; N
	LDX	6,X	; SIZE
	INT	$EE
	TSX
	LDX	8,X	; S
	PSHB
	PSHA
	PSHX
	TSX
	LDAA	7,X	; STREAM
	INT	$4C
freader	STAA	ERRNO
	INS
	INS
	INS
	INS
	TSTA
	BEQ	ok
	CLRB
	CLRA
exit	JMP	RTSC
ok	XGDX
	BRA	exit
	ENDP

;
;--------------------------------------------
;  fwrite(s, size, n, stream)
fwrite	PROC
	DB	86
	TSX
	LDD	4,X	; N
	LDX	6,X	; SIZE
	INT	$EE
	TSX
	LDX	8,X	; S
	PSHB
	PSHA
	PSHX
	TSX
	LDAA	7,X	; STREAM
	INT	$4D
	BRA	freader
	ENDP

;
;--------------------------------------------
;  feof(stream)
feof	PROC
	DB	86
	TSX
	LDAA	3,X
	CLRB
	INT	$58
	STAA	ERRNO
	CLRA
	JMP	RTSC
	ENDP

;
;--------------------------------------------
;  fflush(stream)
fflush	PROC
	DB	86
	TSX
	LDAA	3,X
	INT	$5A
	STAB	ERRNO
	JMP	RTSC
	ENDP

;
;--------------------------------------------
;  fseek(stream, offset, whence)
fseek	PROC
	DB	86
	TSX
	LDAB	3,X
	LDAA	7,X
	LDX	4,X
	INT	$50
	STAA	ERRNO
	TSTA
	BEQ	ok
	LDD	#-1
exit	JMP	RTSC
ok	CLRB
	BRA	exit
	ENDP

;
;--------------------------------------------
;  ftell(stream, *pos)
;  return pos pointer
ftell	PROC
	DB	86
	TSX
	LDAA	5,X
	LDX	2,X
	INT	$53
	STAA	ERRNO
	TSTA
	BEQ	ok
	LDD	#-1
exit	JMP	RTSC
ok	XGDX
	BRA	exit
	ENDP

;
;--------------------------------------------
;  unlink(name)
unlink	PROC
	DB	86
	TSX
	LDX	2,X
	INT	$4F
	STAA	ERRNO
	BEQ	ok
	LDD	#-1
exit	JMP	RTSC
ok	CLRB
	BRA	exit
	ENDP

;
;--------------------------------------------
;  system(cmdline)
system	PROC
	DB	86
	TSX
	LDX	2,X
	INT	$39
	STAA	ERRNO
	TAB
	CLRA
	JMP	RTSC
	ENDP

;
;--------------------------------------------
;  geterrno()
geterrno PROC
	DB	86
	LDAB	ERRNO
	CLRA
	JMP	RTSC
	ENDP

;
;--------------------------------------------
;  getstrerr(err, str)
getstrerr PROC
	DB	86
	TSX
	LDAA	5,X
	LDX	2,X
	INT	$3D
	JMP	RTSC
	ENDP

;
;
;****************************************************
;
	DS	$800-*,$FF
HERE	EQU	*	;   END OF INTERPRETER
; 
; END
