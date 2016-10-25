#include "trylist.h"

static int game_size;


int tl_set_game_size(int size)
{
	if (size > MAX_GAME_SIZE) {
		return -1;
	}

	game_size = size;
	return 0;
}


void tl_init(trylist_t *new_tl, int number)
{
	new_tl->number = number;
	if (number != 0) {
		new_tl->count = 0;  // The number is already set, no need for possibilites
		return;
	} else {
		new_tl->count = game_size;
	}

	for (int i = 0; i < game_size; i++) {
		new_tl->list[i] = 1;
	}
}


int tl_remove(trylist_t *tl, int n)
{
	if (tl->number != 0) {
		return 0;
	}

	n--;  // Number starts at 1, array starts at 0

	if (tl->list[n] == 0) {  // Already unset?
		return 0;
	}

	tl->list[n] = 0;
	tl->count--;
	if (tl->count == 1) {
		// Update the number that is set
		for (int i = 0; i < game_size; i++) {
			if (tl->list[i] != 0) {
				tl->number = i + 1;
				break;
			}
		}
		return 1;
	}

	return 0;
}


int tl_find_next(const trylist_t *tl, int start)
{
	start--;  // Number starts at 1, array starts at 0
	for ( ; start < game_size; start++) {
		if (tl->list[start] != 0) {
			return start + 1;
		}
	}

	return 0;
}
