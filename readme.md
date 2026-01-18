# CPU Emulator & Assembler

A custom CPU emulator with assembler/disassembler for the Dedinsky system programming course.

## Quick Start

### Build
```bash
mkdir build && cd build
cmake ..
cd ..
cmake --build build --preset debug    # Debug build
cmake --build build --preset release  # Release build
```

### Usage
```bash
# 1. Assemble (.myasm → .bin)
./assembler program.myasm program.bin

# 2. Execute
./cpu_emulator program.bin

# 3. Disassemble (.bin → .myasm)
./disassembler program.bin
./disassembler program.bin output.myasm  # Save to file
```

Executables are in `build/debug/` or `build/release/`.

## Assembly Syntax

### Numbers
- **Only hexadecimal**: `10` means 0x10 (16 in decimal)
- **No 0x prefix**: `0x10` will cause an error
- **Valid examples**: `A`, `FF`, `100`

### Labels
```assembly
:main            ; Label definition
baw :main        ; Jump to label
cfn :function    ; Call function by label
```

### Comments
```assembly
mov x1, A        ; Load 0x0A into register x1
```

## Instructions Overview

### Basic Operations
- `mov dst, src` - Move value
- `inp reg` - Read hex from stdin
- `out reg` - Write hex to stdout
- `hlt` - Stop execution

### Arithmetic
- `add dst, a, b` - Addition
- `sub dst, a, b` - Subtraction  
- `mul dst, a, b` - Multiplication
- `div dst, a, b` - Division
- `sqr dst, src` - Square root

### Control Flow
- `baw addr` - Branch always
- `beq/beq/bgt/blt/bge/ble a, b, addr` - Conditional branches
- `cfn addr` - Call function
- `ret` - Return

### Stack
- `str val, *addr` - Store to stack
- `ldr reg, *addr` - Load from stack

## Addressing Modes

| Prefix | Meaning | Example |
|--------|---------|---------|
| `x` | Register | `x1` = register 1 |
| `*` | Stack address | `*100` = address 0x100 |
| `*x` | Register indirect | `*x1` = address in register 1 |

## Registers
- `x0`-`xf`: General purpose (16 registers)
- `x10`: Program counter (rpc)
- `x11`: Base pointer (rbp)  
- `x12`: Cycle counter (rcc)

## Example Programs
Check `examples/` directory:
- `fibonacci.myasm`
- `iterative_factorial.myasm` 
- `recursive_factorial.myasm`
- `stack_arithmetic.myasm`

## Project Structure
```
cpu_backend/     # Core emulator code
├── src/assembler/       # Assembler
├── src/cpu_emulator/    # CPU core
└── src/disassembler/    # Disassembler
examples/        # Sample programs
vendor/          # Dependencies (stack, exceptions)
```

**Note**: Numbers are always hexadecimal. `10` = 0x10 (16 decimal). `0x10` will cause an error.
