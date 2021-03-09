# Xcode Conceptual Overview

## Project

An Xcode project is a repository for all the files, resources, and information required to build one or more software products. A project contains all the elements used to build your products and maintains the relationships between those elements. It contains one or more targets, which specify how to build products. A project defines default build settings for all the targets in the project (each target can also specify its own build settings, which override the project build settings).

An Xcode project file contains the following information:

References to source files:
	- Source code, including header files and implementation files
	- Libraries and frameworks, internal and external
	- Resource files
	- Image files
	- Interface Builder (nib) files

- Groups used to organize the source files in the structure navigator

- Project-level build configurations. You can specify more than one build configuration for a project; for example, you might have debug and release build settings for a project.

- Targets, where each target specifies:
	- A reference to one product built by the project
	- References to the source files needed to build that product
	- The build configurations that can be used to build that product, including dependencies on other targets and other settings; the project-level build settings are used when the targets’ build configurations do not override them

- The executable environments that can be used to debug or test the program, where each executable environment specifies:
	- What executable to launch when you run or debug from Xcode
	- Command-line arguments to be passed to the executable, if any
	- Environmental variables to be set when the program runs, if any

A project can stand alone or can be included in a workspace.

You use Xcode schemes to specify which target, build configuration, and executable configuration is active at a given time.




## Target

A target specifies a product to build and contains the instructions for building the product from a set of files in a project or workspace. A target defines a single product; it organizes the inputs into the build system—the source files and instructions for processing those source files—required to build that product. Projects can contain one or more targets, each of which produces one product.

The instructions for building a product take the form of build settings and build phases, which you can examine and edit in the Xcode project editor. A target inherits the project build settings, but you can override any of the project settings by specifying different settings at the target level. There can be only one active target at a time; the Xcode scheme specifies the active target.

A target and the product it creates can be related to another target. If a target requires the output of another target in order to build, the first target is said to depend upon the second. If both targets are in the same workspace, Xcode can discover the dependency, in which case it builds the products in the required order. Such a relationship is referred to as an implicit dependency. You can also specify explicit target dependencies in your build settings, and you can specify that two targets that Xcode might expect to have an implicit dependency are actually not dependent. For example, you might build both a library and an application that links against that library in the same workspace. Xcode can discover this relationship and automatically build the library first. However, if you actually want to link against a version of the library other than the one built in the workspace, you can create an explicit dependency in your build settings, which overrides this implicit dependency.


## Workspace

A workspace is an Xcode document that groups projects and other documents so you can work on them together. A workspace can contain any number of Xcode projects, plus any other files you want to include. In addition to organizing all the files in each Xcode project, a workspace provides implicit and explicit relationships among the included projects and their targets.

Workspaces Extend the Scope of Your Workflow
A project file contains pointers to all the files in the project, along with build configurations and other project information. In Xcode 3 and earlier, the project file is always the root of the group and file structure hierarchy. Although a project can contain references to other projects, working on interrelated projects in Xcode 3 is complicated; most workflows are confined to a single project. In Xcode 4 and later, you have the option of creating a workspace to hold one or more projects, plus any other files you wish to include.

In addition to providing access to all the files in each included Xcode project, a workspace extends the scope for many important Xcode workflows. For example, because indexing is done across the whole workspace, code completion, Jump to Definition, and all other content-aware features work seamlessly through all projects in the workspace. Because refactoring operations act across all the content of the workspace, you can refactor the API in a framework project and in several application projects that use that framework all in one operation. When building, one project can make use of the products of other projects in the workspace.

The workspace document contains pointers to the included projects and other files, but no other data. A project can belong to more than one workspace. The figure shows a workspace containing two Xcode projects (Sketch and TextEdit) plus a documentation project (Xcode4TransGuideDocPlan).

## Scheme

An Xcode scheme defines a collection of targets to build, a configuration to use when building, and a collection of tests to execute.

You can have as many schemes as you want, but only one can be active at a time. You can specify whether a scheme should be stored in a project—in which case it’s available in every workspace that includes that project, or in the workspace—in which case it’s available only in that workspace. When you select an active scheme, you also select a run destination (that is, the architecture of the hardware for which the products are built).

## Build Settings

A build setting is a variable that contains information about how a particular aspect of a product’s build process should be performed. For example, the information in a build setting can specify which options Xcode passes to the compiler.

You can specify build settings at the project or target level. Each project-level build setting applies to all targets in the project unless explicitly overridden by the build settings for a specific target.

Each target organizes the source files needed to build one product. A build configuration specifies a set of build settings used to build a target's product in a particular way. For example, it is common to have separate build configurations for debug and release builds of a product.

A build setting in Xcode has two parts: the setting title and the definition. The build setting title identifies the build setting and can be used within other settings. The build setting definition is a constant or a formula Xcode uses to determine the value of the build setting at build time. A build setting may also have a display name, which is used to display the build setting in the Xcode user interface.

In addition to the default build settings provided by Xcode when you create a new project from a project template, you can create user-defined build settings for your project or for a particular target. You can also specify conditional build settings. The value of a conditional build setting depends on whether one or more prerequisites are met. This mechanism allows you to, for example, specify the SDK to use to build a product based on the targeted architecture.

