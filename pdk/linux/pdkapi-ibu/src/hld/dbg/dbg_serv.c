/*****************************************************************************
*	Copyrights(C) 2012 Acer Laboratries inc. All Rights Reserved.
*
*	FILE NAME:		adr_dbg_serv.c
*
*	DESCRIPTION:	Create a task and a message queue to receive debug command 
*					from client.
*
*	HISTORY:
*						Date 	 Author      Version 	  Notes
*					=========	=========	=========	===========
*					2012-09-30	 Leo.Ma      Ver 1.0	Create File
*****************************************************************************/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <osal/osal_task.h>
#include <hld/dbg/dbg.h>
#include <hld/dbg/dbg_parser.h>
#include <version.h>
#include <hld_cfg.h>
char g_ali_serv_process_name[16];

//#define __ADR_DBG_SERV

#ifdef __ADR_DBG_SERV
	#define ADR_DBG_SERV_PRINT(fmt, args...)  \
			do { \
					ADR_DBG_PRINT(fmt, ##args); \
			} while(0)
#else
	#define ADR_DBG_SERV_PRINT(...)	do{}while(0)
#endif

#define SERVER_INFO(fmt, args...)  \
			do { \
					ADR_DBG_PRINT("[ PID %d %s ]"fmt, getpid(), g_ali_serv_process_name, ##args); \
			} while(0)

#define ADR_DBG_SERV_INFO(fmt, args...)  \
						do { \
								ADR_DBG_PRINT(fmt, ##args); \
						} while(0)

			
static int dbg_msgq_id;
static ADR_DBG_INFO dbg_msg_cli;

static int dbg_init = 0;

static INT32 g_ali_dbg_fifo_fd = -1;
static RET_CODE dbg_msgq_init(void)
{
	key_t key;
	INT32 ret = -1;
	
	
	ret = access(DBG_FIFO_DIR, F_OK);
	if (0 != ret)  
	{		
		ADR_DBG_SERV_PRINT("[ %s %d ], ret = %d, \n", 
			__FUNCTION__, __LINE__, ret);		 
		ret = mkdir(DBG_FIFO_DIR, 0777);
		ret |= chmod(DBG_FIFO_DIR, DBG_FIFO_PERMS);	
		if (0 != ret)
		{
			ADR_DBG_SERV_PRINT("[ %s %d ], error, ret = %d\n", 
				__FUNCTION__, __LINE__, ret);
			perror("");	
			return RET_FAILURE;
		}
	}	
	
	ret = access(DBG_FIFO_USR_TO_SERVER_PATH, F_OK);	
	if (0 != ret)  
	{		
		ret = mkfifo(DBG_FIFO_USR_TO_SERVER_PATH, DBG_FIFO_PERMS);	
		ret |= chmod(DBG_FIFO_USR_TO_SERVER_PATH, DBG_FIFO_PERMS);		
	    if(ret < 0)
		{  
	        ADR_DBG_SERV_PRINT("[ %s %d ], error, ret = %d\n", 
	        	__FUNCTION__, __LINE__, ret);
	        perror("");	     
	        return RET_FAILURE;
	    } 
    }   
    
    g_ali_dbg_fifo_fd = open(DBG_FIFO_USR_TO_SERVER_PATH, O_RDWR);
    if (g_ali_dbg_fifo_fd < 0)
    {
    	ADR_DBG_SERV_PRINT("[ %s %d ], error, g_ali_dbg_fifo_fd = %d.\n", 
    		__FUNCTION__, __LINE__, g_ali_dbg_fifo_fd);
        perror("");     
        return RET_FAILURE;
    }
   
	
	return RET_SUCCESS;
}


size_t get_executable_path( char* process_dir,char* process_name, size_t len)
{
    char* path_end;
    if(readlink("/proc/self/exe", process_dir,len) <=0)
            return -1;
    path_end = strrchr(process_dir,  '/');
    if(path_end == NULL)
            return -1;
    ++path_end;
    strcpy(process_name, path_end);
    *path_end = '\0';
    return (size_t)(path_end - process_dir);
}


static RET_CODE dbg_msgq_recv(void)
{
	int ret;
	  	

	memset(&dbg_msg_cli, 0, sizeof(dbg_msg_cli));
	
	//ADR_DBG_SERV_PRINT("[ %s ] Waiting for msg receiving...\n", g_ali_serv_process_name);

	ret = read(g_ali_dbg_fifo_fd, &dbg_msg_cli, DBG_FIFO_LEN);
	if (ret < 0)
	{
		SERVER_INFO("Msg received fail! ret %d\n", ret);

		return RET_FAILURE;
	}	
	ADR_DBG_SERV_INFO("[ PID %d %s ], cnt=%d, %s\n", 
		getpid(), g_ali_serv_process_name, dbg_msg_cli.arg_cnt, dbg_msg_cli.arg_val);
	
	return RET_SUCCESS;
}

static void dbg_task(UINT32 param1, UINT32 param2)
{
	int ret = 0, i = 0, j = 0, alloc_nr = 0;
	char **pp_arg = NULL;

	for (;;)
	{
		osal_task_sleep(100);
	
		if (dbg_msgq_recv() == RET_FAILURE)
		{
			break;
		}

		if (dbg_msg_cli.arg_cnt > alloc_nr)
		{
			pp_arg = realloc(pp_arg, 2 * dbg_msg_cli.arg_cnt * sizeof(*pp_arg));
			if (NULL == pp_arg)
			{
				SERVER_INFO("Out of memory!\n");
				continue;
			}
			alloc_nr = dbg_msg_cli.arg_cnt * 2;
		}

		//ADR_DBG_SERV_PRINT("arg cnt %d\n", dbg_msg_cli.arg_cnt);

		ret = strlen(dbg_msg_cli.arg_val);	
		if (ret > DBG_MSG_MAX_SIZ)
		{
			continue;
		}
		for (i = 0, j = 0; (i < ret); i++)
		{
			if ((i < sizeof(dbg_msg_cli.arg_val)) && (' ' == dbg_msg_cli.arg_val[i]))
			{
				dbg_msg_cli.arg_val[i] = '\0';	
				continue;
			}
			
			if (0 == j) 
			{
				if (NULL == pp_arg)
				{
					return;
				}
				pp_arg[j++] = &dbg_msg_cli.arg_val[i];
			}
			else if ((i >= 1) && ('\0' == dbg_msg_cli.arg_val[i - 1])
				&& (' ' != dbg_msg_cli.arg_val[i]))
			{
				pp_arg[j++] = &dbg_msg_cli.arg_val[i];
			}
		}

		for (i = 0; i < dbg_msg_cli.arg_cnt; i++)
		{
			if (pp_arg[i] != NULL)
			{
				ADR_DBG_SERV_PRINT("[DBGSERV] Msg received as \"%s\"\n", pp_arg[i]);
			}
		}

		ret = dbg_cmd_parse(dbg_msg_cli.arg_cnt, pp_arg);
		if (ret < 0)
		{
			ADR_DBG_SERV_PRINT("[DBGSERV] Debug command parsed fail r=%d\n", ret);
		}
	}
	free(pp_arg);

	return;
}

static RET_CODE dbg_task_init(void)
{
    ID              task_id;
    OSAL_T_CTSK     t_ctsk;
	
	// Create debug task.
    memset(&t_ctsk, 0, sizeof(OSAL_T_CTSK ));

    t_ctsk.stksz    = 0x1000;
    t_ctsk.quantum  = 10;
    t_ctsk.itskpri  = OSAL_PRI_NORMAL;
    t_ctsk.name[0]  = 'D';
    t_ctsk.name[1]  = 'B';
    t_ctsk.name[2]  = 'G';
    t_ctsk.task = (FP)dbg_task;
    task_id = osal_task_create(&t_ctsk);
    if(OSAL_INVALID_ID == task_id)
    {
        SERVER_INFO("Create debug task fail\n");
        return RET_FAILURE;
    }

    return RET_SUCCESS;
}




RET_CODE adr_dbg_init(void)
{
	char path[64];
   	char adrdbg_name[] = "adrdbg";  	
   	
	
	if(dbg_init == 1)
		return RET_SUCCESS;

	memset(path, 0x00, sizeof(path));
	memset(g_ali_serv_process_name, 0x00, sizeof(g_ali_serv_process_name));
	get_executable_path(path, g_ali_serv_process_name, sizeof(path));  	
	
	if (0 != strcmp(adrdbg_name, g_ali_serv_process_name))
	{
		SERVER_INFO("%s, Adr Hld Library version %s \n", g_ali_serv_process_name, MAIN_VER);	
	}	

	dbg_msgq_init();
	
	if (dbg_task_init())
		return RET_FAILURE;

	dbg_mod_list_init();

	dbg_init = 1;	
	return RET_SUCCESS;
}

