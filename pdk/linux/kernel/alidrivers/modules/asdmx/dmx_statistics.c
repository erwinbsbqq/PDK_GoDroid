
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h> 
#include <linux/moduleparam.h>
#include <linux/interrupt.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>

#include "dmx_internal.h"



/*
 * More complex function where we determine which varible is being accessed by
 * looking at the attribute for the "baz" and "bar" files.
 */
ssize_t dmx_global_stat_show
(
    struct kobject        *kobj,
    struct kobj_attribute *attr,
    char                  *buf
)
{
    struct dmx_device *dmx;

    dmx = container_of(attr, struct dmx_device, attr_ts_in_cnt);

	return sprintf(buf, "%8x:%8x:%8x:\n", dmx->pkt_total_in_cnt, dmx->pkt_total_sync_erro_cnt, dmx->pkt_total_erro_cnt);
}

ssize_t dmx_ts_services_show
(
    struct kobject        *kobj,
    struct kobj_attribute *attr,
    char                  *buf
)
{
    __u32              i;
    struct dmx_device *dmx;

    dmx = container_of(attr, struct dmx_device, attr_ts_services);

    //sprintf(buf, "%s\n", dmx->name);

    for (i = 0; i < DMX_TOTAL_TS_SERVICE; i++)
    {
        if (dmx->services.ts_service[i].status != DMX_TS_SERVICE_STATUS_IDLE)
        {
            snprintf(buf + strlen(buf), PAGE_SIZE,
                     "%3d:%4d:%1d:%8x:%8x:%8x:%8x\n",
                     i,
                     dmx->services.ts_service[i].pid,
                     dmx->services.ts_service[i].status,
                     dmx->services.ts_service[i].pkt_in_cnt,
                     dmx->services.ts_service[i].scram_cnt,
                     dmx->services.ts_service[i].duplicate_cnt,
                     dmx->services.ts_service[i].discon_cnt);
        }
    }

	return (strlen(buf));
}



__s32 dmx_sysfs_entry_create
(
    struct dmx_device  *dmx,
    struct attribute  **attrs
)
{
	DMX_INT32 ret;

	dmx->sysfs_dir_kobj = kobject_create_and_add("status", &dmx->device->kobj);

	if (!dmx->sysfs_dir_kobj)
    {
		dev_warn(dmx->device, "kobject_create_and_add err\n");

        return(-1);
    }

    memset(&dmx->sysfs_attr_group, 0, sizeof(dmx->sysfs_attr_group));

    dmx->sysfs_attr_group.attrs = attrs;

	/* Create the files associated with this kobject */
	ret = sysfs_create_group(dmx->sysfs_dir_kobj, &dmx->sysfs_attr_group);

	if (ret)
    {
		dev_warn(dmx->device, "create sysfs group err: %d\n", ret);

        kobject_put(dmx->sysfs_dir_kobj);

        return(-1);
    }

    return(0);
}


























