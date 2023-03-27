# webserv

An external which uses the [mongoose](https://github.com/cesanta/mongoose) embedded webserver and attempts to embed a webserver in a max external.

The mongoose platform was chose because it is quite lightweight and versatile. See [Documentation and Examples](https://mongoose.ws/documentation/).

## Status

This works well as a proof of concept. The webserver can be switched on and off via messages and is run in a thread. It can serve http requests and also serve static files locally. It defaults to the subproject folder and port 3000.

The implementation is based on the max-sdk's `simplethread` example.
