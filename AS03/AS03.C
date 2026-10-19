/* as03.c  - MC6801/HD6303 and MC68HC11 assembler */
/* 10-Feb-2019  v0.1  - A.Chukov     */

/* as65.c  -  MOS Technology 6502 assembler */
/* 11-Apr-1989  v0.70 - A.J.Travis */
/* 22-Jun-1991  v0.71 - J.G.Harston
 *   system-specifics removed to local.h
 *   uses '/' instead of '_' in filenames
 *   main() ends with exit(0);
 */

/*
 * implementation dependant pathnames, and i/o modes
 */

#define stdout 1

#define MNEMTAB "mnemtab"		/* opcode mnemonics */
#define OPTAB   "optab"			/* opcode table */
#define IO_W    "w"			/* binary write mode */
#define ILLEGAL    0			/* 0 - no opcode */

#define NULL 0

#define FILE int

/*
 * boolean constants
 */
#define TRUE        1
#define FALSE       0
#define ERROR      -1
#define SAME        0

/*
 * symbol table parameters
 */
#define LINESIZE  128
#define HASHSIZE  257
#define SYMSIZE 13000
#define PROCSIZE   16
#define IFDEFSIZE  16
#define INCFISIZE  16

#define NEXTPTR     0
#define VALUE       2
#define NAME        4
#define BYTE      256

#define SYMBLNK 0x80
#define SYMBLBL 0x40
#define SYMBCAL 0x20
#define SYMBDEF 0x10
#define SYMBBSS 1
#define SYMBEXT 2
#define SYMBPUB 4
#define SYMBFWD 8

/*
 * 6800 addressing modes
 */
#define OPFLAG	0
#define INHERIT	1
#define IMMED	2
#define DIRECT	3
#define EXTEND	4
#define IND_X	5
#define REL	6
#define NMODES	7

/* binary type */
#define BIN_PGM	0
#define BIN_CMD	1

/*
 * global variables
 */
char obj[LINESIZE];			/* object code output buffer */
char ibuf[LINESIZE];			/* input buffer */
char sbuf[LINESIZE];			/* symbol buffer */
char prepool[192];
int pretbl[32];
int precnt, preused;
int opnames[224], opvals[224], opcount;
int cpu_hc11, hc_index;
char *symmore,*symend2,*symsplit;
int symcap,symused;
char symtab[SYMSIZE];			/* symbol table */
char *ofile;				/* output file */
char *endsym;				/* end of symbol table */
char *start;				/* start of symbol table */
char *freeptr;				/* next free location in hash chain */
char *ip;				/* input buffer pointer */
char * loc;				/* location counter */
char * origin;				/* assembly origin */
char * _text;				/* start of text segment */
char * _data;				/* start of data segment */
char * _end;				/* end of program */
char * textsz;				/* size of text segment */
char * datasz;				/* size of data segment */
int hashtab[HASHSIZE];			/* hash table */
FILE *in[INCFISIZE];			/* input stream */
FILE *out;				/* output stream */
int endflag;				/* .end pseudo-op flag */
int list;				/* produce assembler listing */
int pass;				/* pass 1/2 */
int nbytes;				/* number of bytes in obj[] */
int errcnt;				/* error count */
int warncnt;				/* warning count */

int incnt;				/* input files counter */

int nset;				/* number of setloc pseudo-ops */

int nsect;				/* section pseudo-op */
char *locsave;				/* save old location */

int nproc;				/* proc pseudo-op */
int nloc[PROCSIZE];			/* proc counter */
int procsym[PROCSIZE];			/* proc symbols */

int liston;				/* temporary enable/disable listing */
int truncon;				/* truncate DB, DW listing */
char *lfile;				/* listing file */
FILE *outlst;				/* listing stream */

int chksumon;				/* enable checksum */
char *chkpoint;				/* checksum address */
int csumval;				/* checksum value */
char *chkpos;				/* checksum file position */
char *filepos;				/* output file size */

int linecnt[INCFISIZE];			/* lines counter */
int linestot;				/* total lines */

int bintype;				/* output binary type */
int flgrel;				/* flag if relative (label)
					   is used in expression */
int flgunk;				/* flag if undefined (label)
					   is used in expression */
FILE *outpgm;				/* pgm output */
char pfile[40];				/* pgm file name */
char bfile[40];				/* bin file name */
int relsize;				/* size of reltable */
int makeobj, npub, nextidx, sectkind, radix, flgext;
int refvalue, refpart, entry, entryset;
char *codesize, *bssused;
char rfile[40];


int ifdefcnt;				/* ifdef stack counter */
char ifdefstk[IFDEFSIZE];		/* ifdef stack */
int incifbase[INCFISIZE];        /* condition depth at each include entry */
char ignotdef;				/* undefined symbol return 0 */

