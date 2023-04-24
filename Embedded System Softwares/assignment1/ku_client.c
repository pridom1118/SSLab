#include <stdio.h>

#include "ku_binder_lib.c"

struct test_struct
{
    char a[20];
};

int main(void)
{
    int ret, service_num;
    struct test_struct ts = {1, 2, 3, 4, 5};

    ret = kbinder_init();

    service_num = kbinder_query("vol");
    if (service_num < 0)
        return -1;

    for (int i = 0; i < 10; i++)
    {
        strcpy(ts.a, "Hello World!");

        ret = kbinder_rpc(service_num, i, &ts);

        printf("ret: %d, a[0]: %d, a[1]: %d, a[2]: %d, a[3]: %d, a[4]: %d \n",
               ret, ts.a[0], ts.a[1], ts.a[2], ts.a[3], ts.a[4]);
    }
}

