
#include <malloc.h>
#include <jffs2/load_kernel.h>
#include <linux/list.h>
#include <linux/ctype.h>
#include <linux/mtd/mtd.h>


#include <common.h>
#include <asm/io.h>
#include <boot_common.h>
#include <asm/dma-mapping.h>
#include <linux/err.h>
#include <nand.h>
#include <ali_nand.h>



#define ALI_NAND_ERROR printf
#if 0
#define ALI_NAND_DEBUG(args...)  do{}while(0)
#else
#define ALI_NAND_DEBUG  printf
#endif



//extern struct list_head devices;
//struct part_info *ali_nand_part[128] = {0};
//u8 ali_nand_part_num = 0;


#define BLOCK_REWRITE_TBL_SIZE 4096
extern u8 block_rewrite_tbl[];
uint8_t bak_blk_pattern[4]={'B','a','k','0'};

#if 0
void ali_nand_show_parts_info(void)
{
	struct list_head *dentry, *pentry;
	struct part_info *part;
	struct mtd_device *dev;
	int part_num;

	list_for_each(dentry, &devices) {
		dev = list_entry(dentry, struct mtd_device, link);
		/* list partitions for given device */
		part_num = 0;
		ali_nand_part_num = 0;

		ALI_NAND_DEBUG("\ndevice %s%d <%s>, # parts = %d\n",
				MTD_DEV_TYPE(dev->id->type), dev->id->num,
				dev->id->mtd_id, dev->num_parts);
		ALI_NAND_DEBUG(" #: name\t\tsize\t\toffset\t\tmask_flags\n");

		list_for_each(pentry, &dev->parts) {
			part = list_entry(pentry, struct part_info, link);
			ALI_NAND_DEBUG("%2d: %-20s0x%08x\t0x%08x\t%d\n",
					part_num, part->name, part->size,
					part->offset, part->mask_flags);

			part_num++;
			if (strcmp((char *) dev->id->mtd_id, "ali_nand") == 0)
			{
				ali_nand_part[ali_nand_part_num] = part;
				ALI_NAND_DEBUG("%2d: %-20s0x%08x\t0x%08x\t%d\n",
					ali_nand_part_num, ali_nand_part[ali_nand_part_num]->name, ali_nand_part[ali_nand_part_num]->size,
					ali_nand_part[ali_nand_part_num]->offset, ali_nand_part[ali_nand_part_num]->mask_flags);
				ali_nand_part_num++;
			}
		}
	}

	if (list_empty(&devices))
	{
		ALI_NAND_DEBUG("no partitions defined\n");
	}
}
#endif

/**
 * ali_nand_show_bbt_offs -
 * @mtd:	mtd info structure
 * @chip:	nand chip info structure
 * @page:	page number to read
 * @sndcmd:	flag whether to issue read command or not
 */
void ali_nand_show_bbt_offs(struct mtd_info *mtd)
{
	struct nand_chip *this = mtd->priv;
	struct nand_bbt_descr *td = this->bbt_td;
	struct nand_bbt_descr *md = this->bbt_md;
	int i, chips;

	chips = this->numchips;
	
	/* Check, if we found a bbt for each requested chip */
	for (i=0; i<chips; i++) {
		if (td->pages[i] == -1)
		{
			ALI_NAND_DEBUG("Bad block table not found for chip %d\n", i);
		}
		else
		{
			ALI_NAND_DEBUG("Bad block table found at page %d, version 0x%02X\n", td->pages[i],
			       td->version[i]);
		}
	}
	
	/* Check, if we found a bbt for each requested chip */
	for (i=0; i<chips; i++) {
		if (md->pages[i] == -1)
		{
			ALI_NAND_DEBUG("Bad block table not found for chip %d\n", i);
		}
		else
		{
			ALI_NAND_DEBUG("Bad block table found at page %d, version 0x%02X\n", md->pages[i],
			       md->version[i]);
		}
	}
}


/**
 * ali_nand_get_bak_blk -
 * @mtd:	mtd info structure
 * @chip:	nand chip info structure
 * @page:	page number to read
 * @sndcmd:	flag whether to issue read command or not
 */
