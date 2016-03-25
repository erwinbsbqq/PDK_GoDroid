import sys
import bytes
import bytefp
import scriptxml

tvsystem = {
    "PAL": "0x0",
    "NTSC": "0x1",
    "PAL_M": "0x2",
    "PAL_N": "0x3",
    "PAL_60": "0x4",
    "NTSC_443": "0x5",
    "SECAM": "0x6",
    "MAC": "0x7",
    "LINE_720_25": "0x8",
    "LINE_720_30": "0x9",
    "LINE_1080_25": "0xA",
    "LINE_1080_30": "0xB",
    
    "LINE_1080_50": "0xC",
    "LINE_1080_60": "0xD",
    "LINE_1080_24": "0xE",
    "LINE_1152_ASS": "0xF",
    "LINE_1080_ASS": "0x10",
    "PAL_NC": "0x11",    
    
    "LINE_576P_50_VESA": "0x12",
    "LINE_720P_60_VESA": "0x13",
    "LINE_1080P_60_VESA": "0x14",       
}

progressive = {
    "FALSE": "0x0",
    "TRUE": "0x1",
}

tvratio = {
    "TV_ASPECT_RATIO_43": "0x0",
    "TV_ASPECT_RATIO_169": "0x1",
    "TV_ASPECT_RATIO_AUTO": "0x2",
}

displaymode = {
    "DISPLAY_MODE_NORMAL": "0x0",
    "DISPLAY_MODE_LETTERBOX": "0x1",
    "DISPLAY_MODE_PANSCAN": "0x2",
}

scartout = {
    "SCART_CVBS": "0x0",
    "SCART_RGB": "0x1",
    "SCART_SVIDEO": "0x2",
    "SCART_YUV": "0x3",
}
        
vdac = {
    "VDAC_CVBS": "0x0",
    "VDAC_SVIDEO_Y": "0x4",
    "VDAC_SVIDEO_C": "0x5",
    "VDAC_YUV_Y": "0x8",
    "VDAC_YUV_U": "0x9",
    "VDAC_YUV_V": "0xA",
    "VDAC_RGB_R": "0xC",
    "VDAC_RGB_G": "0xD",
    "VDAC_RGB_B": "0xE",
    "VDAC_SCVBS": "010",
    "VDAC_SSV_Y": "014",
    "VDAC_SSV_C": "0x15",
    "VDAC_NULL": "0xFF",
}

videoformat = {
    "SYS_DIGITAL_FMT_BY_EDID": "0x0",
    "SYS_DIGITAL_FMT_RGB": "0x1",
    "SYS_DIGITAL_FMT_RGB_EXPD": "0x2",
    "SYS_DIGITAL_FMT_YCBCR_444": "0x3",
    "SYS_DIGITAL_FMT_YCBCR_422": "0x4",
}

audioout = {
    "SYS_DIGITAL_AUD_BS": "0x0",
    "SYS_DIGITAL_AUD_LPCM": "0x1",
    "SYS_DIGITAL_AUD_AUTO": "0x2",
}

   
def get_tvsystem(node, outfile):
    type = node.getAttribute("type")
    size = "1"
    str = node.childNodes[0].nodeValue   

    if str in tvsystem:
        string = tvsystem[str]
    else:
        print "unknown tvsystem: "+str
        return
                        
    bytefp.write_bytefp(type, size, string, outfile)

def get_progressive(node, outfile):
    type = node.getAttribute("type")
    size = "1"
    str = node.childNodes[0].nodeValue
    
    if str in progressive:
        string = progressive[str]
    else:
        print "unknown progressive: "+str
        return

    bytefp.write_bytefp(type, size, string, outfile)

        
def get_tvratio(node, outfile):
    type = node.getAttribute("type")
    size = "1"
    str = node.childNodes[0].nodeValue
    
    if str in tvratio:
        string = tvratio[str]
    else:
        print "unknown tvratio: "+str
        return

    bytefp.write_bytefp(type, size, string, outfile)

def get_displaymode(node, outfile):
    type = node.getAttribute("type")
    size = "1"
    str = node.childNodes[0].nodeValue
    
    if str in displaymode:
        string = displaymode[str]
    else:
        print "unknown displaymode: "+str
        return

    bytefp.write_bytefp(type, size, string, outfile)

def get_scartout(node, outfile):
    type = node.getAttribute("type")
    size = "1"
    str = node.childNodes[0].nodeValue
    
    if str in scartout:
        string = scartout[str]
    else:
        print "unknown scartout: "+str
        return

    bytefp.write_bytefp(type, size, string, outfile)
    
def get_vdacout(node, outfile):
    type = node.getAttribute("type")
    size = node.getAttribute("size")
    segs = node.childNodes[0].nodeValue.split(",")
    
    if bytes.str2dec(size)!=len(segs):
        print "vdacout invalid"
        return
        
    for str in segs:
        if str in vdac:
            string = vdac[str]
        else:
            print "unknown vdac: "+str
            return

        bytefp.write_bytefp(type, "1", string, outfile)

def get_videoformat(node, outfile):
    type = node.getAttribute("type")
    size = "1"
    str = node.childNodes[0].nodeValue
    
    if str in videoformat:
        string = videoformat[str]
    else:
        print "unknown videoformat: "+str
        return

    bytefp.write_bytefp(type, size, string, outfile)
    
def get_audioout(node, outfile):
    type = node.getAttribute("type")
    size = "1"
    str = node.childNodes[0].nodeValue
    
    if str in audioout:
        string = audioout[str]
    else:
        print "unknown audioout: "+str
        return    
    
    bytefp.write_bytefp(type, size, string, outfile)
                
avfunc = {
    "tvSystem": get_tvsystem,
    "progressive": get_progressive,
    "tv_ratio": get_tvratio,
    "display_mode": get_displaymode,
    "scart_out": get_scartout,
    "vdac_out": get_vdacout,
    "video_format": get_videoformat,
    "audio_output": get_audioout,
}

def parser_avinfo(root, outfile):
    nodelist = root.childNodes
    for node in nodelist:
        if node.nodeType != 1:
            continue            

	#print node.nodeName + ":" 
	#print node.childNodes[0].nodeValue
	
        if node.nodeName in avfunc:
            avfunc.get(node.nodeName)(node,outfile)
        else:            
            scriptxml.parser_node(node,outfile)  
def parser_element(root,outfile):  
    
    nodelist = root.childNodes
    for node in nodelist:
        if node.nodeType != 1:
            continue            
        size = scriptxml.parser_node(node,outfile)  
                            
if __name__ == "__main__":
    args = len(sys.argv)
    if args < 2:
        infile = "baseparmas.xml"
        outfile = "baseparmas.bin"
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
            
        if node.nodeName == "avinfo":
            parser_avinfo(node, outfile)
        elif node.nodeName == "sysinfo":
            parser_element(node,outfile)	            
        else:
            scriptxml.parser_node(node,outfile)
            
    outfile.close()    