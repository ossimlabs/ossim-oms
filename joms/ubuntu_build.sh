#!/bin/bash
ant clean install mvn-install
gcc -shared -Wall swig/oms_wrap.o -o libjoms.so -L/home/okramer/dev/ossim/ossimlabs_build/lib -lossim -loms

