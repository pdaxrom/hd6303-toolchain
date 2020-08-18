#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#define byte unsigned char
#define word unsigned short

#define LD1IM		2*0
#define LD1SOFF		2*1
#define LD1		2*2
#define LDB1		2*3
#define LD1R		2*4
#define LDB1R		2*5
#define ST1		2*6
#define STB1		2*7
#define ST1SP		2*8
#define STB1SP		2*9
#define PUSHR1		2*10
#define EXG1		2*11
#define JMPL		2*12
#define BRZL		2*13
#define JSRL		2*14
#define JSRSP		2*15
#define RTSC		2*16
#define MODSP		2*17
#define DBL1		2*18
#define ADDS		2*19
#define SUBFST		2*20
#define MUL1		2*21
#define DIV1		2*22
#define MOD		2*23
#define ORS		2*24
#define XORS		2*25
#define ANDS		2*26
#define ASRS		2*27
#define ASLS		2*28
#define NEGR		2*29
#define NOTR		2*30
#define INCR		2*31
#define DECR		2*32
#define ZEQ		2*33
#define ZNE		2*34
#define ZLT		2*35
#define ZLE		2*36
#define ZGT		2*37
#define ZGE		2*38
#define ULT		2*39
#define ULE		2*40
#define UGT		2*41
#define UGE		2*42
#define ASMC		2*43

#define ZVM_BASE	0xC000

#define f_fclose	ZVM_BASE+0x104
#define f_fopen		ZVM_BASE+0x108
#define f_getc		ZVM_BASE+0x10C
#define f_getchar	ZVM_BASE+0x110
#define f_gets		ZVM_BASE+0x114
#define f_putc		ZVM_BASE+0x118
#define f_putchar	ZVM_BASE+0x11C
#define f_puts		ZVM_BASE+0x120
#define f_RTSC		ZVM_BASE+0x124
#define f_isalpha	ZVM_BASE+0x128
#define f_isdigit	ZVM_BASE+0x12C
#define f_isalnum	ZVM_BASE+0x130
#define f_islower	ZVM_BASE+0x134
#define f_isupper	ZVM_BASE+0x138
#define f_isspace	ZVM_BASE+0x13C
#define f_toupper	ZVM_BASE+0x140
#define f_tolower	ZVM_BASE+0x144
#define f_strclr	ZVM_BASE+0x148
#define f_strlen	ZVM_BASE+0x14C
#define f_strcpy	ZVM_BASE+0x150
#define f_strcat	ZVM_BASE+0x154
#define f_strcmp	ZVM_BASE+0x158
#define f_exit		ZVM_BASE+0x15C
#define f_fgets		ZVM_BASE+0x160
#define f_fputs		ZVM_BASE+0x164
#define f_fread		ZVM_BASE+0x168
#define f_fwrite	ZVM_BASE+0x16C
#define f_feof		ZVM_BASE+0x170
#define f_fflush	ZVM_BASE+0x174
#define f_fseek		ZVM_BASE+0x178
#define f_ftell		ZVM_BASE+0x17C
#define f_unlink	ZVM_BASE+0x180
#define f_system	ZVM_BASE+0x184
#define f_geterrno	ZVM_BASE+0x188
#define f_getstrerr	ZVM_BASE+0x18C

#define f_first		f_fclose
#define f_last		f_getstrerr

FILE *fdtab[256];

byte mem[0x10000];
word load = 0x100;
word start = 0x140;

#ifdef APP
byte app[] = {
#include APP
};
#endif

#define GETBYTE(a) (mem[a])
#define GETWORD(a) ((GETBYTE(a) << 8) | GETBYTE((a) + 1))
#define GETDWORD(a) ((GETWORD(a) << 16) | GETWORD((a) + 2))

#define GETARG1() GETWORD(*sp + 3)
#define GETARG2() GETWORD(*sp + 5)
#define GETARG3() GETWORD(*sp + 7)
#define GETARG4() GETWORD(*sp + 9)

