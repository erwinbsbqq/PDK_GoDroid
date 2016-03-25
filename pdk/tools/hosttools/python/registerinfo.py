import sys
import bytes
import bytefp
import scriptxml

def parser_register(node,outfile):
    nodelist = node.childNodes
    for n in nodelist:
        if n.nodeType != 1:
            continue      
           
        scriptxml.parser_node(n,outfile)
        
                            
if __name__ == "__main__":
    args = len(sys.argv)
    if args < 2:
        infile = "registerinfo.xml"
        outfile = "registerinfo.bin"
    elif args == 2:
        infile = sys.argv[1]
        outfile = infile.replace(".xml",".bin")
    else:
        infile = sys.argv[1]
        outfile = sys.argv[2]  
        
    doc = scriptxml.load_xml(infile)
    #scriptxml.print_xml(doc)
    root = scriptxml.get_root(doc)
    
    e = root.getElementsByTagName("MAX_REGISTER_NUM")
    if (len(e)==0) or (len(e[0].childNodes)==0):
        print "miss MAX_REGISTER_NUM"
        
    max_num = bytes.str2dec(e[0].childNodes[0].nodeValue)

    outfile = open(outfile, "wb")
    
    reglist = root.getElementsByTagName("register")    
    valid_count = len(reglist)    
    
    if (valid_count>max_num):
        print "register count(%d) is larger than max num(64)"%(valid_count)       
        
    bytefp.write_num("uint","%d"%(valid_count),outfile)
    
    for register in reglist:
        parser_register(register,outfile)          
            
    outfile.close()    