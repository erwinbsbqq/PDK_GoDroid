#include <linux/crc32.h>
#include <linux/err.h>
#include <asm/div64.h>
#include "ubi.h"

extern char micron_l73;
extern char micron_l74;
extern char micron_l83;
extern char micron_l84;


#define DBG_PRINT_BACKUP 0

#if DBG_PRINT_BACKUP
#define bak_msg(fmt, ...) printk(KERN_WARNING "[UBI PATCH] " fmt "\n", ##__VA_ARGS__)
#else
#define bak_msg(fmt, ...) do{} while(0)
#endif

static u8 corruption = 0;
static struct mtd_oob_ops *oob_ops=NULL;
static loff_t original_data_addr=-1;
 int backup_pnum = 0;

/**
 * check_backup_volume - check if backup volume has been built
 * @ubi: UBI device description object
 *
 * This function returns 1 in case of  backup volume have been built
 * 0 in case of don't find
 */
int ubi_check_backup_volume(struct ubi_device *ubi)
{
	return ubi->bakup_info->volume_built;
}


/**
 * ubi_check_pairpage - check pair page for MLC nand
 * @addr: physical address
 * if addr is upper, return low page address
 * else return -1
 */
static loff_t ubi_check_pairpage(struct ubi_device *ubi,loff_t addr)
{
	
	int phy_page_num;

	phy_page_num = (addr & ubi->mtd->erasesize_mask) >> ubi->mtd->writesize_shift;
  
  /* for l83 */  
  if (micron_l83 || micron_l84) {	  
	  switch(phy_page_num)
		{
			case 0:
			case 1:                 
	    case 2:
			case 3:
	    case 254:
			case 255:    //0, 1, 2, 3 page is low, 
				return -1;
			
			default:
				if(phy_page_num%4 <= 1)   //2 3 is high page, 0 1 is low page
					return -1;   //low page
				else
					return (addr - 6*ubi->min_io_size);
				break;
		}	
	}
	
	/* for l73, l74 */
	else if (micron_l73 || micron_l74) {
		switch(phy_page_num)
		{
			case 0:
			case 1:    //0, 1 page is low, 
				return -1;
			case 254:
			case 255:   // 254, 255 is upper page
				return (addr - 4*ubi->min_io_size);
			default:
				if(phy_page_num%4 > 1)   // 2 3 is low page, 0 1 is highpage
					return -1;   //low page
				else
					return (addr - 6*ubi->min_io_size);
				break;
		}
	}	
	else
	{	
		bak_msg("fix me, un-support nand type\n");
		return -1;   //low page
	}
}


static int allocate_new_block_for_backup_volume(struct ubi_device *ubi)
{
	struct ubi_vid_hdr *vid_hdr;
	struct ubi_volume *vol;
	struct ubi_bakup_info *backup_info = ubi->bakup_info;
	int pnum;
	int err;
	int tries = 0;
	
	
	vol = ubi->volumes[vol_id2idx(ubi, UBI_BACKUP_VOLUME_ID)];
	
	vid_hdr = ubi_zalloc_vid_hdr(ubi, GFP_NOFS);
	if (!vid_hdr)
		return -ENOMEM;

	vid_hdr->sqnum = cpu_to_be64(ubi_next_sqnum(ubi));
	vid_hdr->vol_id = cpu_to_be32(UBI_BACKUP_VOLUME_ID);
	vid_hdr->lnum = cpu_to_be32(0);
	vid_hdr->compat = UBI_BACKUP_VOLUME_COMPAT;
	vid_hdr->vol_type = UBI_VID_DYNAMIC;
	vid_hdr->data_size = vid_hdr->used_ebs = vid_hdr->data_pad = cpu_to_be32(0);

retry:
	/* use new version */
	pnum = ubi_wl_get_peb(ubi);
	/*
	pnum = ubi_wl_get_peb(ubi, UBI_SHORTTERM);
	*/
	
	if (pnum < 0) {
		err = pnum;
		goto out_leb_unlock;
	}

	err = ubi_io_write_vid_hdr(ubi, pnum, vid_hdr);
	if (err) {
		ubi_warn("failed to write VID header to LEB %d:%d, PEB %d",
			 UBI_BACKUP_VOLUME_ID, 0, pnum);
		goto write_error;
	}

	if (vol->eba_tbl[0] >= 0) {
		err = ubi_wl_put_peb(ubi, vol->vol_id, vid_hdr->lnum, vol->eba_tbl[0], 0);
		if (err)
			goto out_leb_unlock;
	}

	vol->eba_tbl[0] = pnum;

	// init backup variable 	
	backup_info->pnum = pnum;
	backup_info->volume_built = 1;
	backup_info->page = ubi->leb_start >> ubi->mtd->writesize_shift;
	original_data_addr = -1;	
	bak_msg("%s, blk=0x%x, page=0x%x\n", __func__, pnum, backup_info->page);

out_leb_unlock:
	ubi_free_vid_hdr(ubi, vid_hdr);
	return err;

write_error:
	/*
	 * Bad luck? This physical eraseblock is bad too? Let's try to
	 * get another one.
	 */
	ubi_warn("failed to write to PEB %d", pnum);
	ubi_wl_put_peb(ubi, vol->vol_id, vid_hdr->lnum, pnum, 1);
	if (++tries > UBI_IO_RETRIES) {
		ubi_free_vid_hdr(ubi, vid_hdr);
		return err;
	}
	ubi_msg("try again");
	goto retry;
}

