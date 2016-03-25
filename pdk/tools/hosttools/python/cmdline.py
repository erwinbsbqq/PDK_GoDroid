from ctypes import *
import struct
import sys
import bytes
import bytefp
import scriptxml

class PART(Structure):
    _fields_ = [
        ("name", c_char*32),
        ("fs_type", c_char*16),
        ("fs_flag", c_char*16),
        ("flash_type", c_char*16),
        ("offset", c_uint),
        ("size", c_uint),
        ("logicoffset", c_uint),
        ("logicsize", c_uint),
        ("readonly", c_ubyte),
        ("locked", c_ubyte),
        ("hidden", c_ubyte),       
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
    part.fs_flag = node.getAttribute("fs_flag")   
    
    flag = node.getElementsByTagName("flag")
    if (len(flag)>0) and (len(flag[0].childNodes)>0):
        s = flag[0].childNodes[0].nodeValue
        flags = s.split(",")
        for f in flags:
            if f=="readonly":
                part.readonly = 1
            elif f=="locked":
                part.locked = 1
            elif f=="hidden":
                part.hidden = 1
        
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

def hex2mem(value):
    if value%0x100000 > 0:
        return str(value>>10)+"K"
    else:
        return str(value>>20)+"M"

def parser_mdtdef(node):
    nodelist = node.childNodes
    for n in nodelist:
        if n.nodeName != "part":
            continue
        parser_part(n)    
    
    if part_check()<0:
        return
        
    flash_type = node.getAttribute("flash_type")
    if flash_type=="nand":
        mtddef = "mtdparts=ali_nand:"
    elif flash_type=="emmc":
        mtddef = "blkdevparts=ali_mmc:"

    mtddef_rcv = mtddef
    
    i = 0
    j = 0
    for p in part_list:
        partdef = hex2mem(p.size)
        if p.offset>=0:
            partdef += "@"+hex2mem(p.offset)
        partdef +="("+p.name+")"
        if p.readonly>0:
            partdef += "ro"
        if p.locked>0:
            partdef += "lk"
    
        if p.hidden==0:
            if i>0:
                mtddef += ","
            i += 1
            mtddef += partdef
            
        if j>0:
            mtddef_rcv += ","
        j += 1
        mtddef_rcv += partdef
    
    return (mtddef,mtddef_rcv)
 
def parser_rootfs(rootfs, hiddenChk):    
    root = ""
    mtdid = 0
    for p in part_list:
        if p.name == rootfs:        
            fs_flag = p.fs_flag
            if len(fs_flag)==0:
                fs_flag = "rw"
                
            if p.fs_type == "ubifs":
                root = "ubi.mtd=%d root=ubi0:rootfs rootfstype=ubifs"%(mtdid)
            elif p.fs_type == "yaffs2":
                root = "root=/dev/mtdblock%d rootfstype=yaffs2 rootflags=inband-tags"%(mtdid)
            elif p.fs_type == "cramfs":
                root = "root=/dev/mtdblock%d rootfstype=cramfs"%(mtdid)
            else:
                root = "rootfstype=initrd"
        
            root += " "+fs_flag
    
        if (hiddenChk==0 or p.hidden==0):
            mtdid += 1
    return root

def parser_nfs(node):
    # nfs_enable: 0--disabel; 1--enable
    nfs_enable = node.getAttribute("NFS_enable")
    if len(nfs_enable)==0:
        return ""        
    if (bytes.str2dec(nfs_enable)==0):
        return ""   

    # server
    server = node.getElementsByTagName("server")[0]
    if (server==None):
        print "%s miss server"%(node.nodeName)
        return ""
    
    # server IP
    server_ip = server.getAttribute("IP")
    
    # server path
    server_path = server.getAttribute("path")
    
    nfsroot = "root=/dev/nfs rw nfsroot=%s:%s,rsize=1024,wsize=1024 init=/init"%(server_ip,server_path)
 
    # DHCP_enable
    # 0x00: DHCP disable, DNS enable, must config board_net_config
    # 0x01: DHCP enable, need not config board_net_config
    dhcp_enable = node.getAttribute("DHCP_enable")
    if len(dhcp_enable)==0:
        return ""        
        
    if bytes.str2dec(dhcp_enable)==0:        
        board = node.getElementsByTagName("board")[0]
        if (board==None):
            print "%s miss board"%(node.nodeName)
            return ""
            
        # board IP
        board_ip = board.getAttribute("IP")
    
        # gateway
        gateway = board.getAttribute("gateway")

        # netmask
        netmask = board.getAttribute("netmask")
        
        nfsroot += " ip=%s:%s:%s:%s::eth0:off"%(board_ip,server_ip,gateway,netmask)
    else:
        nfsroot += " ip=:::::eth0:dhcp"
    
    return nfsroot.encode("utf-8")
                       
if __name__ == "__main__":
    args = len(sys.argv)
    if args <= 1:
        infile = "Ali_nand_desc.xml"
    else:
        infile = sys.argv[1]            

    rootmtd = "rootfs"
    rootmtd_rsv = "recovery"
    for arg in sys.argv:
        if arg == "-r":
            rootmtd = sys.argv[sys.argv.index(arg)+1]
        elif arg == "-v":
            rootmtd_rsv = sys.argv[sys.argv.index(arg)+1]     
    
    doc = scriptxml.load_xml(infile)
    #scriptxml.print_xml(doc)
    root = scriptxml.get_root(doc)
   
    node = root.getElementsByTagName("part_loop")[0]
    mtddef,mtddef_rcv=parser_mdtdef(node)    
    
    node = root.getElementsByTagName("NFS_config")[0]
    rootfs_nfs = parser_nfs(node)   
    
    rootfs = parser_rootfs(rootmtd,1)
    rootfs_rsv = parser_rootfs(rootmtd_rsv,0)  
    
    if len(rootfs_nfs)==0:
        cmdline = rootfs
        cmdline_rcv = rootfs_rsv
    else:
        cmdline = rootfs_nfs
        cmdline_rcv = rootfs_nfs
    
    node = root.getElementsByTagName("cmdline")
    if (len(node)>0) and (len(node[0].childNodes)>0):
        cmd = node[0].childNodes[0].nodeValue.encode("utf-8")
        cmdline += " "+cmd
        cmdline_rcv += " "+cmd
    
    cmdline += " "+mtddef
    cmdline_rcv += " "+mtddef_rcv
    
    outfile = "cmdline"        
    cmdfile = open(outfile, "w")
    cmdfile.write(cmdline)
    cmdfile.close 
    
    outfile_rcv = "cmdline_rcv"
    cmdfile_rcv = open(outfile_rcv, "w")
    cmdfile_rcv.write(cmdline_rcv)
    cmdfile_rcv.close 
