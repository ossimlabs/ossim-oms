# ossim-oms
Contains Java Native Interface (JNI) bindings to OSSIM core library.

## Build Instructions for JOMS
* cd oms/joms
* (ontime setup)  cp local.properties.template local.properties
* ant mvn-install
* Should produce:  (oms/lib/joms-${OSSIM_VERSION}.jar and oms/joms/libjoms.${jnilib|so|dll}
