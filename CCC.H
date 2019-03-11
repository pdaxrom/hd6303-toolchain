;*  SMALL-C HEADER FILE

		ORG	$800

fclose		EQU $104
fopen		EQU $108
getc		EQU $10C
getchar		EQU $110
gets		EQU $114
putc		EQU $118
putchar		EQU $11C
puts		EQU $120
RTSC		EQU $124
isalpha		EQU $128
isdigit		EQU $12C
isalnum		EQU $130
islower		EQU $134
isupper		EQU $138
isspace		EQU $13C
toupper		EQU $140
tolower		EQU $144
strclr		EQU $148
strlen		EQU $14C
strcpy		EQU $150
strcat		EQU $154
strcmp		EQU $158
exit		EQU $15C
fgets		EQU $160
fputs		EQU $164
fread		EQU $168
fwrite		EQU $16C
feof		EQU $170
fflush		EQU $174
fseek		EQU $178
ftell		EQU $17C
unlink		EQU $180
system		EQU $184
geterrno	EQU $188
getstrerr	EQU $18C
EOF		EQU -1
