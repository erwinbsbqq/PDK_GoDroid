import struct
import bytes
import mac
    
def write_num(type,string,fp):       
    value = bytes.getnum(type,string)    
    fp.write(value)
    return bytes.gettypesize(type)
        
def write_array(type,size,string,fp):
    if string!=None:
        segs = string.split(",")
        l = len(segs)
        if l>size:
            l = size
    else:
        l = 0
        
    for i in range(0,l): 
        write_num(type,segs[i],fp)
        
    for i in range(l,size):
        write_num(type,"0",fp)
        
    return bytes.gettypesize(type)*size        

def write_nums(type,size,string,fp):   
    if size==1:
        return write_num(type,string,fp)
    else:
        return write_array(type,size,string,fp)

def write_string(string,size,fp):  
    value = bytes.getstring(string,size)    
    fp.write(value)
    return size 

def write_mac(string,fp):
    seg_list = mac.str2mac(string)
    seg_list = mac.standard_chksum_gen(seg_list,8)
    
    for i in range(0,8):
        value = struct.pack("B",seg_list[i]);
        fp.write(value)

    return 8

def write_file(filename,size,fp):
    if filename == None:
        file_len = 0
    else:
    	#check open file safe
	try:
        	infile = open(filename, "rb")
	except IOError:
		print "open "+filename+"fail ,file does not exist!!!!!!!!!!!!!!!!!!!!!!"
		file_len = 0
		if file_len < size:
       			fp.write((size-file_len)*"\0")
            
    		return file_len
	
        infile.seek(0,2)
        file_len = infile.tell()    
        infile.seek(0,0)
        c = infile.read(file_len)
        fp.write(c)
        
        infile.close()
    
    if file_len < size:
        fp.write((size-file_len)*"\0")
            
    return file_len

def write_bytefp(type,size,string,fp):
    if type == "string":
        return write_string(string,bytes.str2dec(size),fp)
    elif type == "mac":
        return write_mac(string,fp)
    elif type == "file":
        return write_file(string,bytes.str2dec(size),fp)
    else:
        return write_nums(type,bytes.str2dec(size),string,fp)
    

def align16B(num):
    return (num+0xf)&0xfffffff0