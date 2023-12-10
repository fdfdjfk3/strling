# Stdlib module 'Prelude'

This stuff is included by default.

## Globals:

None.

## Functions:

### `print(str: any) 0`

Prints `str` to stdout

### `println(str: any) 0`

Prints `str` to stdout with an added newline at the end

### `getchar() <=1`

Gets a character from stdin

### `getline() <2048`

Get a line from stdin

### `pop(ref str: any) <=1`

Removes a byte from the right side of `str` and returns it

### `popl(ref str: any) <=1`

Removes a byte from the left side of `str` and returns it

### `pop_substr(ref str: any, delim: any) any`

Removes a substring delimited by `delim` from the right side of `str`, and returns it

### `popl_substr(ref str: any, delim: any) any`

Removes a substring delimited by `delim` from the left side of `str`, and returns it

### `rev(str: any) any`

Returns `str` reversed

### `replace(str: any, target: any, replacement: any) any`

Returns `str` with all instances of substring `target` replaced with `replacement`
