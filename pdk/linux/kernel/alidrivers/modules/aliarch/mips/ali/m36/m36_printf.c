/****************************************************************************(I)(S)
 *  (C)
 *  ALi (Shanghai) Corporation. 2010 Copyright (C)
 *  (C)
 *  File: m36_printf.c
 *  (I)
 *  Description:
 *  (S)
 *  History:(M)
 *      	Date        			Author         	Comment
 *      	====        			======		=======
 * 0.		2010.06.03			Sam			Create

*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version
* 2 of the License, or (at your option) any later version.
*

 ****************************************************************************/
 
#define DIAG_PRINTF_USE_VARARG

#ifdef DIAG_PRINTF_USE_VARARG

// by Charlie, for system call
#include <linux/linkage.h>

#include "stdarg.h"
#include "m36_printf.h"

#endif
#define NULL 0
#define true 1
#define false 0
#define HAL_soc_WRITE_CHAR(c) hal_soc_write_char(c);

#ifndef R4K_MERGING_PSEMU

void soc_write_num(
    unsigned long  n,              /* number to write              */
    unsigned char base,           /* radix to write to            */
    unsigned char sign,           /* sign, '-' if -ve, '+' if +ve */
    int    pfzero,         /* prefix with zero ?           */
    unsigned char width           /* min width of number          */
    );


/*----------------------------------------------------------------------*/
/* Write single char to output                                          */

//#define PREFIX() HAL_DIAG_WRITE_CHAR(0x0F)
#define SWAPIOBASE 0xb0008204
#define STATIOBASE 0xb0008208

void write_log(unsigned long c,unsigned long addr)
{
	while(*(volatile unsigned long *)(STATIOBASE)&0x1);
	*(volatile unsigned long *)(SWAPIOBASE)= c;
	*(volatile unsigned long *)(STATIOBASE)= (unsigned long)(0xf003+(addr<<16));
	while(*(volatile unsigned long *)(STATIOBASE)&0x1);
}


void hal_soc_write_char(char c)
{
	#define ICE_PRINT_PORT	0xff20003c
	*(volatile unsigned long *)(ICE_PRINT_PORT) = (unsigned long)((c&0xff)|0xde000000);
	*((volatile unsigned char *)(0xB8018300)) = c; //write_data('c');
#if 0
	while((*(volatile unsigned long *)(STATIOBASE))&0x1);
	*(volatile unsigned long *)(SWAPIOBASE)= (unsigned long) c;
#ifdef Message_BOX	
	*(volatile unsigned long *)(STATIOBASE)= (unsigned long)(0x1ffe1003);
#else
	*(volatile unsigned long *)(STATIOBASE)= (unsigned long)(0x1fff1003);
#endif	
	while((*(volatile unsigned long *)(STATIOBASE))&0x1);
#endif
}


void soc_write_char(char c)
{    
    /* Translate LF into CRLF */
    
    if( c == '\n' )
    {
        HAL_soc_WRITE_CHAR('\r');        
    }

    HAL_soc_WRITE_CHAR(c);
}

/*----------------------------------------------------------------------*/
/* Write zero terminated string                                         */
  
void soc_write_string(const char *psz)
{
#define UART_BASE  0xB8018300
#define UTBR	   0	
#define ULSR	   5	

	while( *psz )
	{
		/* for s3602 erom dbg*/

		volatile unsigned char status;
		while(1){
			status = *((volatile unsigned char *)(UART_BASE+ULSR));
			if(status & 0x40)
				break;
		}
		*((volatile unsigned char *)(UART_BASE+UTBR)) = *psz++;
	}

    // while( *psz ) soc_write_char( *psz++ );
/*
	int i;
	for(i=0; i < 10; i++)
	{
		if(*psz)
			*((volatile unsigned int *)(0xB8018300)) = *psz++; //write_data('c');
	}
			//soc_write_char(*psz++);
*/
}

/*----------------------------------------------------------------------*/
/* Write decimal value                                                  */

void soc_write_dec( long n)
{
    unsigned char sign;

    if( n < 0 ) n = -n, sign = '-';
    else sign = '+';
    
    soc_write_num( n, 10, sign, false, 0);
}

/*----------------------------------------------------------------------*/
/* Write hexadecimal value                                              */

