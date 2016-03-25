from ctypes import *
import struct
import sys
import bytes
import bytefp
import scriptxml

class PART(Structure):
    _fields_ = [
        ("name", c_char*32),
        ("file", c_char*256),
        ("fs_type", c_char*16),
        ("flash_type", c_char*16),
        ("offset", c_uint),
        ("size", c_uint),
        ("logicoffset", c_uint),
        ("logicsize", c_uint),
    ]
	
part_list = []		
		
def parser_part(node):
    global part_list

    part = PART()
        
    name = node.getAttribute("name")
    if len(name)==0:
        print "miss part name"
        return        
    part.name = name

    file = node.getAttribute("file")
    if len(name)!=0:       
        part.file = file
    
    offset = node.getElementsByTagName("offset")
    if (len(offset)>0) and (len(offset[0].childNodes)>0):
        part.offset = bytes.str2dec(offset[0].childNodes[0].nodeValue)
    else:
        part.offset = 0xFFFFFFFF
    
    size = node.getElementsByTagName("size")
    if (len(size)==0) or (len(size[0].childNodes)==0):
        print "miss part["+part.name+"] size"
        return    
    part.size = bytes.str2dec(size[0].childNodes[0].nodeValue)
    
    logicoffset = node.getElementsByTagName("logicoffset")
    if (len(logicoffset)>0) and (len(logicoffset[0].childNodes)>0):
        part.logicoffset = bytes.str2dec(logicoffset[0].childNodes[0].nodeValue)

    logicsize = node.getElementsByTagName("logicsize")
    if (len(logicsize)>0) and (len(logicsize[0].childNodes)>0):
        part.logicsize = bytes.str2dec(logicsize[0].childNodes[0].nodeValue)    

    part.fs_type = node.getAttribute("fs_type")   
            
    part_list.append(part)   
       
def part_check():
    offset = 0
    for p in part_list:
        if p.offset==0xFFFFFFFF:
            p.offset = offset
            
        if offset!=p.offset:
            print p.name+" offset is valid"
            print hex(offset)
            print hex(p.offset)
            return -1
            
        offset += p.size
        
    return 0

def genparts(node, outfile):
    nodelist = node.childNodes
    for n in nodelist:
        if n.nodeName != "part":
            continue
        parser_part(n)    
    
    if part_check()<0:
        return -1
    
    idx = 0
    for p in part_list:
        if p.name == "boot":
            continue
        idx += 1
        
        # [PARTITION]
        outfile.write("[PARTITION%d]\n"%(idx))      
        # NAME
        outfile.write("NAME = %s\n"%(p.name))      
        # SIZE
        outfile.write("SIZE = 0x%x\n"%(p.size))      
        # FILE
        outfile.write("FILE = %s\n"%(p.file))      
        # FILE_TYPE
        if len(p.fs_type)>0:
            outfile.write("FILE_TYPE = %s\n"%(p.fs_type))      
        
        outfile.write("\n")
    
    # [PARTITION-COUNT]
    outfile.write("[PARTITION-COUNT]\n")
    # COUNT
    outfile.write("COUNT = %d\n"%(idx))
    # TOTAL-SIZE
    p = part_list[-1]
    outfile.write("TOTAL-SIZE = 0x%x\n"%(p.offset+p.size))

    outfile.write("\n")
    
    return 0

def gen_SYSINI(node, outfile):
    # [SYSINI]
    outfile.write("[%s]\n"%(node.nodeName))
    
    # FlashTable
    s = node.getAttribute("FlashTable")
    if len(s)==0:
        print "miss FlashTable"
        return -1
    outfile.write("FlashTable = %s\n"%(s))

    # ALI_CHIP
    s = node.getAttribute("ALI_CHIP")
    if s==None:
        print "miss ALI_CHIP"
        return -1
    outfile.write("ALI_CHIP = %s\n"%(s))

    outfile.write("\n")
    return 0

