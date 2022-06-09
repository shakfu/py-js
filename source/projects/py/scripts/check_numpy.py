
try:
    import numpy
    print(numpy.get_include())
except ImportError:
    import sys
    sys.exit(1)

