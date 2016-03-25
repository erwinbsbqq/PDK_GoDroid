/* The following convention is used to named the printer port
   The "port" is a number corresponding to a physical I/O address
   This number is between 0 and 8 :
   0 --> 0x3bc
   1 --> 0x378 (This is generally LPT1)
   2 --> 0x278 (This is often LPT2)
   3 --> 0x368
   4 --> 0x268
   5 --> 0x358
   6 --> 0x258
   7 --> 0x348
   8 --> 0x248 */
/* the "speed" is an integer which give the period of I2C SCL */
/* these number must be given to the init function */

#include <bus/spp_i2c.h>

//#define DEBUG_DATA

/* puts a delay between the clock cycles of I2C data */
//#define SLOW_I2C 10000

struct dibSPPI2CConfig {
	uint8_t inverted;

/* SDAOUT = AUTO_FD_XT (pin 14 of sub D 25) hardware inverted on cable */
/* bit 1 of ctrl port (base+2) */
	uint8_t sdaout;

/* SCLOUT = SLCT_IN (pin 17 of sub D 25) hardware inverted on cable */
/* bit 3 of ctrl port (base+2) */
	uint8_t sclout;

/* SDAIN = BUSY (pin 11 of sub D 25) hardware inverted on cable */
/* bit  of status port (base+1) */
	uint8_t sdain;
};

static const struct dibSPPI2CConfig default_dibcom_spp_config = {
	0,
	0x02,
	0x08,
	0x80
};

struct spp_host_state {
	struct dibSPPDescriptor *desc;
	const struct dibSPPI2CConfig *cfg;

	struct dibBusAdapter bus_adap;

	struct dibDataBusHost tuner_if;    /* n_idleState = 1 */
	struct dibDataBusHost dibctrl_if;  /* n_idleState = 0 */

	struct dibDataBusHost i2c_if;

	uint16_t portaddr;
	int mindelay;
	int ctrl;

	uint8_t n_idleState;
};

/*  elementary functions (private) */
static void databit_out(struct spp_host_state *i2c, uint8_t b)
{
	if (i2c->cfg->inverted)
		b = !b;

	if (b == 0)
		i2c->ctrl &= ~i2c->cfg->sdaout;
	else
		i2c->ctrl |=  i2c->cfg->sdaout;

	i2c->desc->outb(i2c->desc, i2c->portaddr+2, i2c->ctrl);
}

static void clock_out(struct spp_host_state *i2c, uint8_t b)
{
	if (i2c->cfg->inverted)
		b = !b;

	if (b == 0)
		i2c->ctrl &= ~i2c->cfg->sclout;
	else
		i2c->ctrl |=  i2c->cfg->sclout;

	i2c->desc->outb(i2c->desc, i2c->portaddr+2, i2c->ctrl);
}

static int databit_in(struct spp_host_state *i2c)
{
	uint8_t b;
	b = i2c->desc->inb(i2c->desc, i2c->portaddr+1) & i2c->cfg->sdain;
	if (i2c->cfg->inverted)
		b = !b;
	return !!b;
}

static void cycleLH(struct spp_host_state *state)
{
	clock_out(state, 0);
	clock_out(state, 1);
}

static void cycleHL(struct spp_host_state *state)
{
	clock_out(state, 1);
	clock_out(state, 0);
}

static void send_data_dibctrl(struct spp_host_state *state, uint32_t data, uint8_t size)
{
	do {
		size--;
		databit_out(state, (uint8_t) ((data >> size) & 0x1));
		cycleLH(state);
	} while (size);
}

static uint32_t recv_data_dibctrl(struct spp_host_state *state, uint8_t size)
{
	uint32_t data = 0;
	do {
		size--;
		data <<= 1;
		cycleHL(state);
		data |= databit_in(state);
	} while (size);
	return data;
}


