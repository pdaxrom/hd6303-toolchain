VM = ./scvm

.DELETE_ON_ERROR:

all: unicc unias unild uniar UNICC.CMD UNIAS.CMD UNILD.CMD UNIAR.CMD

CFLAGS = -O2

scvm: scvm.o
	$(CC) -o $@ $^

SMALLCX.CMD: scvm SMALL-C/SMALLCX.C
	$(VM) BOOTSTRAP/SMALLC.CMD -o SMALLCX.ASM SMALL-C/SMALLCX.C
	$(VM) BOOTSTRAP/AS03.CMD -o SMALLCX.CMD SMALLCX.ASM

AS03X.CMD: scvm AS03/AS03.C AS03/OBJIO.C SMALLCX.CMD
	cd AS03 && ../$(VM) ../SMALLCX.CMD -o ../AS03X.ASM AS03.C
	$(VM) BOOTSTRAP/AS03.CMD -o AS03X.CMD AS03X.ASM

SMALLC.CMD: scvm SMALL-C/SMALLCX.C SMALLCX.CMD AS03X.CMD
	$(VM) SMALLCX.CMD -o SMALLC.ASM SMALL-C/SMALLCX.C
	$(VM) AS03X.CMD -o SMALLC.CMD SMALLC.ASM

AS03.CMD: scvm AS03/AS03.C AS03/OBJIO.C AS03X.CMD SMALLC.CMD
	cd AS03 && ../$(VM) ../SMALLC.CMD -o ../AS03.ASM AS03.C
	$(VM) AS03X.CMD -o AS03.CMD AS03.ASM

unicc: scvm.c unicc.h
	$(CC) -o $@ scvm.c -DAPP=\"unicc.h\" -Wall $(CFLAGS)

unicc.h: SMALLC.CMD
	hexdump -v -e '"\t" 16/1 "0x%02x, " "\n"' $^ | sed 's/0x  ,//g' > $@

unias: scvm.c unias.h
	$(CC) -o $@ scvm.c -DAPP=\"unias.h\" -Wall $(CFLAGS)

unias.h: AS03.CMD
	hexdump -v -e '"\t" 16/1 "0x%02x, " "\n"' $^ | sed 's/0x  ,//g' > $@

UNIAS.CMD: AS03.CMD ZCODE.ROM
	cp -f AS03.CMD UNIAS.CMD

UNICC.CMD: SMALLC.CMD ZCODE.ROM
	$(VM) SMALLC.CMD -o UNICC.ASM SMALL-C/SMALLC.C
	$(VM) AS03.CMD UNICC.ASM

ZCODE.ROM: AS03.CMD ZCODE.ASM
	$(VM) AS03.CMD -l ZCODE.LST -o ZCODE.CMD ZCODE.ASM
	mv -f ZCODE.CMD ZCODE.ROM

clean:
	rm -f scvm *.o out.sym AS03X.CMD AS03X.ASM SMALLCX.CMD SMALLCX.ASM AS03.CMD AS03.ASM SMALLC.CMD SMALLC.ASM
	rm -f unicc unicc.h
	rm -f unias unias.h unild unild.h uniar uniar.h
	rm -f UNILD.ASM UNILD.CMD UNIAR.ASM UNIAR.CMD
	rm -f UNICC.ASM UNIAS.CMD UNICC.CMD UNIAS.CMD ZCODE.ROM ZCODE.LST
	rm -f test-scvm pyldin/*.ROM pyldin/*.LST pyldin/*.CMD pyldin/*.ASM pyldin/SYSTEM.H

# FPGA target: a common Small-C ABI, selected at load time by RAMLOAD /CVM.
.PHONY: pyldin
pyldin: unias unicc pyldin/ZVM6303.ROM pyldin/ZVM6800.ROM pyldin/UNIAS.CMD pyldin/UNICC.CMD pyldin/UNILD.CMD pyldin/UNIAR.CMD pyldin/SYSTEM.H


pyldin/ZVM6303.ROM: ZCODE.ASM unias
	mkdir -p pyldin
	./unias -D PYLDIN601 -l pyldin/ZVM6303.LST -o $@ ZCODE.ASM

pyldin/ZVM6800.ROM: ZCODE.ASM unias
	mkdir -p pyldin
	./unias -D PYLDIN601 -D CPU6800 -l pyldin/ZVM6800.LST -o $@ ZCODE.ASM

pyldin/UNIAS.CMD: AS03.ASM SYSTEM.H unias
	mkdir -p pyldin
	./unias -D PYLDIN601 -o $@ AS03.ASM

pyldin/UNICC.ASM: SMALL-C/SMALLC.C unicc
	mkdir -p pyldin
	./unicc -o $@ SMALL-C/SMALLC.C

pyldin/UNICC.CMD: pyldin/UNICC.ASM SYSTEM.H unias
	./unias -D PYLDIN601 -o $@ pyldin/UNICC.ASM

pyldin/SYSTEM.H: SYSTEM.H
	mkdir -p pyldin
	printf '    .define PYLDIN601\n' > $@
	cat SYSTEM.H >> $@

.PHONY: test

test: scvm unias unild uniar
	$(CC) -O1 -g -fsanitize=address,undefined tests/test_scvm.c -o test-scvm
	./test-scvm
	python3 tests/test_objects.py

# Linker and librarian are compiled with the same Small-C VM as UniAS.
UNILD.ASM: UNILD.C TOOLIO.C unicc
	./unicc -o $@ UNILD.C
UNIAR.ASM: UNIAR.C TOOLIO.C unicc
	./unicc -o $@ UNIAR.C
UNILD.CMD: UNILD.ASM unias
	./unias -o $@ $<
UNIAR.CMD: UNIAR.ASM unias
	./unias -o $@ $<
unild.h: UNILD.CMD
	hexdump -v -e '"\t" 16/1 "0x%02x, " "\n"' $< | sed 's/0x  ,//g' > $@
uniar.h: UNIAR.CMD
	hexdump -v -e '"\t" 16/1 "0x%02x, " "\n"' $< | sed 's/0x  ,//g' > $@
unild: scvm.c unild.h
	$(CC) -o $@ scvm.c -DAPP=\"unild.h\" -Wall $(CFLAGS)
uniar: scvm.c uniar.h
	$(CC) -o $@ scvm.c -DAPP=\"uniar.h\" -Wall $(CFLAGS)
pyldin/UNILD.CMD: UNILD.ASM SYSTEM.H unias
	mkdir -p pyldin
	./unias -D PYLDIN601 -o $@ UNILD.ASM
pyldin/UNIAR.CMD: UNIAR.ASM SYSTEM.H unias
	mkdir -p pyldin
	./unias -D PYLDIN601 -o $@ UNIAR.ASM