int ali_nand_get_bak_blk(struct mtd_info *mtd, loff_t offs)
{
	struct nand_chip *this = mtd->priv;
	int i, chips, bbt_page_offs, bak_blk;

	ALI_NAND_DEBUG("%s\n", __FUNCTION__);

	struct nand_bbt_descr *td = this->bbt_td;
	struct nand_bbt_descr *md = this->bbt_md;
	
	if (!(this->options & NAND_BBT_SCANNED)) {
		this->options |= NAND_BBT_SCANNED;
		this->scan_bbt(mtd);
	}
	
	chips = this->numchips;

	// td...
	/* Check, if we found a bbt for each requested chip */
	for (i=0; i<chips; i++) {
		if (md->pages[i] == -1)
		{
			ALI_NAND_DEBUG("Bad block table not found for chip %d\n", i);
			bbt_page_offs = -1;
			return -1;
		}
		else
		{
			ALI_NAND_DEBUG("Bad block table found at page %d, version 0x%02X\n", md->pages[i],
			       md->version[i]);
			bbt_page_offs = md->pages[i];
		}
	}

	if(offs % (2*mtd->erasesize))
		bak_blk = (mtd->size >> this->bbt_erase_shift) - td->maxblocks + 1;
	else
		bak_blk = (mtd->size >> this->bbt_erase_shift) - td->maxblocks;
	for(i=bak_blk; i<md->pages[0]; i+=2)
	{
		if(nand_block_isbad(mtd, i<< this->bbt_erase_shift) != 0)
			break;
	}

	if(i < md->pages[0])
	{
		ALI_NAND_DEBUG("Bak block create at #%d\n", i);
		return i;
	}
	else
	{
		ALI_NAND_DEBUG("Bak block not found\n");
		return -1;
	}
	//write oob bytes 4~7
}

/**
 * ali_nand_bak_blk_check_pattern - [GENERIC] check if a pattern is in the buffer
 * @buf:	the buffer to search
 * @len:	the length of buffer to search
 * @paglen:	the pagelength
 * @td:		search pattern descriptor
 *
 * Check for a pattern at the given place. Used to search bad block
 * tables and good / bad block identifiers.
 * If the SCAN_EMPTY option is set then check, if all bytes except the
 * pattern area contain 0xff
 *
*/
static int ali_nand_bak_blk_check_pattern(uint8_t *buf)
{
	int i, end = 0;
	uint8_t *p = buf;
	
	/* Compare the pattern */
	for (i=0; i<4; i++) {
		if (p[i] != bak_blk_pattern[i])
			return -1;
	}

	return 0;
}

