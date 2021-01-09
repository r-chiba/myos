import os
import sys
import argparse
import re

def line2value(line):
    val = 0
    for i, c in enumerate(line):
        if c == '*':
            #val |= (1 << (7-i))
            val |= 1<<i
    return val

def parse_font(fontTxt):
    arr = []
    with open(fontTxt, "r") as font:
        read = 0
        char = []
        found = False
        for l in font:
            if re.match(r"^char", l) is not None:
                found = True
            if found:
                v = line2value(l)
                char.append(v)
                read += 1
            if read == 16:
                found = False
                read = 0
                arr.append(char)
                char = []
    print(arr)
    return arr

def make_font(fontTxt, outPath):
    arr = parse_font(fontTxt)
    with open(outPath, "w") as out:
        out.write("const uint8_t font[0x100][16] = {\n")
        for c in arr:
            #out.write("    {\n")
            #out.write("        \n")
            out.write("    {")
            for i, v in enumerate(c):
                out.write("0x{val:02x}".format(val=v))
                if i < 15:
                    out.write(", ")
                else:
                    out.write("},\n")
        out.write("};\n")

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("fontTxt", help="path to font txt file")
    parser.add_argument("outputPath", help="path to output source file")
    args = parser.parse_args()
    make_font(args.fontTxt, args.outputPath)
