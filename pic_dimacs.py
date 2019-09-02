#!/usr/bin/python3
import argparse
import os
import png

parser = argparse.ArgumentParser(description="make pictures of DIMACS files")
parser.add_argument("list_file")
args = parser.parse_args()


def read_lines(filename):
    with open(filename) as f:
        return [s.rstrip("\n") for s in f]


def read_dimacs(filename):
    global variables
    global clauses
    variables = set()
    clauses = []
    for s in read_lines(filename):
        if not s:
            continue
        if s[0].isalpha():
            continue
        s = [int(t) for t in s.split()]
        c = []
        for a in s:
            if not a:
                break
            variables.add(abs(a))
            c.append(a)
        clauses.append(c)
    variables = sorted(list(variables))
    if variables[-1] != len(variables):
        print(
            filename
            + ": discontinuous variables: "
            + str(len(variables))
            + ": "
            + str(variables[-1])
        )


files = read_lines(args.list_file)
for filename in files:
    # dimacs
    read_dimacs(filename)

    # image
    width = len(variables)
    rows = []
    for c in clauses:
        row = [0] * width
        for a in c:
            i = abs(a) - 1
            row[i] = 1 if a > 0 else 2
        rows.append(row)

    # png
    palette = [(0x00, 0x00, 0x00), (0x00, 0xFF, 0x00), (0xFF, 0x00, 0x00)]
    w = png.Writer(width, len(rows), palette=palette, bitdepth=2)

    # file
    try:
        os.mkdir("pic")
    except:
        pass
    base = os.path.splitext(filename)[0]
    f = open("pic/" + base + ".png", "wb")
    w.write(f, rows)
    f.close()
