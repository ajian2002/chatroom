#include "config.h"
#include PROJECT_SERVERHEAD

zlog_category_t *my_zlog_init(char *cate)
{
    if (zlog_init(PROJECT_LOGCONFIG))
    {
        perror("zlog_init error");
        exit(-1);
    }

    zlog_category_t *ser = zlog_get_category(cate);
    if (!ser)
    {
        zlog_fini();
        perror("zlog_get_category error");
        exit(-1);
    }
    return ser;
}