//search bak blk. return src blk no.
static int ali_nand_search_src_bak_blk(struct mtd_info *mtd, uint8_t *buf, int *src_blk, int *bak_blk)
{
	struct nand_chip *this = mtd->priv;
	int i, chips;
	int startblock, block, dir;
	int bbtblocks = -1;
	int srcblocks = -1;

	struct nand_bbt_descr *td = this->bbt_td;
	struct nand_bbt_descr *md = this->bbt_md;

	/* Search direction top -> down ? */
	if (td->options & NAND_BBT_LASTBLOCK) {
		startblock = (mtd->size >> this->bbt_erase_shift) - 1;
		dir = -1;
	} else {
		startblock = 0;
		dir = 1;
	}

	/* Do we have a bbt per chip ? */
	if (td->options & NAND_BBT_PERCHIP) {
		chips = this->numchips;
		bbtblocks = this->chipsize >> this->bbt_erase_shift;
		startblock &= bbtblocks - 1;
	} else {
		chips = 1;
		bbtblocks = mtd->size >> this->bbt_erase_shift;
	}

	for (i=0; i<chips; i++) {
		/* Scan the maximum number of blocks */
		for (block=0; block<td->maxblocks; block++) {

			int actblock = startblock + dir * block;
			loff_t offs = (loff_t)actblock << this->bbt_erase_shift;
			
//			ALI_NAND_DEBUG("%s %d\n", __FUNCTION__, actblock);
			/* Read first page */
			//scan_read_raw_oob(mtd, buf, offs, mtd->writesize);
			{
				struct mtd_oob_ops ops;
				int res;
				ops.mode = MTD_OOB_PLACE;
				ops.ooboffs = 0;
				ops.ooblen = mtd->oobsize;
				ops.oobbuf = buf + mtd->writesize;
				ops.datbuf = buf;
				ops.len = mtd->writesize;
				res = mtd->read_oob(mtd, offs, &ops);

//				ALI_NAND_DEBUG("ali_nand_search_src_bak_blk 0x%012llx\n", offs);
				srcblocks = ((((((buf[mtd->writesize+7]<<8)|buf[mtd->writesize+6])<<8)|buf[mtd->writesize+5])<<8)|buf[mtd->writesize+4]);
				if (!ali_nand_bak_blk_check_pattern(ops.oobbuf)) {
					ALI_NAND_DEBUG("bak blk found @ %d\n", srcblocks);
//					ALI_NAND_DEBUG("scan_read_raw_oob %02x %02x %02x %02x %02x %02x %02x %02x\n",
//					 *(ops.oobbuf+7), *(ops.oobbuf+6), *(ops.oobbuf+5), *(ops.oobbuf+4), *(ops.oobbuf+3), *(ops.oobbuf+2), *(ops.oobbuf+1), *(ops.oobbuf));
//					break;
					*bak_blk = actblock;
					*src_blk = srcblocks;
					return 0;
				}
			}
		}
		startblock += this->chipsize >> this->bbt_erase_shift;
	}
	return 0;
}

/**
 * ali_nand_erase_bak_blk - [GENERIC] (Re)write the bad block table
 *
 * @mtd:	MTD device structure
 * @buf:	temporary buffer
 * @td:		descriptor for the bad block table
 * @md:		descriptor for the bad block table mirror
 * @chipsel:	selector for a specific chip, -1 for all
 *
 * (Re)write the bad block table
 *
*/
static int ali_nand_erase_bak_blk(struct mtd_info *mtd, uint8_t *buf)
{
	struct nand_chip *this = mtd->priv;
	int i, chips;
	int startblock, block, dir;
	int bbtblocks = -1;
	int bakblocks = -1;
	int res;

	struct nand_bbt_descr *td = this->bbt_td;
	struct nand_bbt_descr *md = this->bbt_md;

	/* Search direction top -> down ? */
	if (td->options & NAND_BBT_LASTBLOCK) {
		startblock = (mtd->size >> this->bbt_erase_shift) - 1;
		dir = -1;
	} else {
		startblock = 0;
		dir = 1;
	}

	/* Do we have a bbt per chip ? */
	if (td->options & NAND_BBT_PERCHIP) {
		chips = this->numchips;
		bbtblocks = this->chipsize >> this->bbt_erase_shift;
		startblock &= bbtblocks - 1;
	} else {
		chips = 1;
		bbtblocks = mtd->size >> this->bbt_erase_shift;
	}

	for (i=0; i<chips; i++) {
		/* Scan the maximum number of blocks */
		for (block=0; block<td->maxblocks; block++) {

			int actblock = startblock + dir * block;
			loff_t offs = (loff_t)actblock << this->bbt_erase_shift;
			
			ALI_NAND_DEBUG("%s %d\n", __FUNCTION__, actblock);
			/* Read first page */
//			scan_read_raw_oob(mtd, buf, offs, mtd->writesize);
			{
				struct mtd_oob_ops ops;
				struct erase_info einfo;

				ops.mode = MTD_OOB_PLACE;
				ops.ooboffs = 0;
				ops.ooblen = mtd->oobsize;
				ops.oobbuf = buf + mtd->writesize;
				ops.datbuf = buf;
				ops.len = mtd->writesize;
				res = mtd->read_oob(mtd, offs, &ops);

				ALI_NAND_DEBUG("bak blk read offs 0x%012llx\n", offs);
				bakblocks = ((((((buf[mtd->writesize+7]<<8)|buf[mtd->writesize+6])<<8)|buf[mtd->writesize+5])<<8)|buf[mtd->writesize+4]);
				if (!ali_nand_bak_blk_check_pattern(ops.oobbuf)) {
				
					ALI_NAND_DEBUG("erase bak blk @ %d\n", bakblocks);
//					ALI_NAND_DEBUG("scan_read_raw_oob %02x %02x %02x %02x %02x %02x %02x %02x\n",
//					*(ops.oobbuf+7), *(ops.oobbuf+6), *(ops.oobbuf+5), *(ops.oobbuf+4), *(ops.oobbuf+3), *(ops.oobbuf+2), *(ops.oobbuf+1), *(ops.oobbuf));

					memset(&einfo, 0, sizeof(einfo));
					einfo.mtd = mtd;
					einfo.addr = offs;
					einfo.len = 1 << this->bbt_erase_shift;
					res = nand_erase_nand(mtd, &einfo, 1);
					if (res < 0)
						goto outerr;
				}
			}
		}
		startblock += this->chipsize >> this->bbt_erase_shift;
	}

	return 0;
	
 outerr:

	printk(KERN_WARNING
	       "nand_bbt: Error while writing bad block table %d\n", res);
	return res;
}

