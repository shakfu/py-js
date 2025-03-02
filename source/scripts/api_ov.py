#!/usr/bin/env python3

# api overview script provides a high level overview of api.pyx module

import os
import re

API_PYX = 'source/projects/py/api.pyx'

# re patterns

class_header = re.compile(f'^cdef class (.+):')
class_c_method = re.compile(r'^    cdef (.+)\((.+)\).*:')
class_p_method = re.compile(r'^    def (.+)\((.+)\).*:')

# class_c_method = re.compile(r'^    cdef (.+) (.+)\((.+)\).*:')
# class_p_method = re.compile(r'^    def (.+) (.+)\((.+)\).*:')

# class_c_method = re.compile(r'    cdef (.+) (.+)\(self, (.+)\).*:')
# class_p_method = re.compile(r'    def (.+)\(self, (.+)\).*:')


with open(API_PYX) as f:
	lines = f.readlines()

# os.system(f'cloc {API_PYX}')
spacer = ' '*4

def handle_class(line):
	m = class_header.match(line)
	if m:
		print()
		print(line.strip())

def handle_c_method(line):
	m = class_c_method.match(line)
	if m:
		print(spacer, line.strip()[:-1])

def handle_p_method(line):
	m = class_p_method.match(line)
	if m:
		print(spacer, line.strip()[:-1])

for line in lines:
	handle_class(line)
	handle_c_method(line)
	handle_p_method(line)
