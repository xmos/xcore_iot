// Copyright (c) 2020, XMOS Ltd, All rights reserved

#include <platform.h>

extern "C" {
    void main_tile0(void);
    void main_tile1(void);
}

int main(void)
{
    par {
        on tile[0]: main_tile0();
        on tile[1]: main_tile1();
    }

    return 0;
}
