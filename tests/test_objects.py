"""Exercise real tool binaries and legacy OBJ/LIB fixtures, not a host linker."""
from pathlib import Path
import shutil
import struct
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
FIX = ROOT/'tests/objects'

def word(data, offset):
    return struct.unpack_from('>H', data, offset)[0]

def pgm(path):
    b = path.read_bytes()
    assert word(b, 0) == 0xa55a
    n, off, size, entry, bss = struct.unpack_from('>5H', b, 2)
    assert off == 16+2*n and len(b) == off+size and entry < size
    rels = [word(b, 16+2*i) for i in range(n)]
    assert rels == sorted(rels) and all(x+1 < size for x in rels)
    return b[off:], bss, entry, rels

def obj(code, pubs=(), exts=(), refs=(), entry=0xffff, bss=0):
    header = struct.pack('>5HI2H', 0x5aa5, 1, len(pubs), len(exts),
                         len(code), 32+20*(len(pubs)+len(exts)), entry, bss)
    symbols = b''.join(n.encode().ljust(16,b'\0')+struct.pack('>2H',v,t)
                       for n,v,t in (*pubs, *((n,0,0) for n in exts)))
    return header.ljust(32,b'\0')+symbols+code+struct.pack('>H',len(refs))+b''.join(
        struct.pack('>BHH', *r) for r in refs)+b'\0'

