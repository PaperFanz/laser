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

#define USES_OPERAND
#include "laser.h"

int8_t arrcmp (char *str, const char *arr[][2], uint8_t size)
{
    for (uint8_t i = 0; i < size; i++) {
        if (strcmp (str, arr[i][0]) == 0 ||
            strcmp (str, arr[i][1]) == 0) return i;
    }
    return -1;
}

int8_t brrcmp (char *str, const char *arr[][3], uint8_t size)
{
    for (uint8_t i = 0; i < size; i++) {
        if (strcmp (str, arr[i][0]) == 0 ||
            strcmp (str, arr[i][1]) == 0 ||
            strcmp (str, arr[i][2]) == 0) return i;
    }
    return -1;
}

int8_t isregister (Token *token)
{
    const char *regs[][2] = {
        {"R0", "r0"},
        {"R1", "r1"},
        {"R2", "r2"},
        {"R3", "r3"},
        {"R4", "r4"},
        {"R5", "r5"},
        {"R6", "r6"},
        {"R7", "r7"}
    };

    return arrcmp (token->str, regs, 8);
}

int8_t isbranch (Token *token)
{
    const char *brs[][3] = {
        {"BR", "BR", "br"},
        {"BRP", "BRp", "brp"},
        {"BRZ", "BRz", "brz"},
        {"BRZP", "BRzp", "brzp"},
        {"BRN", "BRn", "brn"},
        {"BRNP", "BRnp", "brnp"},
        {"BRNZ", "BRnz", "brnz"},
        {"BRNZP", "BRnzp", "brnzp"}
    };

    int8_t tmp = brrcmp (token->str, brs, 8);
    tmp = (tmp == 0) ? 7 : tmp;
    return tmp;
}

int8_t istrap (Token *token)
{
    const char *traps[][2] = {
        {"GETC", "getc"},
        {"OUT", "out"},
        {"PUTS", "puts"},
        {"IN", "in"},
        {"PUTSP", "putsp"},
        {"HALT", "halt"},
        {"TRAP", "trap"}
    };

    int8_t tmp = arrcmp (token->str, traps, 7);
    if (tmp == -1) return -1;
    else if (tmp == 6) return 0;
    else return 32 + tmp;
}

int8_t isoperand (Token *token)
{
    const char *ops[][2] = {
        {"BR", "br"},
        {"ADD", "add"},
        {"LD", "ld"},
        {"ST", "st"},
        {"JSR", "jsr"},
        {"AND", "and"},
        {"LDR", "ldr"},
        {"STR", "str"},
        {"RTI", "rti"},
        {"NOT", "not"},
        {"LDI", "ldi"},
        {"STI", "sti"},
        {"JMP", "jmp"},
        {"", ""},
        {"LEA", "lea"},
        {"TRAP", "trap"}
    };

    int8_t op = arrcmp (token->str, ops, 16);
    if (op < 0) {																// could be branch, trap, or assembler shortcut
        if (isbranch (token) >= 0) {
            op = BR;
        } else if (istrap (token) >= 0) {
            op = TRAPS;
        } else if (strcmp (token->str, "JSRR") == 0 ||
                   strcmp (token->str, "jsrr") == 0) {
            op = JSRR;
        } else if (strcmp (token->str, "RET") == 0 ||
                   strcmp (token->str, "ret") == 0) {
            op = RET;
        }
    } else if (op == 13) {														// invalid op
        op = -1;
    }
    return op;
}