/**
 * ali_nand_write_bak_blk - [GENERIC] (Re)write the bad block table
 *
 * @mtd:	MTD device structure
 * @buf:	temporary buffer
 * @td:		descriptor for the bad block table
 * @md:		descriptor for the bad block table mirror
 * @chipsel:	selector for a specific chip, -1 for all
 *
 * (Re)write the bad block table
 *
*/
static int ali_nand_write_bak_blk(struct mtd_info *mtd, uint8_t *buf, loff_t offs)
{
//write oob bytes 4~7
	struct nand_chip *this = mtd->priv;
	struct erase_info einfo;
	struct mtd_oob_ops ops;
	int res = 0;
	int block = 0;
	size_t len = 0;
	loff_t to = 0;
	uint32_t oob[2] = {0xFF};

	ALI_NAND_DEBUG("%s offs 0x%012llx\n", __FUNCTION__, offs);
	ops.ooblen = mtd->oobsize;
	ops.ooboffs = 0;
	ops.datbuf = NULL;
	ops.mode = MTD_OOB_PLACE;
	
	block = ali_nand_get_bak_blk(mtd, offs);
	if(block < 0)
		goto outerr;

	to = ((loff_t) block) << this->phys_erase_shift;
	
	memset(&einfo, 0, sizeof(einfo));
	einfo.mtd = mtd;
	einfo.addr = to;
	einfo.len = 1 << this->bbt_erase_shift;
	res = nand_erase_nand(mtd, &einfo, 1);
	if (res < 0)
		goto outerr;
	memcpy(oob, "Bak0", 4);
	oob[1] = offs >> this->phys_erase_shift;
	ALI_NAND_DEBUG(KERN_DEBUG "src block @ 0x%x\n", oob[1]);
	
	len = mtd->erasesize;
	ops.mode = MTD_OOB_PLACE;
	ops.ooboffs = 0;
	ops.ooblen = 8;//mtd->oobsize;
	ops.datbuf = buf;
	ops.oobbuf = (uint8_t *)oob;
	ops.len = len;

	res = mtd->write_oob(mtd, to, &ops);
	if (res < 0)
		goto outerr;

	return 0;

 outerr:
	printk(KERN_WARNING
	       "nand_bbt: Error while writing bad block table %d\n", res);
	return res;
}

/**
 * ali_nand_dump_ecc_cnt_tbl - 
 * @mtd:	mtd info structure
 * @chip:	nand chip info structure
 * @page:	page number to read
 * @sndcmd:	flag whether to issue read command or not
 */
