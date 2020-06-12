
#include "config.h"
#include <stdio.h>

#include "bitmessage.h"
#include "bm.h"

int BM_init(void)
{
    return 1;
}

static int bm_init()
{
    return 0;
}

static int bm_cleanup()
{
    return 0;
}

static int global_init()
{
}

static int global_cleanup()
{
}

static int arg_parse(struct arg_t *argt, int argc, char **argv)
{
}

static char *get_config_filepath()
{
}

/**
 *  --xxx arg
 *  --xxx=arg
 *  default arg
 *  short arg
 * */
static int config_parse(char *filename)
{
}

static char *getString()
{
}

static int getInt()
{
}
