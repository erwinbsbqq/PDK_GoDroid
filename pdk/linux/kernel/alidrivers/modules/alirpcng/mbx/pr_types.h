/****************************************************************************
 *
 *  ALi (shenzhen) Corporation, All Rights Reserved. 2012 Copyright (C)
 *
 *  File: pr_types.h
 *
 *  Description: Portable runtime unified types definition header file.
 *
 *  History:
 *      Date            Author          Version         Comment
 *      ====            ======          =======         =======
 *  1.  2013.01.25      David.L         0.1.000         Initial
 ****************************************************************************/

#ifndef  __PR_TYPES_H__
#define  __PR_TYPES_H__

typedef int                 Int32;
typedef long long           Int64;
typedef unsigned char       Uint8;
typedef char                Int8;
typedef unsigned short      Uint16;
typedef short               Int16;
typedef unsigned int        Uint32;
typedef unsigned long long  Uint64;
typedef signed int          Char32;
typedef long                Long;
typedef unsigned long       Ulong;
typedef unsigned long       Size_t;
typedef long                SSize_t;

#ifndef Bool
typedef Long                Bool;
#endif

#ifndef False
#define False               (0)
#endif

#ifndef True
#define True                (!False)
#endif

#endif // __PR_TYPES_H__

