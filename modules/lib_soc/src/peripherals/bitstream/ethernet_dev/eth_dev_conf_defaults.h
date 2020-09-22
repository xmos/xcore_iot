// Copyright (c) 2019, XMOS Ltd, All rights reserved

#ifndef ETH_DEV_CONF_DEFAULTS_H_
#define ETH_DEV_CONF_DEFAULTS_H_

/* MII defaults */
#ifndef ETHCONF_MII_BUFSIZE
#define ETHCONF_MII_BUFSIZE         (4096)
#endif


/* SMI defaults */
#ifndef ETHCONF_SMI_PHY_ADDRESS
#define ETHCONF_SMI_PHY_ADDRESS     (0)
#endif

#ifndef ETHCONF_SMI_MDIO_BIT_POS
#define ETHCONF_SMI_MDIO_BIT_POS    (1)
#endif

#ifndef ETHCONF_SMI_MDC_BIT_POS
#define ETHCONF_SMI_MDC_BIT_POS     (0)
#endif


/* RT MAC defaults */
#ifndef ETHCONF_USE_RT_MAC
#define ETHCONF_USE_RT_MAC          (0)
#endif

#ifndef ETHCONF_RT_MII_RX_BUFSIZE
#define ETHCONF_RT_MII_RX_BUFSIZE   (ETHCONF_MII_BUFSIZE)
#endif

#ifndef ETHCONF_RT_MII_TX_BUFSIZE
#define ETHCONF_RT_MII_TX_BUFSIZE   (ETHCONF_MII_BUFSIZE)
#endif

#ifndef ETHCONF_USE_SHAPER
#define ETHCONF_USE_SHAPER          (0)
#endif

#endif /* ETH_DEV_CONF_DEFAULTS_H_ */