void chkfunc(word *sp, word *pc, word *reg)
{
    if ((*pc >= (f_first)) && (*pc <= (f_last))) {
	word tmp, tmp1, tmp2, tmp3;
//	fprintf(stderr, "External function request, PC=$%04X\n", *pc);

	switch (*pc) {
	case f_fclose:
		tmp = GETARG1();
		if (fdtab[tmp]) {
		    fclose(fdtab[tmp]);
		    fdtab[tmp] = 0;
		}
		*reg = 0;
		break;
	case f_fopen:
		tmp1 = GETARG1();
		tmp = GETARG2();
		FILE *f = fopen((char *)&mem[tmp], (char *)&mem[tmp1]);
		if (f) {
		    *reg = fileno(f);
		    if (*reg > sizeof(fdtab) / sizeof(FILE *)) {
			fprintf(stderr, "fd > fdtab, error!\n");
			exit(-1);
		    }
		    fdtab[*reg] = f;
		} else {
		    *reg = 0;
		}
		break;
	case f_getc: tmp = GETARG1(); *reg = fgetc(fdtab[tmp]); break;
	case f_getchar: *reg = getchar(); break;
	case f_gets: tmp = GETARG1(); *reg = gets(&mem[tmp]) ? tmp : 0; break;
	case f_putc:
		tmp1 = GETARG1();
		tmp = GETARG2();
		*reg = fputc(tmp, fdtab[tmp1]);
		break;
	case f_putchar: tmp = GETARG1(); printf("%c", tmp); *reg = tmp; break;
	case f_puts: tmp = GETARG1(); printf("%s", &mem[tmp]); *reg = 0; break;

	case f_isalpha: tmp = GETARG1(); *reg = isalpha(tmp); break;
	case f_isdigit: tmp = GETARG1(); *reg = isdigit(tmp); break;
	case f_isalnum: tmp = GETARG1(); *reg = isalnum(tmp); break;
	case f_islower: tmp = GETARG1(); *reg = islower(tmp); break;
	case f_isupper: tmp = GETARG1(); *reg = isupper(tmp); break;
	case f_isspace: tmp = GETARG1(); *reg = isspace(tmp); break;
	case f_toupper: tmp = GETARG1(); *reg = toupper(tmp); break;
	case f_tolower: tmp = GETARG1(); *reg = tolower(tmp); break;
	case f_strclr:
		tmp1 = GETARG1();
		tmp = GETARG2();
		memset(&mem[tmp], 0, tmp1);
		*reg = tmp;
		break;
	case f_strlen: tmp = GETARG1(); *reg = strlen((char *)&mem[tmp]); break;
	case f_strcpy:
		tmp1 = GETARG1();
		tmp = GETARG2();
		// GCC 9 - no overlap in strcpy
		memmove((char *)&mem[tmp], (char *)&mem[tmp1], strlen((char *)&mem[tmp1]) + 1);
		*reg = tmp;
		break;
	case f_strcat:
		tmp1 = GETARG1();
		tmp = GETARG2();
		strcat((char *)&mem[tmp], (char *)&mem[tmp1]);
		*reg = tmp;
		break;
	case f_strcmp:
		tmp1 = GETARG1();
		tmp = GETARG2();
		*reg = strcmp((char *)&mem[tmp], (char *)&mem[tmp1]);
		break;

	case f_exit: tmp = GETARG1(); exit((short)tmp); break;

	case f_fgets:
		tmp  = GETARG1();
		tmp1 = GETARG2();
		tmp2 = GETARG3();
		*reg = fgets((char *)&mem[tmp2], tmp1, fdtab[tmp]) ? tmp2 : 0;
		break;
	case f_fputs:
		tmp  = GETARG1();
		tmp1 = GETARG2();
		*reg = fputs((char *)&mem[tmp1], fdtab[tmp]);
		break;
	case f_fread:
		tmp  = GETARG1();
		tmp1 = GETARG2();
		tmp2 = GETARG3();
		tmp3 = GETARG4();
		*reg = fread(&mem[tmp3], tmp2, tmp1, fdtab[tmp]);
		break;
	case f_fwrite:
		tmp  = GETARG1();
		tmp1 = GETARG2();
		tmp2 = GETARG3();
		tmp3 = GETARG4();
		*reg = fwrite(&mem[tmp3], tmp2, tmp1, fdtab[tmp]);
		break;
	case f_feof:
		tmp  = GETARG1();
		*reg = feof(fdtab[tmp]);
		break;
	case f_fflush:
		tmp  = GETARG1();
		*reg = fflush(fdtab[tmp]);
		break;
	case f_fseek:
		tmp  = GETARG1();
		tmp1 = GETARG2();
		tmp2 = GETARG3();
		*reg = fseek(fdtab[tmp2], GETDWORD(tmp1) , tmp);
		break;
	case f_ftell:
		tmp  = GETARG1();
		tmp1 = GETARG2();
		long pos = ftell(fdtab[tmp1]);
		mem[tmp    ] = (pos >> 24);
		mem[tmp + 1] = (pos >> 16) & 0xFF;
		mem[tmp + 2] = (pos >> 8 ) & 0xFF;
		mem[tmp + 3] = (pos      ) & 0xFF;
		*reg = tmp;
		break;
	case f_unlink: tmp = GETARG1(); *reg = unlink((char *)&mem[tmp]); break;
	case f_system: tmp = GETARG1(); *reg = system((char *)&mem[tmp]); break;
	case f_geterrno: *reg = errno; break;
	case f_getstrerr:
		tmp  = GETARG1();
		tmp1 = GETARG2();
		strcpy((char *)&mem[tmp1], strerror(tmp));
		break;

//	default: return; break; 
	default: fprintf(stderr, "Unimplemented function %X\n", *pc); exit(1); break;
	}

	*pc = (mem[*sp + 1] << 8) | mem[*sp + 2];
	*sp = *sp + 2;
    }
}

