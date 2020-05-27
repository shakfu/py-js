# Cython Notes


## Relationship between .pyx and .pxd files

1. If the .pxd file is the same name as the pyx file, cython implicitly includes all names into the pyx file from the pxd. This means you cannot redefine the c-names

2. If the pxd is not named as the pyx file then all references the c-name have to be qualified but then it is possible to redefine the c-name in python 'def'


Note: with (2), you have to close Max to reload c api otherwise it will read as None. (1) needs to be tested for the same behaviour