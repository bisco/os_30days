#!/usr/bin/env python3

import sys

with open(sys.argv[1], "rb") as f:
    zero_count = 0
    data = f.read()
    start = 0
    end = 0x10
    size = len(data)
    all_zero = True
    zero_count = 0
    while start != size:
        all_zero = True
        for d in data[start: end]:
            if d != 0:
                all_zero = False
                break

        if all_zero == True:
            zero_count += 0x10
            start = end
            end += 0x10
            if end >= size:
                end = size
            continue
        elif all_zero == False and zero_count > 0:
            print("RESB {}".format(zero_count))
            zero_count = 0

        out = ", ".join(["0x{:02x}".format(d) for d in data[start:end]])
        print("DB {}".format(out))
        start = end
        end += 0x10
        if end >= size:
            end = size

    if all_zero:
        print("RESB {}".format(zero_count))
