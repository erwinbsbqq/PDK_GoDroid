/****************************************************************************
 *
 *  ALi (Zhuhai) Corporation, All Rights Reserved. 2012 Copyright (C)
 *
 *  File: ali_rpc_util.c
 *
 *  Description: Ali Remote Prcedure Call driver between main and see CPU.
 *
 *  History:
 *      Date              Author              Version              Comment
 *      ====            ======          =======         =======
 *  1.  2013.03.14     Tony.Zh            0.1.000             Initial
 ****************************************************************************/
#include <ali_rpc_errno.h>
#include <ali_rpc_util.h>
#include <ali_shm.h>

#if 0
/*
* Make a CList element, will allocate a Clist and store data value
* Return: NULL->Failed, Other value->Success
*/
CList *MakeClistElement(void *data)
{
    CList *plist = NULL;
    if ((plist = (CList *)PR_Malloc(sizeof(CList))) == NULL)
    {
        return NULL;
    }
    memset(plist, 0, sizeof(CList));
    plist->data = data;

    return plist;
}
#endif

/*
* Initializes an array as a queue, and sets all elements of it.
* Return: -1->Failed, 0->Success
*/
Int32 QueueInit(void *q, Uint32 elemsize, Uint32 capacity)
{
    Queue *tq = (Queue *)q;
    void  *tbuf = NULL;

    Log(LOG_DEBUG, "QueueInit q:0x%x\n", q);
    if (elemsize == 0 || capacity > MAX_QUEUE_CAPACITY_SIZE)
    {
        return RPC_UTIL_ERR;
    }
    tbuf = PR_Malloc(elemsize * capacity);
    if (!tbuf)
    {
        return RPC_UTIL_ERR;
    }
    memset(tbuf, 0, elemsize * capacity);
    tq->head = tbuf;
    tq->elemsize = elemsize;
    tq->nelts = tq->in = tq->first = tq->out = 0;
    tq->bounds = capacity;

    Log(LOG_DEBUG, "QueueInit capacity:%d, q:0x%x\n", capacity, q);

    return RPC_SUCCESS_VALUE;
}

/*
* Drops all the elements stored in the queue.
* Return:  Non
*/
void QueueDeinit(void *q)
{
    Queue *tq = (Queue *)q;
    
    if (tq->bounds > 0 && tq->elemsize > 0)
    {
        if (tq->head)
        {
            PR_Free(tq->head);
        }
        tq->head = NULL;
        tq->elemsize = tq->nelts = tq->bounds = 0;
    }

}

/*
* Push the specific element into the queue.
* Return: -1->Failed, Other Value->Success
*/
Int32 QueuePush(Queue *q, void *element)
{
    void *ptemp = NULL;
    unsigned long utemp = 0;

    Log(LOG_DEBUG, "QueuePush, q:0x%x, element:0x%x, bounds:0x%x, first:%d, nelt:%d\n", q, element, q->bounds, q->first, q->nelts);
    /*if(q->in <0 )
        tq->nelts = tq->in = tq->first = tq->out = 0;
    */

    if (QueueFull(q))
    {
        return RPC_UTIL_ERR;
    }
    if (q->in < q->bounds)
    {
        ptemp = (Uint8 *)q->head + q->elemsize * q->in;
        utemp = element;
        Log(LOG_DEBUG, "QueuePush utemp:0x%x, element:0x%x, q->first:%d, q->in:%d, q->out:%d, nelt:%d, bound:%d\n", 
        utemp, element, q->first, q->in, q->out, q->nelts, q->bounds);
        memcpy(ptemp, &utemp, sizeof(void *));
        q->out = q->in;
        q->nelts++;
        /*find out the empty location*/
        if (QueueFull(q))
        {
            return q->out;
        }

        q->in++;
        if (q->in >= q->bounds)
        {
            q->in = 0;
        }

    }
    else
    {
        return RPC_UTIL_ERR;
    }

    return RPC_SUCCESS_VALUE;

}

