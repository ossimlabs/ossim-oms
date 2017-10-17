#!/bin/bash
export OSSIM_INSTALL_PREFIX=/home/okramer/dev/ossim/ossimlabs/build
export OSSIM_VERSION_TAG="SNAPSHOT"
ant clean install mvn-install
gcc -shared -Wall swig/oms_wrap.o -o libjoms.so -L/home/okramer/dev/ossim/ossimlabs/build/lib -lossim -loms

