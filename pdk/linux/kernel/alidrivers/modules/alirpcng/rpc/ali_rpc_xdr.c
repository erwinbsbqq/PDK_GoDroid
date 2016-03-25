/****************************************************************************
 *
 *  ALi (Zhuhai) Corporation, All Rights Reserved. 2013 Copyright (C)
 *
 *  File: ali_rpc_xdr.c
 *
 *  Description: Ali Remote Prcedure Call driver between main and see CPU.
 *
 *  History:
 *      Date              Author              Version              Comment
 *      ====            ======          =======         =======
 *  1.  2013.03.14     Tony.Zh            0.1.000             Initial
 ****************************************************************************/
#include <ali_rpc_errno.h>
#include <ali_rpc_xdr.h>

extern Int32   g_BigEndian;


#if defined(__ALI_TDS__)
static Ulong ntohl(Ulong n)
{
    return g_BigEndian ? n : BigLittleSwap32(n);

}

static Ulong htonl(Ulong h)
{
    return g_BigEndian ? h : BigLittleSwap32(h);

}
#endif

/*
 * for unit alignment
 */
static char xdr_zero[BYTES_PER_XDR_UNIT] = { 0, 0, 0, 0 };

/*
 *  bcopy(char *s1, char *s2, int len) --
 *      Copies len bytes from s1 to s2
 */
void
Bcopy(char *s1, char *s2, int len)
{
    for (; len > 0; len--)
    {
        *s2++ = *s1++;
    }
}

/*
 *  bzero(char *s, int len) --
 *      Places len zero byes in s
 */
void
Bzero(char *s, int len)
{
    for (; len > 0; len--)
    {
        *s++ = (char) 0;
    }
}

/*
 *  bcmp() compares byte  string  b1  against  byte  string  b2,
 *  returning  zero  if  they are identical, non-zero otherwise.
 *  Both strings are assumed to be length bytes long.  bcmp() of
 *  length zero bytes always returns zero.
*/
int
Bcmp(char *s1, char *s2, int len)
{
    for (; len > 0; len--, s1++, s2++)
        if (*s1 != *s2)
        {
            return 1;
        }
    return RPC_SUCCESS_VALUE;
}

/*
 * Free a data structure using XDR
 * Not a filter, but a convenient utility nonetheless
 */
void XDR_Free(XDRPROC_t proc, Char *objp)
{
    XDR x;

    x.x_op = XDR_FREE;
    (*proc)(&x, objp);
}

/*
 * XDR nothing
 */
Bool XDR_Void(void)
{

    return (TRUE);
}

/*
 * XDR integers
 */
Bool XDR_Int32(XDR *xdrs, Int32 *ip)
{

#ifdef lint
    (void)(XDR_Int16(xdrs, (Int16 *)ip));
    return (XDR_Long(xdrs, (Long *)ip));
#else
    if (sizeof(Int32) == sizeof(Long))
    {
        return (XDR_Long(xdrs, (Long *)ip));
    }
    else
    {
        return (XDR_Int16(xdrs, (Int16 *)ip));
    }
#endif
}

/*
 * XDR unsigned integers
 */
Bool XDR_Uint32(XDR *xdrs, Uint32 *up)
{

#ifdef lint
    (void)(XDR_Int16(xdrs, (Int16 *)up));
    return (XDR_Ulong(xdrs, (Ulong *)up));
#else
    if (sizeof(Uint32) == sizeof(Ulong))
    {
        return (XDR_Ulong(xdrs, (Ulong *)up));
    }
    else
    {
        return (XDR_Int16(xdrs, (Int16 *)up));
    }
#endif
}

/*
 * XDR long integers
 * same as xdr_u_long - open coded to save a proc call!
 */
Bool XDR_Long(XDR *xdrs, Long *lp)
{

    if (xdrs->x_op == XDR_ENCODE)
    {
        return (XDR_PUTLONG(xdrs, lp));
    }

    if (xdrs->x_op == XDR_DECODE)
    {
        return (XDR_GETLONG(xdrs, lp));
    }

    if (xdrs->x_op == XDR_FREE)
    {
        return (TRUE);
    }

    return (FALSE);
}

/*
 * XDR unsigned long integers
 * same as xdr_long - open coded to save a proc call!
 */
