#!/bin/bash

cd ali_usee/prj/app/demo/combo/sabbat_dual/loader_see && . clean
# cd ../../../../../../.. && make distclean && find ./ -name *.o | xargs rm -f
cd ../../../../../../.. && make distclean
rm tools/bin2text -f