static int spp_host_dibctrl_xfer_write(struct dibDataBusClient *client, struct dibDataBusAccess *msg)
{
	struct spp_host_state *state = client->host->priv;
	uint32_t len=0, i=0;

	// Start
	databit_out(state, 0);
	cycleLH(state);
#ifdef DEBUG_DATA
	dbgp("dibctrl start ");
#endif

	// R/W flag
	databit_out(state, 0);
	cycleLH(state);
#ifdef DEBUG_DATA
	dbgp("wr ");
#endif

	// Chip Addr
	//if (!(msg->mode & DATA_BUS_MODE_NO_CHIP_ADDRESS))
		send_data_dibctrl(state, msg->address, 8);
#ifdef DEBUG_DATA
	dbgp("chipaddr: %02x ", msg->address);
#endif

	switch (msg->mode & DATA_BUS_ACCESS_MODE_MASK) {
		case DATA_BUS_ACCESS_MODE_8BIT:  len = 1; break;
		case DATA_BUS_ACCESS_MODE_16BIT: len = 2; break;
		case DATA_BUS_ACCESS_MODE_32BIT: len = 4; break;
	}
#ifdef DEBUG_DATA
	dbgp("data_size: %d ", len);
#endif

	for (i = 0; i < msg->txlen; i++) {
		send_data_dibctrl(state, msg->tx[i], 8);

#ifdef DEBUG_DATA
		dbgp("%02x ", msg->tx[i]);
#endif

		if (((i+1) % len) == 0) {
			if (i+1 < msg->txlen) // continue
				databit_out(state, 0);
			else // stop
				databit_out(state, 1);
			cycleLH(state);

#ifdef DEBUG_DATA
			dbgp("%s ", i+1 < msg->txlen ? "cont" : "stop");
#endif
		}
	}
#ifdef DEBUG_DATA
	dbgp("\n");
#endif

	return DIB_RETURN_SUCCESS;
}

static int spp_host_dibctrl_xfer_read(struct dibDataBusClient *client, struct dibDataBusAccess *msg)
{
	struct spp_host_state *state = client->host->priv;
	uint32_t len=0, i=0;


	// Start
	databit_out(state, 0);
	cycleLH(state);
#ifdef DEBUG_DATA
	dbgp("dibctrl start ");
#endif

	// R/W flag
	databit_out(state, 1);
	cycleLH(state);
#ifdef DEBUG_DATA
	dbgp("rd ");
#endif

	// Chip Addr
	//if (!(msg->mode & DATA_BUS_MODE_NO_CHIP_ADDRESS))
		send_data_dibctrl(state, msg->address, 8);
#ifdef DEBUG_DATA
	dbgp("chipaddr: %02x ", msg->address);
#endif

	/* give control to the slave */
	databit_out(state, 1); /* 1 is a weak 1 (pull up) */
	clock_out(state, 0);

	switch (msg->mode & DATA_BUS_ACCESS_MODE_MASK) {
		case DATA_BUS_ACCESS_MODE_8BIT:  len = 1; break;
		case DATA_BUS_ACCESS_MODE_16BIT: len = 2; break;
		case DATA_BUS_ACCESS_MODE_32BIT: len = 4; break;
	}

#ifdef DEBUG_DATA
	dbgp("data_size: %d ", len);
#endif

	for (i = 0; i < msg->rxlen; i++) {
		databit_out(state, 1); /* 1 is a weak 1 (pull up) */
		clock_out(state, 0);

		msg->rx[i] = (uint8_t) (recv_data_dibctrl(state, 8));
#ifdef DEBUG_DATA
		dbgp("%02x ", msg->rx[i]);
#endif

		if (((i + 1) % len) == 0) {
			clock_out(state, 1);
			if (i+1 < msg->rxlen) // continue
				databit_out(state, 0);
			else // stop
				databit_out(state, 1);
			cycleLH(state);
#ifdef DEBUG_DATA
			dbgp("%s ", i+1 < msg->rxlen ? "cont" : "stop");
#endif
		}
	}
#ifdef DEBUG_DATA
	dbgp("\n");
#endif

	return DIB_RETURN_SUCCESS;
}

static int spp_host_tuner_if_xfer(struct dibDataBusClient *client, struct dibDataBusAccess *msg)
{
	//if (state->n_idleState == 0) {
	//	dbgpl(&spp_dbg, "using tuner interface but idle-state is for dibctrl - for changing idleState");
	//	state->n_idleState = 1;
	//}
	return DIB_RETURN_SUCCESS;
}

static int spp_host_dibctrl_if_xfer(struct dibDataBusClient *client, struct dibDataBusAccess *msg)
{
	struct spp_host_state *state = client->host->priv;
	int ret;
	if (state->n_idleState == 1) {
		dbgp( "using dibctrl interface but idle-state is for tuner-dibctrl - for changing idleState\n");
		state->n_idleState = 0;
	}
	databit_out(state, 0);
	clock_out(state, 0);

	if (msg->rx == NULL || msg->rxlen == 0)
		ret = spp_host_dibctrl_xfer_write(client, msg);
	else
		ret = spp_host_dibctrl_xfer_read(client, msg);

	databit_out(state, 0);
	clock_out(state, 0);
	return ret;
}

static void send_data_i2c(struct spp_host_state *state, uint32_t data, uint8_t size)
{
	do {
		size--;
		databit_out(state, (uint8_t) ((data >> size) & 0x1));
		cycleHL(state);
	} while (size);
}


