/******************************************************************************
 * File: srpc.c
 *
 * Description: -
 *     simple rpc module, it use to build remote call.
 * History
 * --------------------
 * 1. 2012-9-3, Dong yun written
 * --------------------
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <pthread.h>
#include <errno.h>
#include "osal/srpc.h"

#define SRPC_STOP  0
#define SRPC_RUN   1

#define VALID_PKT(srpc_id) (memcmp((srpc_id), SRPCID, SRPCID_LEN) == 0)

/******************************************************************************
 * Function: __srpc_socket
 * Description: -
 *    create socket for srpc, it can use tcp socket or udp socket
 * Input:
 * Output:
 * Returns:
 * History:
 * -------------------------------------------------
 * 1. 2012-9-18, Doy.Dong Created
 * -------------------------------------------------
 ******************************************************************************/
static int __srpc_socket(srpc *rpc)
{
    int type    = rpc->socktype == SOCK_TYPE_TCP ? SOCK_STREAM : SOCK_DGRAM;
    int sock    = socket(AF_INET, type, 0);
    if(sock > 0 && rpc->non_block)
    {
        fcntl(sock, F_SETFL, O_NONBLOCK); // set non-block mode
    }
    SRPC_DEBUG("srpc create %s socket:%d %s\n", rpc->socktype == SOCK_TYPE_TCP ? "TCP" : "UDP",
               sock, rpc->non_block ? "NONBLOCK" : "");
    return sock;
}

static inline int __srpc_setsockopt(int sockfd, int level, int optname, const void *optval,
                                    socklen_t optlen)
{
    int ret = setsockopt(sockfd, level, optname, optval, optlen);
    if(ret == -1)
    {
        SRPC_DEBUG("__srpc_setsockopt: %s\n", strerror(errno));
    }
    return ret;
}

/******************************************************************************
 * Function: __srpc_connect
 * Description: -
 *    connect to srpc server, tcp socket use it.
 * Input:
 * Output:
 * Returns:
 * History:
 * -------------------------------------------------
 * 1. 2012-9-18, Doy.Dong Created
 * -------------------------------------------------
 ******************************************************************************/
static int __srpc_connect(srpc *rpc, int sock, unsigned int timeout_ms)
{
    int addr_len    = sizeof(struct sockaddr_in);
    if(rpc->non_block) //socket use nonblock mode
    {
        int             ret = 0;
        struct timeval  to;
        fd_set          fdset;
        FD_ZERO(&fdset);
        FD_SET(sock, &fdset);
        to.tv_sec = timeout_ms / 1000;
        to.tv_usec = (timeout_ms % 1000) * 1000;
        /* Connect to remote point, it will return -1 immediately, but it maybe not failed.
         * need to check th errno.
         */
        ret = connect(sock, (struct sockaddr *) &rpc->saddr, addr_len);
        /* errno = EINPROGRESS means that the connection is going.
         */
        if(ret < 0 && errno != EINPROGRESS)
        {
            SRPC_DEBUG("srpc_connect1: %s\n", strerror(errno));
            return -1;
        }
        if(ret == 0) // the connection is completed
        {
            return ret;
        }
        //the connection is going, need to check whether it complete.
        ret = select(sock + 1, NULL, &fdset, NULL, timeout_ms == 0 ? NULL : &to);
        if(ret > 0)
        {
            int error   = 0;
            int len     = sizeof(error);
            if(getsockopt(sock, SOL_SOCKET, SO_ERROR, &error, (socklen_t *) &len) < 0)
            {
                SRPC_DEBUG("srpc_connect2: %s\n", strerror(errno));
                ret = -1;
            }
            if(error)
            {
                SRPC_DEBUG("srpc_connect3: %s\n", strerror(errno));
                ret = -1;
            }
        }
        else if(ret == 0) //timeout
        {
            ret = -1;
            SRPC_DEBUG("connect to server timeout!\n");
        }
        else // error occured
        {
            SRPC_DEBUG("srpc_connect4: %s\n", strerror(errno));
            ret = -1;
        }
        return ret;
    }
    else //block mode
    {
        return connect(sock, (struct sockaddr *) &rpc->saddr, addr_len);
    }
}

