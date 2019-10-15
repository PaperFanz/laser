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

#define USES_FLAG
#define USES_FILE
#define USES_LABEL
#define USES_NOTIFY
#include "laser.h"

/*
    Checkextension: checks if the passed string ends in the given extension

    Inputs: filename as a string, extension as a string
    Outputs: return 0 if no match, return 1 if match
*/
int8_t checkextension (char *file, const char *extension)
{
    char *dot = strrchr(file, '.');
    if (dot == NULL)
        return 0;
    else if (strcmp (dot, extension) == 0)
        return 1;
    else
        return 0;
}

/*
    replaceextension: replaces the extension of a given string

    inputs: filename as a string, extension as a string
    outputs: none
*/
void replaceextension (char *file, const char *extension)
{
    char *dot = strrchr(file, '.');
    int i=0;
    while (dot[i] != '\0') {
        dot[i] = extension[i];
        i++;
    }
}

uint8_t openasmfiles (filearr_t *f, char *file)
{
    uint8_t failed = 0;
    f->fp = fopen (file, "r");
    if (f->fp == NULL) {
        notify ("Unable to open %s!\n", file);
        failed++;
    }
    replaceextension (file, ".sym");
    
    f->sym = fopen (file, "w");
    if (f->sym == NULL) {
        notify ("Unable to open %s!\n", file);
        failed++;
    }
    replaceextension (file, ".bin");

    f->bin = fopen (file, "w");
    if (f->bin == NULL) {
        notify ("Unable to open %s!\n", file);
        failed++;
    }
    replaceextension (file, ".hex");

    f->hex = fopen (file, "w");
    if (f->hex == NULL) {
        notify ("Unable to open %s!\n", file);
        failed++;
    }
    replaceextension (file, ".obj");

    f->obj = fopen (file, "wb");
    if (f->obj == NULL) {
        notify ("Unable to open %s!\n", file);
        failed++;
    }
    replaceextension (file, ".lst");

    f->lst = fopen (file, "w");
    if (f->lst == NULL) {
        notify ("Unable to open %s!\n", file);
        failed++;
    }
    replaceextension (file, ".asm");

    return failed;
}

const char *extensions[] = {
    ".sym",
    ".bin",
    ".hex",
    ".lst",
    ".obj",
    ".log"
};

int8_t clean (char *file)
{
    bool err = 0;
    notify ("\nCleaning up...\n");
    for (int i = 0; i < (islogging () ? 6 : 5); i++) {
        replaceextension (file, extensions[i]);
        notify ("Deleting \"%s\"...\n", file);
        if (remove (file)) {
            notify ("Unable to delete '%s'!\n", file);
            err = 1;
        }
    }
    replaceextension (file, ".sym");
    return err;
}

void closeasmfiles (filearr_t *f)
{
    if (f->fp != NULL) fclose (f->fp);
    if (f->sym != NULL) fclose (f->sym);
    if (f->bin != NULL) fclose (f->bin);
    if (f->hex != NULL) fclose (f->hex);
    if (f->obj != NULL) fclose (f->obj);
    if (f->lst != NULL) fclose (f->lst);
}

#define DEFAULT_FILEBUF_SIZE 256

void writefilebuf (filebuf_t *buf, uint16_t addr, uint16_t ins, uint32_t ln)
{
    uint16_t index = buf->ind;
    if (index == buf->cap) {
        buf->cap *= 2;
        buf->addrbuf = (uint16_t*) realloc (buf->addrbuf, buf->cap * sizeof (uint16_t));
        buf->insbuf = (uint16_t*) realloc (buf->insbuf, buf->cap * sizeof (uint16_t));
        buf->lnbuf = (uint32_t*) realloc (buf->lnbuf, buf->cap * sizeof (uint32_t));
    }
    buf->addrbuf[index] = addr;
    buf->insbuf[index] = ins;
    buf->lnbuf[index] = ln;
    buf->ind++;
}

