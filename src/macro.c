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