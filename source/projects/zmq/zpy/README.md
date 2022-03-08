# zpy

A max external which uses zmq to connect to a separate python process



## Status

- [ ] rudimentary proof-of-concept




## TODO

- how to launch python server automatically and close it with the patch


## Alternatives

- [ ] run as subprocess and read and write from stdin and stdout via a pipe

```bash
$ echo 'print(1+1)' | python3 -
2
```


## Research

- https://developer.apple.com/library/archive/documentation/MacOSX/Conceptual/BPSystemStartup/Chapters/CreatingLaunchdJobs.html

- https://github.com/sheredom/subprocess.h