Bool XDR_Ulong(XDR *xdrs, Ulong *ulp)
{

    if (xdrs->x_op == XDR_DECODE)
    {
        return (XDR_GETLONG(xdrs, (Long *)ulp));
    }
    if (xdrs->x_op == XDR_ENCODE)
    {
        return (XDR_PUTLONG(xdrs, (Long *)ulp));
    }
    if (xdrs->x_op == XDR_FREE)
    {
        return (TRUE);
    }
    return (FALSE);
}

/*
 * XDR short integers
 */
Bool XDR_Int16(XDR *xdrs, Int16 *sp)
{
    Long l = 0;

    switch (xdrs->x_op)
    {

        case XDR_ENCODE:
            l = (Long) * sp;
            return (XDR_PUTLONG(xdrs, &l));

        case XDR_DECODE:
            if (!XDR_GETLONG(xdrs, &l))
            {
                return (FALSE);
            }
            *sp = (short) l;
            return (TRUE);

        case XDR_FREE:
            return (TRUE);
        default:
        	  break;
    }
    return (FALSE);
}

/*
 * XDR unsigned short integers
 */
Bool XDR_UInt16(XDR *xdrs, Uint16 *usp)
{
    Ulong l = 0;

    switch (xdrs->x_op)
    {

        case XDR_ENCODE:
            l = (Ulong) * usp;
            return (XDR_PUTLONG(xdrs, &l));

        case XDR_DECODE:
            if (!XDR_GETLONG(xdrs, &l))
            {
                return (FALSE);
            }
            *usp = (Uint16) l;
            return (TRUE);

        case XDR_FREE:
            return (TRUE);
        default:
        	  break;
    }
    return (FALSE);
}


/*
 * XDR a char
 */
Bool XDR_Char(XDR *xdrs, Char *cp)
{
    int i = 0;

    i = (*cp);
    if (!XDR_Int32(xdrs, &i))
    {
        return (FALSE);
    }
    *cp = i;
    return (TRUE);
}

/*
 * XDR an unsigned char
 */
Bool XDR_Uchar(XDR *xdrs, Uchar *cp)
{
    Uint32 u = 0;

    u = (*cp);
    if (!XDR_Uint32(xdrs, &u))
    {
        return (FALSE);
    }
    *cp = u;
    return (TRUE);
}

/*
 * XDR booleans
 */
Bool XDR_Bool(XDR *xdrs, Bool *bp)
{
    Long lb = 0;

    switch (xdrs->x_op)
    {

        case XDR_ENCODE:
            lb = *bp ? XDR_TRUE : XDR_FALSE;
            return (XDR_PUTLONG(xdrs, &lb));

        case XDR_DECODE:
            if (!XDR_GETLONG(xdrs, &lb))
            {
                return (FALSE);
            }
            *bp = (lb == XDR_FALSE) ? FALSE : TRUE;
            return (TRUE);

        case XDR_FREE:
            return (TRUE);
        default:
        	  break;
    }
    return (FALSE);
}

/*
 * XDR enumerations
 */
Bool XDR_Enum(XDR *xdrs, Enum *ep)
{
#ifndef lint
    enum sizecheck { SIZEVAL = 0 }; /* used to find the size of an enum */

    /*
     * enums are treated as ints
     */
    if (sizeof(enum sizecheck) == sizeof(Long))
    {
        return (XDR_Long(xdrs, (Long *)ep));
    }
    else if (sizeof(enum sizecheck) == sizeof(Int16))
    {
        return (XDR_Int16(xdrs, (Int16 *)ep));
    }
    else
    {
        return (FALSE);
    }
#else
    (void)(XDR_Int16(xdrs, (Int16 *)ep));
    return (XDR_Long(xdrs, (Long *)ep));
#endif
}

/*
 * XDR opaque data
 * Allows the specification of a fixed size sequence of opaque bytes.
 * cp points to the opaque object and cnt gives the byte length.
 */
Bool XDR_Opaque(XDR *xdrs, Opaque cp, Uint32 cnt)
{
    Uint32 rndup = 0;
    
    static crud[BYTES_PER_XDR_UNIT];

    /*
     * if no data we are done
     */
    if (cnt == 0)
    {
        return (TRUE);
    }

    /*
     * round byte count to full xdr units
     */
    rndup = cnt % BYTES_PER_XDR_UNIT;
    if (rndup > 0)
    {
        rndup = BYTES_PER_XDR_UNIT - rndup;
    }

    if (xdrs->x_op == XDR_DECODE)
    {
        if (!XDR_GETBYTES(xdrs, cp, cnt))
        {
            return (FALSE);
        }
        if (rndup == 0)
        {
            return (TRUE);
        }
        return (XDR_GETBYTES(xdrs, crud, rndup));
    }

    if (xdrs->x_op == XDR_ENCODE)
    {
        if (!XDR_PUTBYTES(xdrs, cp, cnt))
        {
            return (FALSE);
        }
        if (rndup == 0)
        {
            return (TRUE);
        }
        return (XDR_PUTBYTES(xdrs, xdr_zero, rndup));
    }

    if (xdrs->x_op == XDR_FREE)
    {
        return (TRUE);
    }

    return (FALSE);
}