main(argc, argv)
int argc;
int *argv[];
{
	int file;			/* source file no. */
	char *ifile;
	char *tag;
	int fpos[2];

	out = NULL;
	outlst = stdout;
	ifile = NULL;
	ofile = NULL;		/* default output file name */
	lfile = NULL;
	list = FALSE;			/* default no listing */
	bintype = BIN_PGM;
        entry = -1;
	outpgm = NULL;

	printf("UniAS HD6303/MC68HC11 Assembler v1.2\n");

	if (argc < 2)
		usage();

        symmore=symextra; symend2=symmore+3200;
        symcap=SYMSIZE+3200;
        if (symmore==0) {
          symcap=SYMSIZE;
          tag=52224;
          if (mgetw(tag)==22103 & mgetw(tag+2)==1) {
            symmore=mgetw(tag+4);
            symend2=symmore+mgetw(tag+6);
            symcap=symcap+mgetw(tag+6);
          }
        }
	endsym = symtab + SYMSIZE;
	inithash();
	prehash();

	--argc;
	argv++;
	while (argc > 0) {
		if (strcmp(argv[0], "-D") == SAME) {
			if (argc < 2)
				usage();
			else {
				tag = install(argv[1], 1);
				*(tag + NAME) = *(tag + NAME) | SYMBDEF;
				--argc;
				argv++;
			}
/*		} else 	if (strcmp(argv[0], "-U") == SAME) {
			if (argc < 2)
				usage();
			else {
				tag = install(argv[1], 0);
				*(tag + NAME) = *(tag + NAME) | SYMBDEF;
				--argc;
				argv++;
			} */
		} else if (strcmp(argv[0], "-m") == SAME) {
                        if (argc < 2) usage();
                        hc_mode(argv[1]);
                        --argc; argv++;
		} else if (strcmp(argv[0], "-c") == SAME) {
                        makeobj = 1;
                } else if (strcmp(argv[0], "-l") == SAME) {
			list++;
			if (argc > 3) {
				lfile = argv[1];
				if (*lfile != '-') {
					--argc;
					argv++;
				} else
					lfile = NULL;
			}
		} else if (strcmp(argv[0], "-o") == SAME) {
			if (argc < 2)
				usage();
			else {
				ofile = argv[1];
				--argc;
				argv++;
			}
		} else break;
		--argc;
		argv++;
	}

	if (argc == 0)
		usage();
        if (cpu_hc11) {
          tag=install("__HC11__",1);
          tag[NAME]=tag[NAME]|SYMBDEF;
        }

	ifile = argv[0];
        if (ofile) {
          tag = ofile + strlen(ofile);
          if (tag - ofile >= 4) {
            tag = tag - 4;
            if (strcmp(tag,".OBJ")==0 | strcmp(tag,".obj")==0)
              makeobj = 1;
          }
        }

	if (lfile) {
		if ((outlst = fopen(lfile, IO_W)) == NULL)
			printf("unias: Cannot open %s\n", lfile);
	}

	filepos = 0;
	chksumon = 0;
	errcnt = 0;
	warncnt = 0;
	pass = 1;
	while(pass < 3) {
		incnt = 0;
		ifdefcnt = 0;
                incifbase[0] = 0;
		ignotdef = 0;
		nsect = 0;
                sectkind = 0; bssused = 0; radix = 10;
                entryset = 0; entry = -1;
		nproc = 0;
		nset = 0;
		loc = 0;
		origin = 0;
		endflag = FALSE;
		liston = 1;
		truncon = 1;
		linecnt[incnt] = 0;
		linestot = 0;

		printf("--- Pass %d ---\n", pass);

		if ((in[incnt] = fopen(ifile, "r")) == NULL) {
			printf("unias: Cannot open %s\n", ifile);
			fatal(-1);
		}

                if (pass == 2) beginout(ifile);

		doasm();
                if (nsect) ends();
                while (incnt>0) fclose(in[incnt--]);
		fclose(in[0]); in[0]=0;
                if (pass == 1) {
                  codesize = loc;
                  rebasebss();
                }
                if (pass==2 & bintype!=BIN_CMD & filepos!=codesize)
                  error("Phase error: code size changed");
                pass++;
	}

	if (list)
		dumpsym();

	printf("\n%d Lines assembled\n", linestot);
	printf("%d Bytes code\n", filepos);
	printf("%d Bytes free of symbol table\n", symcap-symused);

	if ((chksumon != 0) & (errcnt == 0)) {
		fpos[0] = 0;
		fpos[1] = chkpos;
		if (fseek(out, fpos, 0) == 0)
			putc(0xFF ^ (csumval & 0xFF), out);
		else {
			printf("unias: Cannot set checksum position\n");
			fatal(-1);
		}
		fclose(out);
        } else if (outpgm) finishout();
        else fclose(out);

	if (outlst > 2)
		fclose(outlst);

	if (warncnt) {
		printf("unias: %d warnings\n", warncnt);
	}

	if (errcnt) {
		printf("unias: %d errors\n", errcnt);
		exit(-1);
	}
	exit(0);
}

addext(str1, str2, ext)
char *str1;
char *str2;
char *ext;
{
    strcpy(str1, str2);
    while (*str1) {
	if (*str1 == '.')
		*str1 = 0;
	else
	    str1++;
    }
    strcat(str1, ext);
}

/*
 * report correct usage and exit
 */
usage()
{
	printf("unias [-m hd6303|hc11] [-c] [-D ident]\n");
        printf("      [-l list] [-o output] file\n");
	fatal(-1);
}

/*
 * assemble one pass
 */
doasm()
{
	int mem;

	*ibuf = 0;
	while (1) {
		if (inline_() == NULL) {
		    if (ifdefcnt != incifbase[incnt]) {
				warning("Non paired condition directives.");
				ifdefcnt = incifbase[incnt];
		    }
		    if (incnt > 0) {
			fclose(in[incnt--]);
			continue;
		    } else
			break;
		}
		mem = loc;
		nbytes = 0;
		assem();
		if (pass == 2)
			output(mem);
		loc = loc + nbytes;
		if (endflag)
			break;
	}
	putchar(10);
}

/*
 * input line
 * ----------
 * ignore full line comments and empty lines
 */
inline_()
{
	int c;
	char *p;

	while ((p = fgets(ibuf, LINESIZE, in[incnt])) != NULL) {
		linecnt[incnt]++;
		linestot++;

		if (list == 0 |
		    (list != 0 & (outlst != stdout | pass == 1))) {
			printf("%d\r", linestot);
		}

		if (*p == ';') {
			if (pass == 1 | list == FALSE)
				continue;
		}
		break;
	}

	if (p == NULL)
		return NULL;

	c = strlen(ibuf);
	if (c > 0) {
		ip = p + c - 1;
		if (*ip != 10 & *ip != 13 & *ip != 0x1a) {
			if (c == LINESIZE - 1)
				warning("Line too long (%d).", c);
			else {
				warning("No new line.");
				*++ip = 10;
				*++ip = 0;
			}
		}
	}

	ip = p;

	/* force upper case */

	while (c = toupper(*p)) {
		if (c == ';')
			break;
		else if ((c == 13) | (c == 0x1a)) {
		    *p++ = 10;
		    *p++ = 0;
		    break;
		} else if (c == '"' | c == 0x27) {
			while (*++p) {
				if (*p == c)
					break;
			}
			p++;
		}
		else
			*p++ = c;
	}

	return(ip);
}

/*
 * assemble one line
 */
assem()
{
	int ismnem;

	flgrel = 0;
	flgunk = 0;

	ismnem = isspace(*ip);

	if (sym(sbuf)) {
		if (ismnem == 0) {
			qlabel();
			if (sym(sbuf)) {
				if (qmnem())
					return;
			} else
				return;
		} else if (qmnem())
			return;
	}

	if (*ip == 10)
		return;
	else if (match(';'))
		return;
	else
		error("symbol/pseudo-op required");
}

/*
 * prehash mnemonics into symbol table
 * -----------------------------------
 * check current directory first for data files
 */
