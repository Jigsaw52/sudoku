#ifndef TRYLIST_H
#define TRYLIST_H

/*
 * A trylist is a data structure that stores the possibilities for one
 * sudoku position or the current number in the position (if any)
 */

#define MAX_GAME_SIZE  225  // Reducing this value decreases memory use

typedef struct {
	unsigned char number;  // The number in the position (0 if none)
	unsigned char count;   // The number of possibilities
	unsigned char list[MAX_GAME_SIZE];
} trylist_t;


/*
 * tl_set_game_size - sets the maximum size of the puzzle
 *  size - puzzle size, maximum size is MAX_GAME_SIZE
 * returns: -1 if the size is too big, 0 otherwise
 */
int tl_set_game_size(int size);


/*
 * tl_init - initializes a new trylist
 *  new_tl - where to store the new trylist
 *  number - initial number set, if 0 it will have all the possibilities
 */
void tl_init(trylist_t *new_tl, int number);


/*
 * tl_remove - removes a possibility from the trylist
 *             if there is only one possibility after removal
 *             that possibility will become the number set
 *  tl - trylist
 *  n - possibility to remove
 * returns: 1 if the number set was changed, 0 otherwise
 */
int tl_remove(trylist_t *tl, int n);


/*
 * tl_find_next - Finds the next possibility in the list
 *  tl - trylist
 *  start - the number to start from
 * returns: 0 if there is no next possibility
 */
int tl_find_next(const trylist_t *tl, int start);

#endif  // TRYLIST_H
