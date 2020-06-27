# Cython Notes


## Extension-reload problem

How to prevent reloads of extensions before application is restarted?

Intuitively, it would involve some persistent flag (existence of a file for example) that is is set after a successful import of an extension occurs. If the file 

```python

if application restarted:
	allow_extension_imports()
else:
	block_extension_imports()


Maybe a tempfile!?
```



## Relationship between .pyx and .pxd files

1. If the .pxd file is the same name as the pyx file, cython implicitly includes all names into the pyx file from the pxd. This means you cannot redefine the c-names

2. If the pxd is not named as the pyx file then all references the c-name have to be qualified but then it is possible to redefine the c-name in python 'def'

Note: with (2), you have to close Max to reload c api otherwise it will read as None. (1) needs to be tested for the same behaviour


## Why Extension Types

From the cython docs:

There are two kinds of function definitions in Cython:

1. Python functions are defined using the def statement, as in Python. 
   They take Python objects as parameters and return Python objects.

2. C functions are defined using the new cdef statement.
   They take either Python objects or C values as parameters, and can
   return either Python objects or C values.

Because of this limitation, you can use extension types to wrap arbitrary
C data structures and provide a Python-like interface to them.