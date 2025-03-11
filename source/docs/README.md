# About this section

This is documentation section intended to be used to consolidate and organize all project documentation and notes into a user/dev guide.

The 'guide' is being developed using [quarto](https://quarto.org), an open-source scientific and technical publishing system and [leo-editor](https://leo-editor.github.io/leo-editor/) a programmable outline-based editor with literate programming features.

## Outline editing

The outline of the book can be edited in outline by [leo-editor](https://leo-editor.github.io/leo-editor), the `workbook.leo` document just contains references to the documents and does not contain them.

Leo `v6.7.7` is used in this case.


## Idiosyncrasies

Quarto can fail:

- if a bibliographic reference is not used!

```latex
updating existing packages
ERROR:
compilation failed- error
LaTeX Error: Something's wrong--perhaps a missing \item.

See the LaTeX manual or LaTeX Companion for explanation.
Type  H <return>  for immediate help.
 ...

l.1806 \end{CSLReferences}
```
