# jmx: jupyter client for Max

This is a proof-of-concept subproject to build a [jupyter client](https://jupyter-client.readthedocs.io/en/stable/messaging.html) as a Max/MSP external.

## Implementation Notes

There's an insidious linker error that emerges at attempts to use `jsonwriter` and the below post sugests having to link the `commonsys.c` in the sdk.

see: https://cycling74.com/forums/linker-error

Also according to the above forum post, one must include `common_symbols_init();` in the `ext_main()` method.

## Jupyter Message Protocol

### Header

```python
{
    'msg_id' : str, # typically UUID, must be unique per message
    'session' : str, # typically UUID, should be unique per session
    'username' : str,
    # ISO 8601 timestamp for when the message is created
    'date': str,
    # All recognized message type strings are listed below.
    'msg_type' : str,
    # the message protocol version
    'version' : '5.0',
}
```

> The session id in a message header identifies a unique entity with state, such as a kernel process or client process.

> A client session id, in message headers from a client, should be unique among all clients connected to a kernel. When a client reconnects to a kernel, it should use the same client session id in its message headers. When a client restarts, it should generate a new client session id.

