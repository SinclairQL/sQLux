#!/usr/bin/env python3
"""
validate_cycles.py - Check the sQLux 68000 cycle table against the Tom Harte
ProcessorTests (SingleStepTests/680x0, directory 68000/v1).

Usage:
    cc -DCYCLES68K_TOOL -o cycles_tool cycles68k.c
    ./cycles_tool 68000 table68000.bin
    python3 tools/validate_cycles.py table68000.bin path/to/68000/v1/*.json.gz

For every test the opcode (initial.prefetch[0]) is taken and the predicted
cost (table + dynamic part, computed from the initial state) is compared with
the 'length' field. Tests that end in an exception (address error, division
by zero, privilege violation...) are skipped; they are detected because the
final PC points to the target of a vector present in the initial RAM.
"""

import gzip
import json
import struct
import sys
from collections import Counter, defaultdict

K_NONE, K_BCC, K_DBCC, K_SCC, K_SHIFT, K_MOVEM_W, K_MOVEM_L, K_MULU, K_MULS, K_DIVU, K_DIVS = range(11)


def divu_cycles(dividend, divisor):
    dividend &= 0xFFFFFFFF
    if (dividend >> 16) >= divisor:
        return 10
    m, hd = 38, divisor << 16
    for _ in range(15):
        temp = dividend
        dividend = (dividend << 1) & 0xFFFFFFFF
        if temp & 0x80000000:
            dividend = (dividend - hd) & 0xFFFFFFFF
        else:
            m += 2
            if dividend >= hd:
                dividend = (dividend - hd) & 0xFFFFFFFF
                m -= 1
    return m * 2


def divs_cycles(dividend, divisor):
    dividend = dividend - (1 << 32) if dividend & 0x80000000 else dividend
    divisor = divisor - (1 << 16) if divisor & 0x8000 else divisor
    m = 6 + (1 if dividend < 0 else 0)
    if divisor == 0 or (abs(dividend) >> 16) >= abs(divisor):
        return (m + 2) * 2
    aquot = abs(dividend) // abs(divisor)
    m += 55
    if divisor >= 0:
        m += -1 if dividend >= 0 else 1
    for _ in range(15):
        if not (aquot & 0x8000):
            m += 1
        aquot <<= 1
    return m * 2

# 68000 dynamic costs (must match cycles_init)
BCC_TAKEN, BCC_NT_B, BCC_NT_W = 10, 8, 12
DBCC_TRUE, DBCC_BRANCH, DBCC_EXPIRED = 12, 10, 14
SCC_TRUE, SCC_FALSE = 6, 4
MOVEM_W, MOVEM_L = 4, 8


def load_table(path):
    with open(path, "rb") as f:
        raw = f.read()
    table = struct.unpack("<65536H", raw[:131072])
    kind = raw[131072:131072 + 65536]
    return table, kind


def cond_true(cc, sr):
    c, v, z, n = sr & 1, (sr >> 1) & 1, (sr >> 2) & 1, (sr >> 3) & 1
    return [
        True, False, not c and not z, c or z, not c, c, not z, z,
        not v, v, not n, n, n == v, n != v,
        (n == v) and not z, z or (n != v),
    ][cc]


def dynamic(op, kind, st):
    sr = st["sr"]
    d = [st["d%d" % i] for i in range(8)]
    ext = st["prefetch"][1]
    if kind == K_BCC:
        if cond_true((op >> 8) & 15, sr):
            return BCC_TAKEN
        return BCC_NT_B if (op & 0xFF) else BCC_NT_W
    if kind == K_DBCC:
        if cond_true((op >> 8) & 15, sr):
            return DBCC_TRUE
        return DBCC_EXPIRED if (d[op & 7] & 0xFFFF) == 0 else DBCC_BRANCH
    if kind == K_SCC:
        return SCC_TRUE if cond_true((op >> 8) & 15, sr) else SCC_FALSE
    if kind == K_SHIFT:
        return 2 * (d[(op >> 9) & 7] & 63)
    if kind == K_MOVEM_W:
        return MOVEM_W * bin(ext & 0xFFFF).count("1")
    if kind == K_MOVEM_L:
        return MOVEM_L * bin(ext & 0xFFFF).count("1")
    if kind == K_MULU:
        return 2 * bin(d[op & 7] & 0xFFFF).count("1")
    if kind == K_MULS:
        x = (d[op & 7] & 0xFFFF) << 1
        return 2 * bin((x ^ (x >> 1)) & 0xFFFF).count("1")
    if kind == K_DIVU:
        return divu_cycles(d[(op >> 9) & 7], d[op & 7] & 0xFFFF)
    if kind == K_DIVS:
        return divs_cycles(d[(op >> 9) & 7] & 0xFFFFFFFF, d[op & 7] & 0xFFFF)
    return 0


def ended_in_exception(test, op):
    if (op & 0xFFF0) == 0x4E40:          # TRAP: the exception is the expected result
        return False
    ram = {a: v for a, v in test["initial"]["ram"]}
    final_pc = test["final"]["pc"]
    for vec in list(range(2, 12)) + list(range(32, 48)):
        a = vec * 4
        if all((a + i) in ram for i in range(4)):
            target = (ram[a] << 24) | (ram[a + 1] << 16) | (ram[a + 2] << 8) | ram[a + 3]
            if final_pc in (target, target + 2, target + 4):
                return True
    return False


def main():
    if len(sys.argv) < 3:
        print(__doc__)
        return 1
    table, kind = load_table(sys.argv[1])
    grand = Counter()
    for path in sys.argv[2:]:
        opener = gzip.open if path.endswith(".gz") else open
        with opener(path, "rt") as f:
            tests = json.load(f)
        stats = Counter()
        bad = defaultdict(Counter)
        for t in tests:
            op = t["initial"]["prefetch"][0] & 0xFFFF
            if ended_in_exception(t, op):
                stats["skipped"] += 1
                continue
            k = kind[op]
            ours = table[op] + (dynamic(op, k, t["initial"]) if k else 0)
            exp = t["length"]
            if ours == exp:
                stats["ok"] += 1
            else:
                stats["bad"] += 1
                bad[op][(exp, ours)] += 1
        grand.update(stats)
        name = path.split("/")[-1]
        total = stats["ok"] + stats["bad"]
        pct = 100.0 * stats["ok"] / total if total else 0.0
        print("%-28s ok %5d  bad %5d  skipped %5d  (%.1f%%)" %
              (name, stats["ok"], stats["bad"], stats["skipped"], pct))
        worst = sorted(bad.items(), key=lambda kv: -sum(kv[1].values()))[:3]
        for op, cnt in worst:
            (exp, ours), n = cnt.most_common(1)[0]
            print("    opcode %04X: expected %d, table %d  (%d cases)" %
                  (op, exp, ours, sum(cnt.values())))
    total = grand["ok"] + grand["bad"]
    if total:
        print("\nTOTAL: %d/%d correct (%.2f%%), %d skipped" %
              (grand["ok"], total, 100.0 * grand["ok"] / total, grand["skipped"]))
    return 0


if __name__ == "__main__":
    sys.exit(main())
