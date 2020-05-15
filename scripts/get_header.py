#!/usr/bin/env python

import sys
from pathlib import Path
from subprocess import run


includes = Path(r'/Users/sa/Documents/Max 8/Packages/max-sdk-8.0.3/source/c74support/max-includes')

p = includes / sys.argv[1]
res = run(['/usr/local/bin/stripcmt', p], capture_output=True)

lines = res.stdout.decode('utf8').splitlines()

singles = '\n'.join([l for l in lines if l])

final = singles.replace(';', '')
print(final)