/******************************************************************************
 * Function: __srpc_send_req
 * Description: -
 *    send srpc call request to remote point. it maybe is a request or response.
 * Input:
 * Output:
 * Returns:
 * History:
 * -------------------------------------------------
 * 1. 2012-9-18, Doy.Dong Created
 * -------------------------------------------------
 ******************************************************************************/
static int __srpc_send_req(srpc_req *req)
{
    SRPC_DEBUG("send service id: %d\n", req->param.service_id);
    memcpy(req->param.srpc_id, SRPCID, SRPCID_LEN);
    if(req->rpc->ehdl.on_send != NULL)
    {
        req->rpc->ehdl.on_send(req);
    }
    if(req->rpc->socktype == SOCK_TYPE_TCP)
    {
        return srpc_send(req->sock, (char *) &req->param, sizeof(srpc_param), 0);
    }
    else
    {
        return srpc_sendto(req->sock, (char *) &req->param, sizeof(srpc_param), 0,
                             (struct sockaddr *) &req->remote, sizeof(struct sockaddr_in));
    }
}

/******************************************************************************
 * Function: __srpc_find_service
 * Description: -
 *    find srpc service by id in service table.
 * Input:
 * Output:
 * Returns:
 * History:
 * -------------------------------------------------
 * 1. 2012-9-18, Doy.Dong Created
 * -------------------------------------------------
 ******************************************************************************/
static srpc_service * __srpc_find_service(srpc *rpc, unsigned int service_id)
{
    unsigned int i = 0;
    srpc_service   *service = NULL;
    if(rpc->srv_table == NULL)
    {
        return NULL;
    }
    //find service
    for(i = 0;i < rpc->srv_count;i++)
    {
        if(rpc->srv_table[i].id == service_id)
        {
            service = &rpc->srv_table[i];
            break;
        }
    }
    return service;
}


/******************************************************************************
 * Function: __srpc_proc_req
 * Description: -
 *    process the request from remote point, it maybe a request or response.
 * Input:
 * Output:
 * Returns:
 * History:
 * -------------------------------------------------
 * 1. 2012-9-18, Doy.Dong Created
 * -------------------------------------------------
 ******************************************************************************/
static int __srpc_proc_req(srpc_req *req)
{
    srpc_service   *service = __srpc_find_service(req->rpc, req->param.service_id);
    if(service != NULL && service->func != NULL)
    {
        SRPC_DEBUG("receive service id:0x%x\n", req->param.service_id);
        service->func(req);
        if(srpc_get_flag(&req->param, SRPC_FLAG_NOACK) == 0) //need ack
        {
            __srpc_send_req(req); //send ack
        }
        return 1;
    }
    else //not support
    {
        SRPC_DEBUG("unknown service id: 0x%x\n", req->param.service_id);
        return 0;
    }
}


/******************************************************************************
 * Function: __srpc_accept
 * Description: -
 *    accept a connection request, it is for tcp socket.
 * Input:
 * Output:
 * Returns:
 * History:
 * -------------------------------------------------
 * 1. 2012-9-18, Doy.Dong Created
 * -------------------------------------------------
 ******************************************************************************/
static int __srpc_accept(srpc *rpc, int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
    if(rpc->non_block)
    {
        int             ret = -1;
        fd_set          fdset;
        struct timeval  to;
        while(rpc->state == SRPC_RUN)
        {
            to.tv_sec = 1;
            to.tv_usec = 0;
            FD_ZERO(&fdset);
            FD_SET(sockfd, &fdset);
            ret = select(sockfd + 1, &fdset, NULL, NULL, &to);
            if(ret == 0)
            {
                //SRPC_DEBUG("__srpc_accept timeout\n");
                continue;
            }
            else if(ret > 0)
            {
                ret = accept(sockfd, addr, addrlen);
            }
            else // error ocurred
            {
                sleep(1);
            }
            break;
        }
        return ret;
    }
    else
    {
        return accept(sockfd, addr, addrlen);
    }
}


