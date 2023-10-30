/* Daisy, a probability calculator for blackjack.
 * Copyright (C) 2023 Isak Ellmer
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "truecount.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

double deck_true(deck_t deck) {
	double running = 0;
	for (card_t card = TWO; card <= SIX; card++)
		running -= 1.0 * deck[card];
	for (card_t card = TEN; card <= ACE; card++)
		running += 1.0 * deck[card];

	return 52 * running / deck[TOTAL];
}

/* Returns the variance, the mean is assumed to be 0. */
double simulate_true(unsigned long shoes, unsigned decks, double penetration) {
	srand(0);
	double var = 0;
	unsigned long n = 0;
	for (unsigned long i = 0; i < shoes; i++) {
		deck_t deck;
		deck_init(deck, decks);

		int running = 0;

		while (deck[TOTAL] >= 52 * decks * (1.0 - penetration)) {
			card_t card = deck_random(deck, rand());
			deck_draw(deck, card);
			
			if (card <= SIX)
				running++;
			else if (card >= TEN)
				running--;

			double true_count = 52.0 * running / deck[TOTAL];

			var += true_count * true_count;
			n++;
		}
	}

	var /= n - 1;
	var = sqrt(var);

	return var;
}
