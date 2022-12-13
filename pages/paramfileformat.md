\section paramfileformat   Parameter file format.

This section describes the format of the parameter output file.  The
parameter output file is built in a way that supports low over head reconstruction
of the parameters in each event.

The format recognizes that the parameter ids
of the program recovering data may differ from those of the same name in the
program that created the data.  Therefore sufficient information is provided to
construct mappings between the two so that parameters named the same in the producing
and consuming program have the same meaning.

Furthermore, the output file recognizes that knowing the values of the steering
variables that produced the parameter file is useful.

Refer to AnalysisRingItems.h when reading this section.  It defines the structs
that are written to file.

The file, as with all NSCLDAQ event files is written as a stream of ring items.
A ring item has a header (frib::analysis::RingItemHeader)
that, in the case of files written by this framework
contains the following fields:

| name   | type |  meaning |
|--------|------|----------|
| s_size | std::uint32_t | Size of the entire item in bytes (self inclusive) |
| s_type | std::uint32_t | Ring Item type \[1\] |
| s_unused | std::uint32_t | Constant value of sizeof(std::uint32_t) \[2\] |

Notes:

1.      The ring item types written by the framework are distinct from those
written by NSCLDAQ.  They live in the user item type space (above 32767).
Note that input ring items that are not **PHYSICS_EVENT** items are considered
passthrough items and are pushed by the dealer directly to the ouputtter.
2.      In NSCLDAQ, this field would be the start of the body header or an indicator
that there is no body header.  The framework does not produce body headers, therefore,
the value is the size of `std::uint32_t` which, in NSCLDAQ-12 indicates no body header.
This field allows body finding software in e.g. NSCLDAQ to consistently  locate the
body of the ring item.

The first two ring iitems are always of type
frib::analysis::PARAMETER_DEFINITIONS 
and frib::analysis::VARIABLE_VALUES.

These provide information about the parameter definitions and steering variables.

\subsection paramdefs Parameter Defintion ring items.

Note that the parameter definitions included in the file will only be those
read by the parameter reader passed in when the application is run.  Any other
parameters are not known to the outputter unless they are explictly declared in the
outputter.

`PARAMETER_DEFINITIONS` items provide sufficient information to allow consuming
programs to map the data into its own parameters.  This ring item
is an frib::analysis::ParameterDefinitions and has the following fields:

|  name  |  type |  Meaning          |
|--------|-------|-------------------|
| s_header | frib::analysis::RingItemHeader | The ring item header |
| s_numParameters | std::uint32_t | the number of parameter definitions that follow |
| s_parameters | frib::analysis::ParameterDefinition\[\] | The first of the parameter definitions. |

`s_parameters` is the first of a sequence of frib::analysis::ParameterDefinition structs.
each of those structs has the following fields:

| name   | type  | Meaning    |
|--------|-------|------------|
| s_parameterNumber | std::uint32_t | The parameter ID used for this parameter |
| s_parameterName   | char \[\] | Null terminated string that is the name of the parameters |

Note that since `s_parameterName` is variable sized, the following shows code to
iterate over the  parameter definitions:

```
using namespace frib::analysis;
...

const ParameterDefintions*  pDefs = ... ; //set the definitions pointer.
const ParamterDefinition*   pDef  = pDefs->s_parameters;  // first param def.

for (int i =0; i < pDefs->s_numParameters; i++) {
   // Do something with the definition.
   ...
   // Point to the next definition.
   
   const std::uint8_t* p = reinterpret_cast<const std::uint8_t*>(pDef);
   p += sizeof(ParameterDefinition) + strlen(pDef->s_parameterName) + 1;
   pDef = reinterpret_cast<const ParameterDefinition*>(p);
}
```

Where the `+ 1` takes into account the null byte terminator of the string.

\subsection varitem Variable Definition Ring Items


Following the parameter definition item will always be a
frib::analysis::VARIABLE_VALUES ring item.  This will be a
frib::analysis::VariableItem struct.  These have he following fields:

| Name    |    type       |   Meaning   |
|---------|---------------|-------------|
| s_header | frib::analysis::RingItemHeader | The header of the ring item |
| s_numVars | std::uint32_t | Number of variable descriptions that follow |
| s_variables | frib::analysis::Variable \[\] | First of the variable definitions |

Each of the `s_variables` has the following fields:

| name | type    | Meaning |
|------|---------|---------|
| s_value | double | Value of the variable |
| s_variableUnits | char \[frib::analysis::MAX_UNITS_LENGTH\] | Unis of measure |
| s_variablename | char \[\] | Name of the variable |

Note that the variable definitions are variable length.  The following code
shows how to iterate over tham:

```
using namespace frib::analysis;
...

const VariableItem* pItem = ...;   // initilized some how
const Variable*     pVar = pItem->s_numVars;  // first one.

for (int i =0; i < pItem->s_numVars; i++) {
    // Do something with the definitions.
    ...
    // Point to the next definition:
    
    const std::uint8_t* p = reinterpret_cast<const std::uint8_t*>(pVar);
    p += sizeof(Variable) + strlen(pVar->s_variableName) + 1;
    pVar = reinterpret_cast<const Variable*>(p);
}
```

Note that the units of measure field is fixed length.  The size is generous enough
to support standard units of measure (e.g. `furlongs/fortnight` fits easily).


\subsection parevent Event as parameters

Once the documentation records have been emitted, the outputter will output a
a series of ring items.  These will consist of standard NSCLDAQ ring items that
bypass event processing and
frib::analysis::ParameterItem (type frib::analysis::PARAMETER_DATA item types).
PARAMETER_DATA item types will contain events that have been decoded into
parameters.  These ring items will be frib::analysis::ParameterItem structs which
have the following fields:

|  name    |  type | Meaning |
|----------|--------|--------|
| s_header | frib::analysis::RingitemHeader | The standard ring item header |
| s_triggerCount | std::uint64_t | The trigger number (the number of the event in the input file) |
| s_parameterCount | std::uint32_t | The number of parameters unpacked from the event |
| s_parameters | frib::analysis::ParameterValue \[\] | The first of a sequence of parameter values |

The parameter values (`s_parameters`) are each an frib::analysis::ParameterValue struct
with the fields:

| name    | type     | Meaning  |
|---------|----------|----------|
| s_number | std::uint32_t | Parameter number.  If the outputter knows all of the parameters this  will be one of the numbers in the parameter definition ring item |
| s_value  | double  | Value of the parameter for this event |

Note that if a parameter is not assigned a value it will not appear in the event.
