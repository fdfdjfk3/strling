# Strling

The world's first stringly typed programming language! Strling approaches
programming in a never-before-seen way, leveraging
SOP (String Oriented Programming) to create programs that are both natural
and efficient. Just look how easy it is to print "Hello world" to the terminal!

```
println("Hello, world!")
```

The language is currently in a very early stage, and there are probably
a *lot* of bugs. Anyways, check out the documentation in `docs/`!

# Compiling

You can compile this project by running the `build.lua` file. By default,
it will generate a binary named `strling` with -O3, but you can override
the flags by passing them as arguments. Example for a debug build:

`luajit build.lua -g -o strling_debug`