int ali_nand_dump_ecc_cnt_tbl(void)
{
	int i;

	ALI_NAND_DEBUG("entry %s\n", __FUNCTION__);
	for(i=0; i<BLOCK_REWRITE_TBL_SIZE; i++)
	{
		if(block_rewrite_tbl[i] == 0xFF)
		{
			ALI_NAND_DEBUG("[WRN] ecc cnt over threshold @ block #%d ", i);
		}
	}
	ALI_NAND_DEBUG("\n");
	
	return 0;
}

/**
 * ali_nand_refresh - 
 * @mtd:	mtd info structure
 * @chip:	nand chip info structure
 * @page:	page number to read
 * @sndcmd:	flag whether to issue read command or not
 */
int ali_nand_refresh(void)
{
	struct erase_info einfo;
	nand_info_t *mtd = &nand_info[nand_curr_device];
	struct nand_chip *this = mtd->priv;
	uint8_t *datbuf, *datbuf_tmp;
	uint8_t oobbuf[8];
	loff_t offset;
	loff_t to;
	size_t retlen = 0;
	int i = 0;
	int res = -1;
	
	ALI_NAND_DEBUG("entry %s\n", __FUNCTION__);
//	ali_nand_show_bbt_offs(mtd);
//	ali_nand_get_bak_blk(mtd, 0);//arthur test
//	ali_nand_show_parts_info();
	ali_nand_dump_ecc_cnt_tbl();
	
	datbuf_tmp = (uint8_t *)kmalloc(mtd->erasesize+0x20, GFP_KERNEL);
	if(datbuf_tmp == NULL)
	{
		ALI_NAND_ERROR("[ERR] NO Memory\n");
		return -1;
	}
	datbuf = (uint8_t *)((u32)(datbuf_tmp + 0x1F) & (u32)(~0x1F));


	for	(i=0; i<BLOCK_REWRITE_TBL_SIZE; i++)
	{
		if(block_rewrite_tbl[i] == 0xFF)
		{
			offset = i << this->bbt_erase_shift;

			retlen = mtd->erasesize;
			if (nand_read(mtd, offset, &retlen, datbuf))
			{
				ALI_NAND_ERROR("[ERR] %s line%d\n", __FUNCTION__, __LINE__);
				kfree(datbuf_tmp);
				return -1;
			}
			
			// 1. erase bak blk
			// copy src to bak blk
			ALI_NAND_DEBUG("1. erase bak blk. copy src blk #%d to bak blk\n", i);
			res = ali_nand_write_bak_blk(mtd, datbuf, offset);
			if (res)
			{
				ALI_NAND_ERROR("[ERR] %s line%d\n", __FUNCTION__, __LINE__);
				kfree(datbuf_tmp);
				return -1;
			}
			
			// 2. erase src blk
			ALI_NAND_DEBUG("2. src block erase @ 0x%012llx\n", offset);
			memset(&einfo, 0, sizeof(einfo));
			einfo.mtd = mtd;
			einfo.addr = offset;
			einfo.len = 1 << this->bbt_erase_shift;
			res = nand_erase_nand(mtd, &einfo, 1);
			if (res)
			{
				ALI_NAND_ERROR("[ERR] %s line%d\n", __FUNCTION__, __LINE__);
				kfree(datbuf_tmp);
				return -1;
			}

			// 3. write src blk
			ALI_NAND_DEBUG("3. write src blk offset 0x%012llx, len 0x%x\n", offset, mtd->erasesize);
			retlen = mtd->erasesize;
			
			//arthur test power cut			res = -1;
			res = nand_write(mtd, offset, &retlen, datbuf);
			if (res)
			{
				ALI_NAND_ERROR("[ERR] %s line%d\n", __FUNCTION__, __LINE__);
				kfree(datbuf_tmp);
				return -1;
			}
			else
			{
			// 4. check src blk ecc & erase bak blk
				ALI_NAND_DEBUG("4. check src blk ecc & erase bak blk\n");
				retlen = mtd->erasesize;
				if (nand_read(mtd, offset, &retlen, datbuf))
				{
					ALI_NAND_ERROR("[ERR] %s line%d\n", __FUNCTION__, __LINE__);
					kfree(datbuf_tmp);
					return -1;
				}
				else
				{
//				ali_nand_bak_blk_recovery test: tmp rm
					ali_nand_erase_bak_blk(mtd, datbuf);
				}
			}
		}
	}

	kfree(datbuf_tmp);
	ALI_NAND_DEBUG("exit %s\n", __FUNCTION__);
	return 0;
}

