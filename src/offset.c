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

#include "laser.h"

// extends isxdigit functionality by returning decimal value (0-16)
int8_t isxchar (char c)
{
    const char hex_chars[16] = {'0','1','2','3',
                                '4','5','6','7',
                                '8','9','A','B',
                                'C','D','E','F'};
    for(int8_t i=0; i<=15; i++){
        if( (i<=9) && (c == hex_chars[i])) {
            return i;
        }
        else if( (i>9) && ((c == hex_chars[i])||(c == hex_chars[i] + 0x20))) {
            return i;
        }
    }
    return -1;
}

int16_t hextodec (char hex[])
{
    int16_t dec_num = 0;
    for (int8_t i = 0; isxchar (hex[i]) >= 0; i++) {
        dec_num = dec_num * 16 + isxchar (hex[i]);
    }

    return dec_num;
}

int16_t hexoffset (char *offset)
{
    int16_t offval = 0;
    char *hex = strchr (offset, 'x');       // for normal people who use 0x00
    if (!hex) hex = strchr (offset, 'X');   // for the heathens that use 0X00
    hex++;
    if (hex[0] == '-') {
        hex++;
        offval = -hextodec (hex);
    } else {
        offval = hextodec (hex);
    }

    return offval;
}


int16_t twocomtodec(int8_t bin[], int8_t size)
{
    int16_t dec_num = 0, i = 0, r, n = 0;
    while (i <= size - 1) {
        n *= 10;
        n += bin[i];
        i++;
    }

    i = 0;

    while (n > 0) {
        r = n % 10;
        n /= 10;
        dec_num += r * (1 << i);
        i++;
    }

    if (bin[0] == 1) dec_num -= r * (1 << i);

    return dec_num;
}

int16_t binoffset (char *offset)
{
    int8_t neg = 0;
    int16_t offval = 0;
    // move pointer past leading characters
    char *bin = strchr (offset, 'b');       // for normal people who use 0b00
    if (!bin) bin = strchr (offset, 'B');   // for the heathens that use 0B00

    bin++;
    if (bin[0] == '-') {
        bin++;
        neg = 1;
    }

    uint8_t i = 0, j = 16;
    int8_t bin16[16] = {0};
    for (i = strlen (bin); i >= 0; i--) {
        bin16[j] = bin[i] - 0x30;
        j--;
    }

    // sign extend
    for (i = j; i >= 0; i--) {
        bin16[i] = bin16[j + 1];
    }

    offval = twocomtodec (bin16, 16);
    if (neg) offval = -offval;

    return offval;
}

int16_t decoffset (char *offset)
{
    int8_t neg = 0;
    char *dec = strchr (offset, '#');
    if (dec == NULL) {
        dec = offset;
    } else {
        dec++;
    }

    if (dec[0] == '-') {
        neg = 1;
        dec++;
    }

    int16_t off = 0, i = 0;
    while (dec[i] != '\0') {
        off = off * 10 + (dec[i] - 0x30);
        i++;
    }

    if (neg) off = -off;
    return off;
}

enum OFF_TYPE {
    INVALID_OFF,
    HEX,
    BIN,
    DEC
};

uint8_t offtype (Token *tok)
{
    char *token = tok->str;
    char *tmp;
    char c;
    if ((tmp = strchr (token, 'x')) || (tmp = strchr (token, 'X'))) {
        tmp++;
        if (*tmp == '-') tmp++;
        for (uint16_t i = 0; (c = tmp[i]); i++) {
            if (!isxdigit (c)) return INVALID_OFF;
        }
        return HEX;
    } else if ((tmp = strchr (token, 'b')) || (tmp = strchr (token, 'B'))) {
        tmp++;
        if (*tmp == '-') tmp++;
        for (uint16_t i = 0; (c = tmp[i]); i++) {
            if (!(c == '0' || c == '1')) return INVALID_OFF;
        }
        return BIN;
    } else if ((tmp = strchr (token, '#')) != NULL) {
        tmp++;
        if (*tmp == '-') tmp++;
        for (uint16_t i = 0; (c = tmp[i]); i++) {
            if (!isdigit (c)) return INVALID_OFF;
        }
        return DEC;
    } else if ((tmp = token) != NULL) {
        if (*tmp == '-') tmp++;
        for (uint16_t i = 0; (c = tmp[i]); i++) {
            if (!isdigit (c)) return INVALID_OFF;
        }
        return DEC;
    } else {
        return INVALID_OFF;
    }
}

int16_t offset (uint8_t off_type, Token *tok)
{
    char *token = tok->str;
    int16_t off = 0;

    if (off_type == HEX) {
        off = hexoffset (token);
    } else if (off_type == BIN) {
        off = binoffset (token);
    } else if (off_type == DEC) {
        off = decoffset (token);
    }

    return off;
}