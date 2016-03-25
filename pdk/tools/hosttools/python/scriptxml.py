from  xml.dom import  minidom
import bytefp
    
def load_xml(filename="user.xml"):
    doc = minidom.parse(filename)    
    return doc

def print_xml(doc):
    print doc.toxml("UTF-8")

def get_root(doc):
    return doc.documentElement
    
def parser_node(node, fp):
    # print node.nodeName
    type = node.getAttribute("type")
    size = node.getAttribute("size")
    
    if len(size) == 0:
        size = "1"
    if len(node.childNodes)==0:
        string = None
    else:
        string = node.childNodes[0].nodeValue    
        
    return bytefp.write_bytefp(type, size, string, fp) 
