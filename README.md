# Solver for large sudokus

This a sudoku solver that supports large boards (up to 225x225).

It is written in C and focuses on performance.

Test sudokus and utilities can be found on tests folder.


## Sudoku format

The first line of the file has the square root of the size of the puzzle (e.g.: 3 for a 9x9 puzzle).

The remaining lines contain the numbers separated by any kind or amount of spaces.

Zero is used for empty positions.

Example board:

```
3
5 3 0 0 7 0 0 0 0
6 0 0 1 9 5 0 0 0
0 9 8 0 0 0 0 6 0
8 0 0 0 6 0 0 0 3
4 0 0 8 0 3 0 0 1
7 0 0 0 2 0 0 0 6
0 6 0 0 0 0 2 8 0
0 0 0 4 1 9 0 0 5
0 0 0 0 8 0 0 7 9
```
