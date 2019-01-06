// include statements
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "config.h"

#ifndef LASER_H
#define LASER_H

int checkFlags(char *c);

int parseArgs(int argc, char **argv);

int main (int argc, char **argv);

#endif