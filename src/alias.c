/*
*   Laser- a command line utility to assemble LC3 assembly code
*
*   Copyright Notice:
*
*       Copyright 2018, 2019 Zhiyuan Fan
*
*   License Notice:
*
*       Laser is free software: you can redistribute it and/or modify
*       it under the terms of the GNU General Public License as published by
*       the Free Software Foundation, either version 3 of the License, or
*       (at your option) any later version.
*
*       Laser is distributed in the hope that it will be useful,
*       but WITHOUT ANY WARRANTY; without even the implied warranty of
*       MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*       GNU General Public License for more details.
*
*       You should have received a copy of the GNU General Public License
*       along with Laser.  If not, see <https://www.gnu.org/licenses/>.
*/

#define USES_ALIAS
#define USES_OPERAND
#include "laser.h"

#define DEFAULT_ALIAS_SIZE 8

void addalias (aliasarr_t *a, uint32_t ln, Token *word, Token *reg)
{
    uint16_t index = a->ind;
    if (index == a->cap) {
        a->cap *= 2;
        a->arr = (alias_t*) realloc (a->arr, a->cap * sizeof (alias_t));
    }

    a->arr[index].count = 0;
    a->arr[index].ln = ln;
    a->arr[index].word = (Token*) malloc (sizeof (Token));
    copytoken (a->arr[index].word, word);
    a->arr[index].reg = (Token*) malloc (sizeof (Token));
    copytoken (a->arr[index].reg, reg);

    ++a->ind;
}

int32_t findalias (aliasarr_t *a, Token *word)
{
    for (uint16_t i = 0; i < a->ind; i++) {
        if (strcmp (word->str, a->arr[i].word->str) == 0) {
            return i;
        }
    }
    return -1;
}

aliasarr_t* initaliasarr (void)
{
    aliasarr_t *a = (aliasarr_t*) malloc (sizeof (aliasarr_t));
    a->ind = 0;
    a->cap = DEFAULT_ALIAS_SIZE;
    a->arr = (alias_t*) malloc (a->cap * sizeof (alias_t));
    return a;
}

void freealiasarr (aliasarr_t *a)
{
    if (a == 0) return;

    for (uint16_t i = 0; i < a->ind; i++) {
        if (a->arr[i].word) freetoken (a->arr[i].word);
        if (a->arr[i].reg) freetoken (a->arr[i].reg);
    }

    free (a->arr);
    free (a);
}