/*
 * XDR counted bytes
 * *cpp is a pointer to the bytes, *sizep is the count.
 * If *cpp is NULL maxsize bytes are allocated
 */
Bool XDR_Bytes(XDR *xdrs, Bytes *cpp, Uint32 *sizep, Uint32 maxsize)
{
    Char *sp = *cpp;  /* sp is the actual string pointer */
    Uint32 nodesize = 0;

    /*
     * first deal with the length since xdr bytes are counted
     */
    if (!XDR_Uint32(xdrs, sizep))
    {
        return (FALSE);
    }
    nodesize = *sizep;
    if ((nodesize > maxsize) && (xdrs->x_op != XDR_FREE))
    {
        return (FALSE);
    }

    /*
     * now deal with the actual bytes
     */
    switch (xdrs->x_op)
    {

        case XDR_DECODE:
            if (nodesize == 0)
            {
                return (TRUE);
            }
            if (sp == NULL)
            {
                *cpp = sp = (char *)mem_alloc(nodesize);
            }
            if (sp == NULL)
            {
                return (FALSE);
            }
            /* fall into ... */

        case XDR_ENCODE:
            return (XDR_Opaque(xdrs, sp, nodesize));

        case XDR_FREE:
            if (sp != NULL)
            {
                mem_free(sp, nodesize);
                *cpp = NULL;
            }
            return (TRUE);
        default:
        	  break;
    }
    return (FALSE);
}


/*
 * XDR a descriminated union
 * Support routine for discriminated unions.
 * You create an array of xdrdiscrim structures, terminated with
 * an entry with a null procedure pointer.  The routine gets
 * the discriminant value and then searches the array of xdrdiscrims
 * looking for that value.  It calls the procedure given in the xdrdiscrim
 * to handle the discriminant.  If there is no specific routine a default
 * routine may be called.
 * If there is no specific or default routine an error is returned.
 */
Bool XDR_Union(XDR *xdrs, Enum *dscmp, Char *unp, struct _XDRDiscrim *choices, XDRPROC_t dfault)
{
    Enum dscm;

    /*
     * we deal with the discriminator;  it's an enum
     */
    if (!XDR_Enum(xdrs, dscmp))
    {
        return (FALSE);
    }
    dscm = *dscmp;

    /*
     * search choices for a value that matches the discriminator.
     * if we find one, execute the xdr routine for that value.
     */
    for (; choices->proc != NULL_xdrproc_t; choices++)
    {
        if (choices->value == dscm)
        {
            return ((*(choices->proc))(xdrs, unp, LASTUNSIGNED));
        }
    }

    /*
     * no match - execute the default xdr routine if there is one
     */
    return ((dfault == NULL_xdrproc_t) ? FALSE :
            (*dfault)(xdrs, unp, LASTUNSIGNED));
}


/*
 * Non-portable xdr primitives.
 * Care should be taken when moving these routines to new architectures.
 */


/*
 * XDR null terminated ASCII strings
 * xdr_string deals with "C strings" - arrays of bytes that are
 * terminated by a NULL character.  The parameter cpp references a
 * pointer to storage; If the pointer is null, then the necessary
 * storage is allocated.  The last parameter is the max allowed length
 * of the string as specified by a protocol.
 */