prehash()
{
	char *p, *ps;
	int n;

	ps = memtabl;
	n = 0;
	while (*ps) {
		p = ps;
		while (*p++ != 10) /* '\n' */
			;
		*--p = 0; /* '\0' */
		installop(ps, opcodes + n++ * NMODES);
		ps = p + 1;
	}

	installop("ORG",	org   | 0x8000);
	installop("DS",		fillb | 0x8000);
	installop("DB",		byte  | 0x8000);
	installop("DW",		dbyte | 0x8000);
	installop("END",	psend | 0x8000);
	installop("INCLUDE",	file  | 0x8000);
	installop("SECTION",	sect  | 0x8000);
	installop("ENDS",	ends  | 0x8000);
	installop("PROC",	proc  | 0x8000);
	installop("ENDP",	endp  | 0x8000);
	installop("GLOBAL",	global| 0x8000);
	installop("CHECKSUM",	chksum| 0x8000);
	installop("ERROR",	chkerr| 0x8000);
	installop("LIST",	listop| 0x8000);
	installop("TRUNC",	trunop| 0x8000);
	installop(".IF",	dif   | 0x8000);
	installop(".IFDEF",	difdef| 0x8000);
	installop(".IFNDEF",	difndef|0x8000);
	installop(".ELSE",	delse | 0x8000);
	installop(".ENDIF",	dendif| 0x8000);
	installop(".DEFINE",	ddefine|0x8000);
	installop(".UNDEF",	dundef| 0x8000);
        installop("PUBLIC", public_ | 0x8000);
        installop("EXTERN", extern_ | 0x8000);
        installop("MAIN", main_ | 0x8000);
        installop("RADIX", radix_ | 0x8000);
        installop("CHKSUM", chksum | 0x8000);

	start = freeptr;
}

/*
 * check for labelled statement
 * ----------------------------
 * Exit status indicates whether
 * processing of line is complete
 */
qlabel()
{
	char *tag;

	tag = 0;

/* LABEL = XXX Always global - UniCROSS compatibility */
	if (match('='))
		ip--;
	else if (nproc) {
		if (pass == 1) {
			if (symorder(tag = hashfind(sbuf)) < procsym[nproc])
				tag = 0;
		}
		locsuffix(sbuf, nloc[nproc]);
	}

	if (ifdefcnt > 0) {
		if (ifdefstk[ifdefcnt])
			return;
	}


	if (pass == 1)
		return(putlab(tag));
	else
		return(getlab());
}

/*
 * put new label in symbol table
 * -----------------------------
 * 6502 Mnemonics are reserved symbols.
 * Exit status indicates whether
 * processing of line is complete
 */
putlab(gtag)
char *gtag;
{
	char *tag;
	int ret;
	char *tmp;

	tag = hashfind(sbuf);
        if (tag == NULL | ((tag[NAME] & SYMBFWD) != 0)) {
		if (match('=')) {
			tag = installsym();
			ret = TRUE;
		} else if (ip[0] == 'E' & ip[1] == 'Q' & ip[2] == 'U' &
		    (ip[3] == ' ' | ip[3] == 9)) {
			ip = ip + 3;
			tag = installsym();
			ret = TRUE;
		} else {
			if (tag == NULL) tag = install(sbuf,loc);
                        mputw(tag+VALUE,loc);
                        tag[NAME] = (tag[NAME] & SYMBPUB) | labelkind();
			ret = FALSE;
		}
		if (gtag) {
			mputw(gtag + VALUE, tag);
			*(gtag + NAME) = *(gtag + NAME) | SYMBLNK;
		}
		return ret;
	} else {
		error("label redefined");
		return(FALSE);
	}
}

/*
 * update labels on second pass
 * ----------------------------
 * Exit status indicates that
 * processing of line is complete
 */
getlab()
{
	char *tag;

	if (issym(tag = hashfind(sbuf))) {
		if (match('=')) {
			updatesym(tag);
			return(TRUE);
		}
		if (ip[0] == 'E' & ip[1] == 'Q' & ip[2] == 'U' &
		    (ip[3] == ' ' | ip[3] == 9)) {
			ip = ip + 3;
			updatesym(tag);
			return(TRUE);
		} else {
			mputw(tag + VALUE, loc);
			return(FALSE);
		}
	} else {
/*
		mnem(tag);
		return(TRUE);
 */
		printf("Internal error (missed label %s)\n", sbuf);
		fatal(-1);
	}
}

installsym()
{
 char *tag;
 int val, kind;
 val = exp_(); kind = typeflag();
 tag = hashfind(sbuf);
 if (tag == NULL) tag = install(sbuf,val);
 mputw(tag+VALUE,val);
 tag[NAME] = (tag[NAME] & SYMBPUB) | kind;
 return tag;
}

updatesym(tag)
char *tag;
{
 int val;
 val = exp_();
 mputw(tag+VALUE,val);
 tag[NAME] = (tag[NAME] & SYMBPUB) | typeflag();
}

/*
 * Check for mnemonic
 * ------------------
 * Exit status indicates whether
 * processing of line is complete
 */
qmnem()
{
	char *tag;

	if (ifdefcnt > 0) {
		if ((strcmp(sbuf, ".ELSE") != 0) &
		    (strcmp(sbuf, ".ENDIF") != 0) &
		    (strcmp(sbuf, ".IF") != 0) &
		    (strcmp(sbuf, ".IFDEF") != 0) &
		    (strcmp(sbuf, ".IFNDEF") != 0)) {
			if (ifdefstk[ifdefcnt])
				return;
		}
	}

        tag=findop(sbuf);
        if (tag) { mnem(tag); return TRUE; }
        return FALSE;
}

/*
 * process mnemonic
 */
mnem(tag)
char *tag;
{
	char *p;
	int mode;

	p = mgetw(tag);

	if (p & 0x8000) {
		nbytes = 0;
		p = p & 0x7FFF;
		#asm
		DB	20 ; PUSHR1
		DB	30 ; JSRSP
		#endasm
		return;
	}

	if (cpu_hc11) { hc_mnem(p); return; }
        if (p[OPFLAG]&2) {
          error("MC68HC11 instruction requires -m hc11"); return;
        }

	if ((obj[0] = p[INHERIT]) != ILLEGAL) {
		nbytes = 1;
		return;
	}
	else {
		if ((obj[0] = p[REL]) != ILLEGAL) {
			relative();
			return;
		}
		else {
			mode = getmode(p);
			if ((obj[0] = p[mode]) != ILLEGAL)
				return;
			else if (pass == 1)
				return;
			else
				error("Illegal address mode");
		}
	}
}

/*
 * get address mode of opcode
 */
