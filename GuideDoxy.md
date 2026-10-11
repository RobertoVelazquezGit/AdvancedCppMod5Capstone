# General Doxygen Guide for C++ Projects

This guide summarizes the Doxygen configuration and workflow used so far
for a generic C++ project named `EjemploDoxy`.

> This guide does **not** cover the installation of Doxygen or Graphviz.
> It assumes both tools are already installed and available.

------------------------------------------------------------------------

## 1. Project Structure

The project has a structure similar to:

``` text
EjemploDoxy/
├── README.md
├── Doxyfile
├── main.cpp
├── ExampleClass.cpp
├── ExampleClass.h
└── docs/
    └── html/
```

The important documentation-related files are:

-   `Doxyfile`: Doxygen configuration file.
-   `README.md`: project introduction and main Doxygen page.
-   `docs/html/`: generated HTML documentation.
-   `docs/html/index.html`: entry point for browsing the generated
    documentation.

------------------------------------------------------------------------

## 2. Doxygen Comments Used in the C++ Code

The project uses triple-slash comments:

``` cpp
/// @brief Calculates the population standard deviation of recent log returns
/// @param symbol Asset whose price history is used
/// @param windowSize Maximum number of consecutive returns to include
/// @return Non-annualized volatility
std::optional<double> calculateVolatility(
    const std::string& symbol,
    int windowSize = 30);
```

Doxygen recognizes `///` as a documentation comment.

Some of the tags used in the project are:

-   `@file` - documents a file.
-   `@class` - documents a class.
-   `@brief` - short description.
-   `@details` - more detailed description.
-   `@param` - documents a function parameter.
-   `@return` - documents the return value.
-   `@tparam` - documents a template parameter.
-   `@note` - adds an important note.
-   `@warning` - adds a warning.
-   `@pre` - describes a precondition.
-   `@post` - describes a postcondition.
-   `@code` / `@endcode` - includes a code example.

Example for a template:

``` cpp
/// @brief Calculates the arithmetic mean of a collection of asset returns
/// @tparam PriceContainer Container type holding the return values
/// @param returns Collection of asset returns
/// @param riskFreeRate Reserved risk-free rate
/// @return Arithmetic mean of the returns
template<typename PriceContainer>
double calculateSharpeRatio(
    const PriceContainer& returns,
    double riskFreeRate = 0.02);
```

------------------------------------------------------------------------

## 3. Initial Doxywizard Project Configuration

The working directory was configured as the root directory of the
project.

Example:

``` text
C:\Dev\Projects\EjemploDoxy
```

The basic project information was configured in:

``` text
Wizard -> Project
```

with values similar to:

``` text
Project name:        EjemploDoxy
Project synopsis:    Example C++ project documented with Doxygen
Project version:     1.0
```

The source directory was set to the project directory.

Recursive scanning was enabled so Doxygen can also process files if the
project is later organized into subdirectories such as:

``` text
src/
include/
tests/
```

The destination directory was configured as:

``` text
docs
```

Therefore, the generated HTML documentation is placed under:

``` text
docs/html/
```

------------------------------------------------------------------------

## 4. Documentation Mode

The project was initially configured to document entities that contain
Doxygen documentation.

The source language is C++.

The main output format enabled is:

``` text
HTML
```

LaTeX/PDF output has not been configured at this stage.

------------------------------------------------------------------------

## 5. The Doxyfile

The Doxygen configuration was saved in the project root as:

``` text
Doxyfile
```

This file contains all the configuration required to regenerate the
documentation.

The documentation can therefore be generated without opening Doxywizard:

``` powershell
doxygen Doxyfile
```

The normal workflow is:

``` text
C++ source files
      +
   README.md
      +
   Doxyfile
      |
      v
    Doxygen
      |
      v
 docs/html/
      |
      v
 index.html
```

------------------------------------------------------------------------

## 6. Opening the Generated Documentation

