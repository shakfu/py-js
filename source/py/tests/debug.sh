
clang -g -std=c99 -ggdb3 -O0 -pedantic-errors -Wall -Wextra \
      -fpie $(python3.8d-config --cflags --embed) -o $1 \
      $1.c $(python3.8d-config --embed --ldflags)