getmode(p)
char *p;
{
	int bitop;
	int oper;

	bitop = 0;

	if (match('#')) {
		if (p[OPFLAG] & 0x80) {
			if (p[OPFLAG] & 0x60) {
				error("Bit number required.");
			}
			bitop = 1;
			immediate(p[OPFLAG]);
			if (match(',') == 0)
				return IMMED;
		} else
			return(immediate(p[OPFLAG]));
	} else if (p[OPFLAG] & 0x60) {
		oper = exp_();
		if (oper >= 0 & oper < 8) {
			if (p[OPFLAG] & 0x20) {
				oper = 0xFF ^ (1 << oper);
			} else
				oper = 1 << oper;
			obj[1] = oper & 0xFF;
		} else
			error("Illegal bit number.");
		nbytes = 2;
		bitop = 1;
		if (match(',') == 0)
			return IMMED;
	}

	skip();

	if (((p[OPFLAG] & 0x80) == 0x80) & (bitop == 0)) {
		if ((p[OPFLAG] & 0xE0) == 0x80)
			error("#Imm value missed.");
		else
			error("Bit number missed.");
		bitop = 1; /* forced for byte counter */
	}

	if (*ip == 'X' & isalnum(ip[1]) == 0) {
		ip++;
		if (match(',')) {
			oper = exp_();
			return(xindex(oper, bitop));
		}
		return(xindex(0, bitop));
	}

	oper = exp_();
	if (match(',')) {
		if (match('X'))
			return(xindex(oper, bitop));
		error("'X' expected");
		return(IND_X);
	} else if ((oper >= 0 & oper < BYTE) & (p[DIRECT] != ILLEGAL)) {
		/* direct jsr ($9D) second pass issues fix */
		if ((((p[DIRECT] & 0xFF) == 0x9D) &
			((oper < 0x28) | (bintype != BIN_CMD))) |
		    ((bintype != BIN_CMD) & (flgrel != 0) & (p[EXTEND] != ILLEGAL))) {
			return(extended(oper));
		}
		return(direct(oper, bitop));
	} else
		return(extended(oper));
}

/*
 * data immediately after opcode
 */
immediate(flag)
int flag;
{
	int val;
	val = exp_();
	if (flag & 1) {
		if (pass == 2 & bintype != BIN_CMD) {
			if (flgrel) emitref(loc+1,3);
		}
		nbytes = 3;
		obj[1] = val >> 8;
		obj[2] = val & 0xFF;
	} else {
		nbytes = 2;
                if (pass == 2 & flgrel!=0 & bintype != BIN_CMD)
                  emitref(loc+1,refpart+1-(refpart!=0));
                obj[1] = val & 0xFF;
	}
	return(IMMED);
}

/*
 * index register X
 */
xindex(oper, offs)
int oper;
int offs;
{
	nbytes = 2 + offs;
	obj[1 + offs] = oper & 0xFF;
	return(IND_X);
}

/*
 * direct (zero page) address
 */
direct(oper, offs)
int oper;
int offs;
{
	nbytes = 2 + offs;
	obj[1 + offs] = oper & 0xFF;
	return(DIRECT);
}

/*
 * 16-bit extended address
 */
extended(oper)
int oper;
{
	if (pass == 2 & bintype != BIN_CMD) {
		if (flgrel) emitref(loc+1,3);
	}
	nbytes = 3;
	obj[1] = oper >> 8;
	obj[2] = oper & 0xFF;
	return(EXTEND);
}

/*
 * program counter relative
 */
relative()
{
	int offset;

	nbytes = 2;
	offset = exp_() - loc - 2;
        if (flgrel == 3) error("External branch not allowed");
	if (offset < -128 | offset > 127) {
		if (pass == 1)
			offset = 0;
		else
			error("branch out of range");
	}
	obj[1] = offset;
	return(REL);
}

/*
 * set location counter
 */
org()
{
	if (makeobj | nsect) error("ORG not allowed here");
        bintype = BIN_CMD; /* relocation disabled = CMD */

	loc = exp_();
	if (nset++ == 0)
		origin = loc;
}

/*
 * fill (reserve) bytes
 */
fillb()
{
	char *oldloc;
	int count,fill;
	nbytes = 0;
	fill = 0;
	count = exp_();
	if (match(','))
	    fill = exp_() & 0xFF;

	oldloc = loc;
	loc = loc + count;

	if (loc < oldloc) error("Address overflow");
        if (nsect & fill!=0) error("Initialized data in SECTION");
        if (pass == 2 & nsect == 0) {
		while (oldloc++ < loc) {
			putc(fill, out);
			csumval = csumval + fill;
			filepos++;
		}
	}
}

/*
 * initialise memory byte
 */
byte()
{
	char delim;
	delim = 0;
	nbytes = 0;

	skip();
	while(nbytes < LINESIZE) {
		if (delim) {
			if (*ip == 0 | *ip == 10 | *ip == 13)
				break;
			if (*ip != delim) {
				obj[nbytes++] = *ip++;
				continue;
			}
			delim = 0;
			ip++;
		} else if (*ip == '"' | *ip == 0x27) {
		    delim = *ip++;
		    continue;
		} else {
			obj[nbytes] = exp_() & 255;
                        if (pass == 2 & flgrel!=0 & bintype != BIN_CMD)
                          emitref(loc+nbytes,refpart+1-(refpart!=0));
                        nbytes++;
		}
		if (match(',') == 0)
			break;
		skip();
	}
	if (delim)
		error("Expected close quote.");
}

/*
 * initialise memory word, high byte first
 */
dbyte()
{
	int word;

	nbytes = 0;
	while(nbytes < LINESIZE) {
		flgrel = 0;
		word = exp_();
		if (pass == 2 & bintype != BIN_CMD) {
			if (flgrel) emitref(loc+nbytes,3);
		}
		obj[nbytes++] = word >> 8;
		obj[nbytes++] = word & 0xFF;
		if (match(',') == 0)
			break;
	}
}

/*
 * switch input to new source file
 */
file()
{
	char *bp;

	if (incnt + 1 == INCFISIZE) {
	    printf("unias: Maximum nested include files reached.\n");
	    fatal(-1);
	}

	skip();
	bp = sbuf;
	while ((isspace(*ip) == 0) & (*ip != ';')) /* '\n' */
		*bp++ = *ip++;
	*bp = 0; /* '\0' */

	printf("\nInclude %s\n", sbuf);

	if ((in[++incnt] = fopen(sbuf, "r")) == NULL) {
		printf("unias: Cannot open %s\n", sbuf);
		fatal(-1);
	}
	incifbase[incnt] = ifdefcnt;
	linecnt[incnt] = 0;
}

psend()
{
 skip();
 if (*ip != 0 & *ip != 10 & *ip != 13 & *ip != ';') {
   entry = exp_(); entryset = 1;
   if (flgrel == 2 | flgrel == 3) error("Invalid entry point");
 }
 endflag = TRUE;
}

