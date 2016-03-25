#ifndef __TYPES_H__
#define __TYPES_H__

typedef int RET_CODE;

/* Return value of function calls */
#define SUCCESS         0       /* Success return */
#define FAIL            1       /* Fail return */
#define ERR_NO_MEM      -1      /* Not enough memory error */
#define ERR_LONG_PACK   -2      /* Package too long */
#define ERR_RUNT_PACK   -3      /* Package too short */
#define ERR_TX_BUSY     -4      /* TX descriptor full */
#define ERR_DEV_ERROR   -5      /* Device work status error */
#define ERR_DEV_CLASH   -6      /* Device clash for same device in queue */
#define ERR_QUEUE_FULL  -7      /* Queue node count reached the max. val*/
#define ERR_NO_DEV      -8      /* Device not exist on PCI */
#define ERR_FAILURE     -9      /* Common error, operation not success */

typedef char			INT8;
typedef unsigned char	UINT8;
typedef short			INT16;
typedef unsigned short	UINT16;
typedef long			INT32;
typedef unsigned long	UINT32;
typedef unsigned long long UINT64;
typedef long long          INT64;
typedef int				BOOL;

#ifndef FALSE
#define	FALSE			(0)
#endif
#ifndef	TRUE
#define	TRUE			(!FALSE)
#endif

#endif

