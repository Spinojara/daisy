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
#include <math.h>
#include <time.h>

#include "deck.h"
#include "search.h"
#include "truecount.h"

const double BLACKJACK_PAYS       = 3.0 / 2.0; /* 1.0 to DBL_MAX */
const double SURRENDER_GIVES_BACK = 0.5;       /* 0.0 to 1.0 */
const int    DECKS                = 8;         /* 0 to INT_MAX */
const int    MAX_SPLITS           = 1;         /* 0 to INT_MAX */
const int    DOUBLE_AFTER_SPLIT   = 0;         /* 0 or 1 */
const int    PEEK_BLACKJACK       = 1;         /* 0 or 1 */
const int    DONT_PEEK_10         = 1;         /* 0 or 1 */
const int    HITS_ON_SOFT_17      = 0;         /* 0 or 1 */
const int    DRAW_TO_SPLIT_ACES   = 0;         /* 0 or 1 */
const int    DOUBLE_ONLY_9_10_11  = 0;         /* 0 or 1 */
const int    RESPLIT_ACES         = 0;         /* 0 or 1 */
const int    SURRENDER_ALLOWED    = 1;         /* 0 or 1 */
const int    SURRENDER_VS_ACES    = 1;         /* 0 or 1 */
const int    SURRENDER_ANYTIME    = 0;         /* 0 or 1 */