static int get_i2c_ack(struct spp_host_state *state)
{
	/* check for device Acknowledge */
	databit_out(state, 1);
	clock_out(state, 1);
	if (databit_in(state)!=0)
		return DIB_RETURN_ERROR;
	clock_out(state, 0);
	return DIB_RETURN_SUCCESS;
}

static void send_i2c_ack(struct spp_host_state* state)
{
	// send Acknowledge (if ack==1) to device
	databit_out(state, 0);
	clock_out(state, 1);
	clock_out(state, 0);
	databit_out(state, 1);
}


static uint32_t recv_data_i2c(struct spp_host_state *state, uint8_t size)
{
	uint32_t data = 0;
	do {
		size--;
		data <<= 1;
		clock_out(state, 1);
		data |= databit_in(state);
		clock_out(state, 0);
	} while (size);
	return data;
}

static void i2c_start(struct spp_host_state *state)
{
	databit_out(state, 1); /* if SCL=1 this is a stop condition */
	clock_out(state, 1); /* prepare for start */
	databit_out(state, 0); /* start condition */
	clock_out(state, 0); /* prepare next bit */
}


static void i2c_stop(struct spp_host_state *state)
{
	databit_out(state, 0); /* if SCL=1 this is a start condition */
	clock_out(state, 1);/* prepare for stop */
	databit_out(state, 1);/* stop condition */
}


static int spp_host_i2c_xfer_write(struct dibDataBusClient *client, struct dibDataBusAccess *msg, int stop)
{
	struct spp_host_state *state = client->host->priv;
	uint32_t i;
	int ret = DIB_RETURN_SUCCESS;

#ifdef DEBUG_DATA
	dbgp("i2c_wr: (addr: %02x): ",data_bus_access_device_id(client, msg) >> 1);
	dbg_bufdump(msg->tx,msg->txlen);
	dbgp("\n");
#endif

	if (!msg->tx && msg->txlen) /* check tx buffer */
		return DIB_RETURN_ERROR;
	i2c_start(state);

	send_data_i2c(state, data_bus_access_device_id(client, msg)& 0xfe, 8);
	if ((ret = get_i2c_ack(state)) != DIB_RETURN_SUCCESS)
		goto out;

	for (i=0 ; i<msg->txlen ; i++) {
		send_data_i2c(state,msg->tx[i], 8); /* write txlen data bytes */
		if ((ret = get_i2c_ack(state)) != DIB_RETURN_SUCCESS)
			goto out;
	}

out:
	if (stop)
		i2c_stop(state);
	return ret;
}

static int spp_host_i2c_xfer_read(struct dibDataBusClient *client, struct dibDataBusAccess *msg)
{
	uint32_t i;
    int ret = DIB_RETURN_SUCCESS;
	struct spp_host_state *state = client->host->priv;

	if (!msg->rx && msg->rx) /* check rx buffer */
		return DIB_RETURN_ERROR;

	for (i = 0 ; i < msg->rxlen; i++)	/* initialize receive buffer */
		msg->rx[i] = 0xfe;

	if (msg->txlen > 0)
		if (spp_host_i2c_xfer_write(client, msg, 0) != DIB_RETURN_SUCCESS)
			return DIB_RETURN_ERROR;

#ifdef DEBUG_DATA
	dbgp("i2c_rd: (addr: %02x): ",data_bus_access_device_id(client, msg) >> 1);
//	dbg_bufdump(msg->tx,msg->txlen);
#endif

	i2c_start(state); /* repeated start condition */

	send_data_i2c(state,data_bus_access_device_id(client, msg) | 0x01, 8);	/* device address (read mode) */
	if ((ret = get_i2c_ack(state)) != DIB_RETURN_SUCCESS)
		goto out;

	for (i=0 ; i<msg->rxlen ; i++) {
		msg->rx[i] = (uint8_t) (recv_data_i2c(state, 8));   /* read 1 data byte  */
		if ((i+1) != msg->rxlen)
			send_i2c_ack(state);
	}

out:
	i2c_stop(state);				/* stop */

#ifdef DEBUG_DATA
	dbgp(" - ");
	dbg_bufdump(msg->rx,msg->rxlen);
	dbgp("\n");
#endif
	return ret;
}

static int spp_host_i2c_if_xfer(struct dibDataBusClient *client, struct dibDataBusAccess *msg)
{
	struct spp_host_state *state = client->host->priv;
	if (state->n_idleState == 1) {
		dbgp( "using dibctrl interface but idle-state is for tuner-dibctrl - for changing idleState\n");
		state->n_idleState = 0;
	}

	if (msg->rx == NULL || msg->rxlen == 0)
		return spp_host_i2c_xfer_write(client, msg,1);
	else
		return spp_host_i2c_xfer_read(client, msg);
}

