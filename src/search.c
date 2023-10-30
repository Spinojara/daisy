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

#include "search.h"

#include <assert.h>
#include <stdio.h>
#include <math.h>

#include "util.h"

extern const double BLACKJACK_PAYS;
extern const double SURRENDER_GIVES_BACK;
extern const int    MAX_SPLITS;
extern const int    DOUBLE_AFTER_SPLIT;
extern const int    PEEK_BLACKJACK;
extern const int    DONT_PEEK_10;
extern const int    HITS_ON_SOFT_17;
extern const int    DRAW_TO_SPLIT_ACES;
extern const int    DOUBLE_ONLY_9_10_11;
extern const int    RESPLIT_ACES;
extern const int    SURRENDER_ALLOWED;
extern const int    SURRENDER_VS_ACES;
extern const int    SURRENDER_ANYTIME;

void maximize_root(struct rootsearchinfo *rsi, char str[4]) {
	rsi->expectation = -INFINITY;
	char hit_or_stand = 0;
	if (rsi->hit > rsi->expectation) {
		rsi->expectation = rsi->hit;
		rsi->best = 'H';
		hit_or_stand = 'H';
		if (str)
			sprintf(str, " H ");
	}
	if (rsi->stand > rsi->expectation) {
		rsi->expectation = rsi->stand;
		hit_or_stand = 'S';
		rsi->best = 'S';
		if (str)
			sprintf(str, " S ");
	}
	if (rsi->doubledown > rsi->expectation) {
		rsi->expectation = rsi->doubledown;
		rsi->best = 'D';
		if (str)
			sprintf(str, "D/%c", hit_or_stand);
	}
	if (rsi->split > rsi->expectation) {
		rsi->expectation = rsi->split;
		rsi->best = 'P';
		if (str)
			sprintf(str, " P ");
	}
	if (rsi->surrender > rsi->expectation) {
		rsi->expectation = rsi->surrender;
		rsi->best = 'R';
		if (str)
			sprintf(str, "R/%c", hit_or_stand);
	}
}

double search_daisy(deck_t deck, unsigned player, int blackjack, deck_t daisy) {
	unsigned sum = deck_sum(daisy);
	int daisy_blackjack = daisy[TOTAL] == 2 && sum == 21;
	int daisy_hits = sum < 17;
	if (HITS_ON_SOFT_17 && sum == 17) {
		deck_put(daisy, TEN);
		daisy_hits = deck_sum(daisy) == 17;
		deck_draw(daisy, TEN);
	}
	if (daisy_hits) {
		double p = 0.0;

		card_t up_card = deck_first(daisy);
		card_t impossible = NONE;

		if (PEEK_BLACKJACK) {
			if (up_card == ACE)
				impossible = TEN;
			else if (up_card == TEN && !DONT_PEEK_10)
				impossible = ACE;
		}

		for (card_t card = TWO; card <= ACE; card++) {
			if (card == impossible)
				continue;
			if (deck[card]) {
				double q = (double)deck[card] / (deck[TOTAL] - deck[impossible]);
				deck_draw(deck, card);
				deck_put(daisy, card);
				double r = search_daisy(deck, player, blackjack, daisy);
				deck_draw(daisy, card);
				deck_put(deck, card);
				p += q * r;
			}
		}
		return p;
	}
	else if (sum > 21) {
		return 1.0 + (BLACKJACK_PAYS - 1.0) * blackjack;
	}
	else {
		if (player > sum)
			return 1.0 + (BLACKJACK_PAYS - 1.0) * blackjack;
		else if (player < sum)
			return -1.0;
		else
			return BLACKJACK_PAYS * (blackjack && !daisy_blackjack) - 1.0 * (!blackjack && daisy_blackjack);
	}
}

card_t can_split(deck_t player) {
	if (player[TOTAL] != 2)
		return 0;

	for (card_t card = TWO; card <= ACE; card++)
		if (player[card] == 2)
			return card;

	return 0;
}

