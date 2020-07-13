
#ifndef STUDY_CONFIG_H
#define STUDY_CONFIG_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "gettext.h"
#define _(str) gettext(str)
#include <locale.h>
#include <stdio.h>

#endif
