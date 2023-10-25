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

const double BLACKJACK_PAYS       = 3.0 / 2.0; /* 1.0 to DBL_MAX */
const double SURRENDER_GIVES_BACK = 0.5;       /* 0.0 to 1.0 */
const int    MAX_SPLITS           = 1;         /* 0 to INT_MAX */
const int    DOUBLE_AFTER_SPLIT   = 0;         /* 0 or 1 */
const int    PEEK_BLACKJACK       = 1;         /* 0 or 1 */
const int    HITS_ON_SOFT_17      = 0;         /* 0 or 1 */
const int    DRAW_TO_SPLIT_ACES   = 0;         /* 0 or 1 */
const int    DOUBLE_ONLY_9_10_11  = 0;         /* 0 or 1 */
const int    RESPLIT_ACES         = 0;         /* 0 or 1 */
const int    SURRENDER_VS_ACES    = 1;         /* 0 or 1 */
const int    SURRENDER_ANYTIME    = 0;         /* 0 or 1 */

double maximize_root(struct rootsearchinfo *rsi, char str[4]) {
	double e = -INFINITY;
	char hit_or_stand = 0;
	if (rsi->hit > e) {
		e = rsi->hit;
		hit_or_stand = 'H';
		if (str)
			sprintf(str, " H ");
	}
	if (rsi->stand > e) {
		e = rsi->stand;
		hit_or_stand = 'S';
		if (str)
			sprintf(str, " S ");
	}
	if (rsi->doubledown > e) {
		e = rsi->doubledown;
		if (str)
			sprintf(str, "D/%c", hit_or_stand);
	}
	if (rsi->split > e) {
		e = rsi->split;
		if (str)
			sprintf(str, " P ");
	}
	if (rsi->surrender > e) {
		e = rsi->surrender;
		if (str)
			sprintf(str, "R/%c", hit_or_stand);
	}
	return e;
}

