"""

scripts to get numpy include and test if it exists

"""

import argparse
import sys

try:
    import numpy
    NUMPY_INCLUDE = numpy.get_include()
except ImportError:
    NUMPY_INCLUDE = None



parser = argparse.ArgumentParser()
parser.add_argument('--include', action='store_true', help='get numpy_include')
parser.add_argument('--exists', action='store_true', help='check if numpy exists')

args = parser.parse_args()
if args.include:
    if NUMPY_INCLUDE:
        print(NUMPY_INCLUDE)
        sys.exit()
if args.exists:
    if NUMPY_INCLUDE:
        print(1)
    else:
        print(0)

