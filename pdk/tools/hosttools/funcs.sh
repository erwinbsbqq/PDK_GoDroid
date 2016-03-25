#!/bin/bash

panic()
{
    echo "Panic: $1" 1>&2
    exit 1
}

copy()
{
	if [ ! -f $1 ]; then
		panic "$1 doesn't exist!"
	fi

	if [ ! -f $2 ]; then
		cp -raf $1 $2
	else
	    cmp -s $1 $2
	    RET=$?
	    if [ $RET != 0 ]; then
		cp -raf $1 $2
	    fi
	fi
}

check()
{
	eval $1
	RET=$?

	if [ "$RET" != "0" ]; then
		echo
 		echo "*********************************************"
		echo "Error: Process terminate! please check it!!"
 		echo "*********************************************"
		echo "exit..."
		exit 1
	fi
}

checkit()
{
	RET=$?

	if [ "$RET" != "0" ]; then
		echo
 		echo "*********************************************"
		echo "Error: Process terminate! please check it!!"
 		echo "*********************************************"
		echo "exit..."
		exit 1
	fi
}