/******************************************************************************
 * Function: __srpc_recv_proc_tcp__
 * Description: -
 *    process receive req from remote point when using tcp socket.
 * Input:
 * Output:
 * Returns:
 * History:
 * -------------------------------------------------
 * 1. 2012-9-18, Doy.Dong Created
 * -------------------------------------------------
 ******************************************************************************/
static void * __srpc_recv_proc__(void *args)
{
    srpc_req           *req     = NULL;
    int                 len     = 0;
    int                 sclose  = 0;
    int                 ret     = 0;
	pthread_detach(pthread_self());

    if(args == NULL)
    {
        pthread_exit(0);

        return;
    }

    req = (srpc_req *) args;

    if(req->rpc->socktype == SOCK_TYPE_TCP) //tcp socket, need receive the data
    {
        memset(&req->param, 0, sizeof(srpc_param));
        len = srpc_recv(req->sock, (char *) &req->param, sizeof(srpc_param), 0, SRPC_DEF_TIMEOUT);
        if(len != sizeof(srpc_param) || !VALID_PKT(req->param.srpc_id))
        {
            sclose = 1;
            goto _EXIT;
        }
    }

    if(req->rpc->ehdl.on_recv != NULL) // pre-process data
    {
        req->rpc->ehdl.on_recv(req);
    }

    // process the request
    ret = __srpc_proc_req(req);

     //for tcp, maybe need close the socket
     //for udp, can not close the socket.
    if(req->rpc->socktype == SOCK_TYPE_TCP)
    {
        if(!ret || srpc_get_flag(&req->param, SRPC_FLAG_KEEPALIVE) == 0)
        {
            SRPC_DEBUG("needn't keep alive, close it!\n");
            sclose = 1;
        }
    }

_EXIT:
    if(sclose == 1)
    {
        close(req->sock);
    }
    free(args);
	pthread_exit(0);
    return NULL;
}

/******************************************************************************
 * Function: __srpc_recv_proc_udp
 * Description: -
 *    process receive req from remote point when using udp socket.
 * Input:
 * Output:
 * Returns:
 * History:
 * -------------------------------------------------
 * 1. 2012-9-18, Doy.Dong Created
 * -------------------------------------------------
 ******************************************************************************/
static void __srpc_recv_proc(srpc_req *req)
{
    pthread_t pth;
    int       ret;
    srpc_req   *req_    = malloc(sizeof(srpc_req));
    if(req_ == NULL)
    {
        SRPC_DEBUG("malloc failed: %s\n", strerror(errno));
        return;
    }
    memcpy(req_, req, sizeof(srpc_req));
    ret = pthread_create(&pth, NULL, __srpc_recv_proc__, req_);
    if (ret < 0)
    {
        free(req_);
    }

    return;
}

/******************************************************************************
 * Function: __srpc_task_udp
 * Description: -
 *    srpc server task for udp socket.
 * Input:
 * Output:
 * Returns:
 * History:
 * -------------------------------------------------
 * 1. 2012-9-18, Doy.Dong Created
 * -------------------------------------------------
 ******************************************************************************/
static void * __srpc_task_udp(void *args)
{
    int         len     = sizeof(struct sockaddr_in);
    int         rlen    = 0;
    srpc_req    req;
    srpc    *rpc     = (srpc *) args;
    rpc->state = SRPC_RUN;

    while((rpc->sock = __srpc_socket(rpc)) < 0)
    {
        perror("__srpc_socket");
        sleep(2);
    }
    while(bind(rpc->sock, (struct sockaddr *) &rpc->saddr, len) < 0)
    {
        perror("srpc bind");
        sleep(2);
    }

    SRPC_DEBUG("srpc service running ...\n");
    memset(&req, 0, sizeof(srpc_req));
    req.rpc = rpc;
    req.sock = rpc->sock;
    while(rpc->state == SRPC_RUN)
    {
        memset(&req.param, 0, sizeof(srpc_param));
        rlen = srpc_recvfrom(rpc->sock, (char *) &req.param, sizeof(srpc_param), 0,
                               (struct sockaddr *) &req.remote, (socklen_t *) &len, SRPC_DEF_TIMEOUT);
        if(rlen == sizeof(srpc_param) && VALID_PKT(req.param.srpc_id))
        {
            #if 0
            if(rpc->state != SRPC_RUN)
            {
                break;
            }
            #endif
            SRPC_DEBUG("receive a request from:%s:%d\n", inet_ntoa(req.remote.sin_addr),
                       req.remote.sin_port);
            __srpc_recv_proc(&req);
        }
    };

    close(rpc->sock);
    SRPC_DEBUG("srpc service exit!\n");
    return NULL;
}


