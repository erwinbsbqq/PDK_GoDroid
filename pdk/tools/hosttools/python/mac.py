import re 
import ctypes

def str2mac(string):
    reg1 = "[0-9a-fA-F]{2}"
    reg = reg1+":"+reg1+":"+reg1+":"+reg1+":"+reg1+":"+reg1
    mac = re.match(reg,string)
    if mac == None:
        print "invalid mac"
        return None
    
    segs = string.split(":")        
    seg_list = list(range(0))
        
    for i in range(0, 6):
        seg = int(segs[i], 16)
        seg_list.append(seg)

    seg_list.append(0);
    seg_list.append(0);
    
#    print seg_list
    return seg_list    

def standard_chksum_gen(seg_list, slen):
    acc = 0
    
    for i in range(0, 8, 2):
        acc += seg_list[i]<<8 | seg_list[i+1]

    while (acc>>16) != 0:
        acc = (acc & 0xffff) + (acc >> 16)
       
    seg_list[6] = ctypes.c_ubyte(~((acc & 0xff00) >> 8)).value
    seg_list[7] = ctypes.c_ubyte(~(acc & 0xff)).value
    
#    print seg_list
    return seg_list    
    