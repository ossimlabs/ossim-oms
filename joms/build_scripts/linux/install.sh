pushd `dirname $0` >/dev/null
export SCRIPT_DIR=`pwd -P`
. $SCRIPT_DIR/env.sh

popd >/dev/null


pushd $JOMS_HOME > /dev/null

ant install
antReturnCode=$?
if [ $antReturnCode -ne 0 ];then
    echo "BUILD ERROR: ant failed install..."
    exit 1;
else
    exit 0;
fi
 
popd >/dev/null