/**
 * ubi_backup_data_to_backup_volume - 
 * backup data to the backup volume
 * @ubi: UBI device description object
 * @addr: address will be programmed
 */
int ubi_backup_data_to_backup_volume(struct ubi_device *ubi, loff_t addr)
{
	struct ubi_bakup_info *backup_info = ubi->bakup_info;
	struct mtd_info *mtd_back = ubi->mtd;
	int err;
	int pnum, offset;
	loff_t lpage_addr;   // low page byte address

	
        if (backup_info == NULL)
        {
            bak_msg("FIXME %x %x\n",ubi, addr);
        }

         if (mtd_back == NULL)
        {
            bak_msg("FIXME m %x %x\n",ubi, addr);
        }
    
	lpage_addr = ubi_check_pairpage(ubi, addr);
	if(lpage_addr == -1)
	{
//		bak_msg("This page 0x%llx don't need to backup\n",addr);
		return 0;   //no need to backup 
	}	

	err = leb_write_lock(ubi, UBI_BACKUP_VOLUME_ID, 0);
	if (err)
		return err;
	if(backup_info->page >= (mtd_back->erasesize >> mtd_back->writesize_shift))
	{
		bak_msg("%s backup_info->page 0x%x\n", __func__, backup_info->page);
		err = allocate_new_block_for_backup_volume(ubi);		//free the previous block and allcate a new block
		if(err)
		{
			bak_msg("allocate new block fail\n");
			goto out_unlock;
		}
	}

	pnum = lpage_addr >> mtd_back->erasesize_shift;
	offset = lpage_addr & mtd_back->erasesize_mask;
	err = ubi_io_read(ubi, oob_ops->datbuf, pnum, offset, ubi->min_io_size);
	if (err == -EBADMSG ||err == -EIO)
	{
		bak_msg("ubi_io_read err\n");
		goto out_unlock;
	}	
	
	//bak_msg("R bak blk=0x%x pg=%x\n", pnum, offset/ubi->min_io_size); 
	
	oob_ops->mode = MTD_OPS_PLACE_OOB;
	oob_ops->ooblen = sizeof(loff_t);
	*(loff_t *)(oob_ops->oobbuf) = lpage_addr;
	oob_ops->len = ubi->min_io_size;
	oob_ops->ooboffs = 0;
	oob_ops->retlen = 0;
	oob_ops->oobretlen = 0;
	//bak_msg("W bak b 0x%x, p 0x%x, org p %llx\n",backup_info->pnum, backup_info->page, lpage_addr);
	//err = ubi->mtd->write_oob(ubi->mtd,
  err = mtd_write_oob(ubi->mtd,
		((loff_t) backup_info->pnum << mtd_back->erasesize_shift)|((loff_t)  backup_info->page << mtd_back->writesize_shift),
		oob_ops);
	backup_info->page++;
	if (err)
		bak_msg("write backup error blk 0x%08x, page 0x%08x\n",backup_info->pnum, backup_info->page - 1);

out_unlock:
	leb_write_unlock(ubi, UBI_BACKUP_VOLUME_ID, 0);
	return err;
}

 /* Temporary variables used during scanning */
static struct ubi_ec_hdr *ech;
static struct ubi_vid_hdr *vidh;

/**
 * process_eb - read, check UBI headers, and add them to scanning information.
 * @ubi: UBI device description object
 * @pnum: the physical eraseblock number
 *
 * return -1: found bad backup volume block, need to erase this block
 * return 1: found good backup volume block
 * return 0: found block don't belong to backup volume, ignore it.
 */
