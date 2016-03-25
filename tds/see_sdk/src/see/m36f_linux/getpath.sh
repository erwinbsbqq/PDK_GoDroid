#!/bin/bash

PATH_ROOT=../../../
cd $PATH_ROOT
ROOTPATH=$(pwd)

echo LOCAL_BASE = $ROOTPATH
echo LOCAL_BASE = $ROOTPATH > path.def

echo " " >> path.def
echo BASE_DIR = \$\{LOCAL_BASE\} >> path.def
echo INC_DIR = \$\{BASE_DIR\}/inc >> path.def
echo LIB_DIR = \$\{BASE_DIR\}/obj >> path.def
echo REL_DIR = \$\{BASE_DIR\}/rel >> path.def

echo DDK_DIR = \$\{BASE_DIR\}/ddk >> path.def
echo SDK_DIR = \$\{BASE_DIR\}/sdk >> path.def
echo PLUGINDDK_DIR = \$\{BASE_DIR}/ddk/plugin >> path.def
echo PLUGINEXT_DIR = \$\{BASE_DIR}/ddk/plugin_ext >> path.def
echo BLSDK_DIR = \$\{BASE_DIR\}/sdk/blsdk >> path.def
echo DDK_BLSDK_DIR = \$\{BASE_DIR\}/ddk/blsdk >> path.def

cp path.def src/
rm -f path.def

# copy plugin when in PDK release version
TDS_PATH=`dirname $ROOTPATH`
TDS_PLUGINS_PATH=$TDS_PATH/plugins
if [ -d $TDS_PLUGINS_PATH ]; then
  echo "Copy sdk plugins to ddk/plugin..."
  find $TDS_PLUGINS_PATH -name [^ae_]*.o | xargs -i cp {} $ROOTPATH/ddk/plugin/
fi