void soc_write_hex( unsigned long n)
{
    soc_write_num( n, 16, '+', false, 0);
}    

/*----------------------------------------------------------------------*/
/* Generic number writing function                                      */
/* The parameters determine what radix is used, the signed-ness of the  */
/* number, its minimum width and whether it is zero or space filled on  */
/* the left.                                                            */

void soc_write_num(
    unsigned long  n,              /* number to write              */
    unsigned char base,           /* radix to write to            */
    unsigned char sign,           /* sign, '-' if -ve, '+' if +ve */
    int    pfzero,         /* prefix with zero ?           */
    unsigned char width           /* min width of number          */
    )
{
    char buf[16];
    char bpos;
    char bufinit = pfzero?'0':' ';
    char *digits = "0123456789ABCDEF";

    /* init buffer to padding char: space or zero */
    for( bpos = 0; bpos < (char)sizeof(buf); bpos++ ) buf[(unsigned char)bpos] = bufinit;

    /* Set pos to start */
    bpos = 0;

    /* construct digits into buffer in reverse order */
    if( n == 0 ) buf[(unsigned char)bpos++] = '0';
    else while( n != 0 )
    {
        unsigned char d = n % base;
        buf[(unsigned char)bpos++] = digits[d];
        n /= base;
    }

    /* set pos to width if less. */
    if( (char)width > bpos ) bpos = width;

    /* set sign if negative. */
    if( sign == '-' )
    {
        if( buf[bpos-1] == bufinit ) bpos--;
        buf[(unsigned char)bpos] = sign;
    }
    else bpos--;

    /* Now write it out in correct order. */
    while( bpos >= 0 )
        soc_write_char(buf[(unsigned char)bpos--]);
}

/*----------------------------------------------------------------------*/
/* perform some simple sanity checks on a string to ensure that it      */
/* consists of printable characters and is of reasonable length.        */

static int soc_check_string( const char *str )
{
    int result = true;
    const char *s;

    if( str == NULL ) return false;
    
    for( s = str ; result && *s ; s++ )
    {
        char c = *s;

        /* Check for a reasonable length string. */
        
        if( s-str > 256 ) result = false;

        /* We only really support CR and NL at present. If we want to
         * use tabs or other special chars, this test will have to be
         * expanded.
         */
        
        if( c == '\n' || c == '\r' )
            continue;

        /* Check for printable chars. This assumes ASCII */
        
//        if( c < ' ' || c > '~' )
//            result = false;

    }

    return result;
}

/*----------------------------------------------------------------------*/

void soc_vprintf( const char *fmt,va_list args)
{
#ifndef WIN32
    if( !soc_check_string(fmt) )
    {
#if 0
        int i;
#endif
        soc_write_string("<Bad format string: ");
        soc_write_hex((unsigned long)fmt);
        soc_write_string(" :");
#if 0	//args typs is void * , args[i] ???
        for( i = 0; i < 8; i++ )
        {
            soc_write_char(' ');
            soc_write_hex(args[i]);
        }
#endif
        soc_write_string(">\n");
        return;
    }
    
    while( *fmt != '\0' )
    {
        char c = *fmt;

        if( c != '%' ) soc_write_char( c );
        else
        {
            int pfzero = false;
            char width = 0;
            char sign = '+';
                        
            c = *++fmt;
                        
            if( c == '0' ) pfzero = true;

            while( '0' <= c && c <= '9' )
            {
                width = width*10 + c - '0';
                c = *++fmt;
            }

            switch( c )
            {
            case 'd':
            case 'D':
            {
                long val = va_arg(args, long);
                if( val < 0 ) val = -val, sign = '-';
                soc_write_num(val, 10, sign, pfzero, width);
                break;
            }

            case 'x':
            case 'X':
            {
                unsigned long val = va_arg(args, long);
                soc_write_num(val, 16, sign, pfzero, width);
                break;
            }

            case 'c':
            case 'C':
            {
                char ch = (char)(*args++);
                soc_write_char(ch);
                break;
            }

            case 's':
            case 'S':
            {
//                char *s = (char *)(*args++);
                char *s = (char *)(*((unsigned long*)(args++)));
                long len = 0;
                long pre = 0, post = 0;

                if( s == NULL ) s = "<null>";
                else if( !soc_check_string(s) )
                {
                    soc_write_string("<Not a string: 0x");
                    soc_write_hex((unsigned long)s);
                    s = ">";
                    if( width > 25 ) width -= 25;
                    pfzero = false;
                    /* Drop through to print the closing ">" */
                    /* and pad to the required length.       */
                }
                
                while( s[len] != 0 ) len++;
                if( width && len > width ) len = width;

                if( pfzero ) pre = width-len;
                else post = width-len;

                while( pre-- > 0 ) soc_write_char(' ');

                while( *s != '\0' && len-- != 0)
                    soc_write_char(*s++);

                while( post-- > 0 ) soc_write_char(' ');
                                
                break;
            }

            case 'b':
            case 'B':
            {
                unsigned long val = (unsigned long)(*args++);
                unsigned long i;
                if( width == 0 ) width = 32;

                for( i = width-1; i >= 0; i-- )
                    soc_write_char( (val&(1<<i))?'1':'.' );
                                
                break;
            }
            case '%':
                soc_write_char('%');
                break;

            default:
                soc_write_char('%');
                soc_write_char(c);
                break;
            }
        }

        fmt++;
    }   
	soc_write_char('\0');
#endif
}