The generated documentation is a static HTML website.

The main entry point is:

``` text
docs/html/index.html
```

On Windows it can be opened directly by double-clicking `index.html`.

It can also be opened from PowerShell:

``` powershell
start .\docs\html\index.html
```

The many HTML, CSS, JavaScript, image and SVG files generated inside
`docs/html/` belong to the documentation website and should normally not
be opened individually.

Navigation should normally start from:

``` text
index.html
```

------------------------------------------------------------------------

## 7. Graphviz / Dot Integration

Graphviz support was enabled in Doxygen so that Doxygen can use the
`dot` program to generate diagrams.

The important Doxygen setting is:

``` text
HAVE_DOT = YES
```

Because `dot` is available through the Windows `PATH`, no explicit
`DOT_PATH` was required.

Conceptually:

``` text
Doxygen
   |
   v
 dot / Graphviz
   |
   v
 diagrams
   |
   v
 HTML documentation
```

------------------------------------------------------------------------

## 8. Class and Relationship Diagrams

Graph-related options were enabled, including the relevant class and
relationship diagrams.

The configuration includes options such as:

``` text
CLASS_DIAGRAMS      = YES
CLASS_GRAPH         = YES
COLLABORATION_GRAPH = YES
INCLUDE_GRAPH       = YES
INCLUDED_BY_GRAPH   = YES
```

These diagrams become more useful as the number of classes and
relationships in a project increases.

------------------------------------------------------------------------

## 9. Include Graphs

Include graphs were enabled.

They allow Doxygen to represent dependencies between source/header
files.

For example, a header may conceptually produce a graph such as:

``` text
ExampleClass.h
        |
        +----> vector
        +----> map
        +----> string
        +----> chrono
        +----> optional
```

The reverse graph can show which files include a particular file.

The relevant options are:

``` text
INCLUDE_GRAPH     = YES
INCLUDED_BY_GRAPH = YES
```

These diagrams are useful in larger C++ projects for understanding
header dependencies.

------------------------------------------------------------------------

## 10. Call Graphs

Call graphs were enabled:

``` text
CALL_GRAPH = YES
```

A call graph answers:

> Which functions does this function call?

For example, in a generic project:

``` text
processData()
     |
     v
 validateData()
```

because `generateEfficientFrontier()` contains a call similar to:

``` cpp
if (validateData()) {
    // Continue processing...
}
```

Another example is:

``` text
runApplication()
       |
       v
 processData()
```

Doxygen displays these relationships in the documentation with a section
such as:

``` text
Here is the call graph for this function:
```

Graphviz generates the graphical representation.

------------------------------------------------------------------------

## 11. Caller Graphs

Caller graphs were also enabled:

``` text
CALLER_GRAPH = YES
```

A caller graph answers the opposite question:

> Which functions call this function?

For example, from the point of view of `optimizePortfolio()`:

``` text
processData()
     |
     v
 validateData()
```

The same relationship is now interpreted as:

``` text
validateData() is called by processData()
```

Therefore:

``` text
Call Graph   -> Who do I call?
Caller Graph -> Who calls me?
```

Both are particularly useful when studying or maintaining a large
codebase.

------------------------------------------------------------------------

## 12. Source Browser

Source-code browsing was enabled:

``` text
SOURCE_BROWSER = YES
```

This allows the generated documentation to provide navigation through
the project's source code.

It is useful for developer-oriented documentation because the
documentation can be used not only to inspect API descriptions but also
to navigate the implementation.

The source browser can be accessed through the generated file
documentation.

------------------------------------------------------------------------

## 13. Private Members

Private entities were enabled with:

``` text
EXTRACT_PRIVATE = YES
```

This means Doxygen can also document private members such as:

``` cpp
std::vector<int> internalData;
std::string configurationPath;
bool initialized;
```

This setting is useful for **internal developer documentation**.

