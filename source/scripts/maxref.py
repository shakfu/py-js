#!/usr/bin/env python3

"""maxref.py -- handle maxref files.

usage: maxref-parser [-h] [-d] [-c] name

Handle <name>.maxref.xml files

positional arguments:
  name        enter <name>.maxref.xml name

options:
  -h, --help  show this help message and exit
  -d, --dict  dump parsed maxref a dict
  -c, --code  dump parsed maxref as code

"""


import platform
import sys
from pathlib import Path
from textwrap import fill
from pprint import pprint
from xml.etree import ElementTree
from keyword import iskeyword
import json

try:
    HAVE_YAML = True
    import yaml
except ImportError:
    HAVE_YAML = False

# -----------------------------------------------------------------------------
# constants

PLATFORM = platform.system()


# -----------------------------------------------------------------------------
# helper functions

def replace_tags(text, sub, *tags):
    for tag in tags:
        text = text.replace(f'<{tag}>', sub).replace(f'</{tag}>', sub)
    return text

def to_pascal_case(s):
    return ''.join(x for x in s.title() if not x in ['_','-',' ', '.'])

# -----------------------------------------------------------------------------
# main class


class MaxRefParser:
    OBJECT_SUPER_CLASS = 'Object'
    TRIPLE_QUOTE = '\"\"\"'

    def __init__(self, name: str):
        self.name = name
        self.suffix = '.maxref.xml'
        self.refdict = self.get_refdict()
        self.d = {
            'methods': {},
            'attributes': {},
        }

    def check_exists(self):
        if self.name not in self.refdict:
            raise KeyError(f"cannot find '{self.name}' maxref")

    def _get_refpages(self) -> Path:
        if PLATFORM == 'Darwin':
            for p in Path('/Applications').glob('**/Max.app'):
                if not 'Ableton' in str(p):
                    return p / 'Contents/Resources/C74/docs/refpages'

    def get_refdict(self) -> dict[str, Path]:
        refdict = {}
        refpages = self._get_refpages()
        for prefix in ['jit', 'max', 'msp', 'm4l']:
            ref_dir = refpages / f'{prefix}-ref'
            for f in ref_dir.iterdir():
                name = f.name.replace(self.suffix, '')
                refdict[name] = f
        return refdict

    def _clean_text(self, text: str) -> str:
        backtick = '`'
        return (
            replace_tags(text, backtick, 'm', 'i', 'g', 'o', 'at')
            .replace('&quot;', backtick)
        )

    def load(self) -> ElementTree.Element:
        filename = self.refdict[self.name]
        cleaned = self._clean_text(filename.read_text())
        self.root = ElementTree.fromstring(cleaned)

    def parse(self):
        self.check_exists()
        self.load()
        self.extract_rest()
        self.extract_methods()
        self.extract_attributes()

    def extract_rest(self):
        root = self.root
        self.d.update(self.root.attrib)
        self.d['digest'] =  root.find('digest').text.strip()
        self.d['description'] =  root.find('description').text.strip()
        self.d['inlets'] = []
        inletlist = root.find('inletlist')
        for inlet in inletlist.iter():
            if inlet.attrib:
                self.d['inlets'].append(inlet.attrib)
        self.d['outlets'] = []
        outletlist = root.find('outletlist')
        for outlet in outletlist.iter():
            if outlet.attrib:
                self.d['outlets'].append(outlet.attrib)

    def extract_attributes(self):
        self.d['attributes'] = {}
        methodlist = self.root.find('attributelist')
        d = self.d['attributes']
        for a in methodlist.iter():
            if a.tag == 'attribute':
                name = a.attrib['name']
                d[name] = dict(a.attrib)
                for digest in a.findall('digest'):
                    if digest.text:
                        d[name]['digest'] = digest.text.strip()
                for desc in a.findall('description'):
                    if desc.text:
                        d[name]['description'] = desc.text.strip()

    def extract_methods(self):
        self.d['methods'] = {}
        methodlist = self.root.find('methodlist')
        d = self.d['methods']
        for m in methodlist.iter():
            if m.tag == 'method':
                name = m.attrib['name']
                d[name] = {}
                for arglist in m.findall('arglist'):
                    d[name]['args'] = []
                    for entry in arglist:
                        if entry.tag == 'arg':
                            d[name]['args'].append(dict(entry.attrib))
                        if entry.tag == 'arggroup':
                            for arg in entry.iter():
                                if arg.tag == 'arg':
                                    ad = dict(arg.attrib)
                                    ad.update(entry.attrib)
                                    d[name]['args'].append(ad)

                for digest in m.findall('digest'):
                    if digest.text:
                        d[name]['digest'] = digest.text.strip()
                for desc in m.findall('description'):
                    if desc.text:
                        d[name]['description'] = desc.text.strip()

    def dump_dict(self):
        pprint(self.d)

    def dump_json(self):
        json.dump(self.d, fp=sys.stdout)

    def __get_method_args(self, args: list[str]) -> list[str]:
        _args = []
        for i, arg in enumerate(args):
            if '?' in arg:
                arg = arg.replace('?', '')
            if 'symbol' in arg:
                arg = arg.replace('symbol', 'str')
            if 'any' in arg:
                arg = arg.replace('any', 'object')
            if '-' in arg:
                arg = arg.replace('-', '_')
            parts = arg.split()
            if len(parts) > 2:
                _type, *name_parts = parts
                name = '_'.join(name_parts)
                arg = f"{_type} {name}"
            if 'list' in arg:
                arg = arg.replace('list ', '*')
            _args.append(arg)
        return _args

    def __check_iskeyword(self, name: str) -> str:
        if iskeyword(name):
            name = name+'_'
        return name

    def __get_call(self, method_name: str, method_args: list[str]) -> str:
        _args = []
        for arg in method_args:
            if '*' in arg:
                arg = arg.replace('*', '')
            else:
                _type, name = arg.split()
                arg = name
            _args.append(arg)
        if len(_args) == 0:
            return f'self.call("{method_name}")'
        else:
            params = ", ".join(_args)
            return f'self.call("{method_name}", {params})'

    def dump_code(self):
        tq = self.TRIPLE_QUOTE
        superclass = self.OBJECT_SUPER_CLASS
        spacer = ' '*4
        classname = to_pascal_case(self.name)
        print(f'cdef class {classname}({superclass}):')
        print("{spacer}{tq}{digest}".format(
            spacer=spacer, tq=tq, digest=self.d['digest']))
        print()
        print("{spacer}{desc}".format(
            spacer=spacer, 
            desc=fill(self.d['description'], subsequent_indent=spacer)))
        print(f"{spacer}{tq}")
        print()
        method_args = []
        for name in self.d['methods']:
            m = self.d['methods'][name]
            # method_name = str(name)
            if '(' in name and ')' in name:
                continue

            sig = None
            if 'args' in m:
                args = []
                for arg in m['args']:
                    if 'optional' in arg:
                        args.append('{type} {name}?'.format(**arg))
                    else:
                        args.append('{type} {name}'.format(**arg))

                method_args = self.__get_method_args(args)
                sig = "{name}({self}{args}):".format(
                    name=self.__check_iskeyword(name),
                    self='self, ' if args else 'self',
                    args=", ".join(method_args))
                sig_selfless = "{name}({args})".format(
                    name=name,
                    args=", ".join(args))
            else:
                sig = "{name}(self):".format(name=self.__check_iskeyword(name))
            print(f'{spacer}def {sig}')

            if 'digest' in m:
                print('{spacer}{tq}{digest}'.format(
                    spacer=spacer*2,
                    tq=tq,
                    digest=m['digest']))
            if args:
                print()
                print("{spacer}{sig}".format(spacer=spacer*2, sig=sig_selfless))
            if 'description' in m:
                if m['description'] and 'TEXT_HERE' not in m['description']:
                    print()
                    print('{spacer}{desc}'.format(
                        spacer=spacer*2,
                        desc=fill(m['description'], subsequent_indent=spacer*2)))
            print('{spacer}{tq}'.format(spacer=spacer*2, tq=tq))
            print('{spacer}{call}'.format(
                spacer=spacer*2, call=self.__get_call(name, method_args)))
            print()

    def dump_tests(self):
        spacer = ' '*4
        classname = self.d['name']
        tq = self.TRIPLE_QUOTE

        for name in self.d['methods']:
            m = self.d['methods'][name]
            print(f"def test_{classname}_{name}():")
            print(f"{spacer}{tq}{m['digest']}{tq}")
            print()


    if HAVE_YAML:
        def dump_yaml(self):
            print(yaml.dump(self.d, Dumper=yaml.Dumper))


