# webserv

An external which uses the [mongoose](https://github.com/cesanta/mongoose) embedded webserver and attempts to embed a webserver in a max external.

The mongoose platform was chosen because it is quite lightweight and versatile. See [Documentation and Examples](https://mongoose.ws/documentation/).

## Status

This works well as a proof of concept for a webserver in an external running in its own thread.

The webserver can be switched on and off via messages. It can serve http requests and also serve static files locally. It defaults to the subproject folder and port 8000.

The implementation is based on the max-sdk's `simplethread` example.

Currently, the webserver serves a basic react app with a python code-editor (monaco editor which is used in VS Code)

## TODO

- feed functions calls into a queue to prevent blocking which currently occurs with object creation tests.

- figure out how to do two-way communication between editor and external.

