#include "trylist.h"

#define BMAP_SET(bmap,num)      ((bmap)[(num)>>3]|=(0x1<<((num)&0x07)))
#define BMAP_CLR(bmap,num)      ((bmap)[(num)>>3]&=~((0x1<<((num)&0x07))))
#define BMAP_TOGGLE(bmap,num)   ((bmap)[(num)>>3]^=(0x1<<((num)&0x07)))
#define BMAP_ISSET(bmap,num)    (((bmap)[(num)>>3]&(0x1<<((num)&0x07)))!=0)

static int game_size;
static int list_size;


int tl_set_game_size(int size)
{
	if (size > MAX_GAME_SIZE) {
		return -1;
	}

	game_size = size;
	list_size = BITS2BYTES(size);
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

	for (int i = 0; i < list_size; i++) {
		new_tl->list[i] = 0xFF;
	}
}


int tl_remove(trylist_t *tl, int n)
{
	if (tl->number != 0) {
		return 0;
	}

	n--;  // Number starts at 1, array starts at 0

	if (!BMAP_ISSET(tl->list, n)) {  // Already unset?
		return 0;
	}

	BMAP_CLR(tl->list, n);
	tl->count--;
	if (tl->count == 1) {
		// Update the number that is set
		for (int i = 0; i < list_size; i++) {
			if (tl->list[i] != 0) {
				// Find which bit is set
				unsigned int res = (i << 3);
				unsigned int temp = tl->list[i];
				while ((temp & 1) == 0) {
					res++;
					temp >>= 1;
				}
				tl->number = res + 1;
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

	unsigned int start_bit = start & 0x07;
	for (int start_byte = (start >> 3); start_byte < list_size; start_byte++) {
		unsigned int temp = tl->list[start_byte] >> start_bit;
		if (temp != 0) {
			// Find the first set bit
			unsigned int res = (start_byte << 3) + start_bit;
			while ((temp & 1) == 0) {
				res++;
				temp >>= 1;
			}
			return res + 1;
		}
		start_bit = 0;
	}

	return 0;
}
