import struct

def str2dec(string_num):
    base = 10
    s = string_num.upper()
    if s.startswith("0X"):
        base = 16

    return int(string_num, base)

def str_uint(string):
    value = str2dec(string)
    bytes = struct.pack("I",value)    
    return bytes

def str_int(string):
    value = str2dec(string)
    bytes = struct.pack("i",value)    
    return bytes

def str_ushort(string):
    value = str2dec(string)
    bytes = struct.pack("H",value)    
    return bytes

def str_short(string):
    value = str2dec(string)
    bytes = struct.pack("h",value)    
    return bytes

def str_ubyte(string):
    value = str2dec(string)
    bytes = struct.pack("B",value)    
    return bytes

def str_byte(string):
    value = str2dec(string)
    bytes = struct.pack("b",value)    
    return bytes

def str_string(string,size):
    write_len = len(string)
    if write_len > size:
        write_len = size-1
    
    write_str = string[0:write_len]    
    stuff_len = size - write_len
    stutf_str = stuff_len * "\0"        
    fmt = str(write_len)+"s"+str(stuff_len)+"s"
            
    bytes = struct.pack(fmt,write_str.encode("utf-8"),stutf_str.encode("utf-8"))    
    return bytes  

def getstring(string,size):
    if string==None:
        s = "\0"
    else:
        s = string
        
    return str_string(s,size)
                   
def getnum(type,string):
    if string==None:
        s = "0"
    else:
        s = string
        
    if type == "uint":
        return str_uint(s)
    elif type == "int":
        return str_int(s)
    elif type == "ushort":
        return str_ushort(s)
    elif type == "short":
        return str_short(s)
    elif type == "ubyte":
        return str_ubyte(s)
    elif type == "byte":
        return str_byte(s)        
        
def gettypesize(type):        
    if type == "uint":
        return 4
    elif type == "int":
        return 4
    elif type == "ushort":
        return 2
    elif type == "short":
        return 2
    elif type == "ubyte":
        return 1
    elif type == "byte":
        return 1
