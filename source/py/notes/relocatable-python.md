# Relocatable Python

Some basic links to capture research threads so far on relocatable python:

## py2app

py2app seems like the most promising solution but i haven't seen any embedded python examples so far which fit the max/msp model.

Although it definitely look like it deal with the common use cases, I'm not sure it can deal with our particular requirements.

see: https://github.com/ronaldoussoren/py2app/issues/303

## Python Bug Report asking for greater support for relocatable python

- https://bugs.python.org/issue18309
- https://bugs.python.org/issue42514

Also python-packagin discussion forums:

- https://discuss.python.org/c/packaging

## Repurposing Homebrew Python

This blog post by Joao Ventura, made this project possible since it provided a method to re=use the well-package homebrew python easily.

- https://joaoventura.net/blog/2016/embeddable-python-osx/

## Building Static Python

Best methods so far by [Beeware](https://github.com/beeware)

Specifically:

- https://github.com/beeware/Python-Apple-support
- https://github.com/beeware/briefcase

I've learned a whole lot from their Python-Apple-support project which has been a incredibly useful for self-contained static externals.

## Homebrew Python

This ruby script builds Python for Homebrew packaging. Incredibly interesting and overwhelming.

- https://github.com/Homebrew/homebrew-core/blob/HEAD/Formula/python@3.9.rb

## Making Python (Frameworks) relocatable

[Greg Neagle](https://github.com/gregneagle) is focused on the the specific case of making python relocatable and has a great github project to accomplish this in a straightforward way. It only works for Python.frameworks I think right now..

- https://github.com/gregneagle/relocatable-python
- https://t-lark.github.io/posts/shipping-python/

Another related github project by Infinidata also seems to target the same as well. Must investigate further.

- https://github.com/Infinidat/relocatable-python3

It uses zc.buildout, installs modules without asking permission,
loads custom dependencies from a commercial website.. Not my cup of tea.

## PyInstaller and Py2App and macholib

- https://github.com/ronaldoussoren/macholib
- https://macholib.readthedocs.io/en/latest/

## General Python Packaging topics

- https://stackoverflow.com/questions/16018463/difference-in-details-between-make-install-and-make-altinstall

## Python-shared Relocatablity Blues

Are shared by others:

- https://cd-docdb.fnal.gov/0051/005182/001/relocatable-python.pdf

## @rpath, @loader_path, @exectuable_path

- https://medium.com/@donblas/fun-with-rpath-otool-and-install-name-tool-e3e41ae86172
- https://wincent.com/wiki/@executable_path,_@load_path_and_@rpath
- https://blog.krzyzanowskim.com/2018/12/05/rpath-what/

- https://stackoverflow.com/questions/1517614/using-otool-recursively-to-find-shared-libraries-needed-by-an-app

- https://lessons.livecode.com/m/4071/l/15029-linking-an-osx-external-bundle-with-a-dylib-library

- https://stackoverflow.com/questions/14888668/how-to-set-dyld-library-path-in-xcode

- https://www.mikeash.com/pyblog/friday-qa-2009-11-06-linking-and-install-names.html
