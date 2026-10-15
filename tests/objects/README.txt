OMAIN.OBJ and OHELP.OBJ: original native-src/UNICROSS.PAS, version 3.10,
compiled unchanged by Free Pascal 3.2.2 in Turbo Pascal mode (-Mtp).
Inputs are the adjacent OMAIN.ASM and OHELP.ASM; command: UNICROSS input ; output.
ORIG.LIB: original native-src/UNILIB.PAS, version 1.00, same compiler,
command UNILIB ORIG.LIB +HELP.OBJ, where HELP.OBJ is from enhanced UniAS.
It is byte-identical to UniAR output for the same member.
The original native ULINK.CMD from ROM disk is tested in pyldin601-fpga.
ONOENT.OBJ: original UniCROSS on ONOENT.ASM, explicit END noentry
where noentry EQU $FFFF. This avoids v0's implicit zero entry in a helper.
ARREF.LIB: original UniLIB ARREF.LIB +RHELP.OBJ. RHELP.OBJ is enhanced
UniAS output from RHELP.ASM (one RTS, no internal relocations).
The latter isolates container compatibility from a historical ULINK defect
in preallocating PGM relocations for archive members.
