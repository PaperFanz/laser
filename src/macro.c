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

#define USES_MACRO
#define USES_OPERAND
#include "laser.h"

#define DEFAULT_MACRO_SIZE 8

void addmacro (macroarr_t *m, uint32_t ln, Token *macro, Token *replace)
{
    uint16_t index = m->ind;
    if (index == m->cap) {
        m->cap *= 2;
        m->arr = (macro_t*) realloc (m->arr, m->cap * sizeof (macro_t));
    }

    m->arr[index].count = 0;
    m->arr[index].ln = ln;
    m->arr[index].macro = (Token*) malloc (sizeof (Token));
    copytoken (m->arr[index].macro, macro);
    m->arr[index].replace = (Token*) malloc (sizeof (Token));
    copytoken (m->arr[index].replace, replace);

    m->ind++;
}

int32_t findmacro (macroarr_t *m, Token *macro)
{
    for (uint16_t i = 0; i < m->ind; i++) {
        if (strcmp (macro->str, m->arr[i].macro->str) == 0) {
            return i;
        }
    }
    return -1;
}

TokenBuffer* replacemacro (TokenBuffer *buf, Token *replace)
{
    TokenBuffer *tmp = tokenize (replace->str);
    for (uint16_t i = 0; i < tmp->toknum; ++i) {
        if (strcmp (tmp->token[i]->str, "$") == 0) {
            if (buf->toknum > 1) {
                freetoken (tmp->token[i]);
                tmp->token[i] = (Token*) malloc (sizeof (Token));
                copytoken (tmp->token[i], buf->token[1]);
            } else {
                freetokenarr (buf);
                freetokenarr (tmp);
                return NULL;
            }
        }
    }
    freetokenarr (buf);
    return tmp;
}

macroarr_t* initmacroarr (void)
{
    macroarr_t *m = (macroarr_t*) malloc (sizeof (macroarr_t));
    m->ind = 0;
    m->cap = DEFAULT_MACRO_SIZE;
    m->arr = (macro_t*) malloc (m->cap * sizeof (macro_t));
    return m;
}

void freemacroarr (macroarr_t *m)
{
    if (m == 0) return;

    for (uint16_t i = 0; i < m->ind; i++) {
        freetoken (m->arr[i].macro);
        freetoken (m->arr[i].replace);
    }

    free (m->arr);
    free (m);
}