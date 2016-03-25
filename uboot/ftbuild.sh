#!/bin/bash

SEE_OUT=see_bloader3

gcc tools/bin2text.c -o tools/bin2text

cd ali_usee/prj/app/demo/combo/sabbat_dual/loader_see && . clean && . auto_see && \
cd ../../../../../../.. && \
tools//bin2text -b ali_usee/obj/${SEE_OUT}.abs -d ${SEE_OUT}.h -n ${SEE_OUT}_data && \
mv -f ${SEE_OUT}.h  board/ALi/ali_3921/${SEE_OUT}.h && \
make rebuild_3921