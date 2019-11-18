#!/usr/bin/env python3

import sys

def print_header():
    header = """
#define X )*2+1
#define _ )*2
#define s ((((((((0

char hankaku[4096] = {
"""
    print(header)

def print_footer():
    footer = """
};
#undef X
#undef _
#undef s
"""
    print(footer)


def print_body(filename):
    with open(filename, "r") as f:
        count = 0
        key = ""
        for line in f:
            if "char" in line:
                print("\n/* {} */".format(line.strip()))
                key = line.strip().split(" ")[-1]
                count = 0
                continue
            if "." in line or "*" in line:
                count += 1
                print("s  {}".format(line.strip().replace(".", "_ ").replace("*", "X ")), end="")
                if not (key == "0xff" and count == 16):
                    print(",")

def main():
    if len(sys.argv) != 2:
        print("usage: {} hankaku.txt".format(sys.argv[0]))
        sys.exit(1)

    print_header()
    print_body(sys.argv[1])
    print_footer()


if __name__ == "__main__":
    main()
