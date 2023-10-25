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

#include "deck.h"

#include <assert.h>

void deck_init(deck_t deck, unsigned decks) {
	deck[TOTAL]  = decks * 52;
	deck[LOWACE] = 0;
	deck[TWO]    = 4 * decks;
	deck[THREE]  = 4 * decks;
	deck[FOUR]   = 4 * decks;
	deck[FIVE]   = 4 * decks;
	deck[SIX]    = 4 * decks;
	deck[SEVEN]  = 4 * decks;
	deck[EIGHT]  = 4 * decks;
	deck[NINE]   = 4 * decks;
	deck[TEN]    = 4 * 4 * decks;
	deck[ACE]    = 4 * decks;
}

void deck_single(deck_t deck, card_t card) {
	assert(card);
	deck[TOTAL]  = 1;
	deck[LOWACE] = 0;
	deck[TWO]    = 0;
	deck[THREE]  = 0;
	deck[FOUR]   = 0;
	deck[FIVE]   = 0;
	deck[SIX]    = 0;
	deck[SEVEN]  = 0;
	deck[EIGHT]  = 0;
	deck[NINE]   = 0;
	deck[TEN]    = 0;
	deck[ACE]    = 0;
	deck[card]   = 1;
}

card_t deck_first(deck_t deck) {
	for (card_t card = TWO; card <= ACE; card++)
		if (deck[card])
			return card;
	assert(0);
	return 0;
}

void deck_draw(deck_t deck, card_t card) {
	assert(card);
	assert(deck[TOTAL]);
	assert(deck[card]);
	deck[TOTAL]--;
	deck[card]--;
}

void deck_put(deck_t deck, card_t card) {
	assert(card);
	deck[TOTAL]++;
	deck[card]++;
}

unsigned deck_sum(deck_t deck) {
	unsigned sum = 0;

	for (card_t card = TWO; card <= ACE; card++)
		sum += card * deck[card];

	for (unsigned i = 0; i < deck[ACE]; i++)
		if (sum > 21)
			sum += LOWACE - ACE;

	return sum;
}