/**
 * ali_nand_bak_blk_recovery - do recovery if power cut happened when nand refresh.
 */
int ali_nand_bak_blk_recovery(void)
{
	int res = -1;
	nand_info_t *mtd = &nand_info[nand_curr_device];
	struct nand_chip *this = mtd->priv;
	struct erase_info einfo;
	struct mtd_oob_ops ops;
	uint8_t *datbuf = NULL, *datbuf_tmp = NULL;
	uint8_t oob[8] = {0xFF};
	loff_t offset = 0;
	loff_t to = 0;
	size_t retlen = 0;
	int src_blk = -1, bak_blk = -1;

	ALI_NAND_DEBUG("entry %s\n", __FUNCTION__);

	// scan bbt first or kmalloc out of memory.
	if (!(this->options & NAND_BBT_SCANNED)) {
		this->options |= NAND_BBT_SCANNED;
		this->scan_bbt(mtd);
	}
	
	datbuf_tmp = (uint8_t *)kmalloc(mtd->erasesize+0x20, GFP_KERNEL);
	if(datbuf_tmp == NULL)
	{
		ALI_NAND_ERROR("[ERR] %s NO Memory\n", __FUNCTION__);
		return -1;
	}
	datbuf = (uint8_t *)((u32)(datbuf_tmp + 0x1F) & (u32)(~0x1F));

	// 1. check whether bak blk exist(power cut happened).
	ali_nand_search_src_bak_blk(mtd, datbuf, &src_blk, &bak_blk);

	if((src_blk == -1) && (bak_blk == -1))
	{
		ALI_NAND_DEBUG("%s need not recovery\n", __FUNCTION__);
		kfree(datbuf_tmp);
		return 0;
	}
	
	printf("%s do bak blk %d recovery to src blk %d\n", __FUNCTION__, bak_blk , src_blk);
	
	// 2. Read from bak blk.
	retlen = mtd->erasesize;
	offset = (loff_t) ((loff_t)bak_blk << this->bbt_erase_shift);
	ALI_NAND_DEBUG(KERN_DEBUG "read bak blk @ 0x%012llx\n", offset);
	if (nand_read(mtd, offset, &retlen, datbuf))
	{
		ALI_NAND_ERROR("[ERR] %s Read error happen!!\n", __FUNCTION__);
		kfree(datbuf_tmp);
		return -1;
	}
	
	// 3. write back to src blk.
	to = (loff_t) ((loff_t)src_blk << this->bbt_erase_shift);
	ALI_NAND_DEBUG(KERN_DEBUG "write to src blk @ 0x%012llx\n", to);
	memset(&einfo, 0, sizeof(einfo));
	einfo.mtd = mtd;
	einfo.addr = to;
	einfo.len = 1 << this->bbt_erase_shift;
	res = nand_erase_nand(mtd, &einfo, 1);
	if (res < 0)
	{
		kfree(datbuf_tmp);
		return res;
	}
	retlen = mtd->erasesize;
	res = nand_write(mtd, to, &retlen, datbuf);
	
/* 這是包含oob 才這樣寫的
	ops.mode = MTD_OOB_PLACE;
	ops.ooboffs = 0;
	ops.ooblen = 8;//mtd->oobsize;
	ops.datbuf = datbuf;
	ops.oobbuf = oob;
	ops.len = 1 << this->bbt_erase_shift;

	res = mtd->write_oob(mtd, to, &ops);
*/

	if (res < 0)
	{
		ALI_NAND_ERROR("[ERR] %s write error happen!!\n", __FUNCTION__);
		kfree(datbuf_tmp);
		return res;
	}
	kfree(datbuf_tmp);
	ALI_NAND_DEBUG("exit %s\n", __FUNCTION__);
	return res;
}

