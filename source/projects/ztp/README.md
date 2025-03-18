# ztp: the zeromq + threads + python Max external

```text
┌─────────────────────────────────────────────────┐
│┌───────────┐                       ┌───────────┐│
││           │                       │python code││
││  client   │◀────────zeromq───────▶│  server   ││
││           │                       │ (spawned) ││
│└───────────┘                       └───────────┘│
│                   ztp external                  │
└─────────────────────────────────────────────────┘
```

The `ztp` external (the name refers to python, threads and zeromq) uses `zmq` with threading for non-blocking communication with a spawned python code interpretation server which evaluates and executes python code and sends back the result.

The combination of threads, zeromq and remote process mgmt of the spawned server make this much more usable than `zpy`, an earlier effort which lacked threads and which suffered from blocking during communication with the server.

## Requires

```bash
brew install zmq
```

## Learnings

- This works: one thread per zmq socket as per the zmq rules.
