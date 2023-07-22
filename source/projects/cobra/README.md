# cobra: an ITM-based python evaluator

This project is an experimental attempt to defer the evaluation of a python function via Max's ITM-based sequencing.

It re-uses the mamba single header to reduce boilerplate

## Building

From the root of the `py-js` project

```bash
make projects
```

This will build all subprojects, including `cobra`, using the standard cmake build process.

## Help

See `cobra.maxhelp` in `py-js/help` folder.
