#!/bin/bash
#i686-w64-mingw32-gcc -static -static-libgcc -static-libstdc++ -s -DNDEBUG -O2 -Wall -Wextra -o linker.exe linker.c miniz.c
i686-w64-mingw32-gcc -static -static-libgcc -static-libstdc++ -O0 -g -Wall -Wextra -o linker.exe linker.c miniz.c
#i686-w64-mingw32-strip --strip-all output.exe