sect()
{
 int val, kind;
 if (nsect) { error("Nested SECTION"); return; }
 locsave = loc;
 skip(); kind = 2;
 if (*ip == 0 | *ip == 10 | *ip == 13 | *ip == ';') {
   val = bssused;
   if (pass == 2) val = val + codesize;
 } else {
   val = exp_();
   if (flgunk | flgrel == 3) error("Invalid SECTION address");
   if (flgrel == 0) kind = 0;
 }
 sectkind = kind; loc = val; nsect = 1;
}

ends()
{
 char *size;
 if (nsect == 0) { error("ENDS without SECTION."); return; }
 if (sectkind == 2) {
   size = loc;
   if (pass == 2) size = size - codesize;
   if (size > bssused) bssused = size;
 }
 loc = locsave; nsect = 0; sectkind = 0;
}

proc()
{
	if (nproc + 1 == PROCSIZE)
		error("Too much nested PROC.");
	else {
		nproc++;
		nloc[nproc] = loc;
		procsym[nproc] = symused;
	}
}

endp()
{
	if (nproc) {
		nproc--;
	} else
		error("ENDP without PROC.");
}

global()
{
	char buf[LINESIZE];
	char *tag, *ltag;
	int i;
	i = 0;
	if (nproc == 0)
		error("GLOBAL outside of PROC.");
	else if (pass == 1) {
		while(i < LINESIZE) {
			if (sym(buf)) {
                                tag = rawfind(buf);
                                if (tag) {
                                  if ((tag[NAME]&SYMBFWD)==0) {
                                    error("Redefinition of symbol");
                                    break;
                                  }
                                  tag[NAME]=tag[NAME]&SYMBPUB;
                                } else tag = install(buf, 0);

				locsuffix(buf, nloc[nproc]);

				if ((ltag = hashfind(buf)) != NULL) {
					mputw(tag + VALUE, ltag);
					*(tag + NAME) = *(tag + NAME) | SYMBLNK;
				}
			} else
				break;
			if (match(',') == 0)
				break;
		}
	}
}

chksum()
{
	if (pass == 1) {
		if (chksumon) {
			error("Improper use of CHECKSUM directive.");
			return;
		} else {
			chksumon = 1;
			csumval = 0;
			chkpos = 0;
		}
	} else {
		if (bintype != BIN_CMD)
			error("Improper use of CHECKSUM directive.");
	}
	chkpoint = loc;
	nbytes = 0;
	obj[nbytes++] = 0xFF;
}

chkerr()
{
	if (exp_())
		error("User error.");
}

listop()
{
	liston = chkonoff();
}

trunop()
{
	truncon = chkonoff();
}

chkonoff()
{
	if (sym(sbuf)) {
		if (strcmp(sbuf, "ON") == SAME)
			return 1;
		if (strcmp(sbuf, "OFF") == SAME)
			return 0;
	}
	error("Only ON or OFF allowed.");
}

ddefine()
{
	char *tag;
	if (pass == 1) {
		if (sym(sbuf)) {
			tag = install(sbuf, 1);
			*(tag + NAME) = *(tag + NAME) | SYMBDEF;
			return 0;
		}
		error("Symbol missed.");
	}
}

dundef()
{
	char *tag;
	if (pass == 1) {
		if (sym(sbuf)) {
			if (issym(tag = hashfind(sbuf))) {
				if (*(tag + NAME) & SYMBDEF) {
					mputw(tag + VALUE, 0);
					return 0;
				}
			}
			error("Symbol not defined.");
		}
		error("Symbol missed.");
	}
}

dif()
{
	char symbol[LINESIZE];
	int val;
	char *tag;
	char ifdefcmd;

	ignotdef = 1;
	val = exp_();
	ignotdef = 0;

	if ((ifdefcnt > 0) & (ifdefstk[ifdefcnt] > 0))
		ifdefcmd = 2;
	else if (val == 0)
		ifdefcmd = 1;
	else
		ifdefcmd = 0;
	ifdefstk[++ifdefcnt] = ifdefcmd;
}


difdef()
{
	char symbol[LINESIZE];
	int ifdefval;
	char *tag;
	char ifdefcmd;

	ifdefval = 0;

	if (sym(symbol)) {
		tag = hashfind(symbol);
		if (tag) {
			if (*(tag + NAME) & SYMBDEF)
				ifdefval = mgetw(tag + VALUE);
		}
	}
	if ((ifdefcnt > 0) & (ifdefstk[ifdefcnt] > 0))
		ifdefcmd = 2;
	else if (ifdefval == 0)
		ifdefcmd = 1;
	else
		ifdefcmd = 0;
	ifdefstk[++ifdefcnt] = ifdefcmd;
}

difndef()
{
	char symbol[LINESIZE];
	int ifdefval;
	char *tag;
	char ifdefcmd;

	ifdefval = 0;

	if (sym(symbol)) {
		tag = hashfind(symbol);
		if (tag) {
			if (*(tag + NAME) & SYMBDEF)
				ifdefval = mgetw(tag + VALUE);
		}
	}
	if ((ifdefcnt > 0) & (ifdefstk[ifdefcnt] > 0))
		ifdefcmd = 2;
	else if (ifdefval != 0)
		ifdefcmd = 1;
	else
		ifdefcmd = 0;
	ifdefstk[++ifdefcnt] = ifdefcmd;
}

delse()
{
	if (ifdefstk[ifdefcnt] < 2) {
		if (ifdefstk[ifdefcnt] == 1)
			ifdefstk[ifdefcnt] = 0;
		else
			ifdefstk[ifdefcnt] = 1;
	}
}

dendif()
{
	ifdefcnt--;
}

locsuffix(str, cnt)
char *str;
int cnt;
{
	char *ptr;
	strcpy(str + 1, str);
	*str = ':';
	ptr = str + strlen(str);
	while (cnt != 0) {
            /* The location suffix is an unsigned 16-bit value. */
            *ptr++ = 32 + (((cnt >> 1) & 0x7fff) % 48) * 2 + (cnt & 1);
            cnt = ((cnt >> 1) & 0x7fff) / 48;
	}
	*ptr = 0;
}

/*
 * evaluate expression
 * -------------------
 * '<' returns low byte of expression
 * '>' returns high byte
 */
exp_()
{
 int v, hi;
 refpart=0; hi=match('/');
 v=exp2(); refvalue=v;
 if (hi) { refpart=2; return (v >> 8) & 255; }
 return v;
}