filebuf_t* initfilebuf(void)
{
    filebuf_t *buf = (filebuf_t*) malloc (sizeof (filebuf_t));
    buf->ind = 0;
    buf->cap = DEFAULT_FILEBUF_SIZE;
    buf->addrbuf = (uint16_t*) malloc (buf->cap * sizeof (uint16_t));
    buf->insbuf = (uint16_t*) malloc (buf->cap * sizeof (uint16_t));
    buf->lnbuf = (uint32_t*) malloc (buf->cap * sizeof (uint32_t));
    return buf;
}

void freefilebuf(filebuf_t *buf)
{
    if (buf == 0) return;
    if (buf->addrbuf != 0) free (buf->addrbuf);
    if (buf->insbuf != 0) free (buf->insbuf);
    if (buf->lnbuf != 0) free (buf->lnbuf);
    free (buf);
}

void writeobj (filebuf_t *buf, FILE *obj)
{
    uint16_t index = buf->ind;
    uint32_t bytenum = index * 2;
    uint8_t tmp[bytenum];
    for (uint16_t i = 0; i < index; i++) {
        tmp[i * 2 + 1] = buf->insbuf[i] & 0xFF;
        tmp[i * 2] = buf->insbuf[i] >> 8;
    }

    fwrite (tmp, sizeof (uint8_t), bytenum, obj);
}

void writehex (filebuf_t *buf, FILE *hex)
{
    uint16_t index = buf->ind;
    for (uint32_t i = 0; i < index; i++) {
        fprintf (hex, "%04X\n", buf->insbuf[i]);
    }
}

const char *XtoB[16] = {
    "0000", "0001", "0010", "0011",
    "0100", "0101", "0110", "0111",
    "1000",	"1001",	"1010",	"1011",
    "1100",	"1101",	"1110",	"1111",
};

#define instobin(ins) \
    XtoB[ins >> 12], \
    XtoB[(ins >> 8) & 0xF], \
    XtoB[(ins >> 4) & 0xF], \
    XtoB[ins & 0xF]

void writebin (filebuf_t *buf, FILE *bin)
{
    uint16_t index = buf->ind;
    for (uint32_t i = 0; i < index; i++) {
        uint16_t tmp = buf->insbuf[i];
        fprintf (bin, "%s%s%s%s\n", instobin(tmp));
    }
}

void writesym (labelarr_t *symarr, FILE *sym)
{
    uint16_t symbolnum = symarr->ind;
    for (uint16_t i = 0; i < symbolnum; i++) {
        uint16_t len = symarr->arr[i].label->len;
        if (len >= 75) len = 75;
        fprintf (sym, "%s", symarr->arr[i].label->str);

        for (uint16_t i = (76 - len); i > 0; i--) fprintf (sym, " ");

        fprintf (sym, "x%04X", symarr->arr[i].address);
        fprintf (sym, "\n");
    }
}

const char* lstheader = "  ADDR  |  HEX  |      BINARY      |  LN  |  ASSEMBLY\n";

void writelst (filebuf_t *buf, FILE *fp, FILE *lst)
{
    fseek (fp, 0, SEEK_SET);			// find beginning of .asm file
    fprintf (lst, "%s", lstheader);

    char line[MAX_LEN + 1];
    uint16_t ind = 1, maxind = buf->ind;
    for (uint32_t ln = 1; fgets (line, MAX_LEN + 1, fp); ln++) {
        if (ind != maxind && buf->lnbuf[ind] == ln) {
            uint16_t ins = buf->insbuf[ind];
            uint16_t addr = buf->addrbuf[ind];
            fprintf (lst, " x%04X  | x%04X | %s%s%s%s ", addr, ins, instobin(ins));
            ind++;
        } else {
            fprintf (lst, "        |       |                  ");
        }

        fprintf (lst, "| %4d | %s", ln, line);

        /*
            for cases such as .STRINGZ and .BLKW where a single line of
            assembly generates multiple instructions
        */
        while (ind != maxind && buf->lnbuf[ind] == ln) {
            uint16_t ins = buf->insbuf[ind];
            fprintf (lst, "        | x%04X | %s%s%s%s |      |\n", ins, instobin(ins));
            ind++;
        }
    }
}