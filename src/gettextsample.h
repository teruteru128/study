
#ifndef MAIN_H
#define MAIN_H
#include "config.h"
#include "gettext.h"
#define _(str) gettext(str)
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>

int helloWorld001();

#endif
