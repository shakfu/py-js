"""cli: general commandline interface module

Provides a declarative argparse-based class to be inherited
by applications wishing to provide a basic commandline interface.

This is based on my old `argdeclare` code
    see: http://code.activestate.com/recipes/576935-argdeclare-declarative-interface-to-argparse


TODO:
-----

- make fully recursive! 

like haskell:

    sum :: (Num a) => [a] -> a  
    sum [] = 0  
    sum (x:xs) = x + sum' xs  

concept is

    parse [] = 0
    parse (x:xs) = 
        if x in structure:
            structure[x] = options[x]
        else:
            structure[x] = dummy[x]
        structure[x].children.append(xs)
        parse(xs)
"""
import argparse
import sys
import inspect

# ------------------------------------------------------------------------------
# Generic utility functions and classes for commandline ops


# option decorator
def option(*args, **kwds):
    """decorator to provide an argparse option to a method."""
    def _decorator(func):
        _option = (args, kwds)
        if hasattr(func, 'options'):
            func.options.append(_option)
        else:
            func.options = [_option]
        return func
    return _decorator


# class option:
#     def __init__(self, *args, **kwargs):
#         self.args = args
#         self.kwargs = kwargs
#     def __call__(self, func):
#         _option = (self.args, self.kwargs)
#         if hasattr(func, 'options'):
#             func.options.append(_option)
#         else:
#             func.options = [_option]
#         return func




# arg decorator
arg = option


#combines option decorators
def option_group(*options):
    """Collection of options to simplify common options reuse."""
    def _decorator(func):
        for opt in options:
            func = opt(func)
        return func

    return _decorator


# class option_group:
#     def __init__(self, *options):
#         self.options = options
#     def __add__(self, other):
#         to_tuple = lambda d: tuple((k, d[k]) for k in sorted(d.keys()))
#         combined_set = \
#             set((opt.args, to_tuple(opt.kwargs)) for opt in self.options).union(
#             set((opt.args, to_tuple(opt.kwargs)) for opt in other.options))
#         combined_options = tuple(
#             option(*args, **dict(items)) for args, items in combined_set)
#         return option_group(* combined_options)
#     def __call__(self, func):
#         for opt in self.options:
#             func = opt(func)
#         return func


class MetaCommander(type):
    """Metaclass to provide argparse boilerplate features to its instance class"""
    def __new__(cls, classname, bases, classdict):
        subcmds = {}
        for name, func in list(classdict.items()):
            if name.startswith('do_'):
                name = name[3:]
                subcmd = {
                    'name': name,
                    'func': func,
                    'options': [],
                }
                if hasattr(func, 'options'):
                    subcmd['options'] = func.options
                subcmds[name] = subcmd
        classdict['_argparse_subcmds'] = subcmds
        return type.__new__(cls, classname, bases, classdict)



def add_parser(subparsers, subcmd, name=None):
    if not name:
        name = subcmd['name']
    subparser = subparsers.add_parser(
        name,
        help=subcmd['func'].__doc__)

    for args, kwds in subcmd['options']:
        subparser.add_argument(*args, **kwds)
    subparser.set_defaults(func=subcmd['func'])
    return subparser



class Commander(metaclass=MetaCommander):
    """app: description here
    """
    name = 'app name'
    epilog = ''
    version = '0.1'
    default_args = ['--help']
    _argparse_subcmds: dict    # just to silence static checkers
    _argparse_levels: int = 0  # how many subcommand levels to create
    _argparse_structure: dict = {}


    def cmdline(self):
        """Main commandline function to process commandline arguments and options."""
        parser = argparse.ArgumentParser(
            # prog = self.name,
            formatter_class=argparse.ArgumentDefaultsHelpFormatter,
            description=self.__doc__,
            epilog=self.epilog,
        )

        parser.add_argument('-v',
                            '--version',
                            action='version',
                            version='%(prog)s ' + self.version)

        ## default arg
        # parser.add_argument('db', help='arg')
        # parser.add_argument('--db', help='option')

        # non-subcommands here

        subparsers = parser.add_subparsers(
            title='subcommands',
            description='valid subcommands',
            help='additional help',
            metavar='',
        )

        levels = self._argparse_levels
        structure = self._argparse_structure
        for name in sorted(self._argparse_subcmds.keys()): # pylint: disable=E1101
            subcmd = self._argparse_subcmds[name]          # pylint: disable=E1101
            if not levels:
                subparser = add_parser(subparsers, subcmd)
            else:
                head, *tail = name.split('_')
                if head and not tail: # i.e head == name
                    # scenario: single section name and subcmd is given
                    subparser = add_parser(subparsers, subcmd)
                    if head not in structure:
                        structure[head] = subparser.add_subparsers(
                            title=f"{head} subcommands",
                            description=subcmd['func'].__doc__,
                            help='additional help',
                            metavar='',
                        )
                    # else: # head in structure and not tail
                        # scenario where head was given and now the tail is the same
                        # as a previous head.
                        # add_parser(subparsers, subcmd)
                        # add it but don't overwrite the prior head
                else: # (x:xs)
                    if head in structure:
                        _subparsers = structure[head]
                        subparser = add_parser(_subparsers, subcmd, name="_".join(tail))
                    # else:
                    #     # create a dummy 'head' if it wasn't created manually
                    #     subcmd = {
                    #         'name': head,
                    #         'func': lambda self, args: None,
                    #         'options': [],
                    #     }
                    #     subcmd['func'].__doc__ = f"{head} commands"
                    #     subparser = add_parser(subparsers, subcmd, name=head)

                    #     # add it to structure
                    #     structure[head] = subparser.add_subparsers(
                    #         title=f"{head} subcommands",
                    #         description=subcmd['func'].__doc__,
                    #         help='additional help',
                    #         metavar='',
                    #     )

                    #     # add tail as a child to it
                    #     _subparsers = structure[head]
                    #     subparser = add_parser(_subparsers, subcmd, name="_".join(tail))

        if len(sys.argv) <= 1:
            options = parser.parse_args(self.default_args)
        else:
            options = parser.parse_args()
        options.func(self, options)

