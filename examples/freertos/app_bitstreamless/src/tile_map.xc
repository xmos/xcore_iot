// Copyright (c) 2020, XMOS Ltd, All rights reserved

#include "platform.h"

extern "C" {
    void main_tile0(chanend c);
    void main_tile1(chanend c);
}

int main(void)
{
	chan c;
    par {
        on tile[0]: main_tile0(c);
        on tile[1]: main_tile1(c);
    }

    return 0;
}