/*-----------------------------------------------------------------------*/
/* Formatted diagnostic output.                                          */

//#ifdef soc_PRINTF_USE_VARARG
#ifdef	DIAG_PRINTF_USE_VARARG

void soc_printf(const char *fmt, ... )
{
//	dsio_printf(fmt);
#if 1   
//    unsigned long args[8];

    va_list a;

    va_start(a, fmt);

    /* Move all of the arguments into simple scalars. This avoids
     * having to use varargs to define soc_vprintf().
     */
    
/*    args[0] = va_arg(a,unsigned long);
    args[1] = va_arg(a,unsigned long);
    args[2] = va_arg(a,unsigned long);
    args[3] = va_arg(a,unsigned long);
    args[4] = va_arg(a,unsigned long);
    args[5] = va_arg(a,unsigned long);
    args[6] = va_arg(a,unsigned long);
    args[7] = va_arg(a,unsigned long);
*/
  
    soc_vprintf( fmt, a);
    va_end(a);
#endif
}


#else

/* The prototype for soc_printf in soc.h is defined using K&R syntax   */
/* to allow us to use a variable number of arguments in the call without */
/* using ellipses, which would require use of varargs stuff. If we ever  */
/* need to support arguments that are not simple words, we may need to   */
/* use varargs.                                                          */
/* For the actual implementation, a normal ANSI C prototype is           */
/* acceptable.                                                            */

void soc_printf(const char *fmt, unsigned long a1, unsigned long a2,
                         unsigned long a3, unsigned long a4,
                         unsigned long a5, unsigned long a6,
                         unsigned long a7, unsigned long a8)
{
    
    unsigned long args[8];

    args[0] = a1;
    args[1] = a2;
    args[2] = a3;
    args[3] = a4;
    args[4] = a5;
    args[5] = a6;
    args[6] = a7;
    args[7] = a8;
    
    soc_vprintf( fmt, args);
}

#endif



#define HAL_box_WRITE_CHAR(c) hal_box_write_char(c);
/*----------------------------------------------------------------------*/

void box_write_num(
    unsigned long  n,              /* number to write              */
    unsigned char base,           /* radix to write to            */
    unsigned char sign,           /* sign, '-' if -ve, '+' if +ve */
    int    pfzero,         /* prefix with zero ?           */
    unsigned char width           /* min width of number          */
    );


/*----------------------------------------------------------------------*/
/* Write single char to output                                          */

//#define PREFIX() HAL_DIAG_WRITE_CHAR(0x0F)


void hal_box_write_char(char c)
{
	while((*(volatile unsigned long *)(STATIOBASE))&0x1);
	*(volatile unsigned long *)(SWAPIOBASE)= (unsigned long) c;

	*(volatile unsigned long *)(STATIOBASE)= (unsigned long)(0x1ffe1003);
	while((*(volatile unsigned long *)(STATIOBASE))&0x1);
}