/*
* Refers to the front element of the queue, in other words it is the first element of the queue. Returns NULL if the queue is empty.
* Return: NULL->Empty Queue, Other value->Success got Element
*/
void *QueueFront(Queue *q)
{
    unsigned long  utemp = 0;
    
    if (QueueEmpty(q))
    {
        return NULL;
    }
    //Log(LOG_DEBUG,"QueueFront,q->first:%d,q->in:%d,q->out:%d\n",q->first,q->in,q->out);
    memcpy(&utemp, (void *)(q->head + q->elemsize * q->first), sizeof(void *));

    return (void *)utemp;
}

/*
* Refers to the back element of the queue, in other words it is the last elements of the queue. Returns NULL if the queue is empty.
* Return: NULL->Empty Queue, Other value->Success got Element
*/
void *QueueBack(Queue *q)
{
    unsigned long utemp = 0;
    
    if (QueueEmpty(q))
    {
        return NULL;
    }

    memcpy(&utemp, (void *)(q->head + q->elemsize * q->out), sizeof(void *));

    return (void *)utemp;

}

/*
* Pops the front element of the queue, the popped element room in the queue is empty then.
* Return: Non
*/
void QueuePop(Queue *q)
{
    if (QueueEmpty(q))
    {
        return;
    }

    Log(LOG_DEBUG, "QueuePop I q:0x%x, q->first:%d, q->in:%d, q->out:%d, bound:0x%x\n", q, q->first, q->in, q->out, q->bounds);
    q->nelts--;

    if (QueueEmpty(q))
    {
        q->nelts = q->in = q->first = q->out = 0;
        Log(LOG_DEBUG, "QueuePop II q:0x%x, q->first:%d, q->in:%d, q->out:%d, bounds:0x%x\n", q, q->first, q->in, q->out, q->bounds);
        return;
    }

    q->first++;
    if (q->first >= q->bounds)
    {
        q->first = 0;
    }

    Log(LOG_DEBUG, "QueuePop III q:0x%x, q->first:%d, q->in:%d, q->out:%d\n", q, q->first, q->in, q->out);

    return;

}

/*
* PID like bit map allocation algorithm
*/
int Rid_test_and_set_bit(int offset, void *addr)
{
    unsigned int mask = 1UL << (offset&(sizeof(unsigned int) * BITS_PER_BYTE - 1));
    unsigned int *p = ((unsigned int *)addr) + (offset>>(sizeof(unsigned int) + 1));
    unsigned int old = *p;
    
    *p = old | mask;
	
    return (old & mask) != 0;
}

/*check if offset bit is used*/
int Rid_test_bit(int offset, void *addr)
{
    Int32 ret = 0;
    unsigned int mask = 1UL << (offset&(sizeof(unsigned int) * BITS_PER_BYTE - 1));
    unsigned int *p = ((unsigned int *)addr) + (offset>>(sizeof(unsigned int) + 1));
    unsigned int old = *p;
    
    ret = old & mask;
    return (ret != 0);
}

void Rid_clear_bit(int offset, void *addr)
{
    unsigned int mask = 1UL << (offset&(sizeof(unsigned int) * BITS_PER_BYTE - 1));
    unsigned int *p = ((unsigned int *)addr) + (offset>>(sizeof(unsigned int) + 1));
    unsigned int old = *p;
    
    *p = old & ~mask;
}

int Rid_find_next_zero_bit(void *addr, int size, int offset)
{
    unsigned int *p = NULL;
    unsigned int mask = 0;

    while (offset < size)
    {
        p = ((unsigned int *)addr) + (offset>>(sizeof(unsigned int) + 1));
        mask = 1UL << (offset & (sizeof(unsigned int) * BITS_PER_BYTE - 1));
        if ((~(*p) & mask))
        {
            break;
        }
        ++offset;
    }
    return offset;
}