/*
 * evaluate infix expression
 * -------------------------
 * operator precedence is left to right
 */

exp2()
{
 int n, t, e, r, op;
 n = exp3();
 while (*ip) {
  t=flgrel; e=flgext; op=0;
  if (match('|')) op='|';
  if (op == 0) break;
  r=exp3();
  n=exprbin(n,t,e,r,op);
 }
 return n;
}

exp3()
{
 int n, t, e, r, op;
 n = exp4();
 while (*ip) {
  t=flgrel; e=flgext; op=0;
  if (match('^')) op='^';
  if (op == 0) break;
  r=exp4();
  n=exprbin(n,t,e,r,op);
 }
 return n;
}

exp4()
{
 int n, t, e, r, op;
 n = exp5();
 while (*ip) {
  t=flgrel; e=flgext; op=0;
  if (match('&')) op='&';
  if (op == 0) break;
  r=exp5();
  n=exprbin(n,t,e,r,op);
 }
 return n;
}

exp5()
{
 int n, t, e, r, op;
 n = exp6();
 while (*ip) {
  t=flgrel; e=flgext; op=0;
  if (match('+')) op='+';
  else if (match('-')) op='-';
  if (op == 0) break;
  r=exp6();
  n=exprbin(n,t,e,r,op);
 }
 return n;
}

exp6()
{
 int n, t, e, r, op;
 n = exp7();
 while (*ip) {
  t=flgrel; e=flgext; op=0;
  if (match('*')) op='*';
  else if (match('/')) op='/';
  else if (match('%')) op='%';
  if (op == 0) break;
  r=exp7();
  n=exprbin(n,t,e,r,op);
 }
 return n;
}

exp7()
{
 int v, op;
 op=0;
 if (match('~')) op=1;
 else if (match('-')) op=2;
 v=exp8();
 if (op) {
  if (flgrel) error("Illegal relative expression");
  if (op==1) return 65535 ^ v;
  return -v;
 }
 return v;
}

exp8()
{
    int n;
    if (match('(')) {
	n = exp2();
	if (match(')'))
	    return n;
	else {
	    error("missing bracket");
	    return ILLEGAL;
	}
    }
    return operand();
}

/*
 * return arithmetic operand
 */
operand()
{
	char symbol[LINESIZE];
        flgrel=0; flgext=0;

	if (sym(symbol))
		return(lookup(symbol));
	else if (match('$'))
		return(hexnum());
	else if (match('@'))
		return(octal());
	else if (match('%'))
		return(binary());
	else if (match(0x27)) /* '\'' */
		return(character());
	else if (match('*')) {
		flgrel=1;
                if (nsect) flgrel=sectkind;
                else if (bintype==BIN_CMD) flgrel=0;
		return(loc);
	} else if (isdigit(*ip))
		return(decimal());
	else {
		error("illegal expression");
		return(ILLEGAL);
	}
}

/*
 * look up name in symbol table
 */
lookup(name)
char *name;
{
	char buf[LINESIZE];
	char *tag;
	int n;

	tag = 0;

	if (nproc) {
		n = nproc;
		while (n > 0 & tag == 0) {
			strcpy(buf, name);
			locsuffix(buf, nloc[n--]);
			tag = hashfind(buf);
		}
	}

	if (tag == 0)
		tag = hashfind(name);

	if (pass == 2) {
		if (tag == 0 | (tag[NAME] & SYMBFWD)) {
			if (ignotdef)
			    return 0;
			printf("[%s] ", name);
			error("symbol undefined");
		} else if (issym(tag)==0)
			error("illegal symbol");
	}
        if (tag) {
          if (tag[NAME]&SYMBFWD) {
            flgrel=1; flgunk=1; return 0xEAEA;
          }
        }
	if (tag == 0) {
                flgrel=1;
                if (bintype==BIN_CMD) flgrel=0;
		flgunk = 1;
		return(0xEAEA);
	} else {
		if (*(tag + NAME) & SYMBCAL)
			flgunk = 1;
		else if (tag[NAME] & SYMBEXT) {
                  flgrel=3; flgext=mgetw(tag+VALUE); return 0;
                } else if (tag[NAME] & SYMBLBL) {
                  flgrel=1;
                  if (tag[NAME] & SYMBBSS) flgrel=2;
                }
		return(mgetw(tag + VALUE));
	}
}

/*
 * hexadecimal constant $n
 */
hexnum()
{
	int n;

	if (isxdigit(*ip) == 0)
		return(ERROR);
	else {
		n = 0;
		while (isxdigit(*ip))
			n = n * 16 + toint(*ip++);
		return(n);
	}
}

/*
 * convert char to 'weight' of hex digit
 */
toint(c)
char c;
{
	if (isdigit(c))
		return(c - '0');
	else if (isxdigit(c)) {
		if (isupper(c))
			return(c - 'A' + 10);
		if (islower(c))
			return(c - 'a' + 10);
	}
	else
		return(ERROR);
}

/*
 * decimal constant n
 */
decimal()
{
 int n,c,d;
 n=0;
 while (isxdigit(*ip)) {
  c=toupper(*ip++);
  if (c>='0' & c<='9') d=c-'0';
  else d=c-'A'+10;
  if (d>=radix) error("Digit outside RADIX");
  n=n*radix+d;
 }
 return n;
}

/*
 * octal constant @n
 */
octal()
{
	int n;

	if (*ip < '0' | *ip > '7')
		return(ERROR);
	else {
		n = 0;
		while (*ip >= '0' & *ip <= '7')
			n = n * 8 + *ip++ - '0';
		return(n);
	}
}

/*
 * binary constant %n
 */
binary()
{
	int n;

	if (*ip != '0' & *ip != '1')
		return(ERROR);
	else {
		n = 0;
		while (*ip == '0' | *ip == '1' | *ip == '_') {
			if (*ip == '_') {
				ip++;
			} else
				n = n * 2 + *ip++ - '0';
		}
		return(n);
	}
}

/*
 * character constant 'c'
 */
character()
{
	char c;

	c = *ip++;
	if (*ip == 0x27) /* '\'' */
		ip++;			/* discard optional trailing ' */
	return(c);
}

/*
 * get next symbol
 * ---------------
 * copy alpha prefixed alphanumeric string from input buffer
 * accepts underline, tilde and point as alpha prefix
 * returns length of string
 */
sym(p)
char *p;
{
	char *bp;

	skip();
	bp = p;

	if (isalpha(*ip) | *ip == '_' | *ip == ':' | *ip == '.') {
		*bp++ = *ip++;
		while (isalnum(*ip) | *ip == '_')
			*bp++ = *ip++;
		*bp = 0; /* '\0' */
	}
	return(bp - p);
}