Bool XDR_String(XDR *xdrs, String *cpp, Uint32 maxsize)
{
    Char *sp = *cpp;  /* sp is the actual string pointer */
    Uint32 size = 0;
    Uint32 nodesize = 0;

    /*
     * first deal with the length since xdr strings are counted-strings
     */
    switch (xdrs->x_op)
    {
        case XDR_FREE:
            if (sp == NULL)
            {
                return(TRUE);   /* already free */
            }
            /* fall through... */
        case XDR_ENCODE:
            size = strlen(sp);
            break;
        default:
        	  break;
    }
    if (!XDR_Uint32(xdrs, &size))
    {
        return (FALSE);
    }
    if (size > maxsize)
    {
        return (FALSE);
    }
    nodesize = size + 1;

    /*
     * now deal with the actual bytes
     */
    switch (xdrs->x_op)
    {

        case XDR_DECODE:
            if (nodesize == 0)
            {
                return (TRUE);
            }
            if (sp == NULL)
            {
                *cpp = sp = (char *)mem_alloc(nodesize);
            }
            if (sp == NULL)
            {
                return (FALSE);
            }
            sp[size] = 0;
            /* fall into ... */

        case XDR_ENCODE:
            return (XDR_Opaque(xdrs, sp, size));

        case XDR_FREE:
            mem_free(sp, nodesize);
            *cpp = NULL;
            return (TRUE);
        default:
         	  break;
    }
    return (FALSE);
}

/*
 * Wrapper for xdr_string that can be called directly from
 * routines like clnt_call
 */
Bool XDR_Wrapstring(XDR *xdrs, Wrapstring *cpp)
{
    if (XDR_String(xdrs, cpp, LASTUNSIGNED))
    {
        return (TRUE);
    }
    return (FALSE);
}



/*
 * XDR an array of arbitrary elements
 * *addrp is a pointer to the array, *sizep is the number of elements.
 * If addrp is NULL (*sizep * elsize) bytes are allocated.
 * elsize is the size (in bytes) of each element, and elproc is the
 * xdr procedure to call to handle each element of the array.
 */
Bool XDR_Array(XDR *xdrs, Array *addrp, Uint32 *sizep, Uint32 maxsize, Uint32 elsize, XDRPROC_t elproc)
{
    Uint32 i = 0;
    Array target = *addrp;
    Uint32 c = 0;  /* the actual element count */
    Bool stat = TRUE;
    Uint32 nodesize = 0;

    /* like strings, arrays are really counted arrays */
    if (!XDR_Uint32(xdrs, sizep))
    {
        return (FALSE);
    }
    c = *sizep;
    if ((c > maxsize) && (xdrs->x_op != XDR_FREE))
    {
        return (FALSE);
    }
    nodesize = c * elsize;

    /*
     * if we are deserializing, we may need to allocate an array.
     * We also save time by checking for a null array if we are freeing.
     */
    if (target == NULL)
        switch (xdrs->x_op)
        {
            case XDR_DECODE:
                if (c == 0)
                {
                    return (TRUE);
                }
                *addrp = target = mem_alloc(nodesize);
                if (target == NULL)
                {
                    return (FALSE);
                }
                Bzero(target, nodesize);
                break;

            case XDR_FREE:
                return (TRUE);
            default:
            	  break;
        }

    /*
     * now we xdr each element of array
     */
    for (i = 0; (i < c) && stat; i++)
    {
        stat = (*elproc)(xdrs, target, LASTUNSIGNED);
        target += elsize;
        /*
         *      (wdk): next line avoids implicit pointer arithmetic.
         *      target = (caddr_t)((int)target + elsize);
         */
    }

    /*
     * the array may need freeing
     */
    if (xdrs->x_op == XDR_FREE)
    {
        mem_free(*addrp, nodesize);
        *addrp = NULL;
    }
    return (stat);
}


/*
 * XDR an array of string elements.
 * addrp is a pointer to the string pointer array, *sizep is the number of
 * string elements. If *addrp is NULL (*sizep) char pointers are allocated.
 */
Bool XDR_Strarray(XDR *xdrs, Strarray *addrp, Uint32 *sizep, Uint32 maxsize)
{
    Uint32 i = 0;
    Char **target = *addrp;
    Uint32 c = 0;  /* the actual element count */
    Bool stat = TRUE;

    /* like strings, arrays are really counted arrays */
    if (!XDR_Uint32(xdrs, sizep))
    {
        return (FALSE);
    }
    c = *sizep;
    if ((c > maxsize) && (xdrs->x_op != XDR_FREE))
    {
        return (FALSE);
    }

    /*
     * if we are deserializing, we may need to allocate a pointer array.
     * We also save time by checking for a null array if we are freeing.
     */
    if (target == NULL)
        switch (xdrs->x_op)
        {
            case XDR_DECODE:
                if (c == 0)
                {
                    return (TRUE);
                }
                /* allocate and zero */
                /*Modified by tony*/
                //*addrp = target = (char**) calloc( c, sizeof(char*));
                *addrp = target = (char **) mem_alloc(c * sizeof(char *));
                memset(target, 0, c * sizeof(char *));

                if (target == NULL)
                {
                    return (FALSE);
                }
                break;

            case XDR_FREE:
                return (TRUE);
            default:
            	  break;
        }

    /*
     * now we xdr each element (string) of the array
     */
    for (i = 0; (i < c) && stat; i++)
    {
        stat = XDR_String(xdrs, target, maxsize);
        target ++;
    }

    /*
     * the array may need freeing
     */
    if (xdrs->x_op == XDR_FREE)
    {
        mem_free(*addrp, c * sizeof(char *));
        *addrp = NULL;
    }
    return (stat);
}