static int process_backup_volume(struct ubi_device *ubi, int pnum)
{
	long long uninitialized_var(ec);
	int err, vol_id;

	
	//bak_msg("%s pnum=0x%x\n", pnum);

	/* Skip bad physical eraseblocks */
	err = ubi_io_is_bad(ubi, pnum);
	if (err)     
		return 0;

	err = ubi_io_read_ec_hdr(ubi, pnum, ech, 0);    

      //if (err < 0 || err == UBI_IO_PEB_EMPTY )
  if (err < 0 || err == UBI_IO_FF )  
		return 0;   //don't handle any error, as the process_eb function could do it.
	
	/* OK, we've done with the EC header, let's look at the VID header */
	err = ubi_io_read_vid_hdr(ubi, pnum, vidh, 0);
	//if (err < 0 || err == UBI_IO_PEB_FREE || err == UBI_IO_BAD_VID_HDR)
	if (err < 0 || err == UBI_IO_FF_BITFLIPS || err == UBI_IO_BAD_HDR_EBADMSG)
		return 0;

	vol_id = be32_to_cpu(vidh->vol_id);
	if (vol_id == UBI_BACKUP_VOLUME_ID) {  
		//found backup volume 
		int lnum = be32_to_cpu(vidh->lnum);
		//bak_msg("found backup volume, pnum=0x%x\n", pnum);
		if(lnum)
		{
			bak_msg("bad backup volume lnum = 0x%08x\n", lnum);
			return -1;
		}
		/* Unsupported internal volume */
		if(vidh->compat != UBI_COMPAT_REJECT)
		{
			bak_msg("compat != UBI_COMPAT_REJECT\n");
			return -1;
		}
		//bak_msg("good backup volume, pnum=0x%x\n", pnum);
		return 1;
	}
	return 0;
}

static int check_empty(void *buf, int len)
{
	int i, v, j;
	int c = 0;
	
	//allow some bit not "1"
	for (i=0; i<len; i+=4) 
  {
		v = *(unsigned int *)(buf+i);
		if (v != 0xffffffff)
		{	
			for (j=0; j<32; j++)
			{
				if (!(v & 1))
					c++;
				v = v >> 1;
			}	
			if (c > 39)
			{				
				return 1;
			}
		}	
	}
	return 0;
}


static int check_original_data(struct ubi_device *ubi,struct mtd_oob_ops *oob_ops)
{
	loff_t addr = *(loff_t *)(oob_ops->oobbuf);
	void *buf = ubi->peb_buf1;    //use this buf temporary
	void *buf_back = oob_ops->datbuf;
	int ret,pnum,offset,i;
	
	pnum = (int)(addr >> ubi->mtd->erasesize_shift);
	offset = (int)(addr & ubi->mtd->erasesize_mask);
	//bak_msg("addr = 0x%llx\n",addr);
	//bak_msg("read pnum %d, offset %d",pnum,offset);
	ret = ubi_io_read(ubi, buf, pnum, offset, ubi->min_io_size);
	if(ret<0)
		return 1;
	for(i=0;i<ubi->min_io_size;i +=4)
	{
		if(*(u32 *)buf != *(u32 *)buf_back)
		{	
                     bak_msg("addr = 0x%llx\n",addr);
                    	bak_msg("read pnum %d, offset %d",pnum,offset);
			bak_msg("buf data not same @ 0x%x, buf=0x%x, buf_back=0x%x \n", i, *(u32 *)buf, *(u32 *)buf_back);
			bak_msg("bak data @ last not empty page, ori data @ page 0x%x\n", (int) (addr >> ubi->mtd->writesize_shift));
			return 1;
		}	
		
		buf += 4;
		buf_back += 4;
	}		
	bak_msg("buf data are all same\n");
	return 0;

}


