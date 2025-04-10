#!/usr/bin/env python3

import sys

filename = sys.argv[1]

with open(filename) as f:
    lines = [line.rstrip() for line in f.readlines()]
    with open('cleaned_'+filename, 'w') as g:
        g.write("\n".join(lines))

