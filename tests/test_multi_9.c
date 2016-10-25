#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

#define SIZE        9
#define SQRT_SIZE   3


static int ok_count;
static int error_count;

static int check_board(unsigned char original_board[SIZE][SIZE], unsigned char solved_board[SIZE][SIZE])
{
	// These store the count of the occurrence of each number on each line/column/block
	// First dimension is the line/column/block, second dimension is the count of occurrences
	unsigned char line_counts[SIZE][SIZE];
	unsigned char column_counts[SIZE][SIZE];
	unsigned char block_counts[SIZE][SIZE];
	memset(line_counts, 0, sizeof(line_counts));
	memset(column_counts, 0, sizeof(column_counts));
	memset(block_counts, 0, sizeof(block_counts));

	for (int l = 0; l < SIZE; l++) {
		for (int c = 0; c < SIZE; c++) {
			int n = solved_board[l][c];

			if (n <= 0 || n > SIZE) {  // Check if number is valid
				return -1;
			}

			n--;

			// Calculate the count for each number in each line, column and block
			int b = (l / SQRT_SIZE) * SQRT_SIZE + (c / SQRT_SIZE);
			line_counts[l][n]++;
			column_counts[c][n]++;
			block_counts[b][n]++;

			int n_original = original_board[l][c];
			if (n_original != 0 && n != n_original - 1) {  // Number from original board changed?
				return -1;
			}
		}
	}

	// In a correct sudoku, each number appears exactly once in each line, column and block
	for (int i = 0; i < SIZE; i++) {
		for (int n = 0; n < SIZE; n++) {
			if (line_counts[i][n] != 1 || column_counts[i][n] != 1 || block_counts[i][n] != 1) {
				return -1;
			}
		}
	}

	return 0;
}


int main(int argc, char **argv)
{
	char line[SIZE*SIZE+1+1];
	FILE *f;

	if (argc > 2) {
		fprintf(stderr, "Usage: %s [test_file]\n", argv[0]);
		exit(-1);
	}

	if (argc == 2) {
		if ((f = fopen(argv[1], "rb")) == NULL) {
			fprintf(stderr, "Error opening file: %s\n", argv[1]);
			exit(-1);
		}
	} else {
		f = stdin;
	}

	while(fgets(line, sizeof(line), f) != NULL) {
		int len = strlen(line);
		if (len < SIZE * SIZE) {
			continue;
		}

		int pipe_in[2];
		int pipe_out[2];
		if (pipe(pipe_in) < 0 || pipe(pipe_out) < 0) {
			return -1;
		}

		int pid = fork();
		if (pid == 0) {  // Child
			close(pipe_in[1]);
			close(pipe_out[0]);
			close(0);
			close(1);
			if (dup(pipe_in[0]) < 0 || dup(pipe_out[1]) < 0) {
				return -1;
			}
			close(pipe_in[0]);
			close(pipe_out[1]);
			execl("sudoku", "sudoku", NULL);
		} else {  // Parent
			close(pipe_in[0]);
			close(pipe_out[1]);

			// Write the formatted puzzle to the child and save the original board
			unsigned char original_board[SIZE][SIZE];
			unsigned char *original_board_ptr = (unsigned char *) original_board;
			FILE *child_in = fdopen(pipe_in[1], "w");
			fprintf(child_in, "%d\n", SQRT_SIZE);
			for (int i = 0; line[i] != '\0' && i < SIZE * SIZE; i++) {
				char c = line[i] == '.' ? '0' : line[i];
				original_board_ptr[i] = c - '0';
				fprintf(child_in, "%c ", c);
			}
			fclose(child_in);

			// Read the result from the child
			FILE *child_out = fdopen(pipe_out[0], "r");
			unsigned char solved_board[SIZE][SIZE];
			unsigned char *solved_board_ptr = (unsigned char *) solved_board;
			int numbers_read;
			for (numbers_read = 0; numbers_read < SIZE * SIZE; numbers_read++) {
				int n;
				if (fscanf(child_out, "%d", &n) != 1) {
					break;
				}
				solved_board_ptr[numbers_read] = n;
			}
			fclose(child_out);

			int status;
			wait(&status);

			if (status != 0 || numbers_read != SIZE * SIZE) {
				// No solution or error
				printf("%s", line);
				error_count++;
			} else {
				// Check solution
				if (check_board(original_board, solved_board) < 0) {
					printf("%s", line);
					error_count++;
				} else {
					ok_count++;
				}
			}
		}
	}
	fclose(f);

	fprintf(stderr, "%d of %d OK.\n", ok_count, ok_count + error_count);

	return 0;
}