/*
scan NAND block
return -1: uncorrectable ecc or something others error
return 0: this block is empty
return >0: the unempty page number
*/
static int find_last_unempty_page(struct ubi_device *ubi, int pnum, struct mtd_oob_ops *oob_ops)
{
	struct mtd_info *mtd_back = ubi->mtd;
	int page,start_page,err;

	oob_ops->mode = MTD_OPS_PLACE_OOB;
	oob_ops->ooblen = sizeof(loff_t);
	oob_ops->len = ubi->min_io_size;
	oob_ops->ooboffs = 0;
	oob_ops->retlen = 0;
	oob_ops->oobretlen = 0;

	//bak_msg("mtd_info size blk=0x%x, page=0x%x, err=0x%0x\n", pnum, page, err);
	
	page = (mtd_back->erasesize -1) >> mtd_back->writesize_shift;
	start_page = (ubi->leb_start & (mtd_back->erasesize -1)) >> mtd_back->writesize_shift;
	for(;page>=start_page;page--)
	{
		//err = ubi->mtd->read_oob(ubi->mtd,
    err = mtd_read_oob(ubi->mtd, 
			((loff_t) pnum << mtd_back->erasesize_shift) | ((loff_t)  page << mtd_back->writesize_shift),
			oob_ops);
		
		if(err<0 && err != -EUCLEAN)   //ecc error also return -1, refresh this block
		{	
			bak_msg("read error, blk=0x%x, page=0x%x, err=0x%0x\n", pnum, page, err);
			return -1;
		}	

		if(check_empty(oob_ops->datbuf, ubi->min_io_size))
		{
			bak_msg("found not empty blk=0x%x, page 0x%x\n", pnum, page);
			return page;
		}
	}
	return 0; 
}

/**
 * ubi_backup_volume_scan - scan whole flash to find backup volume
 * @ubi: UBI device description object
 *
 * This function returns 1 in case of found backup volume 
 * -1 in case of allocate resource error
 * 0 : scan complete
 */
// !!! this function will be called before ubi_backup_volume_init. 
// So it will allocate resouce needed by backup volume
int ubi_backup_volume_scan(struct ubi_device *ubi)
{
	int pnum,page,ret,i;
	int backup_blk_number = 0;
/*
* allocate resouce backup volume will be used
* ech and vidh resouce will be release at the end of this function
*/
		//bak_msg("%s %x \n", __func__,  ubi->peb_count);
		
		
		ech = kzalloc(ubi->ec_hdr_alsize, GFP_KERNEL);
		if(!ech)
				goto out_scan;
		
		vidh = ubi_zalloc_vid_hdr(ubi, GFP_KERNEL);
		if(!vidh)
				goto out_ech;
		
		ubi->bakup_info = kzalloc(sizeof(struct ubi_bakup_info), GFP_KERNEL);
		if(!ubi->bakup_info)
				goto out_vidh;
		
		if(!oob_ops)
		{	
			oob_ops = kmalloc(sizeof(struct mtd_oob_ops), GFP_KERNEL);
			if(!oob_ops)
					goto out_bakup_info;
			
			//bak_msg("min_io_size = %08x\n", ubi->min_io_size);
		  oob_ops->datbuf = kmalloc(ubi->min_io_size, GFP_KERNEL);
			if(!oob_ops->datbuf)
					goto out_oob_ops;
			//bak_msg("ubi->mtd->oobsize %08x\n", ubi->mtd->oobsize);
	  
	  	oob_ops->oobbuf = kmalloc(ubi->mtd->oobsize, GFP_KERNEL);
			if(!oob_ops->oobbuf)
				goto out_datbuf;
		}		
		
 	//init backup_info structure
	ubi->bakup_info->page = 0;
	ubi->bakup_info->pnum = -1;    //-1 means no backup volume found
	ubi->bakup_info->volume_built = 0;
	original_data_addr =  -1;

/*
* scan whoile NAND flash to find the backup volume
*/
    for (pnum = backup_pnum; pnum < ubi->peb_count; pnum++) {
		cond_resched();
		ret = process_backup_volume(ubi, pnum);
		if (ret == 1)
		{
			backup_blk_number++;
			if(backup_blk_number >= 2)
				bak_msg("found 2 backup block\n");
				
			page = find_last_unempty_page(ubi, pnum, oob_ops);
			if(page > 0)
			{
				bak_msg("compare blk=0x%x page=0x%x, org page=0x%llx\n", pnum, page, *(loff_t *)(oob_ops->oobbuf));
				
				//set original_data_addr here, david
				original_data_addr = *(loff_t *)(oob_ops->oobbuf);				
				
				if(check_original_data(ubi, oob_ops))
				{
					corruption = 1;   //the original data isn't correct anymore
					//original_data_addr = *(loff_t *)(oob_ops->oobbuf);					
				}
				ubi->bakup_info->page = page+1;
				ubi->bakup_info->pnum = pnum;
                            goto found_backup;
			}
			else if(page == 0)
			{
				ubi->bakup_info->page = ubi->leb_start>>ubi->mtd->writesize_shift;
				ubi->bakup_info->pnum = pnum;
                            goto found_backup;
			}
			else
				bak_msg("read error pnum=0x%x, ubi->peb_count=0x%x\n", pnum, ubi->peb_count);
				//this block will be added to erase list in process_eb function. 
		}
		else if(ret == -1)
		{
			bak_msg("found a invaild backup block,should add to erase list1 \n");
			//this block will be added to erase list in process_eb function. 
		}
	}
found_backup:
	ubi_free_vid_hdr(ubi, vidh);
	kfree(ech);
	return 0;
	
out_datbuf:
	kfree(oob_ops->datbuf);
out_oob_ops:
	kfree(oob_ops);
out_bakup_info:
	kfree(ubi->bakup_info);
out_vidh:
	ubi_free_vid_hdr(ubi, vidh);
out_ech:
	kfree(ech);
out_scan:
	bak_msg("somethings error happen\n");
	return -1;

}

