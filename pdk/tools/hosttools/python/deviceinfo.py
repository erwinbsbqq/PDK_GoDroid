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
        infile = "deviceinfo.xml"
        outfile = "deviceinfo.bin"
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

	if node.nodeName == "magic":
	    outfile.seek(0,0)
            size = scriptxml.parser_node(node,outfile)  
        else:
            if node.nodeName == "hdmi":
	        outfile.seek(16,0)
	    elif node.nodeName == "firmware":
	        outfile.seek(304,0)
	
            parser_element(node,outfile)
            
    outfile.close()    