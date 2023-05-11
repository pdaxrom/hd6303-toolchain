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
unias [-D IDENT] [-l [prog.lst]] [-o prog[.cmd|.pgm]] prog.asm
```

**Where:**
```
-D IDENT Specify an identifier to use with the .ifdef and .ifndef directives.

-l [prog.lst] - listing output (if no file is specified, it is displayed on the screen).

-o prog[.cmd|.pgm] - output file name, extensions are created automatically, depending on compilation mode.
```

Differences between UniAS and [UniCROSS](https://pyldin.info/document/unicross_rus.htm):
- Unimplemented object files generation
- Directives for defining symbols EXTERN, PUBLIC are not supported
- RADIX translation control directive not supported

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