/**
 * ubi_backup_volume_create - create a empty backup volume
 * @ubi: UBI device description object
 * @si: scanning information
 * @vtbl: contents of the volume table
 *
 * This function returns zero in case of success and a negative error code in
 * case of failure.
 */
static int ubi_backup_volume_create(struct ubi_device *ubi, struct ubi_attach_info *ai,
		       struct ubi_bakup_info *bakup_info)
{
	int err, tries = 0;
	static struct ubi_vid_hdr *vid_hdr;
	/* for new version */
	struct ubi_ainf_peb *new_aeb;
	/*
	struct ubi_scan_leb *new_seb;
	*/
	
	//bak_msg("create backup volume.....\n");

	vid_hdr = ubi_zalloc_vid_hdr(ubi, GFP_KERNEL);
	if (!vid_hdr)
		return -ENOMEM;

retry:
	/* for new version */
	new_aeb = ubi_early_get_peb(ubi, ai);
	/*
	new_seb = ubi_scan_get_free_peb(ubi, ai);
	*/
	if (IS_ERR(new_aeb)) {
		err = PTR_ERR(new_aeb);
		goto out_free;
	}

	vid_hdr->vol_type = UBI_VID_DYNAMIC;
	vid_hdr->vol_id = cpu_to_be32(UBI_BACKUP_VOLUME_ID);
	vid_hdr->compat = UBI_BACKUP_VOLUME_COMPAT;
	vid_hdr->data_size = vid_hdr->used_ebs =
			     vid_hdr->data_pad = cpu_to_be32(0);
	vid_hdr->lnum = cpu_to_be32(0);
	vid_hdr->sqnum = cpu_to_be64(++ai->max_sqnum);

	/* The EC header is already there, write the VID header */
	err = ubi_io_write_vid_hdr(ubi, new_aeb->pnum, vid_hdr);
	if (err)
		goto write_error;
	/*
	 * And add it to the scanning information. Don't delete the old
	 * @old_seb as it will be deleted and freed in 'ubi_scan_add_used()'.
	 */
	 
	/* use new version */
	err = ubi_add_to_av(ubi, ai, new_aeb->pnum, new_aeb->ec,
				vid_hdr, 0);
				
	/*
	err = ubi_scan_add_used(ubi, ai, new_aeb->pnum, new_aeb->ec,
				vid_hdr, 0);			
  */
  
	// init backup variable 
	bakup_info->pnum = new_aeb->pnum;
	bakup_info->page = ubi->leb_start>>ubi->mtd->writesize_shift;
       bak_msg("create backup volume.....%x\n", bakup_info->pnum);
	original_data_addr = -1;
	
	kfree(new_aeb);
	ubi_free_vid_hdr(ubi, vid_hdr);
	return err;

write_error:
	if (err == -EIO && ++tries <= 5) {
		/*
		 * Probably this physical eraseblock went bad, try to pick
		 * another one.
		 */
		list_add_tail(&new_aeb->u.list, &ai->corr);
		goto retry;
	}
	kfree(new_aeb);
out_free:
	ubi_free_vid_hdr(ubi, vid_hdr);
	return err;
}


/**
 * ubi_backup_volume_init - init backup volume.
 * @ubi: UBI device description object
 * @si: scanning information
 *
 * This function init the backup volume
 * Returns zero in case of success and a negative
 * error code in case of failure
 */
