\section tcldeffile  Tcl Definition file.

The frib::analysis::TCLParameterReader class can read tree parameter and
tree variable definitions that are provided in the syntax of the Tool command
language, or Tcl.

Describing the syntax of Tcl is beyond the scope of this document.  The Tcl/Tk
team provide references to [numerous resources](https://www.tcl.tk/doc/) to help
you with that.   One distinguishing feature of Tcl is that the language is
easily extended.   This document will describe the extenaions created for
the interpreter used by frib::analysis::TCLParmaeterReader to support the
defnition of tree parameters, tree parameter arrays, tree variables
and treevariable arrays.

For information about the use of frib::analysis::TCLParameterReader simply follow
one of the links on this page to that class's reference material.

\subsection treeparameter  The treeparameter command

The tree parameter command creates a single paramter. It has the following form:

    treeparameter name low high bins units
    
Where:

| parameter | usage |
|-----------|-------|
| name | Is the name of the tree parameter to create |
| low  | Recommended low limit for the parameter |
| high | Recommended  high limit for this parameter |
| bins | Recommended number of bins between *low* and *high* |
| units| Units of measure for the parameter  |

\subsection treeparamarray The treeparameterarray command.

The treeparameterarray command creates an array of treeparameters.
The form of the treeparameterarray command is:

    treeparameterarray name low high bins units elements firstindex
    
Where:

| parameter | usage |
|-----------|-------|
| name      | The basename of the parameter array.  The element names will have a "." and index tacked on |
| low  | Recommended low limit for the parameter |
| high | Recommended  high limit for this parameter |
| bins | Recommended number of bins between *low* and *high* |
| units| Units of measure for the parameter  |
| elements | The number of elements in the array |
| firstindex | The index of the first element of the array |

\subsection treevariable The treevariable command:

The treevariable command creates a single treevariable.  Its form it:

    treevariable name value units
    
where:

| parameter | usage |
|-----------|-------|
| name      | Is the name of the tree variable that will be created |
| value     | Is the value that will be given to the variable. |
| units     | are the units of measure of the variable |


\subsection treevariablearray The treevariablearray command

The treevariablearray command creates an array of tree variables.   The form of
this command is:

    treevariablearray name value units elements firstindex
    
Where:

|  parameter | usage |
|------------|-------|
| name       | base name of the array.  Elements will have a period and index number appended to the name |
| value     | Is the value that will be given to the variable. |
| units     | are the units of measure of the variable |
| elements   | Number of elements in the array |
| firstindex | The index of the first element in the array.




