#!/bin/sh
gcc -std=c99 -Wall -Wextra -fno-stack-protector -fomit-frame-pointer -funsafe-loop-optimizations -fdelete-null-pointer-checks -Ofast -ffast-math -flto -m64 -march=native -o sudoku sudoku.c trylist.c
strip sudoku
