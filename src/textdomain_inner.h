
#ifndef TEXTDOMAIN_INNER_H
#define TEXTDOMAIN_INNER_H
#include "config.h"
#include "gettext.h"
#define _(str) gettext(str)
#include <locale.h>
#endif
