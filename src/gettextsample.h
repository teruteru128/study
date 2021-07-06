
#ifndef MAIN_H
#define MAIN_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "gettext.h"
#define _(str) gettext(str)
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>

void initGettext();
int helloWorld001();

#endif
