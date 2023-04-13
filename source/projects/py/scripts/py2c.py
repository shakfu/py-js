#!/usr/bin/env python3

# from pathlib import Path

def to_cstr(py_code, varname='PY_DEFAULT_MODULE'):
    py_code = py_code.replace('"', '\\"')
    lines = py_code.split('\n')
    lines = [f'"{line}\\n"' for line in lines]
    return f'static char* {varname} =\n'+'\n'.join(lines)+';'


def write_default_module():
    with open('py_mod.py') as f:
        content = f.read()
        with open('py_mod.h', 'w') as g:
            print(to_cstr(content), file=g)


if __name__ == '__main__': 
    write_default_module()
