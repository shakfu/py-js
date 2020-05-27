# Packaging notes

see: https://docs.cycling74.com/max7/vignettes/packages

You may create your own packages, either for your own use or for distribution to others. The folders (ending with a slash) and files comprising a package may optionally include the following (items in folders marked with a star will automatically be included in the searchpath):

```
* clippings/            Patchers to list in the "Paste From..." contextual
                        menu when patching

* code/                 Gen patchers

  collections/          Collections to list in the File Browser that are 
                        associated with the package

  default-definitions/  Definition info for Object Defaults support in UI externals

  default-settings/     Saved color schemes for Object Defaults

* docs/                 Reference pages and Vignettes to be accessible from
                        the Documentation Window

* examples/             Example patchers and supporting material

* extensions/           Special external objects loaded on Max launch

* externals/            External objects

* extras/               Patchers to be listed in the "Extras" menu

* help/                 Help patchers and supporting material

  icon.png              A PNG graphic file (500x500px) for display in 
                        the Package Manager

  init/                 Text files interpreted by Max at launch

  interfaces/           Supporting files for objects to display in the top
                        patcher toolbar and other Max integration.

* java-classes/         Compiled Java classes for use in mxj/mxj~. Place .jar 
                        folders in a 'lib' subfolder.

  java-doc/             Documentation for Java classes

* javascript/           Javascript files to be used by js

* jsextensions/         Extensions to JS implemented as special externals or js files

* jsui/                 Javascript files to be used by jsui, and listed in 
                        the contextual menu for jsui

  license.txt|md        Terms of use / redistribution of your package
                        (plain text or Markdown permitted)

* media/                Media files to be included in the searchpath

* misc/                 Anything

* patchers/             Patchers or abstractions to be included in the searchpath

* object-icons/         An SVG-format object icon for a particular Max object
                        (named <objectname>.svg), used in the Object Browser

  object-prototypes/    Object Prototypes will be listed in the context menu for 
                        a selected UI object

  readme.txt|md         Information about your package (text or Markdown permitted)

  snippets/             Snippets associated with this package

  source/               Source code for external objects, ignored by Max

  support/              Special location for DLL or dylib dependencies of 
                        external objects. Added to the DLL search path on Windows.

  templates/            Patchers to be listed in the "File > New From Template" menu

```

## Directory Structure

- py
    - clippings
    - docs
        - refpages (.maxref.xml)
    - externals (.mxo)
    - extras
    - fonts (.otf)
    - help (.maxhelp)
    - interfaces (svg)
    - jsextensions (.mxo / .js)
    - jsui (.js)
    - media (.png/...)
    - object-icons (.svg)
    - object-prototypes (.maxproto)
    - patchers
    - snippets (.maxsnip)
    - styles (.maxstyle)
    - templates