/*
 * skip white space on input
 */
skip()
{
	while (*ip == ' ' | *ip == 9)
		ip++;
}

/*
 * match literal with input buffer
 */
match(c)
char c;
{
	skip();

	if (*ip != c) return(FALSE);

	ip++;
	return(TRUE);
}

outrel(mem)
int mem;
{
	relsize++;
	outword(mem);
}

outword(mem)
int mem;
{
	putc(mem >> 8, outpgm);
	putc(mem & 0xFF, outpgm);
}

/*
 * output assembly listing and object code
 */
output(mem)
int mem;
{
	int n, byte;
	char incl;
	char *ptr;

	if (incnt > 0) incl = '>';
	else incl = ' ';

	if (list & liston) {
		n = 0;
		while (n < nbytes) {
			if (truncon == 0 | ((truncon == 1) & (n < 3))) {
				fprintf(outlst, "%c %04x  ", incl, mem + n);
				byte = 0;
				while (byte < 3) {
					if (n < nbytes)
						fprintf(outlst, "%02x ", obj[n++] & 0xFF);
					else
						fprintf(outlst, "   ");
					byte++;
				}
				if (n < 4)
					fprintf(outlst, " %s", ibuf);
				else
					fprintf(outlst, "\n");
				/* if (n >= nbytes) break; */
			} else
				break;
		}
		if (nbytes == 0) {
			fprintf(outlst, "%c %04x            %s", incl, mem, ibuf);
		}
	}
	if (nsect == 0) {
		ptr = mem;
		n = 0;
		while (n < nbytes) {
			if (ptr++ == chkpoint)
				chkpos = filepos;
			filepos++;
			csumval = csumval + (obj[n] & 0xFF);
			putc(obj[n++], out);
		}
	}
}

/*
 * print warning message
 * -------------------
 * use stdout, so warning appear in listing
 */
warning(message)
char *message;
{
	char *p;

	p = ibuf;
	while (*p) {
	    if (*p == 10) {
		*p = 0;
		break;
	    }
	    p++;
	}

	warning1(stdout, message);
	if (outlst != stdout)
		warning1(outlst, message);
}

warning1(fd, message)
int fd;
char *message;
{
	fprintf(fd, "Warning: %s\n", message);
	fprintf(fd, "Line %d: %s\n", linecnt[incnt], ibuf);
}

/*
 * print error message
 * -------------------
 * use stdout, so errors appear in listing
 */
error(message)
char *message;
{
	char *p;

	p = ibuf;
	while (*p) {
	    if (*p == 10) {
		*p = 0;
		break;
	    }
	    p++;
	}

	error1(stdout, message);
	if (outlst != stdout)
		error1(outlst, message);

	if (++errcnt > 5) {
		error2(stdout);
		if (outlst != stdout)
			error2(outlst);
		fatal(-1);
	}
}

error1(fd, message)
int fd;
char *message;
{
	fprintf(fd, "Error: %s\n", message);
	fprintf(fd, "Line %d: %s\n", linecnt[incnt], ibuf);
}

error2(fd)
int fd;
{
	fprintf(fd, "unias: too many errors, assembly aborted\n");
}

/*
 * tidy up files and exit on fatal error
 */
fatal(stat)
int stat;				/* exit status */
{
	while (incnt > 0) {
	    fclose(in[incnt--]);
	}

	if (in[0]) fclose(in[0]);

	if (out != NULL)
		fclose(out);
	if (outpgm != NULL)
		fclose(outpgm);
	if (outlst > 2)
		fclose(outlst);
	exit(stat);
}

/*
 * initialise empty hash table
 */
inithash()
{
	int i;

	freeptr = symtab;
	i = 0;
	while (i < HASHSIZE)
		hashtab[i++] = NULL;
}

/*
 * hashing algorithm
 * -----------------
 * returns value in range 0 to HASHSIZE - 1
 * for best results HASHSIZE should be a prime
 */
hash(name)
char *name;
{
	int h;

	h = 0;
	while (*name)
		h = (3 * h + *name++) % HASHSIZE;
	return(h);
}

installop(name,val)
char *name;
int val;
{
 int i;
 if (opcount==224) fatal(-1);
 i=opcount++;
 while (i>0) {
  if (strcmp(opnames[i-1],name)<=0) break;
  opnames[i]=opnames[i-1]; opvals[i]=opvals[i-1]; i--;
 }
 opnames[i]=name; opvals[i]=val;
}

/*
 * install new symbol
 */
install(name,val)
char *name;
int val;
{
 int len,h,k,i,j,stem;
 char *p,*q;
 stem=0; k=-1;
 while (name[stem]!=0 & name[stem]!='_' & stem<15) stem++;
 if (name[stem]=='_' & stem>1 & stem<15) {
  stem++;
  i=0;
  while (i<precnt & k<0) {
   q=pretbl[i]; j=0;
   while (j<stem & q[j]==name[j]) j++;
   if (j==stem & q[j]==0) k=i;
   i++;
  }
  if (k<0 & precnt<32 & preused+stem+1<192) {
   k=precnt++; q=prepool+preused; pretbl[k]=q;
   i=0; while (i<stem) { q[i]=name[i]; i++; }
   q[i]=0; preused=preused+stem+1;
  }
 }
 len=strlen(name)+6;
 if (k>=0) len=len-stem+1;
 if (freeptr+len>endsym) {
  if (symsplit==0 & symmore!=0) {
   symsplit=freeptr; symused=symused+endsym-freeptr;
   freeptr=symmore; endsym=symend2;
  }
  if (freeptr+len>endsym) {
   printf("symbol table full\n"); fatal(-1);
  }
 }
 h=hash(name); p=freeptr;
 mputw(p+NEXTPTR,hashtab[h]); hashtab[h]=p;
 mputw(p+VALUE,val); p[NAME]=0;
 if (k>=0) {
  p[NAME+1]=k+1; strcpy(p+NAME+2,name+stem);
 } else strcpy(p+NAME+1,name);
 freeptr=p+len; symused=symused+len; return p;
}

/*
 * find symbol using hash + chain
 */
hashfind(name)
char *name;
{
	char *tag;

	tag = hashtab[hash(name)];
	while (tag) {
		if (tagcmp(tag,name) == SAME) {
			if (*(tag + NAME) & SYMBLNK)
				tag = mgetw(tag + VALUE);
			break;
		} else
			tag = mgetw(tag + NEXTPTR);
	}
	return(tag);
}

/*
 * put word into memory
 */