void box_write_char(char c)
{    
    /* Translate LF into CRLF */
    
    if( c == '\n' )
    {
        HAL_box_WRITE_CHAR('\r');        
    }

    HAL_box_WRITE_CHAR(c);
}

/*----------------------------------------------------------------------*/
/* Write zero terminated string                                         */
  
void box_write_string(const char *psz)
{
    while( *psz ) box_write_char( *psz++ );
}

/*----------------------------------------------------------------------*/
/* Write decimal value                                                  */

void box_write_dec( long n)
{
    unsigned char sign;

    if( n < 0 ) n = -n, sign = '-';
    else sign = '+';
    
    box_write_num( n, 10, sign, false, 0);
}

/*----------------------------------------------------------------------*/
/* Write hexadecimal value                                              */

void box_write_hex( unsigned long n)
{
    box_write_num( n, 16, '+', false, 0);
}    

/*----------------------------------------------------------------------*/
/* Generic number writing function                                      */
/* The parameters determine what radix is used, the signed-ness of the  */
/* number, its minimum width and whether it is zero or space filled on  */
/* the left.                                                            */

void box_write_num(
    unsigned long  n,              /* number to write              */
    unsigned char base,           /* radix to write to            */
    unsigned char sign,           /* sign, '-' if -ve, '+' if +ve */
    int    pfzero,         /* prefix with zero ?           */
    unsigned char width           /* min width of number          */
    )
{
    char buf[16];
    char bpos;
    char bufinit = pfzero?'0':' ';
    char *digits = "0123456789ABCDEF";

    /* init buffer to padding char: space or zero */
    for( bpos = 0; bpos < (char)sizeof(buf); bpos++ ) buf[(unsigned char)bpos] = bufinit;

    /* Set pos to start */
    bpos = 0;

    /* construct digits into buffer in reverse order */
    if( n == 0 ) buf[(unsigned char)bpos++] = '0';
    else while( n != 0 )
    {
        unsigned char d = n % base;
        buf[(unsigned char)bpos++] = digits[d];
        n /= base;
    }

    /* set pos to width if less. */
    if( (char)width > bpos ) bpos = width;

    /* set sign if negative. */
    if( sign == '-' )
    {
        if( buf[bpos-1] == bufinit ) bpos--;
        buf[(unsigned char)bpos] = sign;
    }
    else bpos--;

    /* Now write it out in correct order. */
    while( bpos >= 0 )
        box_write_char(buf[(unsigned char)bpos--]);
}

/*----------------------------------------------------------------------*/
/* perform some simple sanity checks on a string to ensure that it      */
/* consists of printable characters and is of reasonable length.        */

static int box_check_string( const char *str )
{
    int result = true;
    const char *s;

    if( str == NULL ) return false;
    
    for( s = str ; result && *s ; s++ )
    {
        char c = *s;

        /* Check for a reasonable length string. */
        
        if( s-str > 256 ) result = false;

        /* We only really support CR and NL at present. If we want to
         * use tabs or other special chars, this test will have to be
         * expanded.
         */
        
        if( c == '\n' || c == '\r' )
            continue;

        /* Check for printable chars. This assumes ASCII */
        
        if( c < ' ' || c > '~' )
            result = false;

    }

    return result;
}

/*----------------------------------------------------------------------*/

