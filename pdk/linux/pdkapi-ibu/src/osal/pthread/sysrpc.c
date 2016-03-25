
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "osal/srpc.h"

#if 0
#define RPC_DEBUG(fmt, args...) printf("%s:%d:: "fmt, __FUNCTION__, __LINE__, ##args)
#else
#define RPC_DEBUG(...)
#endif



#define RPCSVR_SERVICE_MOUNTDISK           4
#define RPCSVR_SERVICE_UMOUNTDISK          5
#define RPCSVR_SERVICE_REMOVE_DISK         6
#define RPCSVR_SERVICE_FORMAT              7
#define RPC_SERVICE_NETCONFIG              8
#define RPCSVR_SERVICE_MOUNT                 9
#define RPCSVR_SERVICE_UMOUNT             10
#define RPCSVR_SERVICE_CRYPTSETUP      11
#define RPCSVR_SERVICE_LOSETUP             12
#define RPCSVR_SERVICE_CDGZIPCPIO       13
#define RPCSVR_SERVICE_RMMAPPERFILE   14
#define RPCSVR_SERVICE_CP                        15
#define RPCSVR_SERVICE_RMAPPDATAFILE  16
#define RPCSVR_SERVICE_ETHCONFIG  17

#define RPC_SVR_PORT 32435

#define DISKRPC(rpc) srpc rpc;\
    memset(&rpc,0,sizeof(srpc));\
    rpc.saddr.sin_family = AF_INET;\
    rpc.saddr.sin_addr.s_addr = inet_addr("127.0.0.1");\
    rpc.saddr.sin_port = htons(RPC_SVR_PORT);

int rpc_remove_disk(char *disk)
{
    int ret = 0;
    srpc_req req;
    DISKRPC(rpc);
    memset(&req, 0 , sizeof(srpc_req));
    req.rpc = &rpc;
    req.param.service_id = RPCSVR_SERVICE_REMOVE_DISK;

    if (disk)
    {
        strcpy(req.param.pload, disk);
    }

    RPC_DEBUG("requst umount %s\n", disk == NULL ? "all disk" : disk);

    if (srpc_call(&req, 100 * 1000)) //timeout: 100s
    {
        memcpy(&ret, req.param.pload, sizeof(ret));
    }

    return ret;
}

int rpc_mountdisk(char *dev, char *mnt)
{
    int ret = 0;
    srpc_req req;
    DISKRPC(rpc);

    if (dev == NULL || mnt == NULL)
    {
        return 0;
    }

    memset(&req, 0 , sizeof(srpc_req));
    req.rpc = &rpc;
    req.param.service_id = RPCSVR_SERVICE_MOUNTDISK;
    strcpy(req.param.pload, dev);
    strcpy(req.param.pload + 32, mnt);
    RPC_DEBUG("requst mount:%s\n", dev);

    if (srpc_call(&req, 10 * 1000)) //timeout: 10s
    {
        memcpy(&ret, req.param.pload, sizeof(ret));
    }

    return ret;
}

int rpc_umountdisk(char *path)
{
    int ret = 0;
    srpc_req req;
    DISKRPC(rpc);

    if (path == NULL)
    {
        return 0;
    }

    memset(&req, 0 , sizeof(srpc_req));
    req.rpc = &rpc;
    req.param.service_id = RPCSVR_SERVICE_UMOUNTDISK;
    strcpy(req.param.pload, path);
    RPC_DEBUG("requst umount:%s\n", path);

    if (srpc_call(&req, 10 * 1000)) //timeout: 10s
    {
        memcpy(&ret, req.param.pload, sizeof(ret));
    }

    return ret;
}

int rpc_format(char *dev, char *fs_type)
{
    int ret = -1;
    srpc_req req;
    DISKRPC(rpc);

    if (dev == NULL || fs_type == NULL)
    {
        return ret;
    }

    memset(&req, 0 , sizeof(srpc_req));
    req.rpc = &rpc;
    req.param.service_id = RPCSVR_SERVICE_FORMAT;
    strcpy(req.param.pload, dev);
    strcpy(req.param.pload + 32, fs_type);
    RPC_DEBUG("requst format %s use %s\n", dev, fs_type);

    if (srpc_call(&req, 600 * 1000)) //timeout: 10min
    {
        ret = 0;
    }

    return ret;
}

int rpc_netconfig(char *eth_name)
{
    int ret = 0;
    srpc_req req;
    DISKRPC(rpc);
    memset(&req, 0 , sizeof(srpc_req));
    req.rpc = &rpc;
    req.param.service_id = RPC_SERVICE_NETCONFIG;

    if (strlen(eth_name) > SRPC_PLOAD_SIZE)
    {
        return -1;
    }

    memcpy(req.param.pload, eth_name, strlen(eth_name));

    if (srpc_call(&req, 10 * 1000))
    {
        memcpy(&ret, req.param.pload, sizeof(int));
    }

    return ret;
}

int rpc_mount(char *cmd)
{
    int ret = -1;
    srpc_req req;
    DISKRPC(rpc);
    memset(&req, 0 , sizeof(srpc_req));
    req.rpc = &rpc;
    req.param.service_id = RPCSVR_SERVICE_MOUNT;

    if (strlen(cmd) > SRPC_PLOAD_SIZE)
    {
        return -1;
    }

    memcpy(req.param.pload, cmd, strlen(cmd));

    if (srpc_call(&req, 20 * 1000))
    {
        memcpy(&ret, req.param.pload, sizeof(int));
    }

    return ret;
}

