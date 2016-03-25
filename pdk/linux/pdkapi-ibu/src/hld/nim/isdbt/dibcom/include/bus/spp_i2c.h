#ifndef BUS_DIB0700_H
#define BUS_DIB0700_H

#include <adapter/common.h>

#include <adapter/busdescriptor.h>
#include <adapter/busadapter.h>

extern struct dibBusAdapter * spp_host_interface_attach(struct dibSPPDescriptor *desc, uint16_t port);


//extern struct dibBusAdapter * spp_i2c_attach (struct dibSPPDescriptor *desc, uint16_t port, int timestep);
//extern struct dibBusAdapter * spp_tuner_interface_attach(struct dibSPPDescriptor *desc, uint16_t port, int timestep, struct dibDataBusHost **itf);

//extern struct dibDataBusHost * spp_i2c_to_tuner_interface(struct dibBusAdapter *);


#endif
