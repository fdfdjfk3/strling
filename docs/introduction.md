# 00 - Introduction

Welcome to the documentation of Strling, the world's first string-oriented programming language. This is part of the documentation is simply an introduction to the language. This documentation will assume that you have at least basic programming knowledge.

This is an esoteric programming language, meaning that it's not meant to be used for anything useful. In fact, it solely exists as a creative outlet. I didn't go full 'evil-mode' on this language, so the syntax is actually quite simple and most tasks you'd want to do in another language are probably possible in Strling (I'm not sure if it's turing complete though...)

## Language constructs

As you have probably gathered by now, Strling only has a single data type--the humble string. If you think about it, any arbitrary data you can think of can be stored in a string, given enough memory. Who needs structs when you have the freedom and power of strings?

### Comments

Comments in Strling begin with a `?`. Whenever you see a question mark in Strling code, it's safe to assume the rest of the line is a comment.

```
? this is a comment! hello!
```

### Operators

Strling's operator set is pretty small, but it has some interesting ones.

assignment (`=`)

```
x = "some text"
```

concatenation (`+`)

```
x = "hello " + "world"
```

remove occurrences (`-`)

```
? removes occurrences of a substring from the initial string
x = "blblblblblblbl" - "bl"
? x is ""
```

equals/does not equal (`==`/`!=`)

```
? booleans in Strling are represented with their string variants, to clear any confusion there

x = "foo" == "foo"
y = "foo" != "bar"
```

intersection (`&`)

```
? set intersection
? it currently also takes index into account (but i'm planning on making it not be this way)

a = "hello" & "hell"
? a is "hell"
```

difference (`!&`)

```
? set difference
? it currently also takes index into account (but i'm planning on making it not be this way)

a = "hello" & "hell"
? a is "o"
```

### If / Elif

If the expression in the if or elif statement evaluates to `"true"`, the code in the block runs. Otherwise, it doesn't.

```
if "true" {
    println("this runs")
}

if "foo" == "foo" {
    println("so does this")
}

if "false" {
    println("this doesn't run")
} elif "foo" == "bar" {
    println("nor does this")
}   
```

### While

Same general idea as the If/Elif statement, but the block will keep repeating as long as the condition evaluates to `"true"`

```
foo = ""

? this loop will run 5 times.
while foo != "aaaaa" {
    foo = foo + "a"
}

? this loop will run forever, as "true" always evaluates to "true" (woah, that's crazy)
while "true" {
    println("infinite loop")
}
```

There are also `continue` and `break` which go to the next loop iteration and break from the loop respectively.

### Function declaration

Functions are declared with the keyword `fun`, to ensure that the language is as fun as possible!.

```
fun f() {
    ? ...
}
```

Function parameters are put as a list of identifiers. No type is necessary, as the only type is string.

```
fun f(foo, bar) {
    ? ...
}
```

It is possible to pass in a string by reference, rather than by value as is the default behavior. This has no change to code speed, but is rather to allow for functions that can mutate the original string (to allow for more complex control flow). An argument is passed in by reference by using the `ref` keyword before the parameter.

```
? foo will be passed in by reference, and bar will be passed in by value.

fun f(ref foo, bar) {
   ? ... 
}
```

To return a value from a function, use the `return` keyword (as most other procedural languages use)

```
fun get_foo() {
    return "f" + "o" + "o"
}
```

### Import

Todo.

## End

And that's it. That's the whole language. Check out `stdlib/` for documentation on the standard library.









