# cobra: an ITM-based python evaluator

This project is an experimental attempt to defer the evaluation of a python function via Max's ITM-based sequencing.

Note that it has a dependency on another subproject: it includes mamba's single header c library, `py.h`, to reduce boilerplate and provide python interpreter 'services'.                                               

## Building

From the root of the `py-js` project

```bash
make projects
```

This will build all subprojects, including `cobra`, using the standard cmake buildsystem.

## Help

See `cobra.maxhelp` in `py-js/help` folder.
