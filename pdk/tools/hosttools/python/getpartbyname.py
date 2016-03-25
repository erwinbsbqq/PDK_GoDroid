from  xml.dom import  minidom
import sys

def findpart(node,partname):
    idx = -1
    partlist = node.getElementsByTagName("part")
    for part in partlist:
        name = part.getAttribute("name")
        if name==partname:
            idx = partlist.index(part)
            break

    return idx
                       
if __name__ == "__main__":
    args = len(sys.argv)
    if args < 3:
        print "format error: getpartbyname Ali_nand_desc.xml <part_name>"
        sys.exit(-1)
    
    infile = sys.argv[1]
    partname = sys.argv[2]
    
    doc = minidom.parse(infile)
    #print doc.toxml("UTF-8")
    root = doc.documentElement
   
    loop = root.getElementsByTagName("part_loop")[0]
    partidx = findpart(loop, partname)
    
    sys.exit(partidx)