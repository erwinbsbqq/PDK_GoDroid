import sys
import os
import struct

if __name__ == "__main__":
    args = len(sys.argv)
    if args < 2:
        print "keymodulus.py <key.pub>"
        sys.exit()

    infile = sys.argv[1]
        
    if not os.path.exists(infile):
        print infile+" is not exist"
        sys.exit()  

    infp = open(infile,"rb")
    infp.seek(0x21,0)
    modulus = infp.read(256)
    infp.close
    
    outfile = "verity_key.c"
    outfp = open(outfile,"w")
    
    outfp.write("static unsigned char key_modulus[] = {\n")
    
    i = 0
    for c in modulus:
        n = struct.unpack('B',c)        
        outfp.write("0x%02x,"%(n))
        i += 1
        if i%16==0:
            outfp.write("\n");        

    outfp.write("};\n\n")
    outfp.close
    