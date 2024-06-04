# mpy: micropython external

A proof-of-concept which embeds micropython in a max external

Currently only tested on macOS.

Can be built using `-DBUILD_MICROPYTHON_EXTERNAL` which will default to local micropython code. If the option `-DFETCH_MICROPYTHON` is used cmake will try to build from git clone.

Doesn't do anything now useful except prove that embedding micropytho in an external is possible.. Still a work-in-progress and will be developed further as one familiarizes oneself with the micropython c-api.

