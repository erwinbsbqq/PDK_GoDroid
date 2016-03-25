/*
 * =====================================================================================
 *
 *       Filename:  unified_bsp_desc.c
 *    Description:  API for type '*_desc' operation
 *        Version:  1.0
 *        Created:  05/09/2013 04:09:54 PM
 *
 * =====================================================================================
 */

#include <mach/unified_bsp_board_attr.h>
#include <mach/unified_bsp_desc.h>
#include <linux/module.h>


unsigned long __G_MM_SHARED_MEM_TOP_ADDR;
EXPORT_SYMBOL(__G_MM_SHARED_MEM_TOP_ADDR);

//extern board_desc_t g_board_desc;
extern init_cmd_t *g_board_init_cmd_tbl;

//extern init_cmd_desc_t g_init_cmd_desc;

init_cmd_desc_t * get_init_cmd_desc(void)
{
	//return &g_init_cmd_desc;
}
EXPORT_SYMBOL(get_init_cmd_desc);

void board_desc_init(board_desc_t *p_board_desc)
{
	board_attr_t *p_board_attr = (board_attr_t *)&p_board_desc->board_attr;
	
	__G_MM_SHARED_MEM_TOP_ADDR = p_board_attr->shared_mem_attr.shared_mem_end_addr;
}

void board_porting_init(void)
{
	//board_desc_init(&g_board_desc);
	return NULL;
}

board_attr_t * get_board_attr(void)
{
	//return &g_board_desc.board_attr;
	return NULL;
}
EXPORT_SYMBOL(get_board_attr);

chip_desc_t * get_chip_desc(void)
{
	//return &g_board_desc.chip_desc;
	return NULL;
}
EXPORT_SYMBOL(get_chip_desc);

i2c_desc_t * get_i2c_desc(void)
{
/*	chip_desc_t *p_chip_desc = get_chip_desc();

	if (p_chip_desc)
	{
		return &p_chip_desc->i2c_desc;
	}
	else
	{
		return NULL;
	}
*/
	return NULL;
}
EXPORT_SYMBOL(get_i2c_desc);

irq_desc_t * get_irq_desc(void)
{
/*	chip_desc_t *p_chip_desc = get_chip_desc();

	if(p_chip_desc)
		return &p_chip_desc->irq_desc;
	else
		return NULL;
*/
	return NULL;
}
EXPORT_SYMBOL(get_irq_desc);

void * request_desc(req_desc_t req)
{
	switch (req) {
		case CHIP:
			return get_chip_desc();
		case IRQ:
			return get_irq_desc();
		case I2C:
			return get_i2c_desc();
		default:
			return NULL;	
	}
}
EXPORT_SYMBOL(request_desc);
