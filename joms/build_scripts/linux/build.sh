#!/bin/bash
pushd `dirname ${BASH_SOURCE[0]}` >/dev/null
export SCRIPT_DIR=`pwd -P`
. $SCRIPT_DIR/env.sh
popd >/dev/null

pushd $JOMS_HOME > /dev/null
if [ ! -a local.properties ]
   then 
   cp local.properties.template local.properties
fi

if [ "$GROOVY_HOME" == "" ] ; then
    echo "ERROR: GROOVY_HOME must be defined!"
    exit 1
fi

if [ "$OSSIM_VERSION" == "" ] ; then
    echo "ERROR: OSSIM_VERSION must be defined!"
    exit 1
fi

if [ "$OSSIM_VERSION_TAG" == "" ] ; then
    echo "ERROR: OSSIM_VERSION_TAG must be defined!"
    exit 1
fi

ant clean
ant build
antReturnCode=$?

if [ $antReturnCode -ne 0 ];then
    echo "BUILD ERROR: ant failed dist mvn-install build..."
    popd >/dev/null
    exit 1;
fi
 
popd >/dev/null

