# maxtests: automated tests for max externals

This folder includes files which are required for maxtest test runner to work.

For more information please see https://github.com/Cycling74/max-test


## Setup

Automatic test running is disabled by defaults (since it opens up ports on Max).

To enable it

```bash

$ cp max-test-config-example.json max-test-config.json

```

Feel free to change the default ports as required.


## Building

To build the oscar maxtest runner locally (the source is included) run the following:

```
$ make cmake

```


## Running

Once this is done, you can run automated maxtests by the entering the following from the project root:

```bash

$ make maxtests

```

