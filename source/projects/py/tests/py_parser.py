"""py_parser

An experiment to make an improved parser with lark
see:
    https://github.com/lark-parser/lark 
    https://lark-parser.readthedocs.io/en/latest/


"""
from lark import Lark, Transformer, v_args

py_parser = Lark(r"""
// parse: api.total a b c 10.1 "nice" 12.2 40 50 a=10 b="sa"

start: func_name args kwargs

number: SIGNED_NUMBER

string: ESCAPED_STRING

symbol: CNAME 
    | string

arg: symbol
    | number

args: [arg*]

kwarg: (CNAME"="arg)

kwargs: [kwarg*]

func_name: FUNC_NAME

FUNC_NAME: ("_"|LETTER) ("_"|"."|LETTER|DIGIT)*

%import common.ESCAPED_STRING
%import common.SIGNED_NUMBER
%import common.LETTER
%import common.DIGIT
%import common.CNAME

// Disregard spaces in text
%ignore " "
""")

class PyTransformer(Transformer):
    def __init__(self, **kwds):
        self.env = kwds

    def start(self, x):
        return tuple(x)

    def func_name(self, x):
        return eval(str(x[0]), self.env, self.env)

    @v_args(inline=True)
    def string(self, s):
        return s[1:-1].replace('\\"', '"')

    def symbol(self, x):
        return str(x[0])

    def number(self, x):
        return float(x[0])

    def arg(self, x):
        return x[0]

    def args(self, xs):
        return tuple(xs)

    def kwarg(self, item):
        k,v = item
        return k,v

    def CNAME(self, x):
        return str(x)

    def kwargs(self, items):
        return dict(items)



def test():
    s = 'api.process a b c 10.1 "nice" 12.2 40 50 a=10 b="sa"'

    class api:
        @staticmethod
        def process(*args, **kwargs):
            print(f"args: {args} kwargs: {kwargs}")

    tree = py_parser.parse(s)
    t = PyTransformer(api=api)
    transformed = t.transform(tree)
    print(transformed)
    # test it
    f, args, kwargs = transformed
    print('calling function:')
    f(*args, **kwargs)





if __name__ == '__main__':
    test()