/******************************************************************************
 * Function: __srpc_task_tcp
 * Description: -
 *    srpc server task for tcp socket.
 * Input:
 * Output:
 * Returns:
 * History:
 * -------------------------------------------------
 * 1. 2012-9-18, Doy.Dong Created
 * -------------------------------------------------
 ******************************************************************************/
static void * __srpc_task_tcp(void *args)
{
    int                 len     = sizeof(struct sockaddr_in);
    srpc               *rpc     = (srpc *) args;
    srpc_req            req;

    rpc->state = SRPC_RUN;

    while((rpc->sock = __srpc_socket(rpc)) < 0)
    {
        perror("__srpc_socket");
        sleep(2);
    }
    while(bind(rpc->sock, (struct sockaddr *) &rpc->saddr, len) < 0)
    {
        perror("srpc bind");
        sleep(2);
    }
    while(listen(rpc->sock, 4) == -1)
    {
        perror("srpc listen");
        sleep(2);
    }

    SRPC_DEBUG("srpc service running ...\n");
    memset(&req, 0, sizeof(srpc_req));
    req.rpc = rpc;
    while(rpc->state == SRPC_RUN)
    {
        req.sock = __srpc_accept(rpc, rpc->sock, (struct sockaddr *) &req.remote, (socklen_t *) &len);
        if(req.sock > 0)
        {
            if(rpc->state != SRPC_RUN) //task need exit.
            {
                close(req.sock);
                break;
            }
            SRPC_DEBUG("accept a connect from:%s:%d\n", inet_ntoa(req.remote.sin_addr), req.remote.sin_port);
            __srpc_recv_proc(&req);
        }
    };
    close(rpc->sock);
    SRPC_DEBUG("srpc service exit!\n");
    return NULL;
}

/******************************************************************************
 * Function: srpc_send
 * Description: -
 *    send req to remote point, it is for tcp socket
 * Input:
 * Output:
 * Returns:
 * History:
 * -------------------------------------------------
 * 1. 2012-9-18, Doy.Dong Created
 * -------------------------------------------------
 ******************************************************************************/
int srpc_send(int sockfd, const void *buf, size_t len, int flags)
{
    int             ret = 0;
    struct timeval  to;
    fd_set          fdset;
    FD_ZERO(&fdset);
    FD_SET(sockfd, &fdset);
    to.tv_sec = SRPC_DEF_TIMEOUT / 1000;
    to.tv_usec = (SRPC_DEF_TIMEOUT % 1000) * 1000;
    ret = select(sockfd + 1, NULL, &fdset, NULL, &to);
    if(ret == 0)
    {
        SRPC_DEBUG("srpc_send, timeout!\n");
    }
    if(ret > 0)
    {
        ret = send(sockfd, buf, len, flags);
        if(ret <= 0)
        {
            SRPC_DEBUG("send: %s\n", strerror(errno));
            ret = -1;
        }
    }
    return ret;
}

/******************************************************************************
 * Function: srpc_sendto
 * Description: -
 *    send req to remote point, it is for udp socket
 * Input:
 * Output:
 * Returns:
 * History:
 * -------------------------------------------------
 * 1. 2012-9-18, Doy.Dong Created
 * -------------------------------------------------
 ******************************************************************************/
int srpc_sendto(int sockfd, const void *buf, size_t len, int flags,
                         const struct sockaddr *dest_addr, socklen_t addrlen)
{
    int             ret = 0;
    struct timeval  to;
    fd_set          fdset;
    FD_ZERO(&fdset);
    FD_SET(sockfd, &fdset);
    to.tv_sec = SRPC_DEF_TIMEOUT / 1000;
    to.tv_usec = (SRPC_DEF_TIMEOUT % 1000) * 1000;
    ret = select(sockfd + 1, NULL, &fdset, NULL, &to);
    if(ret == 0)
    {
        SRPC_DEBUG("srpc_send, timeout!\n");
    }
    if(ret > 0)
    {
        ret = sendto(sockfd, buf, len, flags, dest_addr, addrlen);
        if(ret <= 0)
        {
            SRPC_DEBUG("send: %s\n", strerror(errno));
            ret = -1;
        }
    }
    return ret;
}


