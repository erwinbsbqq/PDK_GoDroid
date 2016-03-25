#ifndef SAMPLE_H
#define SAMPLE_H

#define SPP_PORT 0x378

#include <adapter/frontend.h>

extern struct dibDataBusHost * open_spp_i2c();
extern void close_spp_i2c();

#endif