For documentation intended strictly as a public API, private
implementation details would often be hidden instead.

------------------------------------------------------------------------

## 14. Markdown Files

Doxygen is configured to process Markdown files.

The `FILE_PATTERNS` configuration includes:

``` text
*.md
```

`FILE_PATTERNS` should not be confused with:

``` text
FILTER_PATTERNS
```

`FILTER_PATTERNS` is used to associate external filters with particular
file patterns and was not required for the `README.md`.

------------------------------------------------------------------------

## 15. README.md as the Main Page

A file named:

``` text
README.md
```

was created in the root directory of the project.

It contains a project overview with sections such as:

``` markdown
# EjemploDoxy

Example C++ project used to demonstrate source-code documentation with Doxygen.

## Overview

...

## Main Features

...

## Main Classes

- ExampleNamespace::ExampleClass
- ExampleNamespace::AnotherClass

## Requirements

- C++17 or later
- Doxygen
- Graphviz
```

The Doxygen configuration was then changed so this Markdown file becomes
the main documentation page:

``` text
USE_MDFILE_AS_MAINPAGE = README.md
```

After regenerating the documentation:

``` powershell
doxygen Doxyfile
```

the contents of `README.md` appear as the main page in:

``` text
docs/html/index.html
```

------------------------------------------------------------------------

## 16. Automatic Links to C++ Entities

A `README.md` can contain fully qualified class names such as:

``` markdown
## Main Classes

- ExampleNamespace::ExampleClass
- ExampleNamespace::AnotherClass
```

Doxygen recognizes these fully qualified C++ entity names and
automatically converts them into navigable links when it knows the
corresponding classes.

Therefore, it was not necessary to explicitly write:

``` text
@ref
```

for these class names.

An explicit `@ref` can still be useful when custom link text is
required.

For example:

``` text
@ref ExampleNamespace::ExampleClass "example class"
```

------------------------------------------------------------------------

## 17. Current Documentation Workflow

After modifying C++ documentation comments or `README.md`, the
documentation can be regenerated from the project root with:

``` powershell
doxygen Doxyfile
```

Then open or refresh:

``` text
docs/html/index.html
```

The complete workflow is therefore:

``` text
Edit source / README
        |
        v
doxygen Doxyfile
        |
        +----> parses /// comments
        +----> processes README.md
        +----> analyzes C++ entities
        +----> builds source navigation
        +----> invokes Graphviz/dot
        +----> creates call/caller graphs
        +----> creates include diagrams
        |
        v
docs/html/index.html
```

------------------------------------------------------------------------

## 18. Current Relevant Doxygen Settings

The most important settings configured so far are summarized below:

``` text
HTML output               = enabled
RECURSIVE                  = YES
HAVE_DOT                   = YES
CLASS_DIAGRAMS             = YES
CLASS_GRAPH                = YES
COLLABORATION_GRAPH        = YES
INCLUDE_GRAPH              = YES
INCLUDED_BY_GRAPH          = YES
CALL_GRAPH                 = YES
CALLER_GRAPH               = YES
SOURCE_BROWSER             = YES
EXTRACT_PRIVATE            = YES
USE_MDFILE_AS_MAINPAGE     = README.md
FILE_PATTERNS              includes *.md
```

The documentation output directory is:

``` text
docs/html/
```

and its entry point is:

``` text
docs/html/index.html
```

------------------------------------------------------------------------

## 19. Key Idea

The source files remain the primary source of API documentation through
comments such as:

``` cpp
/// @brief ...
/// @param ...
/// @return ...
```

while `README.md` provides the project-level introduction.

The `Doxyfile` defines how everything is processed and Graphviz adds
structural diagrams.

Together:

``` text
README.md                  -> Project-level documentation
C++ /// comments           -> Classes, methods and API documentation
Doxyfile                   -> Documentation configuration
Graphviz                   -> Diagrams
docs/html/index.html       -> Generated documentation website
```
