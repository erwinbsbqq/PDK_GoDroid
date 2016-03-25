from ctypes import *
import struct
import sys
import bytefp
import scriptxml

class BLOCK(Structure):
    _fields_ = [
        ("block_offset", c_uint),
        ("block_len", c_uint),
        ("media_type", c_uint),
        ("play_time", c_uint),
    ]
	
media_block_count = 0
play_count = 0
play_list = list(range(0))
block_list = []
		
def get_mediatype(mediatype):
    if mediatype == "mpeg2":
        return 11
    elif mediatype == "jpeg":
        return 12    
    elif mediatype == "mkv":
        return 110
		
def parser_media(node,offset,outfile):
    global media_block_count
    global block_list
    
    outfile.seek(offset,0)
    name = node.getElementsByTagName("name")[0]
    duration = node.getElementsByTagName("duration")[0]
    size = scriptxml.parser_node(name,outfile) 
    media_block_count += 1
    block = BLOCK()
    block.block_offset = offset
    block.block_len = size           
    block.media_type = get_mediatype(name.getAttribute("media_type"))
    block.play_time = int(duration.childNodes[0].nodeValue)
    block_list.append(block)   
    
    offset = bytefp.align16B(offset+size)
    return offset    

def parser_loop(node,offset,outfile):
    global media_block_count
    global play_count    
    global play_list
    global block_list 
    
    str_loop_count = node.getAttribute("count")

    if len(str_loop_count) == 0:
        loop_count = 0
    else:
        loop_count = int(str_loop_count)

    loop_list = list(range(0))
    nodelist = node.childNodes
    for n in nodelist:
        if n.nodeName != "media":
            continue
        loop_list.append(media_block_count)
        offset = parser_media(n,offset,outfile)
    
    if loop_count==0:
        loop_count = (64-len(play_list)+len(loop_list)-1)/len(loop_list)
    
    for i in range(0,loop_count):
        for j in range(0,len(loop_list)):
            if len(play_list)<64:
                play_list.append(loop_list[j])
    
    return offset

def parser_body(root,outfile):      
    global media_block_count
    global play_count    
    global play_list
    global block_list   

    offset = 256
    outfile.seek(offset,0)

    nodelist = root.childNodes
    for node in nodelist:
        if node.nodeType != 1:
            continue
            
        if node.nodeName == "media":
            if len(play_list)<64:
                play_list.append(media_block_count)
            offset =  parser_media(node,offset,outfile)           
        elif node.nodeName == "loop":
            offset = parser_loop(node,offset,outfile)
            
    play_count = len(play_list)
                        
def parser_head(root,outfile):
    outfile.seek(0,0)     
    nodelist = root.childNodes
    for node in nodelist:
        if node.nodeType != 1:
            continue            
        size = scriptxml.parser_node(node,outfile)  

    outfile.seek(16,0)            
    bytes = struct.pack("2H",media_block_count,play_count)        
    outfile.write(bytes)
    
    for i in play_list:
        bytes = struct.pack("B",i)  
        outfile.write(bytes)
        
    for i in range(play_count,64):
        bytes = struct.pack("B",0)  
        outfile.write(bytes)
    
    for block in block_list:
        bytes = struct.pack("4I",block.block_offset,
            block.block_len,block.media_type,block.play_time)
        outfile.write(bytes)
            
               
if __name__ == "__main__":
    args = len(sys.argv)
    if args < 2:
        infile = "bootmedia.xml"
        outfile = "bootmedia.bin"
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

    node = root.getElementsByTagName("body")[0]
    parser_body(node,outfile)    

    node = root.getElementsByTagName("head")[0]
    parser_head(node,outfile)    

    outfile.close()