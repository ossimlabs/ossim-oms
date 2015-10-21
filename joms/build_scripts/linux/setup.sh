if [ ! -d "$HOME/.sdkman" ]; then
   curl -s get.sdkman.io | bash
fi

source "$HOME/.sdkman/bin/sdkman-init.sh"

if [ ! -d "$HOME/.sdkman/groovy/$GROOVY_VERSION" ]; then
   sdk install groovy $GROOVY_VERSION
fi
