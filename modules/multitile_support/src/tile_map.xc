// This Software is subject to the terms of the XMOS Public Licence: Version 1.
// XMOS Public License: Version 1

#include "platform.h"

extern "C" {
void main_tile0(chanend c0, chanend c1, chanend c2, chanend c3);
void main_tile1(chanend c0, chanend c1, chanend c2, chanend c3);
void main_tile2(chanend c0, chanend c1, chanend c2, chanend c3);
void main_tile3(chanend c0, chanend c1, chanend c2, chanend c3);
}

int main(void) {
#if ((PLATFORM_SUPPORTS_TILE_0 == 1) && (PLATFORM_USES_TILE_0 == 1) && \
     (PLATFORM_SUPPORTS_TILE_1 == 1) && (PLATFORM_USES_TILE_1 == 1))
  chan c_t0_t1;
#endif
#if ((PLATFORM_SUPPORTS_TILE_0 == 1) && (PLATFORM_USES_TILE_0 == 1) && \
     (PLATFORM_SUPPORTS_TILE_2 == 1) && (PLATFORM_USES_TILE_2 == 1))
  chan c_t0_t2;
#endif
#if ((PLATFORM_SUPPORTS_TILE_0 == 1) && (PLATFORM_USES_TILE_0 == 1) && \
     (PLATFORM_SUPPORTS_TILE_3 == 1) && (PLATFORM_USES_TILE_3 == 1))
  chan c_t0_t3;
#endif
#if ((PLATFORM_SUPPORTS_TILE_1 == 1) && (PLATFORM_USES_TILE_1 == 1) && \
     (PLATFORM_SUPPORTS_TILE_2 == 1) && (PLATFORM_USES_TILE_2 == 1))
  chan c_t1_t2;
#endif
#if ((PLATFORM_SUPPORTS_TILE_1 == 1) && (PLATFORM_USES_TILE_1 == 1) && \
     (PLATFORM_SUPPORTS_TILE_3 == 1) && (PLATFORM_USES_TILE_3 == 1))
  chan c_t1_t3;
#endif
#if ((PLATFORM_SUPPORTS_TILE_2 == 1) && (PLATFORM_USES_TILE_2 == 1) && \
     (PLATFORM_SUPPORTS_TILE_3 == 1) && (PLATFORM_USES_TILE_3 == 1))
  chan c_t2_t3;
#endif

  par {
#if (PLATFORM_SUPPORTS_TILE_0 == 1) && (PLATFORM_USES_TILE_0 == 1)
    on tile[0] : main_tile0(null,
#if (PLATFORM_SUPPORTS_TILE_1 == 1) && (PLATFORM_USES_TILE_1 == 1)
                            c_t0_t1,
#else
                            null,
#endif
#if (PLATFORM_SUPPORTS_TILE_2 == 1) && (PLATFORM_USES_TILE_2 == 1)
                            c_t0_t2,
#else
                            null,
#endif
#if (PLATFORM_SUPPORTS_TILE_3 == 1) && (PLATFORM_USES_TILE_3 == 1)
                            c_t0_t3
#else
                            null
#endif
                 );
#endif

#if (PLATFORM_SUPPORTS_TILE_1 == 1) && (PLATFORM_USES_TILE_1 == 1)
    on tile[1] : main_tile1(
#if (PLATFORM_SUPPORTS_TILE_0 == 1) && (PLATFORM_USES_TILE_0 == 1)
                     c_t0_t1,
#else
                     null,
#endif
                     null,
#if (PLATFORM_SUPPORTS_TILE_2 == 1) && (PLATFORM_USES_TILE_2 == 1)
                     c_t1_t2,
#else
                     null,
#endif
#if (PLATFORM_SUPPORTS_TILE_3 == 1) && (PLATFORM_USES_TILE_3 == 1)
                     c_t1_t3
#else
                     null
#endif
                 );
#endif

#if (PLATFORM_SUPPORTS_TILE_2 == 1) && (PLATFORM_USES_TILE_2 == 1)
    on tile[2] : main_tile2(
#if (PLATFORM_SUPPORTS_TILE_0 == 1) && (PLATFORM_USES_TILE_0 == 1)
                     c_t0_t2,
#else
                     null,
#endif
#if (PLATFORM_SUPPORTS_TILE_1 == 1) && (PLATFORM_USES_TILE_1 == 1)
                     c_t1_t2,
#else
                     null,
#endif
                     null,
#if (PLATFORM_SUPPORTS_TILE_3 == 1) && (PLATFORM_USES_TILE_3 == 1)
                     c_t2_t3
#else
                     null
#endif
                 );
#endif

#if (PLATFORM_SUPPORTS_TILE_3 == 1) && (PLATFORM_USES_TILE_3 == 1)
    on tile[3] : main_tile3(
#if (PLATFORM_SUPPORTS_TILE_0 == 1) && (PLATFORM_USES_TILE_0 == 1)
                     c_t0_t3,
#else
                     null,
#endif
#if (PLATFORM_SUPPORTS_TILE_1 == 1) && (PLATFORM_USES_TILE_1 == 1)
                     c_t1_t3,
#else
                     null,
#endif
#if (PLATFORM_SUPPORTS_TILE_2 == 1) && (PLATFORM_USES_TILE_2 == 1)
                     c_t2_t3
#else
                     null,
#endif
                         null);
#endif
  }

  return 0;
}