double search_dealer(deck_t deck, unsigned players, int blackjack, deck_t dealers) {
	unsigned sum = deck_sum(dealers);
	int dealer_blackjack = dealers[TOTAL] == 2 && sum == 21;
	int dealer_hits = sum < 17;
	if (HITS_ON_SOFT_17 && sum == 17) {
		deck_put(dealers, TEN);
		dealer_hits = deck_sum(dealers) == 17;
		deck_draw(dealers, TEN);
	}
	if (dealer_hits) {
		double p = 0.0;

#if 0
		card_t up_card = deck_first(dealers);
#endif
		card_t impossible = LOWACE;
		assert(!deck[impossible]);
#if 0
		assert(!deck[impossible]);
		if (up_card == ACE)
			impossible = TEN;
		else if (up_card == TEN)
			impossible = ACE;
#endif

		for (card_t card = TWO; card <= ACE; card++) {
#if 0
			if (PEEK_BLACKJACK && up_card + card == 21)
				continue;
#endif
			if (deck[card]) {
				double q = (double)deck[card] / (deck[TOTAL] - deck[impossible]);
				deck_draw(deck, card);
				deck_put(dealers, card);
				double r = search_dealer(deck, players, blackjack, dealers);
				deck_draw(dealers, card);
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
		if (players > sum)
			return 1.0 + (BLACKJACK_PAYS - 1.0) * blackjack;
		else if (players < sum)
			return -1.0;
		else
			return BLACKJACK_PAYS * (blackjack && !dealer_blackjack) - 1.0 * (!blackjack && dealer_blackjack);
	}
}

card_t can_split(deck_t players) {
	if (players[TOTAL] != 2)
		return 0;

	for (card_t card = TWO; card <= ACE; card++)
		if (players[card] == 2)
			return card;

	return 0;
}

double search_action(deck_t deck, deck_t players, deck_t dealers, unsigned actions, long splits) {
	if (actions & PEEK && 0) {
		double ret = 0.0;

		card_t up_card = deck_first(dealers);
		card_t impossible = LOWACE;
		assert(!deck[impossible]);
		if (up_card == ACE)
			impossible = TEN;
		else if (up_card == TEN)
			impossible = ACE;

		for (card_t card = TWO; card <= ACE; card++) {
			if (up_card + card == 21) {
				continue;
			}
			if (deck[card]) {
				double q = (double)deck[card] / (deck[TOTAL] - deck[impossible]);
				deck_draw(deck, card);
				deck_put(dealers, card);
				double r = search_action(deck, players, dealers, actions ^ PEEK, splits);
				deck_draw(dealers, card);
				deck_put(deck, card);
				ret += q * r;
			}
		}
		return ret;
	}

	unsigned sum = deck_sum(players);
	if (sum > 21)
		return -1.0;

	unsigned new_actions = STAND | HIT;
	if (SURRENDER_ANYTIME)
		new_actions |= SURRENDER;
	card_t splitcard = can_split(players);

	double e = -INFINITY;
	if (actions & HIT) {
		double hit = 0.0;
		for (card_t card = TWO; card <= ACE; card++) {
			if (deck[card]) {
				double q = (double)deck[card] / deck[TOTAL];
				deck_draw(deck, card);
				deck_put(players, card);
				double r = search_action(deck, players, dealers, new_actions, splits);
				deck_draw(players, card);
				deck_put(deck, card);
				hit += q * r;
			}
		}
		e = MAX(e, hit);
	}
	if (actions & STAND) {
		int blackjack = players[TOTAL] == 2 && sum == 21;
		double stand = search_dealer(deck, sum, blackjack, dealers);
		e = MAX(e, stand);
	}
	if (actions & DOUBLEDOWN && (!DOUBLE_ONLY_9_10_11 || (9 <= sum && sum <= 11))) {
		double doubledown = 0.0;
		for (card_t card = TWO; card <= ACE; card++) {
			if (deck[card]) {
				double q = (double)deck[card] / deck[TOTAL];
				deck_draw(deck, card);
				deck_put(players, card);
				double r = search_action(deck, players, dealers, STAND, splits);
				deck_draw(players, card);
				deck_put(deck, card);
				doubledown += q * r;
			}
		}
		e = MAX(e, 2 * doubledown);
	}
	if (actions & SPLIT && splitcard) {
		double split = 0.0;
		deck_draw(players, splitcard);

		for (card_t card = TWO; card <= ACE; card++) {
			if (deck[card]) {
				double q = (double)deck[card] / deck[TOTAL];
				deck_draw(deck, card);
				deck_put(players, card);
				new_actions &= ~(HIT | SPLIT | DOUBLEDOWN);
				if (splits + 1 < MAX_SPLITS && (RESPLIT_ACES || card != ACE))
					new_actions |= SPLIT;
				if (DRAW_TO_SPLIT_ACES || card != ACE)
					new_actions |= HIT;
				if (DOUBLE_AFTER_SPLIT)
					new_actions |= DOUBLEDOWN;
				double r = search_action(deck, players, dealers, new_actions, splits + 1);
				deck_draw(players, card);
				deck_put(deck, card);
				split += q * r;
			}
		}

		deck_put(players, splitcard);
		e = MAX(e, 2 * split);
	}
	/* DOES NOT WORK PROPERLY WITH !SURRENDER_VS_ACES */
	if (actions & SURRENDER && (SURRENDER_VS_ACES || deck_first(dealers) != ACE)) {
		double surrender = SURRENDER_GIVES_BACK - 1.0;
		e = MAX(e, surrender);
	}

	return e;
}

struct rootsearchinfo *search(struct rootsearchinfo *rsi, deck_t deck, deck_t players, card_t dealers) {
	deck_t dealersdeck;
	deck_single(dealersdeck, dealers);

	deck_draw(deck, dealers);
	for (card_t card = TWO; card <= ACE; card++)
		for (unsigned i = 0; i < players[card]; i++)
			deck_draw(deck, card);

	unsigned peek = PEEK_BLACKJACK ? PEEK : 0;

	/* HIT */
	rsi->hit = search_action(deck, players, dealersdeck, peek | HIT, 0);

	/* STAND */
	rsi->stand = search_action(deck, players, dealersdeck, peek | STAND, 0);

	/* DOUBLEDOWN */
	rsi->doubledown = search_action(deck, players, dealersdeck, peek | DOUBLEDOWN, 0);

	/* SPLIT */
	rsi->split = search_action(deck, players, dealersdeck, peek | SPLIT, 0);

	/* SURRENDER */
	rsi->surrender = search_action(deck, players, dealersdeck, peek | SURRENDER, 0);

	deck_put(deck, dealers);
	for (card_t card = TWO; card <= ACE; card++)
		for (unsigned i = 0; i < players[card]; i++)
			deck_put(deck, card);

	return rsi;
}
