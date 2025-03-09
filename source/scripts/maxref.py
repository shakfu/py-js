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
from pathlib import Path
from textwrap import fill
from pprint import pprint
from xml.etree import ElementTree
import json

try:
    HAVE_YAML = True
    import yaml
except ImportError:
    HAVE_YAML = False

PLATFORM = platform.system()
TQ = '\"\"\"'


def replace_tags(text, sub, *tags):
    for tag in tags:
        text = text.replace(f'<{tag}>', sub).replace(f'</{tag}>', sub)
    return text



class MaxRefParser:
    def __init__(self, name: str):
        self.name = name
        self.suffix = '.maxref.xml'
        self.refdict = self.get_refdict()
        self.d = {
            'methods': {},
            'attributes': {},
        }
        self.check_exists()
        self.root = self.load()

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
        return ElementTree.fromstring(cleaned)

    def parse(self):
        self.extract_rest()
        self.extract_methods()
        self.extract_attributes()

    def extract_rest(self):
        root = self.root
        self.d.update(self.root.attrib)
        # from IPython import embed; embed()
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

    def dump_code(self):
        spacer = ' '*4
        classname = self.name.title()
        print(f'cdef class {classname}:')
        print("{spacer}{TQ}{digest}".format(
            spacer=spacer, TQ=TQ, digest=self.d['digest']))
        print()
        print("{spacer}{desc}".format(
            spacer=spacer, 
            desc=fill(self.d['description'], subsequent_indent=spacer)))
        print(f"{spacer}{TQ}")
        print()
        for name in self.d['methods']:
            m = self.d['methods'][name]

            sig = None
            if 'args' in m:
                args = []
                for arg in m['args']:
                    if 'optional' in arg:
                        args.append('{type} {name}?'.format(**arg))
                    else:
                        args.append('{type} {name}'.format(**arg))

                sig = "{name}({self}{args}):".format(
                    name=name,
                    self='self, ' if args else 'self',
                    args=", ".join(args))
                sig_selfless = "{name}({args})".format(
                    name=name,
                    args=", ".join(args))
            else:
                sig = "{name}(self):".format(name=name)
            print(f'{spacer}def {sig}')

            if 'digest' in m:
                print('{spacer}{TQ}{digest}'.format(
                    spacer=spacer*2,
                    TQ=TQ,
                    digest=m['digest']))
            if args:
                print()
                print("{spacer}{sig}".format(spacer=spacer*2, sig=sig_selfless))
            if 'description' in m:
                if m['description']:
                    print()
                    print('{spacer}{desc}'.format(
                        spacer=spacer*2,
                        desc=fill(m['description'], subsequent_indent=spacer*2)))
            print('{spacer}{TQ}'.format(spacer=spacer*2, TQ=TQ))
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
    parser.add_argument('name', help='enter <name>.maxref.xml name')
    parser.add_argument('-d', '--dict', action='store_true', help="dump parsed maxref as dict")
    parser.add_argument('-j', '--json', action='store_true', help="dump parsed maxref as json")
    parser.add_argument('-c', '--code', action='store_true', help="dump parsed maxref as code")
    if HAVE_YAML:
        parser.add_argument('-y', '--yaml', action='store_true', help="dump parsed maxref as yaml")
    parser.add_argument('-l', '--list', action='store_true', help="list all objects")

    args = parser.parse_args()
    p = MaxRefParser(args.name)
    p.parse()
    if args.list:
        for name in sorted(p.refdict.keys()):
            print(name)
    if args.dict:
         p.dump_dict()

    if args.code:
        p.dump_code()

    if HAVE_YAML and args.yaml:
        p.dump_yaml()


