import sys
import scriptxml
                            
if __name__ == "__main__":
    args = len(sys.argv)
    if args < 2:
        infile = "memmapinfo.xml"
        outfile = "memmapinfo.bin"
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
            
        scriptxml.parser_node(node,outfile)
            
    outfile.close()    