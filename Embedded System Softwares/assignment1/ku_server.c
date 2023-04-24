#include <stdio.h>

#include "ku_binder_lib.c"

struct test_struct
{
    char a[20];
};

struct test_struct_wrapper
{
    int fcode;
    struct test_struct ts;
};

int main(void)
{
    struct test_struct_wrapper tsw;
    int ret, service_num;

    ret = kbinder_init();

    service_num = kbinder_reg("vol");
    if (service_num < 0)
        return -1;

    for (int i = 0; i < 10; i++)
    {
        ret = kbinder_read(service_num, &tsw);
        printf("%d\n", tsw.fcode);
        printf("%s\n", tsw.ts.a);
    }
}
