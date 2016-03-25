import sys
import bytes
import bytefp
import scriptxml

level = {
    "CB_LEVEL_PAL_SD": "0x85",
    "CB_LEVEL_NTSC_SD": "0x85",
    "CR_LEVEL_PAL_SD": "0x55",
    "CR_LEVEL_NTSC_SD": "0x55",
    "LUMA_LEVEL_PAL_SD": "0x51",
    "LUMA_LEVEL_NTSC_SD": "0x4e",
    "CHRMA_LEVEL_PAL_SD": "0x02",
    "CHRMA_LEVEL_NTSC_SD": "0x08",
    "FREQ_RESPONSE_PAL_SD": "0x102",
    "FREQ_RESPONSE_NTSC_SD": "0x102",
}

#tvencoder adjustable register define
TVE_ADJ_register = {
    "TVE_ADJ_COMPOSITE_Y_DELAY": "0",
    "TVE_ADJ_COMPOSITE_C_DELAY": "1",
    "TVE_ADJ_COMPONENT_Y_DELAY": "2",
    "TVE_ADJ_COMPONENT_CB_DELAY": "3",
    "TVE_ADJ_COMPONENT_CR_DELAY": "4",
    "TVE_ADJ_BURST_LEVEL_ENABLE": "5",
    "TVE_ADJ_BURST_CB_LEVEL": "6",
    "TVE_ADJ_BURST_CR_LEVEL": "7",
    "TVE_ADJ_COMPOSITE_LUMA_LEVEL": "8",
    "TVE_ADJ_COMPOSITE_CHRMA_LEVEL": "9",
    "TVE_ADJ_PHASE_COMPENSATION": "10",
    "TVE_ADJ_VIDEO_FREQ_RESPONSE": "11",

#secam adjust value
    "TVE_ADJ_SECAM_PRE_COEFFA3A2": "12",
    "TVE_ADJ_SECAM_PRE_COEFFB1A4": "13",
    "TVE_ADJ_SECAM_PRE_COEFFB3B2": "14",
    "TVE_ADJ_SECAM_F0CB_CENTER": "15",
    "TVE_ADJ_SECAM_F0CR_CENTER": "16",
    "TVE_ADJ_SECAM_FM_KCBCR_AJUST": "17",
    "TVE_ADJ_SECAM_CONTROL": "18",
    "TVE_ADJ_SECAM_NOTCH_COEFB1": "19",
    "TVE_ADJ_SECAM_NOTCH_COEFB2B3": "20",
    "TVE_ADJ_SECAM_NOTCH_COEFA2A3": "21",
    "TVE_ADJ_VIDEO_DAC_FS": "22",
    "TVE_ADJ_C_ROUND_PAR": "23",
}

#advance tvencoder adjustable register define
TVE_ADJ_ADV_register = {
    "TVE_ADJ_ADV_PEDESTAL_ONOFF": "0",
    "TVE_ADJ_ADV_COMPONENT_LUM_LEVEL": "1",
    "TVE_ADJ_ADV_COMPONENT_CHRMA_LEVEL": "2",
    "TVE_ADJ_ADV_COMPONENT_PEDESTAL_LEVEL": "3",
    "TVE_ADJ_ADV_COMPONENT_SYNC_LEVEL": "4",
    "TVE_ADJ_ADV_RGB_R_LEVEL": "5",
    "TVE_ADJ_ADV_RGB_G_LEVEL": "6",
    "TVE_ADJ_ADV_RGB_B_LEVEL": "7",
    "TVE_ADJ_ADV_COMPOSITE_PEDESTAL_LEVEL": "8",
    "TVE_ADJ_ADV_COMPOSITE_SYNC_LEVEL": "9",
	"TVE_ADJ_ADV_PLUG_OUT_EN": "10",
	"TVE_ADJ_ADV_PLUG_DETECT_LINE_CNT_SD": "11",
	"TVE_ADJ_ADV_PLUG_DETECT_AMP_ADJUST_SD": "12",
}

