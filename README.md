# AvaLang

AvaLang, or simply Ava, is an experimental general-purpose programming language.

This repository contains its C bootstrap implementation. AvaLang is being developed incrementally through small, executable slices, beginning with a complete compiler pipeline and interpreter for simple Ava programs.

The name Ava is inspired by Ava, the artificial intelligence in *Ex Machina*. The language’s long-term motivation is to explore programming-language design for machine-learning and AI programs.

## Status

AvaLang is at an early stage. The compiler can read a source file and dump its tokens or a
small syntax tree for `let` bindings with integer literal initializers. It does not execute
programs yet.

```sh
make
./build/ava dump tokens tests/fixtures/bindings.ava
./build/ava dump ast tests/fixtures/bindings.ava
```

The current parser accepts `let name = 42` and `let name: i32 = 42` forms. The type name and
integer literal are recorded as source text; type checking and numeric range checking have
not been added.

## License

AvaLang is available under the MIT License.
