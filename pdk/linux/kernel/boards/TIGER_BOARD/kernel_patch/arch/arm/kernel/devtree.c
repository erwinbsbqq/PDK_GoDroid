/*
 *  linux/arch/arm/kernel/devtree.c
 *
 *  Copyright (C) 2009 Canonical Ltd. <jeremy.kerr@canonical.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/export.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/bootmem.h>
#include <linux/memblock.h>
#include <linux/of.h>
#include <linux/of_fdt.h>
#include <linux/of_irq.h>
#include <linux/of_platform.h>

#include <asm/setup.h>
#include <asm/page.h>
#include <asm/mach/arch.h>
#include <asm/mach-types.h>

void __init early_init_dt_add_memory_arch(u64 base, u64 size)
{
	arm_add_memory(base, size);
}

void * __init early_init_dt_alloc_memory_arch(u64 size, u64 align)
{
	return alloc_bootmem_align(size, align);
}

void __init arm_dt_memblock_reserve(void)
{
	u64 *reserve_map, base, size;

	if (!initial_boot_params)
		return;

	/* Reserve the dtb region */
	memblock_reserve(virt_to_phys(initial_boot_params),
			 be32_to_cpu(initial_boot_params->totalsize));

	/*
	 * Process the reserve map.  This will probably overlap the initrd
	 * and dtb locations which are already reserved, but overlaping
	 * doesn't hurt anything
	 */
	reserve_map = ((void*)initial_boot_params) +
			be32_to_cpu(initial_boot_params->off_mem_rsvmap);
	while (1) {
		base = be64_to_cpup(reserve_map++);
		size = be64_to_cpup(reserve_map++);
		if (!size)
			break;
		memblock_reserve(base, size);
	}
}

/**
 * setup_machine_fdt - Machine setup when an dtb was passed to the kernel
 * @dt_phys: physical address of dt blob
 *
 * If a dtb was passed to the kernel in r2, then use it to choose the
 * correct machine_desc and to setup the system.
 */
 static  int debug_step_dt=0;
struct machine_desc * __init setup_machine_fdt(unsigned int dt_phys)
{
	struct boot_param_header *devtree;
	struct machine_desc *mdesc, *mdesc_best = NULL;
	unsigned int score, mdesc_score = ~1;
	unsigned long dt_root;
	const char *model;

	printk("kinson:[%s]-[%d] dt_phys:0x%08x\n",__FUNCTION__,__LINE__,dt_phys);
	if (!dt_phys)
		return NULL;
    debug_step_dt=1;
	devtree = phys_to_virt(dt_phys);
	printk("kinson:[%s]-[%d] devtree:0x%08x   devtree->magic:0x%x   0x%x    0x%x   0x%x\n",__FUNCTION__,__LINE__,devtree,be32_to_cpu(devtree->magic),devtree->totalsize,
		devtree->off_dt_struct,devtree->off_dt_strings);

	/* check device tree validity */
	if (be32_to_cpu(devtree->magic) != OF_DT_HEADER)
    {
		printk("kinson:[%s]-[%d] devtree->magic error!\n",__FUNCTION__,__LINE__);
	    return NULL;
	}
	/* Search the mdescs for the 'best' compatible value match */
	initial_boot_params = devtree;
	dt_root = of_get_flat_dt_root();

	for_each_machine_desc(mdesc) {

		#if 0
		score = of_flat_dt_match(dt_root, mdesc->dt_compat);
		if (score > 0 && score < mdesc_score) {
			mdesc_best = mdesc;
			mdesc_score = score;
		}
		#else //kinson
		mdesc_best = mdesc;
		#endif
		
	}
	debug_step_dt=2;

	if (!mdesc_best) {
		const char *prop;
		long size;

		early_print("\nError: unrecognized/unsupported "
			    "device tree compatible list:\n[ ");
    debug_step_dt=3;
		prop = of_get_flat_dt_prop(dt_root, "compatible", &size);
		while (size > 0) {
			early_print("'%s' ", prop);
			size -= strlen(prop) + 1;
			prop += strlen(prop) + 1;
		}
		early_print("]\n\n");
	debug_step_dt=4;

		dump_machine_table(); /* does not return */
	}
	debug_step_dt=7;

	model = of_get_flat_dt_prop(dt_root, "model", NULL);
	if (!model)
		model = of_get_flat_dt_prop(dt_root, "compatible", NULL);
	if (!model)
		model = "<unknown>";
	pr_info("Machine: %s, model: %s\n", mdesc_best->name, model);
	debug_step_dt=8;

	/* Retrieve various information from the /chosen node */
	of_scan_flat_dt(early_init_dt_scan_chosen, boot_command_line);
	/* Initialize {size,address}-cells info */
	debug_step_dt=9;

	of_scan_flat_dt(early_init_dt_scan_root, NULL);
	debug_step_dt=10;

	/* Setup memory, calling early_init_dt_add_memory_arch */
	of_scan_flat_dt(early_init_dt_scan_memory, NULL);
	debug_step_dt=11;

	/* Change machine number to match the mdesc we're using */
	__machine_arch_type = mdesc_best->nr;

	return mdesc_best;
}
