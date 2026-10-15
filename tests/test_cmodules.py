"""Compile independent Small-C modules, link and execute their real bytecode."""
from pathlib import Path
import shutil,struct,subprocess,tempfile
ROOT=Path(__file__).resolve().parents[1]
with tempfile.TemporaryDirectory(prefix='unicc-obj-') as folder:
    out=Path(folder)
    for p in (ROOT/'tests/cmodules').iterdir():shutil.copyfile(p,out/p.name)
    for n in ('CVMDEF.INC','CVMABI.INC','SYSTEM.H'):shutil.copyfile(ROOT/'pyldin'/n,out/n)
    def run(tool,*args,ok=True,code=0):
        r=subprocess.run([str(ROOT/tool),*args],cwd=out,capture_output=True,timeout=20)
        assert (r.returncode==code) if ok else r.returncode!=0,(tool,args,r.returncode,r.stdout[-1200:],r.stderr)
        return r.stdout
    def compile(name):
        text=run('unicc','-c','-o',name+'.ASM',name+'.C')
        assert b'There were 0 errors' in text
        run('unias','-c','-o',name+'.OBJ',name+'.ASM')
    for n in ('MMAIN','MHELP','MOTHER','MUNUSED','MIX','MINLINE','SMAIN','SIMPLE'):compile(n)
    for n in ('CSTART','CEND','CVCALL'):
        shutil.copyfile(ROOT/(n+'.ASM'),out/(n+'.ASM'))
        run('unias','-D','PYLDIN601','-c','-o',n+'.OBJ',n+'.ASM')
    run('unias','-c','-o','MNATIVE.OBJ','MNATIVE.ASM')
    run('unias','-c','-o','NENTRY.OBJ','NENTRY.ASM')
    run('uniar','r','CRT.LIB','CEND.OBJ','CVCALL.OBJ')
    run('uniar','r','UTIL.LIB','MHELP.OBJ','MOTHER.OBJ','MUNUSED.OBJ')
    run('unild','-m','APP.MAP','-o','APP.PGM','CSTART.OBJ','MMAIN.OBJ','MHELP.OBJ','MOTHER.OBJ','CRT.LIB')
    run('unild','-o','LIBAPP.PGM','CSTART.OBJ','MMAIN.OBJ','UTIL.LIB','CRT.LIB')
    assert (out/'APP.PGM').read_bytes()==(out/'LIBAPP.PGM').read_bytes()
    assert struct.unpack_from('>H',(out/'APP.PGM').read_bytes(),10)[0]==16
    for app in ('APP.PGM','LIBAPP.PGM'):
        for base in ('0x100','0x4300','0x8000'):
            assert b'C MODULE PASS' in run('scvm','-b',base,app)
    run('unild','-o','MIXAPP.PGM','CSTART.OBJ','MIX.OBJ','MINLINE.OBJ','MNATIVE.OBJ','CRT.LIB')
    run('unild','-o','NENTRY.PGM','NENTRY.OBJ','MIX.OBJ','MINLINE.OBJ','MNATIVE.OBJ','CRT.LIB')
    run('uniar','r','SIMPLIB.LIB','SIMPLE.OBJ')
    run('unild','-o','SLIB.PGM','CSTART.OBJ','SMAIN.OBJ','SIMPLIB.LIB','CRT.LIB')
    assert b'C LIB PASS' in run('scvm','SLIB.PGM')
    run('scvm','MIXAPP.PGM',ok=False)
    truncated=(out/'APP.PGM').read_bytes()[:-1]
    (out/'TRUNC.PGM').write_bytes(truncated)
    run('scvm','TRUNC.PGM',ok=False)
    run('scvm','-b','0xbdf0','APP.PGM',ok=False)
    # Existing single-file output is still a complete absolute CMD.
    (out/'OLD.C').write_text('main()\n{\n puts("LEGACY PASS");\n}\n')
    run('unicc','-o','OLD.ASM','OLD.C');run('unias','-o','OLD.CMD','OLD.ASM')
    assert b'LEGACY PASS' in run('scvm','OLD.CMD')
    # Errors must fail the command and preserve an existing module output.
    (out/'BAD.C').write_text('main()\n{ return missing+; }\n')
    (out/'BAD.ASM').write_bytes(b'old output')
    run('unicc','-c','-o','BAD.ASM','BAD.C',ok=False)
    assert (out/'BAD.ASM').read_bytes()==b'old output'
    assert not (out/'BAD.UCT').exists()
    (out/'STATIC.C').write_text('static int absent();\nmain(){return absent();}\n')
    (out/'STATIC.ASM').write_bytes(b'keep static')
    run('unicc','-c','-o','STATIC.ASM','STATIC.C',ok=False)
    assert (out/'STATIC.ASM').read_bytes()==b'keep static'
    (out/'DUP.C').write_text('int x;\nint x;\nmain(){return x;}\n')
    run('unicc','-c','-o','DUP.ASM','DUP.C',ok=False)
    # Repeated declarations, arrays, and definitions share one symbol.
    (out/'DECL.C').write_text('extern int x;\nint x;\nextern int x;\nextern char a[];\nchar a[4];\nmain(){return x+a[0];}\n')
    compile('DECL')
    print('PASS C modules: shared BSS, static scope, externs, direct/indirect/recursive calls, libraries, relocation, legacy CMD and errors')