double search_action(deck_t deck, deck_t player, deck_t daisy, unsigned actions, long splits, int ply) {
	unsigned sum = deck_sum(player);
	if (sum > 21)
		return -1.0;

	unsigned new_actions = STAND | HIT;
	if (SURRENDER_ANYTIME)
		new_actions |= SURRENDER;
	card_t splitcard = can_split(player);

	double e = -INFINITY;
	if (actions & STAND) {
		int blackjack = player[TOTAL] == 2 && sum == 21 && !splits;
		double stand = search_daisy(deck, sum, blackjack, daisy);
		e = MAX(e, stand);
	}
	if (actions & HIT) {
		double hit = 0.0;
		for (card_t card = TWO; card <= ACE; card++) {
			if (deck[card]) {
				double q = (double)deck[card] / deck[TOTAL];
				deck_draw(deck, card);
				deck_put(player, card);
				double r = search_action(deck, player, daisy, new_actions, splits, ply + 1);
				deck_draw(player, card);
				deck_put(deck, card);
				hit += q * r;
			}
		}
		e = MAX(e, hit);
	}
	if (actions & DOUBLEDOWN && (!DOUBLE_ONLY_9_10_11 || (9 <= sum && sum <= 11))) {
		double doubledown = 0.0;
		for (card_t card = TWO; card <= ACE; card++) {
			if (deck[card]) {
				double q = (double)deck[card] / deck[TOTAL];
				deck_draw(deck, card);
				deck_put(player, card);
				double r = search_action(deck, player, daisy, STAND, splits, ply + 1);
				deck_draw(player, card);
				deck_put(deck, card);
				doubledown += q * r;
			}
		}
		e = MAX(e, 2 * doubledown);
	}
	/* Splits are played one hand at a time instead of at the same
	 * time like normal blackjack. The error should be negligible.
	 */
	if (actions & SPLIT && splitcard) {
		double split = 0.0;
		deck_draw(player, splitcard);

		new_actions &= ~(HIT | SPLIT | DOUBLEDOWN);
		if (splits + 1 < MAX_SPLITS && (RESPLIT_ACES || splitcard != ACE))
			new_actions |= SPLIT;
		if (DRAW_TO_SPLIT_ACES || splitcard != ACE) {
			new_actions |= HIT;
			if (DOUBLE_AFTER_SPLIT)
				new_actions |= DOUBLEDOWN;
		}

		for (card_t card = TWO; card <= ACE; card++) {
			if (deck[card]) {
				double q = (double)deck[card] / deck[TOTAL];
				deck_draw(deck, card);
				deck_put(player, card);
				double r = search_action(deck, player, daisy, new_actions, splits + 1, ply + 1);
				deck_draw(player, card);
				deck_put(deck, card);
				split += q * r;
			}
		}

		deck_put(player, splitcard);
		e = MAX(e, 2 * split);
	}
	if (actions & SURRENDER && (SURRENDER_VS_ACES || deck_first(daisy) != ACE)) {
		double surrender = SURRENDER_GIVES_BACK - 1.0;
		e = MAX(e, surrender);
	}

	return e;
}

struct rootsearchinfo *search(struct rootsearchinfo *rsi, deck_t deck, deck_t player, card_t daisy) {
	deck_t daisy_deck;
	deck_single(daisy_deck, daisy);

	deck_draw(deck, daisy);
	for (card_t card = TWO; card <= ACE; card++)
		for (unsigned i = 0; i < player[card]; i++)
			deck_draw(deck, card);

	/* HIT */
	rsi->hit = search_action(deck, player, daisy_deck, HIT, 0, 0);

	/* STAND */
	rsi->stand = search_action(deck, player, daisy_deck, STAND, 0, 0);

	/* DOUBLEDOWN */
	rsi->doubledown = search_action(deck, player, daisy_deck, DOUBLEDOWN, 0, 0);

	/* SPLIT */
	rsi->split = search_action(deck, player, daisy_deck, SPLIT, 0, 0);

	/* SURRENDER */
	rsi->surrender = SURRENDER_ALLOWED ? search_action(deck, player, daisy_deck, SURRENDER, 0, 0) : -INFINITY;

	deck_put(deck, daisy);
	for (card_t card = TWO; card <= ACE; card++)
		for (unsigned i = 0; i < player[card]; i++)
			deck_put(deck, card);

	return rsi;
}
