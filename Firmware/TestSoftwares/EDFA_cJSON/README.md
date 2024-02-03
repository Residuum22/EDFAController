# Simple cJSON test for EDFA

## Prerequisites

`mkdir build`

## Building in DEBUG

```bash
gcc -c include/cJSON.c -o build/cJSON.o -g
gcc -c -Iinclude main.c -o build/main.o -g
gcc build/main.o build/cJSON.o -o build/executable
```
## Running program

`./build/executable`