mputw(p, val)
int *p;
int val;
{
	*p = val;
}

/*
 * get word from memory
 */
mgetw(p)
int *p;
{
	return(*p);
}

/*
 * dump symbol table
 * -----------------
 * symbols prefixed by tilde are local, and
 * are not written to the global symbol file
 */
dumpsym()
{
 char *p,*q;
 p=start;
 while (p!=freeptr) {
  tagname(p);
  if (*sbuf!=':') {
   q=p;
   if (p[NAME]&SYMBLNK) q=mgetw(p+VALUE);
   fprintf(outlst,"%s =$%04x\n",sbuf,mgetw(q+VALUE));
  }
  p=nextsym(p);
 }
}

/*
 *
 *
 *
 *
 *
 *
 */

isxdigit(c) char c;
{
	return (isdigit(c) | ((c >= 'A') & (c <= 'F')) | ((c >= 'a') & (c <= 'f')));
}

/*
 * utoi -- convert unsigned decimal string to integer nbr
 *	        returns field size, else ERR on error
 */


/*
 * itod -- convert nbr to signed decimal string of width sz
 *	       right adjusted, blank filled ; returns str
 *
 *	      if sz > 0 terminate with null byte
 *	      if sz  =  0 find end of string
 *	      if sz < 0 use last byte for data
 */



/*
 * itou -- convert nbr to unsigned decimal string of width sz
 *	       right adjusted, blank filled ; returns str
 *
 *	      if sz > 0 terminate with null byte
 *	      if sz  =  0 find end of string
 *	      if sz < 0 use last byte for data
 */



/*
 * itox -- converts nbr to hex string of length sz
 *	       right adjusted and blank filled, returns str
 *
 *	      if sz > 0 terminate with null byte
 *	      if sz  =  0 find end of string
 *	      if sz < 0 use last byte for data
 */


/*
** printf(controlstring, arg, arg, ...) -- formatted print
**        operates as described by Kernighan & Ritchie
**        only d, x, c, s, and u specs are supported.
*/
printf(argc)
int argc;
{
	int *nxtarg;
	int i;
#asm       /* fetch arg count from primary reg first */
	DB	22	;EXG1
#endasm
	nxtarg = &argc + (i - (1 << 1));
	return _printf(stdout, nxtarg);
}

fprintf(argc)
int argc;
{
	int *nxtarg;
	int fd;
	int i;
#asm       /* fetch arg count from primary reg first */
	DB	22	;EXG1
#endasm
	nxtarg = &argc + (i - (1 << 1));
	fd = *nxtarg--;
	return _printf(fd, nxtarg);
}

_printf(fd,args)
int fd,*args;
{
 char *ctl,*p,c,pad;
 int w,num;
 ctl=*args;
 while (c=*ctl++) {
  if (c==92) {
   c=*ctl++;
   if (c=='n') c=10;
   if (c=='r') c=13;
   if (c=='t') c=9;
   cout(c,fd);
  } else if (c!='%') cout(c,fd);
  else if (*ctl=='%') { ctl++; cout('%',fd); }
  else {
   w=0; pad=' ';
   if (*ctl=='0') pad='0';
   while (*ctl>='0' & *ctl<='9') w=w*10+*ctl++-'0';
   c=*ctl++; num=*--args;
   if (c=='s') { p=num; while (*p) cout(*p++,fd); }
   else if (c=='c') cout(num,fd);
   else {
    if (c=='d' & num<0) { cout('-',fd); num=-num; w--; }
    fmtnum(fd,num,c,w,pad);
   }
  }
 }
 return 0;
}

cout(c, fd) char c; int fd; {
/*  if(putc(c, fd)==EOF) xout(); */
	if (fd < 3)
		putchar(c);
	else
		putc(c, fd);
}

#include HC11.C
#include OPCODES.ASM

/* Assembler expressions are unsigned 16-bit words, independently of C int. */
udiv16(a, b, rem)
int a, b, rem;
{
 int q, r, bits, carry;
 if (b == 0) { error("division by zero"); return 0; }
 q=0; r=0; bits=16;
 while (bits) {
  carry=r&0x8000;
  r=(r<<1)|((a>>15)&1);
  a=a<<1; q=q<<1;
  if (carry | ((r^0x8000)>=(b^0x8000))) { r=r-b; q=q|1; }
  bits=bits-1;
 }
 if (rem) return r;
 return q;
}

#include OBJIO.C

fmtnum(fd,num,fmt,w,pad)
int fd,num,fmt,w,pad;
{
 char b[7];
 int n,d,lo;
 n=0;
 while (1) {
  if (fmt=='x') { d=num&15; num=(num>>4)&4095; }
  else {
   lo=num&1; num=(num>>1)&32767;
   d=(num%5)*2+lo; num=num/5;
  }
  if (d<10) b[n++]=d+'0';
  else b[n++]=d+'a'-10;
  if (num==0) break;
 }
 while (w-->n) cout(pad,fd);
 while (n) cout(b[--n],fd);
}

tagcmp(p,name)
char *p,*name;
{
 char *q;
 int c;
 p=p+NAME+1; c=*p;
 if (c>0 & c<=32) {
  q=pretbl[c-1]; p++;
  while (*q) { if (*q++!=*name++) return 1; }
 }
 return strcmp(p,name);
}

tagname(p)
char *p;
{
 int c;
 p=p+NAME+1; c=*p;
 *sbuf=0;
 if (c>0 & c<=32) { strcpy(sbuf,pretbl[c-1]); p++; }
 strcat(sbuf,p);
}

findop(name)
char *name;
{
 int lo,hi,mid,cmp;
 lo=0; hi=opcount-1;
 while (lo<=hi) {
  mid=(lo+hi)/2; cmp=strcmp(name,opnames[mid]);
  if (cmp==0) return &opvals[mid];
  if (cmp<0) hi=mid-1;
  else lo=mid+1;
 }
 return 0;
}

/* The second pool may be below the first (host) or in the VM bank. */
issym(p)
char *p;
{
 if (p>=start & p<symtab+SYMSIZE) return 1;
 return symsplit!=0 & p>=symmore & p<freeptr;
}
nextsym(p)
char *p;
{
 p=p+strlen(p+NAME+1)+6;
 if (p==symsplit) p=symmore;
 return p;
}
symorder(p)
char *p;
{
 if (p==0) return -1;
 if (symsplit!=0 & p>=symmore & p<symend2)
  return SYMSIZE+p-symmore;
 return p-symtab;
}

#asm
    .ifdef PYLDIN601
symextra EQU 0
    .else
symextra DS 3200
    .endif
#endasm
