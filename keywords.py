#!/usr/bin/python3
# a symbol is an interned string
# a keyword is a symbol of known significance
# need the list of keywords in two formats
# that must correspond to each other
# so generate both lists


def read_lines(filename):
    with open(filename) as f:
        return [s.rstrip("\n") for s in f]


lines = read_lines("keywords.txt")

# header
with open("keywords.h", "w") as f:
    f.write("// AUTO GENERATED FILE - DO NOT MODIFY\n")
    f.write("enum {\n")
    for s in lines:
        f.write("w_" + s + ",\n")
    f.write("nkeywords\n")
    f.write("};\n")

# data
with open("keywords.cc", "w") as f:
    f.write("// AUTO GENERATED FILE - DO NOT MODIFY\n")
    f.write('#include "main.h"\n')
    f.write("sym keywords [] = {\n")
    for s in lines:
        f.write("{" + str(len(s)) + ',"' + s + '"},\n')
    f.write("};\n")