eTVE_ADJ_FILED = {
    "TVE_COMPOSITE_Y_DELAY": "0",
    "TVE_COMPOSITE_C_DELAY": "1",
    "TVE_COMPOSITE_LUMA_LEVEL": "2",
    "TVE_COMPOSITE_CHRMA_LEVEL": "3",
    "TVE_COMPOSITE_SYNC_DELAY": "4",
    "TVE_COMPOSITE_SYNC_LEVEL": "5",
    "TVE_COMPOSITE_FILTER_C_ENALBE": "6",
    "TVE_COMPOSITE_FILTER_Y_ENALBE": "7",
    "TVE_COMPOSITE_PEDESTAL_LEVEL": "8",
    "TVE_COMPONENT_IS_PAL": "9",
    
    "TVE_COMPONENT_PAL_MODE": "10",    
    "TVE_COMPONENT_ALL_SMOOTH_ENABLE": "11",
    "TVE_COMPONENT_BTB_ENALBE": "12",
    "TVE_COMPONENT_INSERT0_ONOFF": "13",
    "TVE_COMPONENT_DAC_UPSAMPLEN": "14",
    "TVE_COMPONENT_Y_DELAY": "15",
    "TVE_COMPONENT_CB_DELAY": "16",
    "TVE_COMPONENT_CR_DELAY": "17",
    "TVE_COMPONENT_LUM_LEVEL": "18",
    "TVE_COMPONENT_CHRMA_LEVEL": "19",
    
    "TVE_COMPONENT_PEDESTAL_LEVEL": "20",    
    "TVE_COMPONENT_UV_SYNC_ONOFF": "21",
    "TVE_COMPONENT_SYNC_DELAY": "22",
    "TVE_COMPONENT_SYNC_LEVEL": "23",    
    "TVE_COMPONENT_R_SYNC_ONOFF": "24",
    "TVE_COMPONENT_G_SYNC_ONOFF": "25",
    "TVE_COMPONENT_B_SYNC_ONOFF": "26",
    "TVE_COMPONENT_RGB_R_LEVEL": "27",
    "TVE_COMPONENT_RGB_G_LEVEL": "28",
    "TVE_COMPONENT_RGB_B_LEVEL": "29",
    
    "TVE_COMPONENT_FILTER_Y_ENALBE": "30",        
    "TVE_COMPONENT_FILTER_C_ENALBE": "31",
    "TVE_COMPONENT_PEDESTAL_ONOFF": "32",
    "TVE_COMPONENT_PED_RGB_YPBPR_ENABLE": "33",    
    "TVE_COMPONENT_PED_ADJUST": "34",
    "TVE_COMPONENT_G2Y": "35",
    "TVE_COMPONENT_G2U": "36",
    "TVE_COMPONENT_G2V": "37",
    "TVE_COMPONENT_B2U": "38",
    "TVE_COMPONENT_R2V": "39",
    
    "TVE_BURST_POS_ENABLE": "40",    
    "TVE_BURST_LEVEL_ENABLE": "41",
    "TVE_BURST_CB_LEVEL": "42",
    "TVE_BURST_CR_LEVEL": "43",    
    "TVE_BURST_START_POS": "44",
    "TVE_BURST_END_POS": "45",
    "TVE_BURST_SET_FREQ_MODE": "46",
    "TVE_BURST_FREQ_SIGN": "47",
    "TVE_BURST_PHASE_COMPENSATION": "48",
    "TVE_BURST_FREQ_RESPONSE": "49",    

    "TVE_ASYNC_FIFO": "50",    
    "TVE_CAV_SYNC_HIGH": "51",
    "TVE_SYNC_HIGH_WIDTH": "52",
    "TVE_SYNC_LOW_WIDTH": "53",    
    "TVE_VIDEO_DAC_FS": "54",
    "TVE_SECAM_PRE_COEFFA3A2": "55",
    "TVE_SECAM_PRE_COEFFB1A4": "56",
    "TVE_SECAM_PRE_COEFFB3B2": "57",
    "TVE_SECAM_F0CB_CENTER": "58",
    "TVE_SECAM_F0CR_CENTER": "59",    

    "TVE_SECAM_FM_KCBCR_AJUST": "60",    
    "TVE_SECAM_CONTROL": "61",
    "TVE_SECAM_NOTCH_COEFB1": "62",
    "TVE_SECAM_NOTCH_COEFB2B3": "63",    
    "TVE_SECAM_NOTCH_COEFA2A3": "64",
    "TVE_COMPONENT_PLUG_OUT_EN": "65",
    "TVE_COMPONENT_PLUG_DETECT_LINE_CNT_HD": "66",
    "TVE_CB_CR_INSERT_SW": "67",
    "TVE_VBI_LINE21_EN": "68",
    "TVE_COMPONENT_PLUG_DETECT_AMP_ADJUST_HD": "69",
    
    "TVE_SCART_PLUG_DETECT_LINE_CNT_HD": "70",
    "TVE_SCART_PLUG_DETECT_AMP_ADJUST_HD": "71",
    "TVE_COMPONENT_PLUG_DETECT_FRAME_COUNT": "72",
    "TVE_COMPONENT_PLUG_DETECT_VCOUNT": "73",
    "TVE_COMPONENT_VDAC_ENBUF": "74",
    "TVE_ADJ_FIELD_NUM": "75",
}

