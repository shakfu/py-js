#!/usr/bin/env sh
#
# install Leo 6.7.7, main branch, build 9761e97ee3

virtualenv .venv
source .venv/bin/activate
pip install leo==6.7.7
