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

#ifndef SEARCH_H
#define SEARCH_H

#include "deck.h"

enum {
	NONE       =  0x0,
	HIT        =  0x1,
	STAND      =  0x2,
	DOUBLEDOWN =  0x4,
	SPLIT      =  0x8,
	SURRENDER  = 0x10,
	PEEK       = 0x20,
};

struct rootsearchinfo {
	double hit;
	double stand;
	double doubledown;
	double split;
	double surrender;
};

double maximize_root(struct rootsearchinfo *rsi, char str[4]);

struct rootsearchinfo *search(struct rootsearchinfo *rsi, deck_t deck, deck_t players, card_t dealers);

#endif