if __name__ == '__main__':
    import argparse
    parser = argparse.ArgumentParser(
        prog='maxref-parser',
        description='Parse and generate code from *.maxref.xml files.'
    )
    parser.add_argument('name', nargs='?', help='enter <name>.maxref.xml name')
    parser.add_argument('-d', '--dict', action='store_true', help="dump parsed maxref as dict")
    parser.add_argument('-j', '--json', action='store_true', help="dump parsed maxref as json")
    parser.add_argument('-c', '--code', action='store_true', help="generate class outline")
    parser.add_argument('-t', '--test', action='store_true', help="generate tests")
    if HAVE_YAML:
        parser.add_argument('-y', '--yaml', action='store_true', help="dump parsed maxref as yaml")
    parser.add_argument('-l', '--list', action='store_true', help="list all objects")

    args = parser.parse_args()
    p = MaxRefParser(args.name)

    # pre-parsing actions ------- 
    if args.list:
        for name in sorted(p.refdict.keys()):
            print(name)
        sys.exit(0)

    # post-parsing actions ------- 
    p.parse()
    assert args.name, "name in `maxref [options] <name>` must be provided"
    if args.dict:
         p.dump_dict()

    elif args.code:
        p.dump_code()

    elif args.test:
        p.dump_tests()

    elif HAVE_YAML and args.yaml:
        p.dump_yaml()


