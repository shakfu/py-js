#!/usr/bin/env python3

from random import randint

with open('/tmp/out.txt', 'w') as f:
	n = str(randint(0,100))
	f.write(f"{n}\n")
