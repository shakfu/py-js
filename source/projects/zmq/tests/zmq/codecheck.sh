#!/usr/bin/env bash

infer run -- clang -I/usr/local/include -L/usr/local/lib `python3-config --cflags --ldflags` -lpython3.9 -lczmq -lzmq -o server test_pzmq_server.c
infer run -- clang -I/usr/local/include -L/usr/local/lib `python3-config --cflags --ldflags` -lpython3.9 -lczmq -lzmq -o client test_pzmq_client.c

