#include "common/socket.h"

PRIVATE const ip4_t ip4loopback = {127, 0, 0, 1};
PRIVATE const ip4_t ip4any      = {  0, 0, 0, 0};

PRIVATE bool ip6disabled = false;
PRIVATE const uint8_t ip6mappedv4prefix[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xff};
PRIVATE const ip6_t   ip6loopback           = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1};
PRIVATE const ip6_t   ip6any                = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
