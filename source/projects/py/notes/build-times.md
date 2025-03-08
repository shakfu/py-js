# Build Times


These were tested by prefixing `time` before  `make <target>`, so for example:

```sh
time make shared-ext
```

## python 3.12 / macbook air m1

```sh
make homebrew-ext    2.22s    user 1.80s  system 61% cpu 6.561    total
make homebrew-pkg    2.26s    user 3.50s  system 68% cpu 8.443    total
make default         3.58s    user 0.84s  system 66% cpu 6.623    total
make framework-ext   736.81s  user 70.47s system 98% cpu 13:35.52 total
make framework-pkg   758.27s  user 78.94s system 99% cpu 14:05.06 total
make shared-ext  	 757.44s  user 72.90s system 98% cpu 13:59.00 total
make shared-pkg      771.23s  user 73.14s system 98% cpu 14:15.28 total
make static-tiny-ext 878.85s  user 67.33s system 88% cpu 17:46.19 total
make static-ext  	 1061.85s user 75.18s system 88% cpu 21:20.65 total
```

## python 3.13.2 / macbook air m1 / with build optimizations

After optimizing build options, like dropping LTO (link-time-optimization),
the builds are significantly faster:

```sh
make homebrew-pkg     1.84s  user 1.67s  system 45% cpu 7.684   total
make homebrew-ext     2.10s  user 1.54s  system 45% cpu 7.953   total
make default          3.98s  user 0.57s  system 55% cpu 8.211   total
make shared-tiny-ext  81.20s user 20.14s system 92% cpu 1:49.10 total
make static-tiny-ext  81.21s user 20.43s system 91% cpu 1:51.13 total
make static-ext       82.07s user 20.42s system 91% cpu 1:52.58 total
make shared-pkg  	  86.41s user 22.07s system 89% cpu 2:01.26 total
make shared-ext  	  86.62s user 22.69s system 83% cpu 2:10.68 total
make framework-pkg    88.15s user 22.51s system 89% cpu 2:03.16 total
make framework-ext    88.20s user 22.58s system 87% cpu 2:06.27 total
```

Some build sizes for `*-ext` variants:

```sh
shared-tiny-ext-pyjs 	 8.4 MB
static-tiny-ext-pyjs  	 8.5 MB
static-ext-pyjs 		12.0 MB
homebrew-ext-pyjs   	15.4 MB
shared-ext-pyjs     	16.6 MB
framework-ext-pyjs		19.3 MB

shared-tiny-ext-py 		11.3 MB
static-tiny-ext-py  	11.4 MB
static-ext-py 			14.9 MB
homebrew-ext-py 		18.4 MB
shared-ext-py 			19.6 MB
framework-ext-py 		22.2 MB
```


