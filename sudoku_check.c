#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MAX_SQRT_SIZE  15
#define MAX_GAME_SIZE  (MAX_SQRT_SIZE*MAX_SQRT_SIZE)


static int game_data[MAX_GAME_SIZE*MAX_GAME_SIZE];
static int *game_board[MAX_GAME_SIZE];
static int game_size;
static int sqrt_size;


/*
 * Checks if a position is valid (ie, the number is unique in the same line, row and block)
 * Returns 0 if it is valid and -1 otherwise
 */
static int check_position(int x, int y)
{
	// These will store the counts of each number in the line/column/block
	unsigned char line_count[MAX_GAME_SIZE] = {0};
	unsigned char row_count[MAX_GAME_SIZE] = {0};
	unsigned char block_count[MAX_GAME_SIZE] = {0};

	// Check line and row
	for (int i = 0; i < game_size; i++) {

		// Check line
		int n = game_board[x][i];
		line_count[n-1]++;
		if (line_count[n-1] > 1) {
			printf("Number %d appears twice in line %d.\n", n, x + 1);
			return -1;
		}

		// Check row
		n = game_board[i][y];
		row_count[n-1]++;
		if (row_count[n-1] > 1) {
			printf("Number %d appears twice in row %d.\n", n, y + 1);
			return -1;
		}
	}

	// NOTE: Integer division, the result isn't always x;
	int bl_start_x = (x/sqrt_size)*sqrt_size;
	int bl_start_y = (y/sqrt_size)*sqrt_size;

	// Check block
	for (int i = bl_start_x; i < bl_start_x + sqrt_size; i++) {
		for (int j = bl_start_y; j < bl_start_y + sqrt_size; j++) {
			int n = game_board[i][j];
			block_count[n-1]++;
			if (block_count[n-1] > 1) {
				printf("Number %d appears twice in block (%d %d).\n", n, bl_start_x + 1, bl_start_y + 1);
				return -1;
			}
		}
	}

	return 0;
}


int main()
{
	int numbers_read;
	for (numbers_read = 0; numbers_read < MAX_GAME_SIZE * MAX_GAME_SIZE; numbers_read++) {
		if (scanf("%d", &game_data[numbers_read]) != 1) {
			break;
		}
	}

	game_size = sqrt(numbers_read);
	sqrt_size = sqrt(game_size);

	if (game_size < 4 || game_size > MAX_GAME_SIZE) {
		printf("Wrong size! game_size = %d\n", game_size);
		printf("CHECK FAILED!\n");
		return -1;
	}

	for (int i = 0; i < game_size; i++) {
		game_board[i] = game_data + i * game_size;
	}

	for (int i = 0; i < game_size; i++) {
		for (int j = 0; j < game_size; j++) {
			if (game_board[i][j] == 0) {
				printf("Zero in matrix! position = (%d, %d)\n", i+1 , j+1);
				printf("CHECK FAILED!\n");
				return -1;
			}

			if (check_position(i, j) == -1) {
				printf("CHECK FAILED!\n");
				return -1;
			}
		}
	}

	printf("CHECK OK.\n");

	return 0;
}