/*
 * xdr_vector():
 *
 * XDR a fixed length array. Unlike variable-length arrays,
 * the storage of fixed length arrays is static and unfreeable.
 * > basep: base of the array
 * > size: size of the array
 * > elemsize: size of each element
 * > xdr_elem: routine to XDR each element
 */
Bool XDR_Vector(XDR *xdrs, Char *basep, Uint32 nelem, Uint32 elemsize, XDRPROC_t xdr_elem)
{
    Uint32 i = 0;
    Char *elptr = NULL;

    elptr = basep;
    for (i = 0; i < nelem; i++)
    {
        if (!(*xdr_elem)(xdrs, elptr, LASTUNSIGNED))
        {
            return(FALSE);
        }
        elptr += elemsize;
    }
    return(TRUE);
}

/*
 * xdr_float.c, Generic XDR routines impelmentation.
 *
 * Copyright (C) 1984, Sun Microsystems, Inc.
 *
 * These are the "floating point" xdr routines used to (de)serialize
 * most common data items.  See xdr.h for more info on the interface to
 * xdr.
 */
/*!!!Fix me for ARM and MIPS arch*/
Bool XDR_Float(XDR *xdrs, Float *fp)
{


    switch (xdrs->x_op)
    {

        case XDR_ENCODE:
            return (XDR_PUTLONG(xdrs, (Long *)fp));

        case XDR_DECODE:
            return (XDR_GETLONG(xdrs, (Long *)fp));

        case XDR_FREE:
            return (TRUE);
        default:
        	  break;
    }
    return (FALSE);
}

/*!!!Fix me for ARM and MIPS arch*/
Bool XDR_Double(XDR *xdrs, Double *dp)
{
    Long *lp = NULL;

    switch (xdrs->x_op)
    {

        case XDR_ENCODE:
            lp = (Long *)dp;
#if defined(_X86_)
            return (XDR_PUTLONG(xdrs, lp + 1) && XDR_PUTLONG(xdrs, lp));
#else
            return (XDR_PUTLONG(xdrs, lp++) && XDR_PUTLONG(xdrs, lp));
#endif

        case XDR_DECODE:
            lp = (Long *)dp;
#if defined(_X86_)
            return (XDR_GETLONG(xdrs, lp + 1) && XDR_GETLONG(xdrs, lp));
#else
            return (XDR_GETLONG(xdrs, lp++) && XDR_GETLONG(xdrs, lp));
#endif
        case XDR_FREE:
            return (TRUE);
        default:
        	  break;
    }
    return (FALSE);
}


/*
 * XDR an indirect pointer
 * xdr_reference is for recursively translating a structure that is
 * referenced by a pointer inside the structure that is currently being
 * translated.  pp references a pointer to storage. If *pp is null
 * the  necessary storage is allocated.
 * size is the sizeof the referneced structure.
 * proc is the routine to handle the referenced structure.
 */
Bool XDR_Reference(XDR *xdrs, Reference *pp, Uint32 size, XDRPROC_t proc)
{
    Reference loc = *pp;
    Bool stat = FALSE;

    if (loc == NULL)
        switch (xdrs->x_op)
        {
            case XDR_FREE:
                return (TRUE);

            case XDR_DECODE:
                *pp = loc = (Reference) mem_alloc(size);
                if (loc == NULL)
                {
                    return (FALSE);
                }
                Bzero(loc, (int)size);
                break;
                
             default:
             	  break;
        }

    stat = (*proc)(xdrs, loc, LASTUNSIGNED);

    if (xdrs->x_op == XDR_FREE)
    {
        mem_free(loc, size);
        *pp = NULL;
    }
    return (stat);
}


