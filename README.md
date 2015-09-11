# ossim-oms
Contains Java Native Interface (JNI) bindings to OSSIM core library.

## Build Instructions for COMS (the C++ side)
The COMS half of this repository is built as part of the OSSIM suite of repositories, using the centralized CMake build system in [ossimlabs/ossim](http://github.com/ossimlabs/ossim). See the build instructions in [ossim/README.md](http://github.com/ossimlabs/ossim/blob/master/README.md). As long as this repository is located in the same parent directory as [OSSIM](http://github.com/ossimlabs/ossim), CMake will detect it and create the Makefiles in `<build-dir>/<build-type>/ossim-oms`. You can `cd` into that directory and type `make` if you only want to build ossim-oms/COMS.

## Build Instructions for JOMS (the Java side)
* cd oms/joms
* (ontime setup)  cp local.properties.template local.properties
* ant mvn-install
* Should produce: $M2_REPO/org/ossim/joms/${OSSIM_VERSION}/joms-${OSSIM_VERSION}.jar and oms/joms/libjoms.${jnilib|so|dll}
