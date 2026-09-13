# WamrInterrupt POC

This is a small poc which implements a simulation of an Interrupt controller in [wamr](https://github.com/bytecodealliance/wasm-micro-runtime).
The system implements:

- preemption
- masking
- stack unwinding

# Building

The building and the execution is controlled by the *nob* builder. 
To use you just need to compile it:

```sh
gcc nob.c -o nob
```

and run it.

```sh
./nob
```

If you want to know more just do.

```sh
./nob -h
```

