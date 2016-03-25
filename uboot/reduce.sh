#!/bin/bash

cd board
ls |grep -v ALi|grep -v ali-stb|xargs rm -rf
cd ..

cd arch
ls |grep -v arm|grep -v mips|xargs rm -rf
cd ..

cd include/configs
ls |grep -v ali_3921.h|grep -v ali-stb.h|xargs rm -rf
cd ../..

rm doc -rf

if [[ $1 = ft_cleanmore ]];then
fi
