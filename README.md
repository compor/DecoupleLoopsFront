
# LLVM Decouple Loops Front opt pass

This is a LLVM preparatory opt pass for the [Decouple Loops](https://github.com/compor/icsa-dswp) pass.

It splits the basic blocks of a loop to either all iterator-contributing instructions or otherwise.

## How to Build

- make sure clang/clang++ is in your `$PATH` along with required environment variables (see `utils/scripts/` directory):
  - compiler selection is catered by the `exports_local_*` scripts under `utils/scripts` directory for my current 
    machine, so adjust appropriately for your setup.
  - export one of the `exports_deps_*` scripts, depending on the kind of setup you are interested in.
- `mkdir my-build-dir`
- optionally `mkdir my-install-dir`
- `[path to repo]/utils/build.sh [path torepo] [path to installation dir]`
- `cd my-build-dir`
- `make`
- optionally `make install`

## How to execute

### Using opt

- make sure LLVM's opt is in your `$PATH`
- `opt -load [path to plugin]/libLLVMDecoupleLoopsFrontPass.so -decouple-loops-front foo.bc -o foo.out.bc`

### Using clang

- make sure LLVM's clang is in your `$PATH`
- `clang -Xclang -load -Xclang [path to plugin]/libLLVMDecoupleLoopsFrontPass.so foo.c -o foo`
   
## Requirements

- Built and executed with:
  - LLVM 3.7.0
  - LLVM 3.8.0

## Notes

- When the build script uses LLVM's cmake utility functions the `lib` shared library prefix is omitted


