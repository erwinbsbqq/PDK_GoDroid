/* this file contains basic code to use the Parallel port I2C in Linux */
#include "sample.h"

#include <interface/linux_spp.h>
#include <bus/spp_i2c.h>

static struct dibBusAdapter    *bus_adap;
static struct dibSPPDescriptor *sppdesc;

struct dibDataBusHost * open_spp_i2c()
{
	sppdesc = linux_spp_open();
	if (sppdesc == NULL) {
		DibDbgPrint("-E-  SPP Linux could not be opened\n");
		return NULL;
	}

	bus_adap = spp_host_interface_attach(sppdesc, SPP_PORT);
	if (bus_adap == NULL) {
		DibDbgPrint("-E-  SPP port could not be opened (addr: %03x)\n", SPP_PORT);
		return NULL;
	}

	DibDbgPrint("-I-  SPP successfully opened on port %03x\n", SPP_PORT);

	return bus_adap->i2c_adap;
}

void close_spp_i2c()
{
	bus_adapter_release(bus_adap);
	linux_spp_close(sppdesc);
}
