#!/usr/bin/env python3


inputs = {
    'py_prelude.py' : 'PY_PRELUDE_MODULE',
    # 'fn.py'     : 'PY_FUNCTIONAL_MODULE',
}

OUTPUT='py_prelude.h'


def to_cstr(py_code, varname):
    py_code = py_code.replace('"', '\\"')
    lines = py_code.split('\n')
    lines = [f'"{line}\\n"' for line in lines]
    return f'\nstatic const char* {varname} =\n'+'\n'.join(lines)+';'


def write_default_module():
    with open(OUTPUT, 'w') as outfile:
        print("// py_prelude.h: pure python functions for the `py` external", file=outfile)
        print("// generated by `py/scripts/py2c.py`", file=outfile)
        for filename in inputs:
            with open(filename) as f:
                content = f.read()            
                print(to_cstr(content, inputs[filename]), 
                    file=outfile)


if __name__ == '__main__': 
    write_default_module()
