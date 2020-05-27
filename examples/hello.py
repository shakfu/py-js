# core python builtins
import io, sys

# embedded builtin
import api # cythonized max c api

# basic variables
a = 10
c = 1.5
b = "HELLO WORLD???"
d = lambda: "hello Func"

# test stdout/stderr redirection
sys.stdout = io.StringIO()
print('foo')
print('hello max!')
#sys.stderr.write(sys.stdout.getvalue())
api.post("from py: %s", sys.stdout.getvalue())