static void spp_host_release(struct dibBusAdapter *bus_adap)
{
	struct spp_host_state *state = bus_adap->priv;
	data_bus_host_exit(&state->tuner_if);
	data_bus_host_exit(&state->dibctrl_if);
	data_bus_host_exit(&state->i2c_if);
	MemFree(state, sizeof(struct spp_host_state));
}

static uint32_t spp_streaming_mode(struct dibBusAdapter *bus, uint32_t mode)
{
    switch (mode) {
        case MODE_DVBT:
        case MODE_ISDBT: return OUTPUT_MODE_TS_PARALLEL_GATED;
    }
    return OUTPUT_MODE_OFF;
}

static const struct dibBusAdapterInfo spp_host_info;

static int I2C_backend_exist_port(const struct dibSPPDescriptor *desc, uint16_t port)
{
	if (desc->check_permission(desc,port) != DIB_RETURN_SUCCESS) {
		DibDbgPrint("-E-  SPP: you must be SU !\n");
		return DIB_RETURN_ERROR;
	}

	desc->outb(desc, port, 0x80);
	if (desc->inb(desc, port) != 0x80) {
		DibDbgPrint("-E-  checking 0x80 after write failed\n");
		return 1;
	}
	desc->outb(desc, port,0x00);
	if (desc->inb(desc, port) != 0x00) {
		DibDbgPrint("-E-  checking 0x00 after write failed\n");
		return 1;
	}
	return 0;
}

struct dibBusAdapter * spp_host_interface_attach(struct dibSPPDescriptor *desc, uint16_t port)
{
	struct spp_host_state *i2c = MemAlloc(sizeof(struct spp_host_state));
	struct dibBusAdapter  *bus_adap;
	int err = DIB_RETURN_SUCCESS;

	if (i2c == NULL)
		return NULL;
	DibZeroMemory(i2c, sizeof(struct spp_host_state));

	/* base addresses of printer ports on PCs */
	if (I2C_backend_exist_port(desc, port) != 0) {
		DibDbgPrint("-E-  selected port %x does not exist or not accessible\n",port);
		return NULL;
	}

	i2c->desc = desc;

	memcpy(&i2c->bus_adap.info,&spp_host_info,sizeof(struct dibBusAdapterInfo));
	bus_adap = &i2c->bus_adap;
	bus_adap->priv = i2c;

	i2c->portaddr = port;
	//i2c->data     = desc->inb(port);
	//i2c->status   = desc->inb(port+1);
	//i2c->status  &= 0xff;
	i2c->ctrl     = desc->inb(desc, port+2);
	i2c->ctrl    &= 0xf;
	i2c->cfg      = &default_dibcom_spp_config;

	/* set SCl high */
	clock_out(i2c, 1);
	/* set SDA low */
	databit_out(i2c, 0);
    /* check if SDA is 0 */
	if (databit_in(i2c) != 0)
		err = DIB_RETURN_ERROR;
	/* set SDA high */
	databit_out(i2c, 1);
    /* check if SDA is 1 */
	if (databit_in(i2c) == 0)
		err = DIB_RETURN_ERROR;
	/* set SDA low */
	databit_out(i2c, 0);
    /* check if SDA is 0 */
	if (databit_in(i2c) != 0)
		err = DIB_RETURN_ERROR;
	/* set SDA high */
	databit_out(i2c, 1);
    /* check again if SDA is 1 */
	if (databit_in(i2c) == 0)
		err = DIB_RETURN_ERROR;
	if (err) {
		DibDbgPrint("-E-  Could not identify the DiBcom parallel port interface\n");
		goto free_mem;
	}
	//stop(i2c);

	data_bus_host_init(&i2c->tuner_if, DATA_BUS_DIBCTRL_IF, spp_host_tuner_if_xfer, i2c);
	data_bus_host_init(&i2c->dibctrl_if, DATA_BUS_DIBCTRL_IF, spp_host_dibctrl_if_xfer, i2c);
	data_bus_host_init(&i2c->i2c_if, DATA_BUS_I2C, spp_host_i2c_if_xfer, i2c);
	i2c->n_idleState = 0;

	bus_adap->i2c_adap = bus_adap->default_data_bus = &i2c->i2c_if;
	//bus_adap->i2c_adap = bus_adap->default_data_bus = &i2c->dibctrl_if;

	return bus_adap;
	goto free_mem;
free_mem:
	MemFree(i2c,sizeof(struct spp_host_state));
	return NULL;
}

static const struct dibBusAdapterInfo spp_host_info = {
	"DiBcom I2C over SPP",

	{
		NULL,
		NULL,

		NULL,
		NULL,
		NULL,
		NULL,
		NULL,

		NULL,

		NULL,
		NULL,
		NULL,

        spp_streaming_mode,

        NULL,

		spp_host_release
	}
};