eTV_SYS = {
    "SYS_576I": "0",
    "SYS_480I": "1",
    "SYS_576P": "2",
    "SYS_480P": "3",
    "SYS_720P_50": "4",
    "SYS_720P_60": "5",
    "SYS_1080I_25": "6",
    "SYS_1080I_30": "7",
    "SYS_1080P_24": "8",
    "SYS_1080P_25": "9",
    
    "SYS_1080P_30": "10",    
    "SYS_1152I_25": "11",
    "SYS_1080IASS": "12",
    "SYS_1080P_50": "13",
    "SYS_1080P_60": "14",
    "TVE_SYS_NUM": "15",
}

SYS_LINE = {
    "SYS_525_LINE": "0",
    "SYS_625_LINE": "1",
}

def parser_tve_adjust_item(item, outfile):
    # index
    str = item.getAttribute("index")    
    if str in eTVE_ADJ_FILED:
        index = eTVE_ADJ_FILED[str]
    else:
        print "unknown eTVE_ADJ_FILED: "+str
        return    

    bytefp.write_bytefp("uint", "1", index, outfile)  
    
    # value
    value = item.getAttribute("value")    
    if len(value)==0:
        print "miss value"
        return
    
    bytefp.write_bytefp("uint", "1", value, outfile)          

def parser_tve_adjust_table(table, outfile, item_size, item_count):
    # sys_index
    str = table.getAttribute("sys_index")    
    if str in eTV_SYS:
        sys_index = eTV_SYS[str]
    else:
        print "unknown eTV_SYS: "+str
        return
        
    bytefp.write_bytefp("uint", "1", sys_index, outfile)  

    # field_info
    itemlist = table.getElementsByTagName("item")
    for item in itemlist:
        parser_tve_adjust_item(item, outfile)
        
    if len(itemlist)<item_count:
        stufflen = (item_count-len(itemlist))*item_size
        outfile.write(stufflen*"\0")
    
    
def parser_tve_adjust_table_info(node, outfile):
    str = node.getAttribute("table_size")
    table_size = bytes.str2dec(str)

    str = node.getAttribute("table_count")
    table_count = bytes.str2dec(str)

    str = node.getAttribute("item_size")
    item_size = bytes.str2dec(str)

    str = node.getAttribute("item_count")
    item_count = bytes.str2dec(str)
    
    tablelist = node.getElementsByTagName("table")
    for table in tablelist:
        parser_tve_adjust_table(table, outfile, item_size, item_count)
        
    if len(tablelist)<table_count:
        stufflen = (table_count-len(tablelist))*table_size
        outfile.write(stufflen*"\0")        

