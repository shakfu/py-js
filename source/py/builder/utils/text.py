"""General Text class."""
import os
import re
import string
import time
from datetime import datetime


class Text(str):
    """String subclass with color and code manipulation functionality."""

    WHITELIST = string.ascii_letters + string.digits + '-/. '
    UPPERCASE = string.ascii_uppercase
    ATTRIBUTES = dict(bold=1, underline=4)
    COLORS = dict(red=31, green=32, yellow=33, blue=34,
                  magenta=35, cyan=36, white=37)
    RESET = '\033[0m'
    TIMESTAMP_FMT = '%Y%m%d%H%M%S'
    # TIMESTAMP_FMT = '%Y-%m-%d %H:%M:%S'
    _mixed_to_under_re = re.compile(r'[A-Z]+')
    _under_to_mixed_re = re.compile('_.')

    @classmethod
    def wrap(cls, value: str = None) -> 'Text':
        """Easy wrapping of strings in Text class functionality."""
        if value:
            return cls(value)
        return Text()

    @classmethod
    def timestamp(cls) -> 'Text':
        """Generate a file-friendly timestamp."""
        event = time.time()
        tstamp = datetime.fromtimestamp(event).strftime(cls.TIMESTAMP_FMT)
        return Text(tstamp)

    @property
    def quote(self) -> 'Text':
        """Add quotation marks from double quoted text.

        >>> Text('hello').quote
        '"hello"'
        """
        txt = self
        if any(self == s for s in ['"', "'"]):
            return Text('')
        if '"' in txt:
            txt = txt.unquote
        return Text('"{}"'.format(txt))

    @property
    def unquote(self) -> 'Text':
        """Remove quotation marks from double quoted text.

        >>> Text('"hello"').unquote
        'hello'
        """
        return Text(self.replace('"', ''))

    def colored(self, color: str = None, attrs: [str] = None) -> 'Text':
        """Colorize text.

        Available text colors:
            red, green, yellow, blue, magenta, cyan, white.

        Available attributes:
            bold, dark, underline, blink, reverse, concealed.

        Example:
            colored('Hello, World!', 'red', ['blue', 'blink'])
            colored('Hello, World!', 'green')

        """
        text = str(self)
        if os.getenv('ANSI_COLORS_DISABLED') is None:
            fmt_str = '\033[%dm%s'
            if color is not None:
                text = fmt_str % (self.COLORS[color], text)

            if attrs is not None:
                for attr in attrs:
                    text = fmt_str % (self.ATTRIBUTES[attr], text)

            text += self.RESET
        return Text(text)

    def indent(self, n_spaces: int = 4) -> 'Text':
        """Indents text by specified number of spaces.

        >>> Text('s').indent()
        '    s'
        """
        return Text(((n_spaces * ' ') + str(self)))

    def clean(self) -> 'Text':
        """Removes all characters not in WHITELIST.

        >>> Text('Hello!World').clean()
        'HelloWorld'
        """
        return Text(''.join([i for i in self if i in self.WHITELIST]))

    def plural(self) -> 'Text':
        """Pluralizes words.

        >>> Text('activity').plural()
        'activities'

        >>> Text('crisis').plural()
        'crises'
        """
        aberrant = {
            'knife': 'knives',
            'self': 'selves',
            'elf': 'elves',
            'life': 'lives',
            'hoof': 'hooves',
            'leaf': 'leaves',
            'echo': 'echoes',
            'embargo': 'embargoes',
            'hero': 'heroes',
            'potato': 'potatoes',
            'tomato': 'tomatoes',
            'torpedo': 'torpedoes',
            'veto': 'vetoes',
            'child': 'children',
            'woman': 'women',
            'man': 'men',
            'person': 'people',
            'goose': 'geese',
            'mouse': 'mice',
            'barracks': 'barracks',
            'deer': 'deer',
            'nucleus': 'nuclei',
            'syllabus': 'syllabi',
            'focus': 'foci',
            'fungus': 'fungi',
            'cactus': 'cacti',
            'phenomenon': 'phenomena',
            'index': 'indices',
            'appendix': 'appendices',
            'criterion': 'criteria',
        }
        text = str(self)
        if text in aberrant:
            result = '%s' % aberrant[text]
        else:
            postfix = 's'
            if len(text) > 2:
                vowels = 'aeiou'
                if text[-2:] in ('ch', 'sh'):
                    postfix = 'es'
                elif text[-1:] == 'y':
                    if (text[-2:-1] in vowels) or (text[0] in self.UPPERCASE):
                        postfix = 's'
                    else:
                        postfix = 'ies'
                        text = text[:-1]
                elif text[-2:] == 'is':
                    postfix = 'es'
                    text = text[:-2]
                elif text[-1:] in ('s', 'z', 'x'):
                    postfix = 'es'

            result = '%s%s' % (text, postfix)
        return Text(result)

    def abbreviate(self) -> 'Text':
        """Converts sentence to abbreviation.

        >>> Text('Hello World').abbreviate()
        'HW'
        """
        return "".join(word[0] for word in self.split())

    def fullstrip(self) -> 'Text':
        """Strips both ends of text.

        >>> Text(' Hello World ').fullstrip()
        'Hello World'
        """
        return Text(self.lstrip().strip())

    def word_to_under(self) -> 'Text':
        """Converts uppercase to lowercase and whitespace to underscore.

        >>> Text('Hello World').word_to_under()
        'hello_world'
        """
        return Text('_'.join(self.split()).lower())

    def mixed_to_under(self) -> 'Text':
        """Converts class form to underscore form.

        Sample:
            >>> Text('FooBarBaz').mixed_to_under()
            'foo_bar_baz'

            >>> Text('IDFromHere').mixed_to_under()
            'id_from_here'

        Special case for ID:
            >>> Text("FooBarID").mixed_to_under()
            'foo_bar_id'
        """
        if self.endswith('ID'):
            return Text(self[:-2] + "_id").mixed_to_under()

        def _mixed_to_under_sub(match):
            """Utility function."""
            txt = match.group(0).lower()
            if len(txt) > 1:
                return '_%s_%s' % (txt[:-1], txt[-1])
            return '_%s' % txt
        trans = self._mixed_to_under_re.sub(_mixed_to_under_sub, self)
        if trans.startswith('_'):
            trans = trans[1:]
        return Text(trans)

    def under_to_mixed(self) -> 'Text':
        """Converts text from underscore form to camelcase.

        >>> Text('some_large_name_perhaps').under_to_mixed()
        'someLargeNamePerhaps'

        >>> Text('exception_for_id').under_to_mixed()
        'exceptionForID'
        """
        if self.endswith('_id'):
            return Text(self[:-3] + "ID").under_to_mixed()
        return Text(self._under_to_mixed_re.sub(
            lambda m: m.group(0)[1].upper(), self))

    @property
    def camelcase(self) -> 'Text':
        """Synonym for under_to_mixed for use in templates.

        >>> Text('hello_world').camelcase
        'helloWorld'
        """
        return self.under_to_mixed()

    def mixed_to_word(self) -> 'Text':
        """Converts class form to normal lower word form.

        >>> Text('FooBarBaz').mixed_to_word()
        'foo bar baz'
        """
        return Text(self.mixed_to_under().replace('_', ' '))

    def capword(self) -> 'Text':
        """Makes first letter capitalized.

        >>> Text('foo').capword()
        'Foo'
        """
        return Text(self[0].upper() + self[1:])

    def lowerword(self) -> 'Text':
        """Makes first letter lower case.

        >>> Text('Hello').lowerword()
        'hello'
        """
        return Text(self[0].lower() + self[1:])

    def startswith_any(self, values: [str]) -> bool:
        """Checks if lowered self startswith any values.

        >>> Text('Hello').startswith_any(['hello', 'bye'])
        True
        """
        return any([self.lower().startswith(value) for value in values])

    @property
    def classname(self) -> 'Text':
        """Converts text to class form.

        >>> Text('foo_bar_baz').classname
        'FooBarBaz'
        """
        return self.under_to_mixed().capword()

    def under_to_all_caps(self) -> 'Text':
        """Converts text to capitalized form without underscores.

        >>> Text('foo_bar_baz').under_to_all_caps()
        'Foo Bar Baz'
        """
        return Text(' '.join(x.title() for x in self.split('_')))

    def strip_id(self) -> 'Text':
        """Strips '_id' from the end of the string.

        >>> Text('foo_id').strip_id()
        'foo'
        """
        return Text(self[:-3])
