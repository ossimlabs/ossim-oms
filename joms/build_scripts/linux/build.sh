#!/bin/bash
pushd `dirname $0` >/dev/null
export SCRIPT_DIR=`pwd -P`

pushd $SCRIPT_DIR/../.. >/dev/null
export JOMS_HOME=$PWD
echo "@@@@@ SCRIPT_DIR=$SCRIPT_DIR"
echo "@@@@@ JOMS_HOME=$JOMS_HOME"

pushd $JOMS_HOME/../.. >/dev/null
export OSSIM_DEV_HOME=$PWD
export OSSIM_HOME=$OSSIM_DEV_HOME/ossim
export OSSIM_BUILD_DIR=$OSSIM_DEV_HOME/build
export OSSIM_BUILD_TYPE=""
popd >/dev/null

popd >/dev/null


pushd $JOMS_HOME > /dev/null
if [ ! -a local.properties ]
   then 
   cp local.properties.template local.properties
fi

ant -v clean mvn-install
popd >/dev/null

#
popd >/dev/null