/******************************************************************************
 * Function: srpc_recv
 * Description: -
 *    receive req from remote point , it is for tcp socket.
 * Input:
 * Output:
 * Returns:
 * History:
 * -------------------------------------------------
 * 1. 2012-9-18, Doy.Dong Created
 * -------------------------------------------------
 ******************************************************************************/
int srpc_recv(int sockfd, void *buf, size_t len, int flags, unsigned int timeout_ms)
{
    int             ret = 0;
    struct timeval  to;
    fd_set          fdset;
    FD_ZERO(&fdset);
    FD_SET(sockfd, &fdset);
    to.tv_sec = timeout_ms / 1000;
    to.tv_usec = (timeout_ms % 1000) * 1000;
    ret = select(sockfd + 1, &fdset, NULL, NULL, timeout_ms == 0 ? NULL : &to);
    if(ret > 0)
    {
        if(FD_ISSET(sockfd, &fdset))
        {
            ret = recv(sockfd, buf, len, flags);
            if(ret <= 0)
            {
                SRPC_DEBUG("srpc_recv: %s\n", strerror(errno));
            }
            ret = (ret <= 0) ? -1 : ret;
        }
        else
        {
            ret = -1;
        }
    }
    return ret;
}

/******************************************************************************
 * Function: srpc_recvfrom
 * Description: -
 *    receive req from remote point, it is for udp socket.
 * Input:
 * Output:
 * Returns:
 * History:
 * -------------------------------------------------
 * 1. 2012-9-18, Doy.Dong Created
 * -------------------------------------------------
 ******************************************************************************/
int srpc_recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr,
                           socklen_t *addrlen, unsigned int timeout_ms)
{
    int             ret = 0;
    struct timeval  to;
    fd_set          fdset;
    FD_ZERO(&fdset);
    FD_SET(sockfd, &fdset);
    to.tv_sec = timeout_ms / 1000;
    to.tv_usec = (timeout_ms % 1000) * 1000;
    ret = select(sockfd + 1, &fdset, NULL, NULL, timeout_ms == 0 ? NULL : &to);
    if(ret > 0)
    {
        if(FD_ISSET(sockfd, &fdset))
        {
            ret = recvfrom(sockfd, buf, len, flags, src_addr, addrlen);
            if(ret <= 0)
            {
                SRPC_DEBUG("srpc_recvfrom: %s\n", strerror(errno));
            }
            ret = (ret <= 0) ? -1 : ret;
        }
        else
        {
            ret = -1;
        }
    }
    return ret;
}


/******************************************************************************
 * Function: srpc_start
 * Description: -
 *    start a srpc server: rpc
 * Input:
 *    rpc: srpc server
 * Output: <NONE>
 * Returns:
 *   1: rpc start successs
 *   0: start faile.
 * History:
 * -------------------------------------------------
 * 1. 2012-9-3, Dong yun Created
 * -------------------------------------------------
 ******************************************************************************/
int srpc_start(srpc *rpc)
{
    int         ret = 0;
    pthread_t   pth;
    if(rpc == NULL)
    {
        return 0;
    }
    if(rpc->socktype == SOCK_TYPE_TCP)
    {
        SRPC_DEBUG("start a TCP srpc server!\n");
        ret = pthread_create(&pth, NULL, __srpc_task_tcp, rpc);
    }
    else
    {
        SRPC_DEBUG("start a UDP srpc server!\n");
        ret = pthread_create(&pth, NULL, __srpc_task_udp, rpc);
    }
    return ret == 0 ? 1 : 0;
}


/******************************************************************************
 * Function: srpc_stop
 * Description: -
 *    stop the srpc server: rpc
 * Input:
 *    rpc: srpc server
 * Output:  <NONE>
 * Returns: <NONE>
 * History:
 * -------------------------------------------------
 * 1. 2012-9-3, Dong yun Created
 * -------------------------------------------------
 ******************************************************************************/
