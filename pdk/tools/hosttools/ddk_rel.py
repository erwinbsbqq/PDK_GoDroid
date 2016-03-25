#!/usr/bin/python
import argparse
import string
import re
import os.path
import sys


# get the paramenter from the parent script or Makefile
parser = argparse.ArgumentParser(description='release script')
parser.add_argument('-M', "--module" , dest='path_module', help='module path', metavar="module_path", required=True)
parser.add_argument('-R', "--release" , dest='path_release', help='release path', metavar="release_path", required=True)
parser.add_argument('-P', "--platform" , dest='path_platform', help='platform path', metavar="platform_path", required=True)
parser.add_argument('-F', "--file" , dest='dep_file', help='depend file', metavar="depend_file", required=True)
args= parser.parse_args()

path_module = args.path_module
path_release = args.path_release
path_platform = args.path_platform
dep_file = args.dep_file

if path_release[-1] != "/": path_release += "/"

##parse the depend file
filecontent = ""

with open(dep_file, 'r') as f:
    filecontent = f.read(-1)

##parse the lines
filelines = filecontent.splitlines()

objlist= []
sline = ""
for line in filelines:
    if line[-1] == "\\":
        #continue line
        sline = sline + " "  + line[:-1]
    else:
        #end line
        sline = sline + line
        objlist.append(sline)
        sline = ""

#parse every file in object
filelist = []
dirlist = []
for line in objlist:
    t = line.split(":")
    if len(t) < 2:
        continue

    fs = t[1].split()
    for f in fs:
        f = os.path.normpath(f)
        if f[0] != "/":
            f = path_module + "/" + f
            f = os.path.normpath(f)

        dn, fn = os.path.split(f)

        if -1 != dn.find(path_platform, 0):
            if not dn in dirlist:
                dirlist.append(dn)
            if not f in filelist:
                filelist.append(f)

#don't foeget put the Makefile into the file list
filelist.append(path_module + "/Makefile")

root_path = os.path.normpath(path_platform + "/../../")
#geneate the bash script to release
for f in dirlist:
    nf = f.replace(root_path, path_release)
    sys.stderr.write("[ -d %s ] || mkdir -p %s \n" % (nf, nf))

for f in filelist:
    nf = f.replace(root_path, path_release)
    #sys.stdout.write("[ -f %s -a ! -f %s ] && cp %s %s\n" % (f, nf, f, nf))
    #force copy
    sys.stdout.write("[ -f %s ] && cp %s %s\n" % (f, f, nf))
