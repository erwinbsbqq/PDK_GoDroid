/*******************************************************************************
 *
 * FILE NAME          : MaxLinearDataTypes.h
 * 
 * AUTHOR             : Brenndon Lee
 * DATE CREATED       : Jul/31, 2006
 *
 * DESCRIPTION        : This file contains MaxLinear-defined data types.
 *                      Instead of using ANSI C data type directly in source code
 *                      All module should include this header file.
 *                      And conditional compilation switch is also declared
 *                      here.
 *
 *******************************************************************************
 *                Copyright (c) 2006, MaxLinear, Inc.
 ******************************************************************************/

#ifndef __MAXLINEAR_DATA_TYPES_H__
#define __MAXLINEAR_DATA_TYPES_H__




//#include <types.h>
//#include "ALi_nim_mxl241.h"
/******************************************************************************
    Include Header Files
    (No absolute paths - paths handled by make file)
******************************************************************************/

/******************************************************************************
    Macros
******************************************************************************/

/******************************************************************************
    User-Defined Types (Typedefs)
******************************************************************************/
//typedef unsigned char        UINT8;
//typedef unsigned short       UINT16;
//typedef unsigned int         UINT32;
//typedef unsigned long long   UINT64;
//typedef char                 SINT8;
//typedef short                SINT16;
//typedef int                  SINT32;
//typedef long long            SINT64;
//typedef float                REAL32;
//typedef unsigned int        REAL32;
//typedef double                REAL32;
//typedef double               REAL64;


typedef unsigned int           		REAL32;

typedef char                 		SINT8;
typedef short                		SINT16;
typedef int                  		SINT32;
typedef long long               	SINT64;


typedef UINT64                  	REAL64;


typedef enum 
{
  MXL_TRUE = 0,
  MXL_FALSE = 1,  
} MXL_STATUS;

typedef enum
{
  MXL_DISABLE = 0,
  MXL_ENABLE,

  MXL_NO_FREEZE = 0,
  MXL_FREEZE,

  MXL_UNLOCKED = 0,
  MXL_LOCKED
} MXL_BOOL;


/******************************************************************************
    Global Variable Declarations
******************************************************************************/

/******************************************************************************
    Prototypes
******************************************************************************/

#endif /* __MAXLINEAR_DATA_TYPES_H__ */