def parser_sd_tve_adj_item(item, outfile):
    # type
    str = item.getAttribute("type")    
    if str in TVE_ADJ_register:
        type = TVE_ADJ_register[str]
    else:
        print "unknown TVE_ADJ_register: "+str
        return    

    bytefp.write_bytefp("ubyte", "1", type, outfile)  

    # sys
    str = item.getAttribute("sys")    
    if str in SYS_LINE:
        sys = SYS_LINE[str]
    else:
        print "unknown SYS_LINE: "+str
        return    

    bytefp.write_bytefp("ubyte", "1", sys, outfile)  

    # stuff 2 bytes
    outfile.write(2*"\0")
        
    # value
    str = item.getAttribute("value")    
    if str in level:
        value = level[str]
    else:
        value = str
            
    if len(value)==0:
        print "miss value"
        return
    
    bytefp.write_bytefp("uint", "1", value, outfile)        
    
def parser_sd_tve_adj_table(node, outfile):
    str = node.getAttribute("item_size")
    item_size = bytes.str2dec(str)

    str = node.getAttribute("item_count")
    item_count = bytes.str2dec(str)

    itemlist = node.getElementsByTagName("item")
    for item in itemlist:
        parser_sd_tve_adj_item(item, outfile)
        
    if len(itemlist)<item_count:
        stufflen = (item_count-len(itemlist))*item_size
        outfile.write(stufflen*"\0")

def parser_sd_tve_adv_adj_item(item, outfile):
    # type
    str = item.getAttribute("type")    
    if str in TVE_ADJ_ADV_register:
        type = TVE_ADJ_ADV_register[str]
    else:
        print "unknown TVE_ADJ_ADV_register: "+str
        return    

    bytefp.write_bytefp("ubyte", "1", type, outfile)  

    # sys
    str = item.getAttribute("sys")    
    if str in SYS_LINE:
        sys = SYS_LINE[str]
    else:
        print "unknown SYS_LINE: "+str
        return    

    bytefp.write_bytefp("ubyte", "1", sys, outfile)  

    # stuff 2 bytes
    outfile.write(2*"\0")
        
    # value
    str = item.getAttribute("value")    
    if str in level:
        value = level[str]
    else:
        value = str
            
    if len(value)==0:
        print "miss value"
        return
    
    bytefp.write_bytefp("uint", "1", value, outfile)        
    
def parser_sd_tve_adv_adj_table(node, outfile):
    str = node.getAttribute("item_size")
    item_size = bytes.str2dec(str)

    str = node.getAttribute("item_count")
    item_count = bytes.str2dec(str)

    itemlist = node.getElementsByTagName("item")
    for item in itemlist:
        parser_sd_tve_adv_adj_item(item, outfile)
        
    if len(itemlist)<item_count:
        stufflen = (item_count-len(itemlist))*item_size
        outfile.write(stufflen*"\0")
                                            
if __name__ == "__main__":
    args = len(sys.argv)
    if args < 2:
        infile = "tveconfig.xml"
        outfile = "tveconfig.bin"
    elif args == 2:
        infile = sys.argv[1]
        outfile = infile.replace(".xml",".bin")
    else:
        infile = sys.argv[1]
        outfile = sys.argv[2]  
        
    doc = scriptxml.load_xml(infile)
    #scriptxml.print_xml(doc)
    root = scriptxml.get_root(doc)
    
    outfile = open(outfile, "wb")
    
    nodelist = root.childNodes
    for node in nodelist:
        if node.nodeType != 1:
            continue      
            
        if node.nodeName == "tve_adjust_table_info":
            parser_tve_adjust_table_info(node, outfile)
        elif node.nodeName == "sd_tve_adj_table_info":
            parser_sd_tve_adj_table(node, outfile)
        elif node.nodeName == "sd_tve_adv_adj_table_info":
            parser_sd_tve_adv_adj_table(node, outfile)            
        else:
            scriptxml.parser_node(node,outfile)
            
    outfile.close()    