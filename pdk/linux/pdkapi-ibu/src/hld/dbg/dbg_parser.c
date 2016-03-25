/*****************************************************************************
*	Copyrights(C) 2012 Acer Laboratries inc. All Rights Reserved.
*
*	FILE NAME:		adr_dbg_parser.c
*
*	DESCRIPTION:	Parse debug command and distribute to sub-module.
*
*	HISTORY:
*						Date 	 Author      Version 	  Notes
*					=========	=========	=========	===========
*					2012-09-30	 Leo.Ma      Ver 1.0	Create File.
*					2013-05-17	 Leo.Ma      Ver 1.1	Modify the arch.
*****************************************************************************/

#include <hld_cfg.h>
#include <hld/dbg/dbg_parser.h>

//#define __ADR_DBG_PARSER

#ifdef __ADR_DBG_PARSER
	#define ADR_DBG_PARSER_PRINT(fmt, args...)  \
			do { \
					ADR_DBG_PRINT(DBG, fmt, ##args); \
			} while(0)
#else
	#define ADR_DBG_PARSER_PRINT(...)
#endif

#define PARSER_INFO(fmt, args...)  \
			do { \
					ADR_DBG_PRINT("PID %d "fmt, getpid(), ##args); \
			} while(0)
#define PARSER_ERR	PARSER_INFO
			
struct list_head dbg_mod_list;	// Module list head.
struct list_head app_mod_list;	// debug module list when Dbgtool as APP.

static void *adr_dbg_realloc(void *ptr, int size)
{
	void *p;
	
	p = realloc(ptr, size);
	if (NULL == p)
	{
		PARSER_ERR("No free memory");
		return NULL;
	}

	return p;
}

static void cmd_list_reset(struct list_head* cur_cmd_list)
{
	PARSE_COMMAND_LIST *cmd_list;
	
	cmd_list = list_entry(cur_cmd_list, PARSE_COMMAND_LIST, list);
	if (NULL == cmd_list)
		return;
	
	cmd_list->arg_len = 0;
	cmd_list->arg_cnt = 0;
	cmd_list->enable = 0;
	cmd_list->arg_valid = 0;

	return;
}

static void cmd_list_reset_all(struct list_head* cmd_list_head)
{
	struct list_head* cur_cmd_list;
	struct list_head* next_cmd_list;

	list_for_each_safe(cur_cmd_list, next_cmd_list, cmd_list_head)
	{
		cmd_list_reset(cur_cmd_list);
	}
	
	return;
}

struct list_head *find_in_mod_list(const char *module)
{
	struct list_head* cur_mod_list;
	struct list_head* next_mod_list;
	DEBUG_MODULE_LIST *mod_list;

	ADR_DBG_PARSER_PRINT("[DBGPARSER] Finding module \"%s\"...\n", module);
	
	list_for_each_safe(cur_mod_list, next_mod_list, &dbg_mod_list)
	{
		mod_list = list_entry(cur_mod_list, DEBUG_MODULE_LIST, list);
		if (NULL == mod_list || NULL == mod_list->name)
		{
			ADR_DBG_PARSER_PRINT("[DBGPARSER] Invalid module list!\n");
			break;
		}
		
		if (mod_list->registered && !strcmp(module, mod_list->name))
		{
			ADR_DBG_PARSER_PRINT("[DBGPARSER] Debug module \"%s\" found.\n", module);
			break;
		}
	}

	if (&dbg_mod_list == cur_mod_list)
	{
		PARSER_INFO("Debug module \"%s\" not found!\n", module);
		return NULL;
	}
	
	return (&mod_list->command_list);
}

struct list_head *find_in_app_mod_list(const char *module)
{
	struct list_head* cur_mod_list;
	struct list_head* next_mod_list;
	DEBUG_MODULE_LIST *mod_list;

	ADR_DBG_PARSER_PRINT("[DBGPARSER] Finding module \"%s\"...\n", module);
	
	list_for_each_safe(cur_mod_list, next_mod_list, &app_mod_list)
	{
		mod_list = list_entry(cur_mod_list, DEBUG_MODULE_LIST, list);
		if (NULL == mod_list || NULL == mod_list->name)
		{
			ADR_DBG_PARSER_PRINT("[DBGPARSER] Invalid module list!\n");
			break;
		}
		
		if (mod_list->registered && !strcmp(module, mod_list->name))
		{
			ADR_DBG_PARSER_PRINT("[DBGPARSER] Debug module \"%s\" found.\n", module);
			break;
		}
	}

	if (&app_mod_list == cur_mod_list)
	{
		PARSER_INFO("Debug module \"%s\" not found!\n", module);
		return NULL;
	}
	
	return (&mod_list->command_list);
}


int cmd_list_parse_opt(struct list_head *cmd_list_head, char *option)
{
	char lg_cmd_hit, sh_cmd_hit, dbg_cmd_hit;
	struct list_head* cur_cmd_list;
	struct list_head* next_cmd_list;
	PARSE_COMMAND_LIST *cmd_list;
	DEBUG_MODULE_LIST *mod_list;
	
	if (NULL == cmd_list_head)
		return (-1);

	if (NULL == option)
	{
		PARSER_INFO("Need command option with prefix '-' or '--'!\n");
		return (-1);
	}

	mod_list = list_entry(cmd_list_head, DEBUG_MODULE_LIST, command_list);
	ADR_DBG_PARSER_PRINT("[DBGPARSER] Parsing option \"%s\" in module \"%s\"...\n", option, mod_list->name);

	dbg_cmd_hit = 0;

	list_for_each_safe(cur_cmd_list, next_cmd_list, cmd_list_head)
	{
		cmd_list = list_entry(cur_cmd_list, PARSE_COMMAND_LIST, list);
		if (NULL == cmd_list)
		{
			ADR_DBG_PARSER_PRINT("[DBGPARSER] Invalid command list!\n");
			dbg_cmd_hit = 0;
			break;
		}

		lg_cmd_hit = 0;
		sh_cmd_hit = 0;

		if ('-' == *(option + 1) && cmd_list->lg_cmd != NULL)
			lg_cmd_hit = (0 == strcmp(option + 2, cmd_list->lg_cmd)) ? 1 : 0;
		else if (cmd_list->sh_cmd != NULL)
			sh_cmd_hit = (0 == strcmp(option + 1, cmd_list->sh_cmd)) ? 1 : 0;
		
		if (lg_cmd_hit || sh_cmd_hit)
		{
			list_del(&cmd_list->list);
			list_add_tail(&cmd_list->list, cmd_list_head);
			cmd_list->enable = 1;
			cmd_list->arg_valid = 1;
			dbg_cmd_hit = 1;
			if (lg_cmd_hit)
			{
				ADR_DBG_PARSER_PRINT("[DBGPARSER] Option \"%s\" parsed correctly.\n", cmd_list->lg_cmd);
			}
			else
			{
				ADR_DBG_PARSER_PRINT("[DBGPARSER] Option \"%s\" parsed correctly.\n", cmd_list->sh_cmd);
			}
			break;
		}
		else
		{
			cmd_list_reset(cur_cmd_list);
		}
	}
		
	if (!dbg_cmd_hit)
	{
		PARSER_INFO("Invalid option \"%s\"!\n", option);
		cmd_list_reset_all(cmd_list_head);
		return (-1);
	}
	
	return (0);
}

void cmd_list_parse_arg(struct list_head *cmd_list_head, char *option, char *argument)
{
	char lg_cmd_hit, sh_cmd_hit, arg_len;
	struct list_head* cur_cmd_list;
	struct list_head* next_cmd_list;
	PARSE_COMMAND_LIST *cmd_list;
	DEBUG_MODULE_LIST *mod_list;

	if (NULL == cmd_list_head || NULL == argument)
		return;

	if (NULL == option)
	{
		PARSER_INFO("Need command option with prefix '-' or '--'!\n");
		return;
	}

	mod_list = list_entry(cmd_list_head, DEBUG_MODULE_LIST, command_list);
	ADR_DBG_PARSER_PRINT("[DBGPARSER] Parsing argument \"%s\" with option \"%s\" in module \"%s\"...\n", argument, option, mod_list->name);

	list_for_each_safe(cur_cmd_list, next_cmd_list, cmd_list_head)
	{
		cmd_list = list_entry(cur_cmd_list, PARSE_COMMAND_LIST, list);
		if (NULL == cmd_list)
		{
			ADR_DBG_PARSER_PRINT("[DBGPARSER] Invalid command list!\n");
			break;
		}
		
		lg_cmd_hit = 0;
		sh_cmd_hit = 0;

		if ('-' == *(option + 1) && cmd_list->lg_cmd != NULL)
			lg_cmd_hit = (0 == strcmp(option + 2, cmd_list->lg_cmd)) ? 1 : 0;
		else if (cmd_list->sh_cmd != NULL)
			sh_cmd_hit = (0 == strcmp(option + 1, cmd_list->sh_cmd)) ? 1 : 0;
		
		if ((lg_cmd_hit || sh_cmd_hit) && cmd_list->enable)
		{
			arg_len = strlen(argument) + 1;
			if (cmd_list->arg_len + arg_len > cmd_list->arg_buf_siz)
			{
				arg_len = 16 * (arg_len / 16 + 1);	/* ensure that arg_buf is assigned to 16 bytes */
				cmd_list->arg_buf_siz = (arg_len + cmd_list->arg_buf_siz) * sizeof(char) * 2;
				cmd_list->arg_buf = (char *)adr_dbg_realloc(cmd_list->arg_buf, cmd_list->arg_buf_siz);
				if (NULL == cmd_list->arg_buf)
				{
					return;
				}
			}
			if (cmd_list->arg_cnt + 1 > (cmd_list->p_arg_siz / (int)sizeof(char *)))
			{
				cmd_list->p_arg_siz = (4 + cmd_list->p_arg_siz) * sizeof(char *) * 2;
				cmd_list->p_arg = (char **)adr_dbg_realloc(cmd_list->p_arg, cmd_list->p_arg_siz);
				if (NULL == cmd_list->p_arg)
				{
					return;
				}
			}
			cmd_list->p_arg[cmd_list->arg_cnt++] = cmd_list->arg_buf + cmd_list->arg_len;
			while (cmd_list->arg_buf[cmd_list->arg_len++] = *argument++)
				;
			ADR_DBG_PARSER_PRINT("[DBGPARSER] Argument length:%d count:%d.\n", cmd_list->arg_len, cmd_list->arg_cnt);
#ifdef __ADR_DBG_PARSER
			int i;
			for (i = 0; i < cmd_list->arg_cnt; i++)
				ADR_DBG_PARSER_PRINT("[DBGPARSER] %s\n", cmd_list->p_arg[i]);
#endif
			break;
		}
	}

	return;
}

int cmd_list_preview(struct list_head *cmd_list_head, char *option)
{
	char argument_valid, lg_cmd_hit, sh_cmd_hit;
	struct list_head* cur_cmd_list;
	struct list_head* next_cmd_list;
	PARSE_COMMAND_LIST *cmd_list;

	if (NULL == cmd_list_head)
		return (-1);

	if (NULL == option)
	{
		PARSER_INFO("Need option with prefix '-' or '--'!\n");
		return (-1);
	}

	argument_valid = 0;

	list_for_each_safe(cur_cmd_list, next_cmd_list, cmd_list_head)
	{
		cmd_list = list_entry(cur_cmd_list, PARSE_COMMAND_LIST, list);
		if (NULL == cmd_list)
		{
			ADR_DBG_PARSER_PRINT("[DBGPARSER] Invalid command list!\n");
			argument_valid = 0;
			break;
		}

		lg_cmd_hit = 0;
		sh_cmd_hit = 0;

		if ('-' == *(option + 1) && cmd_list->lg_cmd != NULL)
			lg_cmd_hit = (0 == strcmp(option + 2, cmd_list->lg_cmd)) ? 1 : 0;
		else if (cmd_list->sh_cmd != NULL)
			sh_cmd_hit = (0 == strcmp(option + 1, cmd_list->sh_cmd)) ? 1 : 0;
		
		if ((lg_cmd_hit || sh_cmd_hit) && cmd_list->enable)
		{
			if(cmd_list->preview_func != NULL)
			{
				cmd_list->arg_valid = cmd_list->preview_func(cmd_list->arg_cnt, cmd_list->p_arg, option);
				if (cmd_list->arg_valid)
				{
					argument_valid = 1;
				}
				else
				{
					cmd_list_reset(cur_cmd_list);
					argument_valid = 0;
					break;
				}
			}
			else
				argument_valid = 1;
		}
	}

	if (!argument_valid)
	{
		ADR_DBG_PARSER_PRINT("[DBGPARSER] Invalid argument(s) with option \"%s\"!\n", option);
		cmd_list_reset_all(cmd_list_head);
		return (-1);
	}

	return (0);
}

void cmd_list_exec(struct list_head* cmd_list_head)
{
	struct list_head* cur_cmd_list;
	struct list_head* next_cmd_list;
	PARSE_COMMAND_LIST *cmd_list;

	if (NULL == cmd_list_head)
		return;

	list_for_each_safe(cur_cmd_list, next_cmd_list, cmd_list_head)
	{
		cmd_list = list_entry(cur_cmd_list, PARSE_COMMAND_LIST, list);
		if (NULL == cmd_list)
		{
			ADR_DBG_PARSER_PRINT("[DBGPARSER] Invalid command list!\n");
			break;
		}
		
		if (cmd_list->enable && cmd_list->arg_valid && cmd_list->parse_func != NULL)
		{
			cmd_list->parse_func(cmd_list->arg_cnt, cmd_list->p_arg);
			cmd_list_reset(cur_cmd_list);
		}
	}

	return;
}


struct list_head *debug_module_add(const char *mod_name, PARSE_COMMAND_LIST *cmd_list, int list_cnt)
{
	struct list_head* cur_mod_list;
	struct list_head* next_mod_list;
	DEBUG_MODULE_LIST *mod_list;
	
	if (NULL == mod_name || NULL == cmd_list || list_cnt <= 0)
		return NULL;

	list_for_each_safe(cur_mod_list, next_mod_list, &dbg_mod_list)
	{
		mod_list = list_entry(cur_mod_list, DEBUG_MODULE_LIST, list);
		if (NULL == mod_list)
		{
			ADR_DBG_PARSER_PRINT("[DBGTOOL] Invalid module list!\n");
			return NULL;
		}

		if (!strcmp(mod_list->name, mod_name))
		{
			if (mod_list->registered)
			{
				//PARSER_INFO("Debug module \"%s\" already existed!\n", mod_list->name);
				return &mod_list->command_list;
			}

			mod_list->registered = 1;
			while (list_cnt-- > 0)
			{
				INIT_LIST_HEAD(&cmd_list[list_cnt].list);
				list_add(&cmd_list[list_cnt].list, &mod_list->command_list);
			}
			//PARSER_INFO("Debug module \"%s\" registered successfully!\n", mod_list->name);
			break;
		}
	}

	if (&dbg_mod_list == cur_mod_list)
	{
		PARSER_ERR("Unsupported debug module \"%s\"!\n", mod_name);
		return NULL;
	}

	return &mod_list->command_list;
}

void debug_module_delete(const char *mod_name)
{
	struct list_head *cmd_list_head;
	struct list_head *cur_cmd_list;
	struct list_head *next_cmd_list;
	DEBUG_MODULE_LIST *mod_list;

	if (NULL == mod_name || list_empty(&dbg_mod_list))
		return;

	cmd_list_head = (struct list_head *)find_in_mod_list(mod_name);
	if (NULL == cmd_list_head)
		return;

	cmd_list_reset_all(cmd_list_head);
	list_for_each_safe(cur_cmd_list, next_cmd_list, cmd_list_head)
	{
		list_del_init(cur_cmd_list);
	}

	mod_list = list_entry(cmd_list_head, DEBUG_MODULE_LIST, command_list);
	if (mod_list != NULL)
	{
		list_del_init(&mod_list->command_list);
		mod_list->registered = 0;
	}

	//PARSER_INFO("Debug module \"%s\" unregistered successfully!\n", mod_name);

	return;
}


int dbg_cmd_parse(int argc, char** argv)
{
	char c, *opt = NULL;
	char first_argument;
	struct list_head *cmd_list;

	if (argc < 1)
	{
		//PARSER_INFO("Too few arguments!\n");
		return (-1);
	}
 
	// Find the corresponding debug module.
	if ('-' == (*argv)[0])
	{
		if (NULL == (cmd_list = find_in_mod_list("soc")))
			return (-2);
	}
	else
	{
		if (NULL == (cmd_list = find_in_mod_list(*argv)))
			return (-3);

		++argv;
		--argc;
	}

	while (argc > 0)
	{
		first_argument = 0;
		opt = NULL;
		
		// Parse option as debug command.
		if ('-' == **argv)
		{
			opt = *argv;
			if (cmd_list_parse_opt(cmd_list, opt))
				return (-3);
		}

		if (NULL == opt)
			goto NO_DBG_OPT;
		
		// Parse argument(s) as debug command parameters.
		while (--argc > 0 && 
			(('-' != (*++argv)[0]) || (isdigit((*argv)[1]))))
		{
			first_argument = 1;
			cmd_list_parse_arg(cmd_list, opt, *argv);
		}

		// Check command with no argument(s).
		if (!first_argument)
			cmd_list_parse_arg(cmd_list, opt, NULL);

		// Preview arguments and check validity.
		if (cmd_list_preview(cmd_list, opt))
			return (-5);
	}

	if (NULL == opt)
	{
NO_DBG_OPT:
		PARSER_INFO("Need option(s) with prefix '-'\n");
		return (-4);
	}

	cmd_list_exec(cmd_list);
	
	return (0);
}