void srpc_stop(srpc *rpc)
{
    if(rpc == NULL)
    {
        return;
    }

    rpc->state = SRPC_STOP;
    if(rpc->sock > 0)
    {
        close(rpc->sock);
        rpc->sock = 0;
    }
}

/******************************************************************************
 * Function: srpc_call
 * Description: -
 *    send a remote call request to the srpc server: rpc.
 * Input:
 *    rpc           : srpc server
 *    req           : remote call message, only need set the "param" feild.
 *    timeout_ms : timeout,unit:ms, it indicate that how many times need used to run
 *                 this command in server, and not contain the time of build connection.
 * Output:
 * Returns:
 *        1: send remote call success and received the response
 *        0: remote call failed.
 * History:
 * -------------------------------------------------
 * 1. 2012-9-3, Dong yun Created
 * -------------------------------------------------
 ******************************************************************************/
int srpc_call(srpc_req *req, unsigned int timeout_ms)
{
    int                 ret         = 0;
    int                 sock        = 0;
    unsigned int        service_id  = 0;
    int                 rlen        = 1;
    int                 addr_len    = 0;
    struct sockaddr_in  local;

    if(req == NULL || req->rpc == NULL)
    {
        return ret;
    }

    if((sock = __srpc_socket(req->rpc)) == -1)//create socket
    {
        return ret;
    }

    addr_len = sizeof(struct sockaddr_in);
    service_id = req->param.service_id;
    req->sock = sock; //the caller maybe need use the socket.

    memset(&local, 0, sizeof(struct sockaddr_in));
    local.sin_family = AF_INET;
    if(bind(sock, (struct sockaddr *) &local, addr_len) < 0) //bind local address
    {
        SRPC_DEBUG("srpc bind: %s\n", strerror(errno));
        return ret;
    }

    if(req->rpc->socktype == SOCK_TYPE_TCP) //tcp socket, need connect first
    {
        if(__srpc_connect(req->rpc, sock, timeout_ms) < 0)  //connect to srpc server
        {
            SRPC_DEBUG("srpc connect: %s\n", strerror(errno));
            return 0;
        }
    }
    else //udp socket, set req.remote -> svr_addr
    {
        memcpy(&req->remote, &req->rpc->saddr, sizeof(struct sockaddr_in));
    }

    //send remote call request to server
    if(__srpc_send_req(req) == sizeof(srpc_param))
    {
        if(srpc_get_flag(&req->param, SRPC_FLAG_NOACK) == 0) //need receive ack
        {
            SRPC_DEBUG("need receive ack, wait ...\n");
            if(req->rpc->socktype == SOCK_TYPE_TCP) //use tcp socket
            {
                rlen = srpc_recv(sock, (char *) &req->param, sizeof(srpc_param), 0, timeout_ms);
            }
            else //use udp socket
            {
                rlen = srpc_recvfrom(sock, (char *) &req->param, sizeof(srpc_param), 0,
                                       (struct sockaddr *) &req->remote, (socklen_t *) &addr_len,
                                       timeout_ms);
            }
            if(rlen == sizeof(srpc_param))
            {
                if(VALID_PKT(req->param.srpc_id) && req->param.service_id == service_id)
                {
                    if(req->rpc->ehdl.on_recv != NULL) //trigger the event: on_recv
                    {
                        req->rpc->ehdl.on_recv(req);
                    }
                }
            }
            else
            {
                rlen = 0;
            }
        }

        if(rlen > 0) //success
        {
            if(srpc_get_flag(&req->param, SRPC_FLAG_KEEPALIVE) == 0)
            {
                SRPC_DEBUG("needn't keep alive, close the socket!\n");
                close(sock);
                req->sock = 0; //rpc call success
            }
            ret = 1;
        }
        else //error occurred.
        {
            close(sock);
            req->sock = 0;
        }
    }
    else //error occurred.
    {
        SRPC_DEBUG("srpc send: %s\n", strerror(errno));
        close(sock);
        req->sock = 0;
    }
    return ret;
}