int main(void) {
	char letters[] = "  23456789TA";
#if 0
	double var = simulate_true(1000000, 8, 0.5);
	printf("variance: %f\n", var);
#endif
#if 1
	deck_t deck;
	deck_init(deck, DECKS);
	deck_t player = { 0 };
	struct rootsearchinfo rsi = { 0 };

#if 0
	for (unsigned i = 0; i < 2; i++) {
		for (unsigned j = 0; j < 4; j++)
			deck_draw(deck, TEN);
		deck_draw(deck, ACE);
		for (unsigned j = 0; j < 3; j++)
			deck_draw(deck, SEVEN + j);
	}
#endif
#if 1
	for (unsigned i = 0; i < 10; i++) {
		for (card_t card = TWO; card <= SIX; card++)
			deck_draw(deck, card);
		for (unsigned j = 0; j < 3; j++)
			deck_draw(deck, SEVEN + j);
	}
#endif

	double e = 0;
	for (card_t card1 = TWO; card1 <= ACE; card1++) {
		double p = (double)deck[card1] / deck[TOTAL];

		deck_draw(deck, card1);
		deck_put(player, card1);
		for (card_t daisy = TWO; daisy <= ACE; daisy++) {
			double q = (double)deck[daisy] / deck[TOTAL];

			deck_draw(deck, daisy);
			for (card_t card2 = TWO; card2 <= ACE; card2++) {
				double r = (double)deck[card2] / deck[TOTAL];

				deck_put(player, card2);

				deck_put(deck, card1);
				deck_put(deck, daisy);

				search(&rsi, deck, player, daisy);
				maximize_root(&rsi, NULL);

				deck_draw(deck, card1);
				deck_draw(deck, daisy);
				deck_draw(deck, card2);

				if (daisy == ACE && PEEK_BLACKJACK) {
					double s = (double)deck[TEN] / deck[TOTAL];
					int blackjack = deck_sum(player) == 21;
					rsi.expectation = -1.0 * !blackjack * s + rsi.expectation * (1 - s);
				}
				else if (daisy == TEN && PEEK_BLACKJACK && !DONT_PEEK_10) {
					double s = (double)deck[ACE] / deck[TOTAL];
					int blackjack = deck_sum(player) == 21;
					rsi.expectation = -1.0 * !blackjack * s + rsi.expectation * (1 - s);
				}

				e += p * q * r * rsi.expectation;


				unsigned l = ACE - TWO + 1;
				printf("%u/%u, %c%c-%c expectation is currently: %f\n", (card1 - TWO) * l * l + (daisy - TWO) * l + (card2 - TWO),
						l * l * l, letters[card1], letters[card2], letters[daisy], e);

				deck_draw(player, card2);
				deck_put(deck, card2);
			}
			deck_put(deck, daisy);
		}
		deck_draw(player, card1);
		deck_put(deck, card1);
	}

	printf("True: %f\n", deck_true(deck));
#endif
#if 0

	deck_t deck;
	deck_init(deck, DECKS);

	struct rootsearchinfo rsi;
	deck_t player = { 0 };

	card_t card1   = ACE;
	card_t card2   = FOUR;
	card_t daisy   = TEN;
	char str[4];

#if 0
	for (unsigned i = 0; i < 1; i++) {
		for (unsigned j = 0; j < 4; j++)
			deck_draw(deck, TEN);
		deck_draw(deck, ACE);
		for (unsigned j = 0; j < 3; j++)
			deck_draw(deck, SEVEN + j);
	}
#endif
#if 1
	for (unsigned i = 0; i < 8; i++) {
		for (card_t card = TWO; card <= SIX; card++)
			deck_draw(deck, card);
		for (unsigned j = 0; j < 3; j++)
			deck_draw(deck, SEVEN + j);
	}
#endif
	printf("True: %f\n", deck_true(deck));
#if 0
	deck_put(player, card1);
	deck_put(player, card2);

	search(&rsi, deck, player, daisy);
	maximize_root(&rsi, str);
	printf("hit:         %f\n", rsi.hit);
	printf("stand:       %f\n", rsi.stand);
	printf("doubledown:  %f\n", rsi.doubledown);
	printf("split:       %f\n", rsi.split);
	printf("surrender:   %f\n", rsi.surrender);
	printf("expectation: %f\n", rsi.expectation);
	printf("best:        %c\n", rsi.best);

	deck_draw(player, card1);
	deck_draw(player, card2);
#else
	printf(" H    2     3     4     5     6     7     8     9     T     A   \n");
	for (int sum = 4; sum <= 20; sum++) {
		if (sum % 2) {
			card1 = sum / 2;
			card2 = sum / 2 + 1;
		}
		else {
			card1 = sum / 2;
			card2 = sum / 2;
		}
		deck_put(player, card1);
		deck_put(player, card2);
		printf("   +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+\n");
		printf("%2d ", sum);

		assert(deck[TOTAL] == 8 * 52);
		assert(player[TOTAL] == 2);
		for (daisy = TWO; daisy <= ACE; daisy++) {
			search(&rsi, deck, player, daisy);
			rsi.split = -INFINITY;
			maximize_root(&rsi, str);
			printf("| %s ", str);
			fflush(stdout);
		}
		printf("|\n");

		deck_draw(player, card1);
		deck_draw(player, card2);
	}
	for (int sum = 21; sum <= 21; sum++) {
		deck_put(player, THREE);
		deck_put(player, EIGHT);
		deck_put(player, TEN);
		printf("   +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+\n");
		printf("%2d ", sum);

		assert(deck[TOTAL] == 8 * 52);
		assert(player[TOTAL] == 2);
		for (daisy = TWO; daisy <= ACE; daisy++) {
			search(&rsi, deck, player, daisy);
			rsi.split = -INFINITY;
			maximize_root(&rsi, str);
			printf("| %s ", str);
			fflush(stdout);
		}
		printf("|\n");

		deck_draw(player, THREE);
		deck_draw(player, EIGHT);
		deck_draw(player, TEN);
	}
	printf("   +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+\n");
	printf(" S    2     3     4     5     6     7     8     9     T     A   \n");
	for (int sum = 12; sum <= 12; sum++) {
		card1 = card2 = ACE;
		deck_put(player, card1);
		deck_put(player, card2);
		printf("   +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+\n");
		printf("%2d ", sum);

		assert(deck[TOTAL] == 8 * 52);
		assert(player[TOTAL] == 2);
		for (daisy = TWO; daisy <= ACE; daisy++) {
			search(&rsi, deck, player, daisy);
			rsi.split = -INFINITY;
			maximize_root(&rsi, str);
			printf("| %s ", str);
			fflush(stdout);
		}
		printf("|\n");

		deck_draw(player, card1);
		deck_draw(player, card2);
	}
	for (int sum = 13; sum <= 21; sum++) {
		card1 = ACE;
		card2 = sum - card1;
		deck_put(player, card1);
		deck_put(player, card2);
		printf("   +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+\n");
		printf("%2d ", sum);

		assert(deck[TOTAL] == 8 * 52);
		assert(player[TOTAL] == 2);
		for (daisy = TWO; daisy <= ACE; daisy++) {
			search(&rsi, deck, player, daisy);
			maximize_root(&rsi, str);
			printf("| %s ", str);
			fflush(stdout);
		}
		printf("|\n");

		deck_draw(player, card1);
		deck_draw(player, card2);
	}
	printf("   +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+\n");
	printf(" P    2     3     4     5     6     7     8     9     T     A   \n");
	for (card_t card = TWO; card <= ACE; card++) {
		deck_put(player, card);
		deck_put(player, card);
		printf("   +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+\n");
		printf("%c%c ", letters[card], letters[card]);

		assert(deck[TOTAL] == 8 * 52);
		assert(player[TOTAL] == 2);
		for (daisy = TWO; daisy <= ACE; daisy++) {
			search(&rsi, deck, player, daisy);
			maximize_root(&rsi, str);
			printf("| %s ", str);
			fflush(stdout);
		}
		printf("|\n");

		deck_draw(player, card);
		deck_draw(player, card);
	}
	printf("   +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+\n");
	return 0;
#endif
#endif
}
