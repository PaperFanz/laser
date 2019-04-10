#define USES_LABEL
#define USES_OPERAND
#include "laser.h"

#define DEFAULT_LABEL_SIZE 8

void addlabel (labelarr_t *l, uint32_t ln, Token *label, uint16_t addr)
{
    uint16_t index = l->ind;
    if (index == l->cap) {
        l->cap *= 2;
        l->arr = (label_t*) realloc (l->arr, l->cap * sizeof (label_t));
    }

    l->arr[index].count = 0;
    l->arr[index].ln = ln;
    l->arr[index].label = (Token*) malloc (sizeof (Token));
    copytoken (l->arr[index].label, label);
    l->arr[index].address = addr;
    l->ind++;
}

uint8_t isvalidlabel (Token *tok)
{
    char *str = tok->str;
    if (!(isalpha (str[0]) || str[0] == '_')) return 0;

    for (uint16_t i = 0; str[i]; i++) {
        if (!(isalnum (str[i]) || str[i] == '_')) return 0;
    }

    if (isoperand (tok) + isregister (tok) + 2) return 0;

    return 1;
}

uint16_t labeladdr (labelarr_t *l, Token *label)
{
    for (uint16_t i = 0; i < l->ind; i++) {
        if (strcmp (label->str, l->arr[i].label->str) == 0) {
            return l->arr[i].address;
        }
    }
    return 0;
}

labelarr_t* initlabelarr (void)
{
    labelarr_t *l = (labelarr_t*) malloc (sizeof (labelarr_t));
    l->ind = 0;
    l->cap = DEFAULT_LABEL_SIZE;
    l->arr = (label_t*) malloc (l->cap * sizeof (label_t));
    return l;
}

void freelabelarr (labelarr_t *l)
{
    if (l == 0) return;

    for (uint16_t i = 0; i < l->ind; i++) {
        freetoken (l->arr[i].label);
    }

    free (l->arr);
    free (l);
}