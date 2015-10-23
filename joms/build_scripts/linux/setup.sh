#!/bin/bash
echo "Checking for sdkman: $HOME/.sdkman"
if [ ! -d "$HOME/.sdkman" ]; then
   echo "Installing sdkman....."
   curl -s get.sdkman.io | bash
   echo "Done installing sdkman....."
fi

source "$HOME/.sdkman/bin/sdkman-init.sh"

if [ -z "$GROOVY_VERSION" ]; then
   export GROOVY_VERSION=2.4.0
fi

if [ ! -d "$HOME/.sdkman/groovy/$GROOVY_VERSION" ]; then
   sdk install groovy $GROOVY_VERSION
fi
