# hd6303-toolchain

Native UniDOS and cross compilers for assembler and Small-C for CPU HD6303.

Assembler is based on [MOS Technology 6502 assembler for BBC Micro](http://mdfs.net/System/C/BBC/Small-C/v073/source/as65.c) by A.J.Travis and J.G.Harston

Small-C is based on [Ron Cain's Small-C V1.1](https://en.wikipedia.org/wiki/Small-C) adapted for Flex by S. Stepanoff

## UniAS HD6303 Assembler for UniDOS

The original assemblers do not support HD6303 extended commands. In addition, they cannot assemble the source 
texts of system programs without cross-compiling, due to lack of memory. In this regard, it was decided to write
a new assembler that is compatible with [UniCROSS](https://pyldin.info/document/unicross_rus.htm), but works both in cross compilation mode and native mode in UniDOS. 

**Usage:**
```
unias [-c] [-D IDENT] [-l [prog.lst]] [-o prog[.cmd|.pgm|.obj]] prog.asm
```

**Where:**
```
-D IDENT Specify an identifier to use with the .ifdef and .ifndef directives.

-l [prog.lst] - listing output (if no file is specified, it is displayed on the screen).

-o prog[.cmd|.pgm] - output file name, extensions are created automatically, depending on compilation mode.
```

UniAS writes ready-to-load PGM files, including the BSS size and entry point.
`SECTION` without an address starts relative, uninitialized BSS; `ENDS`
returns to code. An open section is closed at end of source. Absolute
`SECTION $E0` workspace keeps its old meaning. No host BSS trimming is needed.
`END label` or `MAIN` selects the entry point. ORG selects absolute CMD/ROM.

`-c`, a .OBJ output name, PUBLIC or EXTERN selects UniCROSS OBJ version 1.
The original 32-byte header, big-endian words, public/external tables and
LSB/MSB/word relocation records are retained; the BSS length is at offset 16.
`RADIX`, `MAIN`, `PUBLIC`, `EXTERN` and the `CHKSUM` alias are supported.
External symbols must be bare operands, as in UniCROSS; external arithmetic,
external EQU aliases and relative branches to external labels are rejected.
Use a separately exported field label for an external structure offset.
Private symbols may be longer, but exported/imported names are at most 15
characters. Relative BSS and code expressions retain their relocation type.

User manual for [UniCROSS](https://pyldin.info/document/unicross_rus.htm)

## UniCC Small C compiler with VM for UniDOS

Small-C is a subset of the C language and is optimized for use on systems with limited resources. It is used to build the UniAS assembler and itself.

**Usage:**
```
unicc [-ctext] [-errstop] [-o outputfile] inputfile
```
**Where:**
```
-ctext - include program lines in assembler listing comments

-errstop - stop compilation on error

-o outputfile - output file name
```

**Usage example**

Using a text editor, create a hello.c program with the following content:
```
main()
{
   puts("Hello, world!");
}
```

Compile the program:
```
unicc hello.c
```

Assemble the resulting assembler file into binary:
```
unias hello.asm
```
Run the compiled program:
```
hello
```

## Host VM and Pyldin FPGA target

The host VM uses 16-bit two's-complement integers. Division truncates toward
zero, remainder has the dividend's sign, and right shift is arithmetic.
Shift counts use the low five bits; left shifts of 16..31 yield zero.
`-32768 / -1` wraps to -32768. Division by zero and unsupported native inline
code fail explicitly. Character helpers follow the ROM's ASCII/identifier ABI.
The bootstrap compiler seed is rebuilt from SMALL-C/SMALLCX.C with the
-32768 decimal-output fix. UniAS expression division/remainder remain unsigned
16-bit operations, independently of the C VM's signed operations.

```
make
make test
make pyldin
```

`make pyldin` creates two 8192-byte ROM modules, ZVM6303.ROM and ZVM6800.ROM,
plus native UNICC.CMD, UNIAS.CMD and SYSTEM.H in `pyldin/`. These use INT EF,
ABI 1: A=0 returns A=1, B=0/1 (MC6800/HD6303), X=5A56; A=1 executes the
bytecode at X and returns through the normal ramloader SWI gateway. The
Lilith/default SYSTEM.H and ZCODE.ROM retain INT E8. FPGA startup is selected
by `-D PYLDIN601`; the distributed pyldin/SYSTEM.H enables it by default.

Copy both ROMs and RAMLOAD.PGM from pyldin601-fpga to one directory. Run
`RAMLOAD /CVM` from the main UniDOS shell before NC. It reads the FPGA CPU
selection and installs only the matching ROM in a pinned RAMMAN bank.
No FPGA rebuild or additional fixed ROM page is required.

C bytecode and the C104..C18C library entry table are shared between CPUs.
The MC6800 variant uses no HD6303 extensions. VM state, original direct page
E0..FF, BREAK and critical-error vectors are restored after each invocation,
including nested calls. A module cannot be unloaded while it is executing.
Normal exit, division by zero, malformed bytecode and BREAK return through
the gateway. Native inline assembly still must match the selected CPU.

Compile in a directory containing the generated SYSTEM.H:

```
UNICC HELLO.C
UNIAS HELLO.ASM
HELLO
```

This remains Small-C, not a full ISO C implementation: source lines are at
most 79 characters, string literals do not process C backslash escapes, and
integer/character sizes and the existing library ABI are unchanged. Native
UniDOS also rejects launching a second absolute CMD while one is running;
`system()` can execute internal shell commands and relocatable PGM programs.

## UniLD linker and UniAR librarian

Both programs are written in this UniCC dialect and run as native CMD files
on either VM. The host wrappers execute the same C bytecode. No host linker
or PGM postprocessor is used.

```
unias -c -o MAIN.OBJ MAIN.ASM
unias -c -o HELP.OBJ HELP.ASM
unild [-o APP.PGM] MAIN.OBJ HELP.OBJ
uniar r UTIL.LIB HELP.OBJ
unild -o APP.PGM MAIN.OBJ UTIL.LIB
uniar t UTIL.LIB
uniar x UTIL.LIB HELP
uniar d UTIL.LIB HELP
```

UniLD accepts OBJ versions 0 and 1, original UniLIB containers, extensionless
names (OBJ then LIB), `/c` for CMD at $0100, `/p` for module page alignment,
combined `/cp`, and `@list` response files. The root module must have an entry
point. Legacy v0 objects have no reliable no-entry marker: the first plain
v0 OBJ supplies the entry, later v0 modules are treated as helpers.
Code and BSS segments are selected through actual relocation dependencies;
unused library code is omitted, including unreferenced EXTERN declarations.
Code streams and relocation tables stay on disk. Limits are 32 input files,
256 modules, 512 public definitions, 255 imports/exports per OBJ, 31-character
paths, and 65535 bytes per OBJ or combined program memory. Library file
positions are 32-bit, so a LIB can exceed 64 KiB. CMD does not allocate BSS;
the application must check and reserve its own data memory.

UniAR uses the original 0BB0 UniLIB format, preserving each member OBJ
byte-for-byte. Operations are `r` create/replace, `a` append without replacing,
`d` delete, `t` list and `x` extract. Member names are uppercase, eight
characters, padded with spaces; they are not Unix ar names. Up to 256
members and `@list` input are supported. Linker output, archive updates and
extracted members are published through temporary files; a failed operation
preserves the old destination.

`make all pyldin test` builds and tests all four tools. Native outputs are
`pyldin/UNIAS.CMD`, `UNICC.CMD`, `UNILD.CMD` and `UNIAR.CMD`. Expand the UniDOS
handle table with `HANDLES 16` before `RAMLOAD /CVM` for large assemblies.
The assembler uses a bounded symbol pool and a small shared-prefix dictionary;
XONIX fits without occupying a second RAMMAN bank.

`tests/test_objects.py` checks PGM/BSS, byte and word relocations, CMD/page
alignment, legacy UniCROSS and UniLIB fixtures, archive replacement/deletion/
extraction, a library larger than 64 KiB, and invalid/truncated inputs.
The pyldin601-fpga suite additionally executes the original native ULINK and
compares native UniCC/UniAS/UniLD/UniAR outputs with host outputs.
The historical ULINK binary has a defect resolving absolute PUBLIC values;
UniLD retains their full 16-bit value. Avoid that historical defect when
comparing old ULINK outputs. Native tests use fresh ULINK output names.

The reference ULINK also underallocates PGM relocation space for a library
member with internal relocations (reproduced with original UniLIB output).
Its PGM outputs with byte-only relocations may contain harmless padding
between the relocation table and code. Tests compare loaded code, relocation
entries, entry point and BSS in that case. The separate no-internal-relocation
library case has identical loaded contents; UniLD correctly links both kinds.