void box_vprintf( const char *fmt, unsigned long *args)
{
    if( !box_check_string(fmt) )
    {
        int i;
        box_write_string("<Bad format string: ");
        box_write_hex((unsigned long)fmt);
        box_write_string(" :");
        for( i = 0; i < 8; i++ )
        {
            box_write_char(' ');
            box_write_hex(args[i]);
        }
        box_write_string(">\n");
        return;
    }
    
    while( *fmt != '\0' )
    {
        char c = *fmt;

        if( c != '%' ) box_write_char( c );
        else
        {
            int pfzero = false;
            char width = 0;
            char sign = '+';
                        
            c = *++fmt;
                        
            if( c == '0' ) pfzero = true;

            while( '0' <= c && c <= '9' )
            {
                width = width*10 + c - '0';
                c = *++fmt;
            }

            switch( c )
            {
            case 'd':
            case 'D':
            {
                long val = (long)(*args++);
                if( val < 0 ) val = -val, sign = '-';
                box_write_num(val, 10, sign, pfzero, width);
                break;
            }

            case 'x':
            case 'X':
            {
                unsigned long val = (long)(*args++);
                box_write_num(val, 16, sign, pfzero, width);
                break;
            }

            case 'c':
            case 'C':
            {
                char ch = (char)(*args++);
                box_write_char(ch);
                break;
            }

            case 's':
            case 'S':
            {
                char *s = (char *)(*args++);
                long len = 0;
                long pre = 0, post = 0;

                if( s == NULL ) s = "<null>";
                else if( !box_check_string(s) )
                {
                    box_write_string("<Not a string: 0x");
                    box_write_hex((unsigned long)s);
                    s = ">";
                    if( width > 25 ) width -= 25;
                    pfzero = false;
                    /* Drop through to print the closing ">" */
                    /* and pad to the required length.       */
                }
                
                while( s[len] != 0 ) len++;
                
                if( width && len > width ) len = width;

                if( pfzero ) pre = width-len;
                else post = width-len;

                while( pre-- > 0 ) box_write_char(' ');

                while( *s != '\0' && len-- != 0)
                    box_write_char(*s++);

                while( post-- > 0 ) box_write_char(' ');
                                
                break;
            }

            case 'b':
            case 'B':
            {
                unsigned long val = (unsigned long)(*args++);
                unsigned long i;
                if( width == 0 ) width = 32;

                for( i = width-1; i >= 0; i-- )
                    box_write_char( (val&(1<<i))?'1':'.' );
                                
                break;
            }

            case '%':
                box_write_char('%');
                break;

            default:
                box_write_char('%');
                box_write_char(c);
                break;
            }
        }

        fmt++;
    }   
	box_write_char('\0');
    
}

/*-----------------------------------------------------------------------*/
/* Formatted diagnostic output.                                          */

//#ifdef box_PRINTF_USE_VARARG
#ifdef	DIAG_PRINTF_USE_VARARG

void soc_MessageBox(const char *fmt, ... )
{

    unsigned long args[8];

    va_list a;

    va_start(a, fmt);

    /* Move all of the arguments into simple scalars. This avoids
     * having to use varargs to define box_vprintf().
     */
    
    args[0] = va_arg(a,unsigned long);
    args[1] = va_arg(a,unsigned long);
    args[2] = va_arg(a,unsigned long);
    args[3] = va_arg(a,unsigned long);
    args[4] = va_arg(a,unsigned long);
    args[5] = va_arg(a,unsigned long);
    args[6] = va_arg(a,unsigned long);
    args[7] = va_arg(a,unsigned long);

    va_end(a);
    
    box_vprintf( fmt, args);
}

#else

/* The prototype for box_printf in box.h is defined using K&R syntax   */
/* to allow us to use a variable number of arguments in the call without */
/* using ellipses, which would require use of varargs stuff. If we ever  */
/* need to support arguments that are not simple words, we may need to   */
/* use varargs.                                                          */
/* For the actual implementation, a normal ANSI C prototype is           */
/* acceptable.                                                            */

void soc_MessageBox(const char *fmt, unsigned long a1, unsigned long a2,
                         unsigned long a3, unsigned long a4,
                         unsigned long a5, unsigned long a6,
                         unsigned long a7, unsigned long a8)
{
    
    unsigned long args[8];

    args[0] = a1;
    args[1] = a2;
    args[2] = a3;
    args[3] = a4;
    args[4] = a5;
    args[5] = a6;
    args[6] = a7;
    args[7] = a8;
    
    box_vprintf( fmt, args);
}

#endif

void exception_log(unsigned long cause,unsigned long epc,unsigned long badaddr)
{
	soc_printf("cause=%08x",cause); 
	soc_printf("epc=%08x",epc);
	soc_printf("badad=%08x",badaddr);
}

#endif	//R4K_MERGING_PSEMU

void wj_assert(int type, const char * file, int line, const char * exp)
{
		soc_printf("assertion %s ",exp);
		soc_printf("failed: file %s, ",file);
		soc_printf("line %d\n",line);
		while(1);
}

asmlinkage long sys_printk(char * str){
	extern int printk(const char *fmt, ...);
	// soc_write_string("Hello world!\n");
	printk("Charlie: Syscall from sys_printk, input str: %s\n", str);
	return 0;
}


