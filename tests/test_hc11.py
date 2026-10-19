"""Run UniAS on unchanged ASM fixtures; compare HC11 bytes and relocations.

No opcode encoder or source/binary patcher lives here. Expected bytes are
literal comments in the checked-in fixture, transcribed from M68HC11ERG.
"""
from pathlib import Path
import re
import shutil
import struct
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / 'tests/hc11'


def pgm(path):
    data = path.read_bytes()
    magic, count, offset, size, entry, bss = struct.unpack_from('>6H', data)
    assert magic == 0xa55a and offset == 16 + count * 2
    assert len(data) == offset + size and entry == 0 and bss == 8
    rels = [struct.unpack_from('>H', data, 16 + n * 2)[0] for n in range(count)]
    return data[offset:], rels


with tempfile.TemporaryDirectory(prefix='unias-hc11-') as tmp:
    work = Path(tmp)
    shutil.copytree(SOURCE, work, dirs_exist_ok=True)

    def run(tool, *args, ok=True):
        result = subprocess.run([str(ROOT/tool), *args], cwd=work,
                                capture_output=True, text=True)
        assert (result.returncode == 0) == ok, (tool, args, result.stdout[-2500:], result.stderr)
        return result

    expected = bytearray()
    cases = 0
    for line in (SOURCE/'OPCODES.ASM').read_text().splitlines():
        match = re.search(r'; ((?:[0-9a-f]{2} ?)+)$', line)
        if match:
            expected.extend(bytes.fromhex(match[1]))
            cases += 1
    for cpu in ('hc11', '68hc11', 'MC68HC11'):
        run('unias', '-m', cpu, '-o', 'OPCODES.CMD', 'OPCODES.ASM')
        actual = (work/'OPCODES.CMD').read_bytes()
        assert actual == expected, (cpu, actual.hex(), expected.hex())
    run('unias', '-o', 'WRONG.CMD', 'OPCODES.ASM', ok=False)
    run('unias', '-m', 'hc12', 'OPCODES.ASM', ok=False)
    for flags, expected_cond in (([], 'aa55'), (['-m', 'hc11'], '112255')):
        result = run('unias', *flags, '-o', 'COND.CMD', 'CONDIT.ASM')
        assert 'Non paired' not in result.stdout
        assert (work/'COND.CMD').read_bytes() == bytes.fromhex(expected_cond)

    for path in sorted((work/'bad').glob('*.ASM')):
        run('unias', '-m', 'hc11', '-o', 'BAD.PGM', str(path), ok=False)

    for name in ('MAIN', 'HELP', 'BSS'):
        run('unias', '-m', 'hc11', '-c', '-o', name+'.OBJ', name+'.ASM')
    run('unild', '-o', 'LINK.PGM', 'MAIN.OBJ', 'HELP.OBJ', 'BSS.OBJ')
    run('unias', '-m', 'hc11', '-o', 'LOCAL.PGM', 'LOCAL.ASM')
    code, rels = pgm(work/'LINK.PGM')
    assert code == bytes.fromhex('18ce001618ff00161a830016bd00113f38183a180839'), code.hex()
    assert rels == [2, 6, 10, 13], rels
    assert pgm(work/'LOCAL.PGM') == (code, rels)
    run('uniar', 'r', 'HC11.LIB', 'HELP.OBJ', 'BSS.OBJ')
    run('unild', '-o', 'LIB.PGM', 'MAIN.OBJ', 'HC11.LIB')
    assert pgm(work/'LIB.PGM') == (code, rels)
    print(f'PASS UniAS HC11: {cases} instruction forms, three CPU aliases, '
          'invalid operands, all prefix pages, CMD/PGM/OBJ/LIB and BSS relocations')