def gen_ALI_PRIVATE_PARTITION0(node, outfile):
    # [ALI-PRIVATE-PARTITION0]
    outfile.write("[%s]\n"%(node.nodeName))
    
    # ALI_PRIVATE_RESERVED_BLOCK
    s = node.getAttribute("ALI_PRIVATE_RESERVED_BLOCK")
    if s==None:
        print "miss ALI_PRIVATE_RESERVED_BLOCK"
        return -1
    outfile.write("ALI_PRIVATE_RESERVED_BLOCK = %s\n"%(s))

    # SecondCpu
    s = node.getAttribute("SecondCpu")
    if s==None:
        print "miss SecondCpu"
        return -1
    outfile.write("SecondCpu = %s\n"%(s))

    # SIZE
    s = node.getAttribute("size")
    if s==None:
        print "miss SIZE"
        return -1
    outfile.write("SIZE = %s\n"%(s))
    
    # NF_READ_CLOCK
    s = node.getAttribute("NF_READ_CLOCK")
    if len(s)!=0:
        outfile.write("NF_READ_CLOCK = %s\n"%(s))    

    # NF_WRITE_CLOCK
    s = node.getAttribute("NF_WRITE_CLOCK")
    if len(s)!=0:
        outfile.write("NF_WRITE_CLOCK = %s\n"%(s))    

    outfile.write("\n")
    return 0    

def gen_SYSTEM_START_ADDRESS(node, outfile):
    # [SYSTEM-START-ADDRESS]
    outfile.write("[%s]\n"%(node.nodeName))
    
    # TDS_ADDR
    s = node.getAttribute("TDS_ADDR")
    if s==None:
        print "miss TDS_ADDR"
        return -1
    outfile.write("TDS_ADDR = %s\n"%(s))

    # START_ADDR
    s = node.getAttribute("START_ADDR")
    if s==None:
        print "miss START_ADDR"
        return -1
    outfile.write("START_ADDR = %s\n"%(s))

    outfile.write("\n")
    return 0    

def gen_STARTUP_FILE(node, outfile):
    # [STARTUP-FILE]
    outfile.write("[%s]\n"%(node.nodeName))
    
    # DRAM
    e = node.getElementsByTagName("DRAM")
    if (len(e)==0):
        print "miss DRAM"
        return -1 
    outfile.write("DRAM = %s\n"%(e[0].childNodes[0].nodeValue))

    # UPDATER
    e = node.getElementsByTagName("UPDATER")
    if (len(e)==0):
        print "miss UPDATER"
        return -1 
    outfile.write("UPDATER = %s\n"%(e[0].childNodes[0].nodeValue))

    # LOADER
    e = node.getElementsByTagName("LOADER")
    if (len(e)==0):
        print "miss LOADER"
        return -1 
    outfile.write("LOADER = %s\n"%(e[0].childNodes[0].nodeValue))

    outfile.write("\n")
    return 0    
            
def geninihead(node, outfile):
    # SYSINI
    e = node.getElementsByTagName("SYSINI")
    if (len(e)==0):
        print "miss SYSINI"
        return -1    
    if gen_SYSINI(e[0], outfile)<0:
        return -1
    
    # ALI-PRIVATE-PARTITION0
    e = node.getElementsByTagName("ALI-PRIVATE-PARTITION0")
    if (len(e)==0):
        print "miss ALI-PRIVATE-PARTITION0"
        return -1    
    if gen_ALI_PRIVATE_PARTITION0(e[0], outfile)<0:
        return -1

    # SYSTEM-START-ADDRESS
    e = node.getElementsByTagName("SYSTEM-START-ADDRESS")
    if (len(e)==0):
        print "miss SYSTEM-START-ADDRESS"
        return -1    
    if gen_SYSTEM_START_ADDRESS(e[0], outfile)<0:
        return -1

    # STARTUP-FILE
    e = node.getElementsByTagName("STARTUP-FILE")
    if (len(e)==0):
        print "miss STARTUP-FILE"
        return -1    
    if gen_STARTUP_FILE(e[0], outfile)<0:
        return -1

    return 0                  
                               
if __name__ == "__main__":
    args = len(sys.argv)
    if args < 2:
        infile = "Ali_nand_desc.xml"
    else:
        infile = sys.argv[1]
    
    outfile = open("ALI.ini", "w")
    
    doc = scriptxml.load_xml(infile)
    #scriptxml.print_xml(doc)
    root = scriptxml.get_root(doc)
   
    node = root.getElementsByTagName("ali_ini_head")[0]
    geninihead(node, outfile)

    node = root.getElementsByTagName("part_loop")[0]
    genparts(node, outfile)

    outfile.close()
