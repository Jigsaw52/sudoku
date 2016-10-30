#include <stdio.h>
#include <stdlib.h>
#include "trylist.h"

typedef struct {
	unsigned char x;
	unsigned char y;
	trylist_t list;
} pos_t;

static int sqrt_size;
static int game_size;
static trylist_t **game_board;

static pos_t *create_blank_list(int *num_blanks);
static int propagate_restrictions(int x, int y);
static int update_board(const pos_t *blanks, int blanks_size);
static int solve_game(const pos_t *blanks, int blanks_size);
static void restore_game(const pos_t *blanks, int blanks_size);
static void print_game(void);

// Used to order blank spaces by number of possibilities (less possibilities first)
#define SORT_NAME pos
#define SORT_TYPE pos_t
#define SORT_CMP(x, y) ((x).list.count - (y).list.count)
#include "sort.h"


int main(int argc, char *argv[])
{
	trylist_t *game_data;
	FILE *f;

	if (argc > 2) {
		fprintf(stderr, "Usage: %s [sudoku_file]\n", argv[0]);
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

	if (fscanf(f, "%d", &sqrt_size) != 1) {
		fprintf(stderr, "Invalid file format.\n");
		exit(-1);
	}

	game_size = sqrt_size * sqrt_size;
	if (tl_set_game_size(game_size) < 0) {
		fprintf(stderr, "Error: game too large. Maximum size is %d.\n", MAX_GAME_SIZE);
		exit(-1);
	}

	// Read the board from the file
	game_data = malloc(game_size * game_size * sizeof(*game_data));
	game_board = malloc(game_size * sizeof(*game_board));
	for (int i = 0; i < game_size; i++) {
		game_board[i] = game_data + (i * game_size);
		for (int j = 0; j < game_size; j++) {
			int n;
			if (fscanf(f, "%d", &n) != 1) {
				fprintf(stderr, "Invalid file format.\n");
				exit(-1);
			}
			tl_init(&game_board[i][j], n);
		}
	}
	fclose(f);

	pos_t *blanks;
	int blanks_count;

	// Create a list with all blanks positions
	if ((blanks = create_blank_list(&blanks_count)) == NULL) {
		printf("No solution\n");
		free(game_board);
		free(game_data);
		exit(-1);
	}

	// Solve the puzzle
	int ret;
	if ((ret = solve_game(blanks, blanks_count)) == 0) {
		// Print the solved puzzle
		print_game();
	} else {
		printf("No solution\n");
	}

	// Free memory
	free(game_board);
	free(game_data);
	free(blanks);

	return ret;
}


/*
 * Prints the game board
 */
static void print_game(void)
{
	for (int i = 0; i < game_size; i++) {
		for (int j = 0; j < game_size; j++) {
			printf("%d\t", game_board[i][j].number);
		}
		printf("\n");
	}
}


/*
 * Updates the possible values to the positions affected by (x, y) (all positions on the same line, row or block)
 * Returns -1 if reaches an impossible state (a position without possible values), otherwise returns 0
 */
static int propagate_restrictions(int x, int y)
{
	int n = game_board[x][y].number;

	// Update line and row
	for (int i = 0; i < game_size; i++) {
		// Update line
		if (i != y) {  // Don't change current position
			if (tl_remove(&game_board[x][i], n) > 0) {
				if (propagate_restrictions(x, i) == -1) {
					return -1;
				}
			}
			if (game_board[x][i].number == n) {
				return -1;
			}
		}

		// Update row
		if (i != x) {  // Don't change current position
			if (tl_remove(&game_board[i][y], n) > 0) {
				if (propagate_restrictions(i, y) == -1) {
					return -1;
				}
			}
			if (game_board[i][y].number == n) {
				return -1;
			}
		}
	}

	// NOTE: Integer division, the result isn't always x;
	int bl_start_x = (x / sqrt_size) * sqrt_size;
	int bl_start_y = (y / sqrt_size) * sqrt_size;

	// Update block
	for (int i = bl_start_x; i < bl_start_x + sqrt_size; i++) {
		for (int j = bl_start_y; j < bl_start_y + sqrt_size; j++) {
			if (i == x || j == y) {  // Don't change positions in the same line or row (they were already updated)
				continue;
			}

			if (tl_remove(&game_board[i][j], n) > 0) {
				if (propagate_restrictions(i, j) == -1) {
					return -1;
				}
			}
			if (game_board[i][j].number == n) {
				return -1;
			}

		}
	}

	return 0;
}


static int update_board(const pos_t *blanks, int blanks_size)
{
	// These store the count of each possibility on each line/column/block
	// First dimension is the line/column/block, second dimension is the number of the possibility
	unsigned char line_counts[game_size][game_size];
	unsigned char column_counts[game_size][game_size];
	unsigned char block_counts[game_size][game_size];
	memset(line_counts, 0, sizeof(line_counts));
	memset(column_counts, 0, sizeof(column_counts));
	memset(block_counts, 0, sizeof(block_counts));

	// Go through the previous blanks and count the number of each possibility for every position
	// Ignore the first position because we've already filled it
	for (int i = 1; i < blanks_size; i++) {
		if (game_board[blanks[i].x][blanks[i].y].number == 0) {  // Still blank after update?
			int x = blanks[i].x;
			int y = blanks[i].y;
			int b = (x / sqrt_size) * sqrt_size + (y / sqrt_size);
			trylist_t *tl = &game_board[x][y];
			int p;
			int j = tl->count;
			for (j--, p = tl_find_next(tl, 1); p != 0; j--, p = j < 0 ? 0 : tl_find_next(tl, p + 1)) {
				if (line_counts[x][p - 1] == 0) {  // First found, store position
					line_counts[x][p - 1] = y + 1;
				} else {  // Found after first, invalidate entry
					line_counts[x][p - 1] = 0xFF;
				}

				if (column_counts[y][p - 1] == 0) {  // First found, store position
					column_counts[y][p - 1] = x + 1;
				} else {  // Found after first, invalidate entry
					column_counts[y][p - 1] = 0xFF;
				}

				block_counts[b][p - 1]++;
			}
		}
	}

	// Check for possibilities which are only present once in the line
	for (int l = 0; l < game_size; l++) {
		for (int p = 0; p < game_size; p++) {
			if (line_counts[l][p] != 0 && line_counts[l][p] != 0xFF) {
				int c = line_counts[l][p] - 1;
				if (game_board[l][c].number == 0) {
					if (tl_find_next(&game_board[l][c], p + 1) == p + 1) {
						game_board[l][c].number = p + 1;
						if (propagate_restrictions(l, c) == -1) {
							return -1;
						}
					}
				}
			}
		}
	}

	// Check for possibilities which are only present once in the column
	for (int c = 0; c < game_size; c++) {
		for (int p = 0; p < game_size; p++) {
			if (column_counts[c][p] != 0 && column_counts[c][p] != 0xFF) {
				int l = column_counts[c][p] - 1;
				if (game_board[l][c].number == 0) {
					if (tl_find_next(&game_board[l][c], p + 1) == p + 1) {
						game_board[l][c].number = p + 1;
						if (propagate_restrictions(l, c) == -1) {
							return -1;
						}
					}
				}
			}
		}
	}

	// Check for possibilities which are only present once in the block
	for (int b = 0; b < game_size; b++) {
		for (int p = 0; p < game_size; p++) {
			if (block_counts[b][p] == 1) {
				int bl_start_x = (b / sqrt_size) * sqrt_size;
				int bl_start_y = (b % sqrt_size) * sqrt_size;

				for (int l = bl_start_x; l < bl_start_x + sqrt_size; l++) {
					for (int c = bl_start_y; c < bl_start_y + sqrt_size; c++) {
						if (game_board[l][c].number == 0) {
							if (tl_find_next(&game_board[l][c], p + 1) == p + 1) {
								game_board[l][c].number = p + 1;
								if (propagate_restrictions(l, c) == -1) {
									return -1;
								}
								// Get out of the innermost loop only because the extra logic to get
								// out of both loops is more expensive than needlessly running the
								// second loop to completion
								break;
							}
						}
					}
				}
			}
		}
	}

	return 0;
}


/*
 * Tries all possible combinations of possibilities to solve the puzzle
 * Returns -1 if the puzzle is unsolvable
 */
static int solve_game(const pos_t *blanks, int blanks_size)
{
	pos_t *new_blanks;
	int new_blanks_size;

	// No blanks, we're done
	if (blanks_size == 0) {
		return 0;
	}

	int j = blanks[0].list.count;
	int p;

	// Go through the possibilities for the first blank
	for (j--, p = tl_find_next(&blanks[0].list, 1); p != 0; j--, p = j < 0 ? 0 : tl_find_next(&blanks[0].list, p + 1)) {

		int x = blanks[0].x;
		int y = blanks[0].y;

		// Set the value
		game_board[x][y].number = p;

		// Update the board with the new value
		if (propagate_restrictions(x, y) < 0 || update_board(blanks, blanks_size) < 0) {
			// If the update failed, we have to restore the previous state
			restore_game(blanks, blanks_size);
			continue;  // Next possibility
		}

		// Create new blank list
		new_blanks_size = 0;
		new_blanks = malloc((blanks_size - 1) * sizeof(*new_blanks));
		// Ignore the first position because we've already filled it
		for (int i = 1; i < blanks_size; i++) {
			if (game_board[blanks[i].x][blanks[i].y].number == 0) {  // Still blank after update?
				new_blanks[new_blanks_size].list = game_board[blanks[i].x][blanks[i].y];
				new_blanks[new_blanks_size].x = blanks[i].x;
				new_blanks[new_blanks_size].y = blanks[i].y;
				new_blanks_size++;
			}
		}

		// Sort the list by number of possibilities
		pos_binary_insertion_sort(new_blanks, new_blanks_size);

		// Solve the updated board
		if (solve_game(new_blanks, new_blanks_size) < 0) {
			free(new_blanks);
			// If the solve failed, we have to restore the previous state
			restore_game(blanks, blanks_size);
			continue;  // Next Possibility
		}

		free(new_blanks);
		return 0;
	}

	return -1;
}


/*
 * Restores the game board empty spaces from the blanks array
 */
inline static void restore_game(const pos_t *blanks, int blanks_size)
{
	for (int i = 0; i < blanks_size; i++) {
		game_board[blanks[i].x][blanks[i].y] = blanks[i].list;
	}
}


/*
 * Updates all the positions in the game board and returns a list with
 * all blanks positions
 */
static pos_t *create_blank_list(int *num_blanks)
{
	int blanks_count = 0;  // blanks_count is an aproximation of the number of blank spaces

	// Update the possible values for all positions
	for (int i = 0; i < game_size; i++) {
		for (int j = 0; j < game_size; j++) {

			// Check for empty position without any possible values
			if (game_board[i][j].number == 0) {
				if (game_board[i][j].count == 0) {
					return NULL;
				}
			} else if (propagate_restrictions(i, j) < 0) {
				return NULL;
			}

			if (game_board[i][j].number == 0) {
				blanks_count++;
			}
		}
	}

	pos_t *blanks = malloc(blanks_count * sizeof(*blanks));
	blanks_count = 0;

	// Create list with all blanks spaces
	for (int i = 0; i < game_size; i++) {
		for (int j = 0; j < game_size; j++) {
			if (game_board[i][j].number == 0) {
				blanks[blanks_count].list = game_board[i][j];
				blanks[blanks_count].x = i;
				blanks[blanks_count].y = j;
				blanks_count++;
			}
		}
	}

	// Sort the list by number of possibilities
	pos_binary_insertion_sort(blanks, blanks_count);

	*num_blanks = blanks_count;
	return blanks;
}