int main(int argc, char *argv[])
{
#ifndef APP
    FILE *inf;
    word len;
#endif

    word sp;
    word pc;

    fprintf(stderr, "SmallC virtual machine\n");

#ifdef APP
    memcpy(&mem[load], app, sizeof(app));
#else
    inf = fopen(argv[1], "rb");
    if (!inf) {
	fprintf(stderr, "Can't open file %s\n", argv[1]);
	return 1;
    }
    len = fread(&mem[load], 1, sizeof(mem) - load, inf);
    fclose(inf);
#endif

    sp = 0xFF00;
    pc = 0xFF20;
#ifdef APP
    for (int i = 0; i < argc; i++) {
#else
    for (int i = 1; i < argc; i++) {
#endif
	strcpy((char *)&mem[pc], argv[i]);
	mem[sp++] = pc >> 8;
	mem[sp++] = pc & 0xFF;
	pc += strlen(argv[i]) + 1;
    }

/*
    pc = 0xFF00;
    for (int i = 0; i < argc - 1; i++) {
	printf("arg%d [%s]\n", i, &mem[(mem[pc] << 8) | mem[pc + 1]]);
	pc += 2;
    }
 */

#ifndef APP
    argc--;
#endif
    sp = 0xFEFF;
    mem[sp--] = argc & 0xFF;
    mem[sp--] = argc >> 8;
    mem[sp--] = 0x00;
    mem[sp--] = 0xFF;

    pc = start;
//    sp = 0xFEFF;	// maximum memory
    word reg = 0;
    word tmp = 0;

    int running = 1;

    while (running) {
	switch (mem[pc++]) {
	case LD1IM:	reg = (mem[pc] << 8) | mem[pc + 1]; pc += 2; break;
	case LD1SOFF:	reg = (mem[pc] << 8) | mem[pc + 1]; pc += 2; reg += sp + 1; break;
	case LD1:	tmp = (mem[pc] << 8) | mem[pc + 1]; pc += 2; reg = (mem[tmp] << 8) | mem[tmp + 1]; break;
	case LDB1:	tmp = (mem[pc] << 8) | mem[pc + 1]; pc += 2; reg = (((mem[tmp] & 0x80)? 0xFF : 0) << 8) | mem[tmp]; break;
	case LD1R:	reg = (mem[reg] << 8) | mem[reg + 1]; break;
	case LDB1R:	reg = (((mem[reg] & 0x80)? 0xFF : 0) << 8) | mem[reg]; break;
	case ST1:	tmp = (mem[pc] << 8) | mem[pc + 1]; pc += 2; mem[tmp] = reg >> 8; mem[tmp + 1] = reg & 0xFF; break;
	case STB1:	tmp = (mem[pc] << 8) | mem[pc + 1]; pc += 2; mem[tmp] = reg & 0xFF; break;
	case ST1SP:	tmp = sp + 1; tmp = (mem[tmp] << 8) | mem[tmp + 1]; mem[tmp] = reg >> 8; mem[tmp + 1] = reg & 0xFF; sp += 2; break;
	case STB1SP:	tmp = sp + 1; tmp = (mem[tmp] << 8) | mem[tmp + 1]; mem[tmp] = reg & 0xFF; sp += 2; break;
	case PUSHR1:	mem[sp--] = reg & 0xFF; mem[sp--] = reg >> 8; break;
	case EXG1:	tmp = (mem[sp + 1] << 8) | mem[sp + 2]; mem[sp + 1] = reg >> 8; mem[sp + 2] = reg & 0xFF; reg = tmp; break;
	case JMPL:	pc = (mem[pc] << 8) | mem[pc + 1]; break;
	case BRZL:	if (reg == 0) { pc = (mem[pc] << 8) | mem[pc + 1]; } else { pc += 2; }; break;
	case JSRL:	tmp = pc + 2; mem[sp--] = tmp & 0xFF; mem[sp--] = tmp >> 8; pc = (mem[pc] << 8) | mem[pc + 1]; chkfunc(&sp, &pc, &reg); break;
	case JSRSP:	tmp = (mem[sp + 1] << 8) | mem[sp + 2]; mem[sp + 1] = pc >> 8; mem[sp + 2] = pc & 0xFF; pc = tmp; chkfunc(&sp, &pc, &reg); break;
	case RTSC:	pc = (mem[sp + 1] << 8) | mem[sp + 2]; sp += 2; break;
	case MODSP:	tmp = (mem[pc] << 8) | mem[pc + 1]; pc += 2; sp += tmp; break;
	case DBL1:	reg <<= 1; break;
	case ADDS:	tmp = (mem[sp + 1] << 8) | mem[sp + 2]; sp += 2; reg = tmp + reg; break;
	case SUBFST:	tmp = (mem[sp + 1] << 8) | mem[sp + 2]; sp += 2; reg = tmp - reg; break;
	case MUL1:	tmp = (mem[sp + 1] << 8) | mem[sp + 2]; sp += 2; reg = tmp * reg; break;
	case DIV1:	tmp = (mem[sp + 1] << 8) | mem[sp + 2]; sp += 2; reg = tmp / reg; break;
	case MOD:	tmp = (mem[sp + 1] << 8) | mem[sp + 2]; sp += 2; reg = tmp % reg; break;
	case ORS:	tmp = (mem[sp + 1] << 8) | mem[sp + 2]; sp += 2; reg = tmp | reg; break;
	case XORS:	tmp = (mem[sp + 1] << 8) | mem[sp + 2]; sp += 2; reg = tmp ^ reg; break;
	case ANDS:	tmp = (mem[sp + 1] << 8) | mem[sp + 2]; sp += 2; reg = tmp & reg; break;
	case ASRS:	tmp = (mem[sp + 1] << 8) | mem[sp + 2]; sp += 2; reg = tmp >> reg; break;
	case ASLS:	tmp = (mem[sp + 1] << 8) | mem[sp + 2]; sp += 2; reg = tmp << reg; break;
	case NEGR:	reg = ~reg; reg++; break;
	case NOTR:	reg = ~reg; break;
	case INCR:	reg++; break;
	case DECR:	reg--; break;
	case ZEQ:	tmp = (mem[sp + 1] << 8) | mem[sp + 2]; sp += 2; reg = (tmp == reg) ? 1: 0; break;
	case ZNE:	tmp = (mem[sp + 1] << 8) | mem[sp + 2]; sp += 2; reg = (tmp != reg) ? 1: 0; break;
	case ZLT:	tmp = (mem[sp + 1] << 8) | mem[sp + 2]; sp += 2; reg = (((short)tmp) < ((short)reg)) ? 1: 0; break;
	case ZLE:	tmp = (mem[sp + 1] << 8) | mem[sp + 2]; sp += 2; reg = (((short)tmp) <= ((short)reg)) ? 1: 0; break;
	case ZGT:	tmp = (mem[sp + 1] << 8) | mem[sp + 2]; sp += 2; reg = (((short)tmp) > ((short)reg)) ? 1: 0; break;
	case ZGE:	tmp = (mem[sp + 1] << 8) | mem[sp + 2]; sp += 2; reg = (((short)tmp) >= ((short)reg)) ? 1: 0; break;
	case ULT:	tmp = (mem[sp + 1] << 8) | mem[sp + 2]; sp += 2; reg = (tmp < reg) ? 1: 0; break;
	case ULE:	tmp = (mem[sp + 1] << 8) | mem[sp + 2]; sp += 2; reg = (tmp <= reg) ? 1: 0; break;
	case UGT:	tmp = (mem[sp + 1] << 8) | mem[sp + 2]; sp += 2; reg = (tmp > reg) ? 1: 0; break;
	case UGE:	tmp = (mem[sp + 1] << 8) | mem[sp + 2]; sp += 2; reg = (tmp >= reg) ? 1: 0; break;
	case ASMC:	if (!((mem[pc] == 0x3f) && mem[pc + 1] == 0x38)) fprintf(stderr, "ASMC is not supported!"); running = 0; break;
	default:	fprintf(stderr, "Unsupported bytecode %02X at %04X\n", mem[pc - 1], pc - 1); running = 0; break;
	}
    }

    return 0;
}
