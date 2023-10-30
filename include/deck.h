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

#ifndef DECK_H
#define DECK_H

enum {
	TOTAL, NONE, LOWACE = 1,
	TWO, THREE, FOUR, FIVE,
	SIX, SEVEN, EIGHT, NINE,
	TEN, ACE,
};

typedef unsigned card_t;
typedef card_t deck_t[ACE + 1];

void deck_init(deck_t deck, unsigned decks);
void deck_single(deck_t deck, card_t card);

card_t deck_first(deck_t deck);
card_t deck_random(deck_t deck, unsigned long long r);

double deck_true(deck_t deck);

void deck_draw(deck_t deck, card_t card);
void deck_put(deck_t deck, card_t card);
unsigned deck_sum(deck_t deck);

#endif
