import struct
import sys
import bytefp
import scriptxml
    
def parser_element(root,outfile):  
    
    nodelist = root.childNodes
    for node in nodelist:
        if node.nodeType != 1:
            continue            
        size = scriptxml.parser_node(node,outfile)  
                            
if __name__ == "__main__":
    args = len(sys.argv)
    if args < 2:
        infile = "bootargs.xml"
        outfile = "bootargs.bin"
    elif args == 2:
        infile = sys.argv[1]
        outfile = infile.replace(".xml",".bin")
    else:
        infile = sys.argv[1]
        outfile = sys.argv[2]  
        
    doc = scriptxml.load_xml(infile)
    # scriptxml.print_xml(doc)
    root = scriptxml.get_root(doc)
    
    outfile = open(outfile, "wb")
    
    offset = 0
    nodelist = root.childNodes
    for node in nodelist:
        if node.nodeType != 1:
            continue
        if node.nodeName!="magic":
            outfile.seek(offset+4,0)
	size = scriptxml.parser_node(node,outfile)  
        if node.nodeName=="magic":	
     	    #offset += 0x2000-12
     	    offset += 16
        else:
	    value = struct.pack("i",size) 
	    outfile.seek(offset,0)
	    outfile.write(value)
	    outfile.seek(0,2)
	    offset = outfile.tell()
            
    outfile.close()    