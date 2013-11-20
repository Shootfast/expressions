expressions
===========

Header only c++ expression parsing library with AST building and GLSL shader generation.
Optional LLVM JIT evaluation for improved performance.

MIT licensed


Syntax
------

The supported expression syntax is similar to C (without bitwise operations).
At present the supported operations are:

1. Mathematical: \+, -, *, /, %, ^
2. Ternary: ? :
3. Equality: ==, !=, <, <=, >, >=
4. Logical: &&, ||
5. Functions: sin, cos, tan, sqrt, ceil, floor, min, max, pow, log, log2, log10


