#!/bin/bash
pushd `dirname $0` >/dev/null
export SCRIPT_DIR=`pwd -P`
popd >/dev/null

pushd $SCRIPT_DIR/../.. >/dev/null
export JOMS_HOME=$PWD
echo "@@@@@ SCRIPT_DIR=$SCRIPT_DIR"
echo "@@@@@ JOMS_HOME=$JOMS_HOME"
popd >/dev/null

pushd $JOMS_HOME/../.. >/dev/null
export OSSIM_DEV_HOME=$PWD
export OSSIM_HOME=$OSSIM_DEV_HOME/ossim
if [ -z "$OSSIM_BUILD_DIR" ]; then
   export OSSIM_BUILD_DIR=$OSSIM_DEV_HOME/build
fi
if [ -z "$OSSIM_INSTALL_PREFIX" ]; then
   export OSSIM_INSTALL_PREFIX=$OSSIM_DEV_HOME/install
fi
popd >/dev/null
