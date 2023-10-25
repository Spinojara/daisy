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

#include <stdio.h>
#include <assert.h>
#include <time.h>

#include "deck.h"
#include "search.h"

int main(void) {
	deck_t deck;
	deck_init(deck, 8);

	struct rootsearchinfo rsi;
	deck_t players = { 0 };

	card_t card1   = TWO;
	card_t card2   = TWO;
	card_t dealers = FOUR;
	char str[4];

#if 1
	deck_put(players, card1);
	deck_put(players, card2);

	search(&rsi, deck, players, dealers);
	double e = maximize_root(&rsi, str);
	printf("hit:         %f\n", rsi.hit);
	printf("stand:       %f\n", rsi.stand);
	printf("doubledown:  %f\n", rsi.doubledown);
	printf("split:       %f\n", rsi.split);
	printf("surrender:   %f\n", rsi.surrender);
	printf("expectation: %f\n", e);

	deck_draw(players, card1);
	deck_draw(players, card2);
#else
	printf(" H    2     3     4     5     6     7     8     9     T     A   \n");
	for (int sum = 5; sum <= 19; sum++) {
		if (sum % 2) {
			card1 = sum / 2;
			card2 = sum / 2 + 1;
		}
		else {
			card1 = sum / 2 - 1;
			card2 = sum / 2 + 1;
		}
		deck_put(players, card1);
		deck_put(players, card2);
		printf("   +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+\n");
		printf("%2d ", card1 + card2);

		assert(deck[TOTAL] == 8 * 52);
		assert(players[TOTAL] == 2);
		for (dealers = TWO; dealers <= ACE; dealers++) {
			search(&rsi, deck, players, dealers);
			maximize_root(&rsi, str);
			printf("| %s ", str);
			fflush(stdout);
		}
		printf("|\n");

		deck_draw(players, card1);
		deck_draw(players, card2);
	}
	printf("   +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+\n");
	printf(" S    2     3     4     5     6     7     8     9     T     A   \n");
	for (int sum = 13; sum <= 21; sum++) {
		card1 = ACE;
		card2 = sum - card1;
		deck_put(players, card1);
		deck_put(players, card2);
		printf("   +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+\n");
		printf("%2d ", card1 + card2);

		assert(deck[TOTAL] == 8 * 52);
		assert(players[TOTAL] == 2);
		for (dealers = TWO; dealers <= ACE; dealers++) {
			search(&rsi, deck, players, dealers);
			maximize_root(&rsi, str);
			printf("| %s ", str);
			fflush(stdout);
		}
		printf("|\n");

		deck_draw(players, card1);
		deck_draw(players, card2);
	}
	printf("   +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+\n");
	printf(" P    2     3     4     5     6     7     8     9     T     A   \n");
	for (card_t card = TWO; card <= ACE; card++) {
		char letters[] = "  23456789TA";
		deck_put(players, card);
		deck_put(players, card);
		printf("   +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+\n");
		printf("%c%c ", letters[card], letters[card]);

		assert(deck[TOTAL] == 8 * 52);
		assert(players[TOTAL] == 2);
		for (dealers = TWO; dealers <= ACE; dealers++) {
			search(&rsi, deck, players, dealers);
			maximize_root(&rsi, str);
			printf("| %s ", str);
			fflush(stdout);
		}
		printf("|\n");

		deck_draw(players, card);
		deck_draw(players, card);
	}
	printf("   +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+\n");
	return 0;
#endif
}
