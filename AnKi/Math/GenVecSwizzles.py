#!/usr/bin/env python3
"""
Generate all valid HLSL swizzles for float2, float3 and float4.
Rules used:
 - float2: components = x, y
 - float3: components = x, y, z
 - float4: components = x, y, z, w
 - Swizzle lengths: 1..4 (HLSL allows swizzles up to length 4)
 - Repetition and any ordering allowed, but only components that exist for the given vector type

Outputs the swizzles to stdout and writes them to files:
 - swizzles_float2.txt
 - swizzles_float3.txt
 - swizzles_float4.txt

Quick usage: python3 hlsl_swizzles.py
"""
from itertools import product

VECTOR_COMPONENTS = {
    'float2': 'xy',
    'float3': 'xyz',
    'float4': 'xyzw',
}

MAX_SWIZZLE_LEN = 4


def comp_to_index(c):
    if c == "x": return "0"
    elif c == "y": return "1"
    elif c == "z": return "2"
    else: return "3"


def swizzle_to_indices(p):
    txt = ""
    for i in range(len(p)):
        txt += comp_to_index(p[i]) + (", " if i < len(p) - 1 else "")
    return txt


def generate_swizzles(components: str, max_len: int = 4):
    """Yield swizzle strings using only the provided components, lengths 1..max_len."""
    comp_count = len(components)
    for L in range(1, max_len + 1):
        for p in product(components, repeat=L):
            if len(p) > 1:
            	yield "TVecSwizzledData<T, {}, {}> {};".format(comp_count, swizzle_to_indices(p), ''.join(p))


if __name__ == '__main__':
    totals = {}
    for vt, comps in VECTOR_COMPONENTS.items():
        swizzles = list(generate_swizzles(comps, MAX_SWIZZLE_LEN))
        totals[vt] = len(swizzles)
        # Print short summary
        print(f"{vt}: {len(swizzles)} swizzles (components: {', '.join(comps)})")
        # Optionally group by length — small readable output
        by_len = {}
        for s in swizzles:
            by_len.setdefault(len(s), []).append(s)
        for L in sorted(by_len):
            print(f"  len={L}: {len(by_len[L])} -> example: {', '.join(by_len[L][:8])}{'...' if len(by_len[L])>8 else ''}")
        # write full list to file
        fname = f"swizzles_{vt}.txt"
        with open(fname, 'w') as f:
            for s in swizzles:
                f.write(s + '\n')
        print(f"  full list written to: {fname}\n")

    total_all = sum(totals.values())
    print(f"Total swizzles generated: {total_all}")