/*
 * xdr_pointer():
 *
 * XDR a pointer to a possibly recursive data structure. This
 * differs with xdr_reference in that it can serialize/deserialiaze
 * trees correctly.
 *
 *  What's sent is actually a union:
 *
 *  union object_pointer switch (boolean b) {
 *  case TRUE: object_data data;
 *  case FALSE: void nothing;
 *  }
 *
 * > objpp: Pointer to the pointer to the object.
 * > obj_size: size of the object.
 * > xdr_obj: routine to XDR an object.
 *
 */
Bool XDR_Pointer(XDR *xdrs, Pointer *objpp, Uint32 obj_size, XDRPROC_t xdr_obj)
{

    Bool more_data = FALSE;

    more_data = (*objpp != NULL);
    if (!XDR_Bool(xdrs, &more_data))
    {
        return (FALSE);
    }
    if (!more_data)
    {
        *objpp = NULL;
        return (TRUE);
    }
    return (XDR_Reference(xdrs, objpp, obj_size, xdr_obj));
}


/*----------------------------------------------------------------
*
* Low level XDR Implementation
*
*-----------------------------------------------------------------*/
static Bool Xdrmem_getlong();
static Bool Xdrmem_putlong();
static Bool Xdrmem_getbytes();
static Bool Xdrmem_putbytes();
static Uint32   Xdrmem_getpos();
static Bool Xdrmem_setpos();
static Long    *Xdrmem_inline();
static void Xdrmem_destroy();

static struct   XDR_OPS xdrmem_ops =
{
    Xdrmem_getlong,
    Xdrmem_putlong,
    Xdrmem_getbytes,
    Xdrmem_putbytes,
    Xdrmem_getpos,
    Xdrmem_setpos,
    Xdrmem_inline,
    Xdrmem_destroy
};


static Bool
Xdrmem_getlong(XDR *xdrs, Long *lp)
{

    if ((xdrs->x_handy -= sizeof(Long)) < 0)
    {
        return (FALSE);
    }
    *lp = (Long)ntohl((Ulong)(*((Long *)(xdrs->x_private))));
    xdrs->x_private += sizeof(Long);
    return (TRUE);
}

static Bool
Xdrmem_putlong(XDR *xdrs, Long *lp)
{

    if ((xdrs->x_handy -= sizeof(Long)) < 0)
    {
        return (FALSE);
    }
    *(Long *)xdrs->x_private = (Long)htonl((Ulong)(*lp));
    xdrs->x_private += sizeof(Long);
    return (TRUE);
}

static Bool
Xdrmem_getbytes(XDR *xdrs, Char *addr, Uint32 len)
{

    if ((xdrs->x_handy -= len) < 0)
    {
        return (FALSE);
    }
    Bcopy(xdrs->x_private, addr, len);
    xdrs->x_private += len;
    return (TRUE);
}

static Bool
Xdrmem_putbytes(XDR *xdrs, Char *addr, Uint32 len)
{

    if ((xdrs->x_handy -= len) < 0)
    {
        return (FALSE);
    }
    Bcopy(addr, xdrs->x_private, len);
    xdrs->x_private += len;
    return (TRUE);
}

static Uint32
Xdrmem_getpos(XDR *xdrs)
{

    return ((Uint32)xdrs->x_private - (Uint32)xdrs->x_base);
}

static Bool
Xdrmem_setpos(XDR *xdrs, Uint32 pos)
{
    Char *newaddr = xdrs->x_base + pos;
    Char *lastaddr = xdrs->x_private + xdrs->x_handy;

    if ((Long)newaddr > (Long)lastaddr)
    {
        return (FALSE);
    }
    xdrs->x_private = newaddr;
    xdrs->x_handy = (int)lastaddr - (int)newaddr;
    return (TRUE);
}

static Long *
Xdrmem_inline(XDR *xdrs, Int32 len)
{
    Long *buf = 0;

    if (xdrs->x_handy >= len)
    {
        xdrs->x_handy -= len;
        buf = (Long *) xdrs->x_private;
        xdrs->x_private += len;
    }
    return (buf);
}

static void
Xdrmem_destroy(void)
{

}


/*
 * The procedure xdrmem_create initializes a stream descriptor for a
 * memory buffer.
 */
void XDR_Create(XDR *xdrs, Char *addr, Uint32 size, enum XDR_OP op)
{
	if(xdrs)
	{
		xdrs->x_op = op;
		xdrs->x_ops = &xdrmem_ops;
		xdrs->x_private = xdrs->x_base = addr;
		xdrs->x_handy = size;
	}
}


