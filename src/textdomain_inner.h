
#ifndef TEXTDOMAIN_INNER_H
#define TEXTDOMAIN_INNER_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "gettext.h"
#define _(str) gettext(str)
#include <locale.h>
#endif
