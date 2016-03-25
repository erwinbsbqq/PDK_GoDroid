#include <adapter/common.h>
#include <adapter/busdescriptor.h>
#include <adapter/busadapter.h>

//#define DEBUG_DATA

struct i2c_state {
    void *priv;
};

static int host_i2c_xfer(struct dibDataBusClient *client, struct dibDataBusAccess *msg)
{
    //struct i2c_state *st = client->host->priv;

#ifdef DEBUG_DATA
    dbgp("i2c_%s: (addr: %02x): ",msg->rx == NULL ? "wr" : "rd",data_bus_access_device_id(client, msg) >> 1);
    dbg_bufdump(msg->tx,msg->txlen);
    if (msg->rx != NULL && msg->rxlen != 0) {
        dbgp(" - ");
        dbg_bufdump(msg->rx,msg->rxlen);
    }
    dbgp("\n");
#endif

    if (msg->rx == NULL || msg->rxlen == 0) {
        // implement here the write function and return DIB_RETURN_SUCCESS in case of success
        // return DIB_RETURN_SUCCESS
        if (!f_dibcom_write(data_bus_access_device_id(client, msg), msg->tx, msg->txlen))
            return DIB_RETURN_SUCCESS;

        printf("f_dibcom_write\n");
        return DIB_RETURN_ERROR;
    }
    else {
        // implement here the read function and return DIB_RETURN_SUCCESS in case of success
        // return DIB_RETURN_SUCCESS
        if (!f_dibcom_read(data_bus_access_device_id(client, msg), msg->tx, msg->txlen, msg->rx, msg->rxlen))
            return DIB_RETURN_SUCCESS;

        printf("f_dibcom_read\n");
        return DIB_RETURN_ERROR;
    }
}

void host_i2c_release(struct dibDataBusHost *i2c_adap)
{
    struct i2c_state *state = i2c_adap->priv;
    DibDbgPrint("-I-  closing I2C\n");

    MemFree(state, sizeof(struct i2c_state));
}

struct dibDataBusHost * host_i2c_interface_attach(void *priv)
{
    struct i2c_state *state = MemAlloc(sizeof(struct i2c_state));
    struct dibDataBusHost *i2c_adap = MemAlloc(sizeof(struct dibDataBusHost));

    data_bus_host_init(i2c_adap, DATA_BUS_I2C, host_i2c_xfer, state);

    state->priv = priv;

    return i2c_adap;
    goto free_mem;
free_mem:
    MemFree(state,sizeof(struct i2c_state));
    return NULL;
}