with tempfile.TemporaryDirectory(prefix='unild-') as tmp:
    work = Path(tmp)
    for f in FIX.iterdir():
        shutil.copyfile(f,work/f.name)
    def run(tool, *args, ok=True):
        r = subprocess.run([str(ROOT/tool),*args],cwd=work,capture_output=True)
        assert (r.returncode == 0) == ok, (tool,args,r.stdout[-1000:],r.stderr)
        return r.stdout
    def assemble(name, text, objects=False, ok=True):
        (work/(name+'.ASM')).write_text(text)
        run('unias',*(['-c'] if objects else []),'-o',
            name+('.OBJ' if objects else '.PGM'),name+'.ASM',ok=ok)
    run('unias','-c','-o','MAIN.OBJ','MAIN.ASM')
    run('unias','-c','-o','HELP.OBJ','HELP.ASM')
    run('unild','-o','APP.PGM','MAIN.OBJ','HELP.OBJ')
    code,bss,entry,rels = pgm(work/'APP.PGM')
    assert len(code)==27 and bss==16 and entry==0 and len(rels)==3
    run('uniar','r','TEST.LIB','HELP.OBJ')
    assert (work/'TEST.LIB').read_bytes() == (work/'ORIG.LIB').read_bytes()
    for lib in ('ORIG.LIB','TEST.LIB'):
        run('unild','-o','LIB.PGM','MAIN.OBJ',lib)
        assert (work/'LIB.PGM').read_bytes()==(work/'APP.PGM').read_bytes()
    run('unild','-o','OLD.PGM','OMAIN.OBJ','OHELP.OBJ')
    assert b'CROSS PASS' in pgm(work/'OLD.PGM')[0]
    run('unild','-o','ONOENT.PGM','OMAIN.OBJ','ONOENT.OBJ')
    assert (work/'ONOENT.PGM').read_bytes()==(work/'OLD.PGM').read_bytes()
    run('uniar','a','TEST.LIB','OHELP.OBJ')
    assert b'OHELP' in run('uniar','t','TEST.LIB')
    original=(work/'TEST.LIB').read_bytes()
    run('uniar','d','TEST.LIB','MISSING',ok=False)
    assert (work/'TEST.LIB').read_bytes()==original
    run('uniar','d','TEST.LIB','OHELP')
    assert (work/'TEST.LIB').read_bytes()==(work/'ORIG.LIB').read_bytes()
    help_obj=(work/'HELP.OBJ').read_bytes();(work/'HELP.OBJ').unlink()
    run('uniar','x','TEST.LIB','HELP')
    assert (work/'HELP.OBJ').read_bytes()==help_obj
    (work/'HELP.OBJ').write_bytes(help_obj.replace(b'OBJECT PASS',b'NEWEST PASS'))
    run('uniar','r','TEST.LIB','HELP.OBJ')
    assert b'NEWEST PASS' in (work/'TEST.LIB').read_bytes() and b'OBJECT PASS' not in (work/'TEST.LIB').read_bytes()
    (work/'HELP.OBJ').write_bytes(help_obj)
    run('uniar','r','TEST.LIB','HELP.OBJ')
    assert (work/'TEST.LIB').read_bytes()==(work/'ORIG.LIB').read_bytes()
    run('uniar','r','TEST.LIB','OHELP.OBJ')
    run('uniar','a','TEST.LIB','HELP.OBJ',ok=False)
    run('uniar','junk','TEST.LIB',ok=False)
    (work/'LINK.LST').write_text('MAIN.OBJ\n# comment\nORIG.LIB\n')
    run('unild','-o','LIST.PGM','@LINK.LST')
    assert (work/'LIST.PGM').read_bytes()==(work/'APP.PGM').read_bytes()
    # Interleaved relative BSS, fixed workspace, nonzero entry and relocations.
    assemble('SECT', '''    db $aa
start ldx #buffer
    dw start
    section
buffer ds 17
    ends
    section $e0
scratch ds 2
    ends
    db $55
    section
more ds 3
    ends
    end start
''')
    c,b,e,rs=pgm(work/'SECT.PGM')
    assert c==bytes.fromhex('aace0007000155')
    assert b==20 and e==1 and rs==[2,4] and word(c,2)==7
    assemble('IMPLICIT','start ldx #buf\n    section\nbuf ds 40000\n    end start\n')
    assert pgm(work/'IMPLICIT.PGM')[1]==40000
    (work/'WRAP.ASM').write_text('    include SECT.ASM\n')
    run('unias','-o','WRAP.PGM','WRAP.ASM')
    assert (work/'WRAP.PGM').read_bytes()==(work/'SECT.PGM').read_bytes()
    assemble('PUBFWD','    public buf\nstart clr buf\n    ldaa buf\n    section\nbuf ds 2\n    end start\n',True)
    run('unild','-o','PUBFWD.PGM','PUBFWD.OBJ')
    assert pgm(work/'PUBFWD.PGM')[:3]==(bytes.fromhex('7f0006b60006'),2,0)
    (work/'CMDFWD.ASM').write_text('    org $c000\n    db next&255\nnext db $11\n    end\n')
    run('unias','-o','CMDFWD.CMD','CMDFWD.ASM')
    assert (work/'CMDFWD.CMD').read_bytes()==bytes.fromhex('0111')
    assemble('RADIX','    radix 16\nstart db 10,3F,0FF,0AB,$10,@10,%10\n    dw 100\n    radix $0a\n    db 10\n    end start\n')
    assert pgm(work/'RADIX.PGM')[0]==bytes.fromhex('103fffab10080201000a')
    assemble('MAINPT','    db $aa\n    main\n    rts\n    end\n',True)
    run('unild','-o','MAINPT.PGM','MAINPT.OBJ')
    assert pgm(work/'MAINPT.PGM')[2]==1
    run('unild','/c','-o','MAINPT.CMD','MAINPT.OBJ',ok=False)
    assemble('GLOB', '''    public exposed
start jsr exposed
    int $38
    proc
exposed rts
    global exposed
    endp
    end start
''',True)
    run('unild','-o','GLOB.PGM','GLOB.OBJ')
    assert len(pgm(work/'GLOB.PGM')[0])==6
    # Every width, external and internal, plus absolute exported symbols.
    assemble('WIDTH','''    extern target,absolute
start db target,/target
    dw target
    db local,/local
    dw local
    dw absolute
local int $38
    end start
''',True)
    assemble('TARGET','''    public target,absolute
target rts
absolute equ $beef
    end
''',True)
    run('unild','-o','WIDTH.PGM','WIDTH.OBJ','TARGET.OBJ')
    c,b,e,rs=pgm(work/'WIDTH.PGM')
    assert c==bytes.fromhex('0c00000c0a00000abeef3f3839')
    assert rs==[1,2,5,6]
    run('unild','/c','-o','WIDTH.CMD','WIDTH.OBJ','TARGET.OBJ')
    assert (work/'WIDTH.CMD').read_bytes()==bytes.fromhex('0c01010c0a01010abeef3f3839')
    run('unild','/cp','-o','ALIGN.CMD','WIDTH.OBJ','TARGET.OBJ')
    assert (work/'ALIGN.CMD').read_bytes()[0:4]==bytes.fromhex('00020200')
    assert len((work/'ALIGN.CMD').read_bytes())==512
    assemble('BADD','    section\nbuf ds 12\n    ends\n    end\n',True)
    assemble('RELERR','    extern missing\nstart bra missing\n    end start\n',True,False)
    assemble('RELERR','    extern missing\nstart dw missing+1\n    end start\n',True,False)
    assemble('RELERR','    public missing\nstart rts\n    end start\n',True,False)
    assemble('RELERR','    public abcdefghijklmnop\n    end\n',True,False)
    # Unresolved, duplicate, corrupted and overflowing inputs preserve output.
    saved=(work/'APP.PGM').read_bytes()
    run('unild','-o','APP.PGM','MAIN.OBJ',ok=False)
    assert (work/'APP.PGM').read_bytes()==saved
    for label,blob in {
        'TRUNC':help_obj[:-1],
        'TYPE':obj(b'\0\0',refs=[(4,0,0)],entry=0),
        'OFFSET':obj(b'\0\0',refs=[(3,0,1)],entry=0),
        'INDEX':obj(b'\0\0',exts=['X'],refs=[(0x83,1,0)],entry=0),
        'BADEND':obj(b'\0',entry=0)[:-1]+b'\x01',
        'NOSYM':obj(b'\0\0',exts=['X'],refs=[(0x83,0,0)],entry=0),
        'DUP':obj(b'\0\0',pubs=[('X',0,0),('X',0,0)],entry=0),
        'OVFL':obj(b'\0\0',bss=65535,refs=[(3,2,0)],entry=0),
    }.items():
        (work/(label+'.OBJ')).write_bytes(blob)
        run('unild','-o','APP.PGM',label+'.OBJ',ok=False)
        assert (work/'APP.PGM').read_bytes()==saved
    # Archives cross 64 KiB, while members and executable stay bounded.
    for i in range(3):
        (work/f'BIG{i}.OBJ').write_bytes(obj(bytes([i])*30000))
    run('uniar','r','BIG.LIB','BIG0.OBJ','BIG1.OBJ','BIG2.OBJ')
    assert (work/'BIG.LIB').stat().st_size>65535
    run('unild','-o','SMALL.PGM','MAIN.OBJ','HELP.OBJ','BIG.LIB')
    assert (work/'SMALL.PGM').read_bytes()==saved
    for i in range(3):(work/f'BIG{i}.OBJ').unlink()
    run('uniar','x','BIG.LIB')
    for i in range(3):assert (work/f'BIG{i}.OBJ').read_bytes()==obj(bytes([i])*30000)
print('PASS OBJ/PGM/BSS, all relocation widths, legacy UniCROSS/UniLIB, archive updates, >64K LIB, invalid inputs')
