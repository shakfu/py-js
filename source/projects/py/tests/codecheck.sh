#!/usr/bin/env bash

infer run -- gcc -g `python3-config --cflags --ldflags` -lpython3.9 $1 -o $(basename $1 .c)
