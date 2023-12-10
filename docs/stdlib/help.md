# How to read the standard library documentation

The standard library is split up into modules, which each modules
containing its own globals and functions. Here's how to make sense of it:

## Globals

Global variables are listed in a 'Globals' section

## Functions

Functions are listed in a 'Functions' section

Here's an example of how to interpret one:

### `pop(ref str: any) <=1`

The function is named `pop`. It takes in one parameter, which is passed in 
by reference as shown by the `ref` keyword. The length bounds of the string
are after the parameter's name; in this case, any length of string is allowed
as an argument to this function. the `<=1` at the end specifies the length bounds 
of the string it will return. In this case, it will either return a string with
a length of 0, or a string with a length of 1.
