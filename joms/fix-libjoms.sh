#!/bin/sh

gcc -shared -Wall swig/*.o -o libjoms.so -L$OSSIM_INSTALL_PREFIX/lib64 -lossim -loms
cp libjoms.so $OSSIM_INSTALL_PREFIX/lib64