int rpc_umount(char *cmd)
{
    int ret = -1;
    srpc_req req;
    DISKRPC(rpc);
    memset(&req, 0 , sizeof(srpc_req));
    req.rpc = &rpc;
    req.param.service_id = RPCSVR_SERVICE_UMOUNT;

    if (strlen(cmd) > SRPC_PLOAD_SIZE)
    {
        return -1;
    }

    memcpy(req.param.pload, cmd, strlen(cmd));

    if (srpc_call(&req, 20 * 1000))
    {
        memcpy(&ret, req.param.pload, sizeof(int));
    }

    return ret;
}

int rpc_cryptsetup(char *cmd)
{
    int ret = -1;
    srpc_req req;
    DISKRPC(rpc);
    memset(&req, 0 , sizeof(srpc_req));
    req.rpc = &rpc;
    req.param.service_id = RPCSVR_SERVICE_CRYPTSETUP;

    if (strlen(cmd) > SRPC_PLOAD_SIZE)
    {
        return -1;
    }

    memcpy(req.param.pload, cmd, strlen(cmd));

    if (srpc_call(&req, 30 * 1000))
    {
        memcpy(&ret, req.param.pload, sizeof(int));
    }

    return ret;
}

int rpc_losetup(char *cmd)
{
    int ret = -1;
    srpc_req req;
    DISKRPC(rpc);
    memset(&req, 0 , sizeof(srpc_req));
    req.rpc = &rpc;
    req.param.service_id = RPCSVR_SERVICE_LOSETUP;

    if (strlen(cmd) > SRPC_PLOAD_SIZE)
    {
        return -1;
    }

    memcpy(req.param.pload, cmd, strlen(cmd));

    if (srpc_call(&req, 30 * 1000))
    {
        memcpy(&ret, req.param.pload, sizeof(int));
    }

    return ret;
}

int rpc_cdgzipcpio(char *path1, char *fpath)
{
    int ret = -1;
    srpc_req req;
    DISKRPC(rpc);
    memset(&req, 0 , sizeof(srpc_req));
    req.rpc = &rpc;
    req.param.service_id = RPCSVR_SERVICE_CDGZIPCPIO;

    if ((strlen(path1) + strlen(fpath)+30) > SRPC_PLOAD_SIZE)
    {
        return -1;
    }

    //cd %s && gzip -dc %s | cpio -divmu
    //memcpy(req.param.pload, cmd, strlen(cmd));
    sprintf(req.param.pload,"%s && gzip -dc %s | cpio -dmvu -i key",path1, fpath);

    if (srpc_call(&req, 45 * 1000))
    {
        memcpy(&ret, req.param.pload, sizeof(int));
    }

    return ret;
}

int rpc_rmmapperfile(char *cmd)
{
    int ret = -1;
    srpc_req req;
    DISKRPC(rpc);
    memset(&req, 0 , sizeof(srpc_req));
    req.rpc = &rpc;
    req.param.service_id = RPCSVR_SERVICE_RMMAPPERFILE;

    if (strlen(cmd) > SRPC_PLOAD_SIZE)
    {
        return -1;
    }

    memcpy(req.param.pload, cmd, strlen(cmd));

    if (srpc_call(&req, 25 * 1000))
    {
        memcpy(&ret, req.param.pload, sizeof(int));
    }

    return ret;
}

int rpc_cpfile(char *cmd)
{
    int ret = -1;
    srpc_req req;
    DISKRPC(rpc);
    memset(&req, 0 , sizeof(srpc_req));
    req.rpc = &rpc;
    req.param.service_id = RPCSVR_SERVICE_CP;

    if (strlen(cmd) > SRPC_PLOAD_SIZE)
    {
        return -1;
    }

    memcpy(req.param.pload, cmd, strlen(cmd));

    if (srpc_call(&req, 25 * 1000))
    {
        memcpy(&ret, req.param.pload, sizeof(int));
    }

    return ret;
}

int rpc_rmappdatafile(char *cmd)
{
    int ret = -1;
    srpc_req req;
    DISKRPC(rpc);
    memset(&req, 0 , sizeof(srpc_req));
    req.rpc = &rpc;
    req.param.service_id = RPCSVR_SERVICE_RMAPPDATAFILE;

    if (strlen(cmd) > SRPC_PLOAD_SIZE)
    {
        return -1;
    }

    memcpy(req.param.pload, cmd, strlen(cmd));

    if (srpc_call(&req, 10 * 1000))
    {
        memcpy(&ret, req.param.pload, sizeof(int));
    }

    return ret;
}

int rpc_ethconfig(char *eth_name,PIP_LOC_CFG cfg)
{
    int ret = 0;
    srpc_req req;
    DISKRPC(rpc);
    memset(&req, 0 , sizeof(srpc_req));
    req.rpc = &rpc;
    req.param.service_id = RPCSVR_SERVICE_ETHCONFIG;

    if (strlen(eth_name)+sizeof(IP_LOC_CFG) > SRPC_PLOAD_SIZE)
    {
        return -1;
    }

    RPC_DEBUG("strlen(eth_name)=%d sizeof(IP_LOC_CFG)=%d\n", strlen(eth_name), sizeof(IP_LOC_CFG));
    memcpy(req.param.pload, cfg, sizeof(IP_LOC_CFG));
    memcpy(req.param.pload+sizeof(IP_LOC_CFG), eth_name, strlen(eth_name));

    if (srpc_call(&req,30 * 1000))
    {
        memcpy(&ret, req.param.pload, sizeof(int));
    }

    return ret;
}
