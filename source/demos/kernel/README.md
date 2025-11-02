# kernel

This project attempts to embed an Jupyter kernel inside a Max external. This will facilitate remote terminal interaction with Max, such that user can send the external object a messge via a Jupyter terminal as it was a Max message.

It does this by linking to the following:

- [xeus](https://github.com/jupyter-xeus/xeus?tab=readme-ov-file)
- [json](https://github.com/nlohmann/json)

## Building


It can be build via:


```sh
make kernel
```

