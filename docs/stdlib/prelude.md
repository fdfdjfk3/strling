# Stdlib module 'Prelude'

## Globals:

None.

## Functions:

### `print(str: string) -> unit`

Prints `str` to stdout

### `println(str: string) -> unit`

Prints `str` to stdout with an added newline at the end

### `getchar() -> byte`

Gets a character from stdin

### `getline() -> string`

Get a line from stdin

### `pop(ref str: string) -> byte|unit`

Removes a byte from the right side of `str` and returns it

### `popl(ref str: string) -> byte|unit`

Removes a byte from the left side of `str` and returns it

### `pop_substr(ref str: string, delim: string) -> string`

Removes a substring delimited by `delim` from the right side of `str`, and returns it

### `popl_substr(ref str: string, delim: string) -> string`

Removes a substring delimited by `delim` from the left side of `str`, and returns it

### `rev(str: string) -> string`

Returns `str` reversed

### `replace(str: string, target: string, replacement: string) -> string`

Returns `str` with all instances of substring `target` replaced with `replacement`
