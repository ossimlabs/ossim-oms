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

# if [ -z "$GROOVY_HOME" ]; then
#    echo "HOME Location: ${HOME}"
#    source "$HOME/.sdkman/bin/sdkman-init.sh"
#    if [ ! -z "$GROOVY_VERSION" ]; then
#       sdk use groovy $GROOVY_VERSION
#    fi
# fi

ant install mvn-install
antReturnCode=$?
if [ $antReturnCode -ne 0 ];then
    echo "BUILD ERROR: ant failed install..."
    exit 1;
else
    exit 0;
fi
 
popd >/dev/null
