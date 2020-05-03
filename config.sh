#!/usr/bin/sh

if ! hash brew 2>/dev/null;
then
    /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install.sh)"
fi

# install the 2 dependencies
brew install glew
brew install glfw3
brew install freetype

