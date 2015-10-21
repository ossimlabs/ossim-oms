if [ ! -d "$HOME/.sdkman" ]; then
   curl -s get.sdkman.io | bash
fi

source "$HOME/.sdkman/bin/sdkman-init.sh"

if [ ! -d "$HOME/.sdkman/groovy/2.4.0" ]; then
   sdk install groovy 2.4.0
else
   sdk use groovy 2.4.0
fi
