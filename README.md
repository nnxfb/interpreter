# Simple Interpreter in C

A lightweight expression parser and calculator implemented in C, supporting basic arithmetic operations, variable assignments, and bracket prioritization.

## Features

- **Operators Supported**
  - Arithmetic: `+`, `-`, `*`, `/`
  - Assignment: `=`
  - Parentheses: `()` for prioritization
  
- **Variables**
  - Store variables with alphanumeric names (e.g., `x`, `total_sum`)
  - Automatic variable creation on first assignment

- **Error Handling**
  - Undefined variable detection
  - Syntax error reporting (e.g., mismatched brackets, missing operands)
  - Division by zero (not explicitly handled; depends on system behavior)

- **Memory Management**
  - Custom memory pool allocator for efficient memory usage
  - Symbol table with MurmurHash2 for variable lookup

## Usage

### Examples
Basic Arithmetic:
```
>> 3 + 5 * (10 - 4)
33
```
Variable Assignment:
```
>> x = 42
>> y = x / 2 + 5
>> y
26
```
Error Handling:
```
>> z + 5
Error: undefined variable
```
```
>> (3 + 5 * 2
Error: missing right bracket
```
