
clang -g -std=c99 -ggdb3 -O0 -pedantic-errors -Wall -Wextra \
      -fpie $(python3.9-config --cflags --embed) -o $1 \
      $1.c $(python3.9-config --embed --ldflags)