int ubi_backup_volume_init(struct ubi_device *ubi, struct ubi_attach_info *ai)
{
	int i, reserved_pebs = 0;
	int ret;
	struct ubi_volume *vol;	

	if(ubi->bakup_info->pnum == -1)        //backup volume hasn't built, create it first.
	{
		ret = ubi_backup_volume_create(ubi, ai, ubi->bakup_info);
		if(ret < 0)
		{
			bak_msg("something err in ubi_backup_volume_create\n");
			return ret;
		}
	}
	/* And add the layout volume */
	vol = kzalloc(sizeof(struct ubi_volume), GFP_KERNEL);
	if (!vol)
		return -ENOMEM;

	vol->reserved_pebs = UBI_BACKUP_VOLUME_EBS;
	vol->alignment = 1;
	vol->vol_type = UBI_DYNAMIC_VOLUME;
	vol->name_len = sizeof(UBI_BACKUP_VOLUME_NAME) - 1;
	memcpy(vol->name, UBI_BACKUP_VOLUME_NAME, vol->name_len + 1);
	vol->usable_leb_size = ubi->leb_size;
	vol->used_ebs = vol->reserved_pebs;
	vol->last_eb_bytes = vol->reserved_pebs;
	vol->used_bytes =
		(long long)vol->used_ebs * (ubi->leb_size - vol->data_pad);
	vol->vol_id = UBI_BACKUP_VOLUME_ID;
	vol->ref_count = 1;
	ubi->volumes[vol_id2idx(ubi, vol->vol_id)] = vol;
	reserved_pebs += vol->reserved_pebs;
	ubi->vol_count += 1;
	vol->ubi = ubi;
	if (reserved_pebs > ubi->avail_pebs)
	{
		ubi_err("not enough PEBs, required %d, available %d",
			reserved_pebs, ubi->avail_pebs);
		return -1;
	}
	ubi->rsvd_pebs += reserved_pebs;
	ubi->avail_pebs -= reserved_pebs;

	ubi->bakup_info->volume_built = 1;
	
	return 0;

}



int ubi_bad_data_recovery(struct ubi_device *ubi)
{
	u32 i,j,pnum,page,correct;
	loff_t addr;
	int ret;
	struct mtd_info *mtd_back = ubi->mtd;
	unsigned char *buf = ubi->peb_buf1;
	struct ubi_volume_desc volume_desc;
	
	if(!corruption)
		return 0;   //no data corrupt, no need recovery

	vidh = ubi_zalloc_vid_hdr(ubi, GFP_KERNEL);
	if(!vidh)
		return -1;

	pnum = original_data_addr >> mtd_back->erasesize_shift;
	page = original_data_addr & mtd_back->erasesize_mask;
	j=0;
	correct=0;
	bak_msg("%s ori data blk 0x%x, page=0x%x\n", __func__, pnum, page);
	
	for(i=ubi->leb_start; i<ubi->peb_size; i+=mtd_back->writesize)
	{
		ret = ubi_io_read(ubi, buf+j, pnum, i, mtd_back->writesize);
		if(ret == -EBADMSG || ret == -EIO)
		{
			if(i == page)
			{
				bak_msg("recovery.....");
				memcpy(buf+j, oob_ops->datbuf, mtd_back->writesize);
				correct = 1;
			}
			else
				bak_msg("unbackup page also corrupt,..pnum:%d,page:%d\n",pnum,i);
		}
		ret = check_empty(buf+j, mtd_back->writesize);
		if(!ret)
			break;    //this page is empty, so the last of pages also are empty.no need to read

		j +=mtd_back->writesize;
	}

	if(page > ubi->leb_start && correct ==0)     //don't found corrupt page
		return 0;

  if (j == 0)		//this is an empty blk
  	return 0;

  bak_msg("%s page=0x%x, ubi->leb_start=0x%x\n", __func__, page, ubi->leb_start);
	ret = ubi_io_read_vid_hdr(ubi, pnum, vidh, 0);
	
	volume_desc.vol = ubi->volumes[be32_to_cpu(vidh->vol_id)];
	volume_desc.mode = UBI_READWRITE;
	
	printk("write %d bytes to LEB %d:%d", j, volume_desc.vol, be32_to_cpu(vidh->lnum));
	
	/* use new version */
	ubi_leb_change(&volume_desc, be32_to_cpu(vidh->lnum), buf, j);
	/*
	ubi_leb_change(&volume_desc, be32_to_cpu(vidh->lnum), buf, j, UBI_UNKNOWN);
	*/

	ubi_free_vid_hdr(ubi, vidh);
	return 0;

}

