#!/bin/bash
# Build script for Just Forge Engine
set echo on

mkdir -p ../build

# Get a list of all the .c files.
cFilenames=$(find . -type f -name "*.c")

# echo "Files:" $cFilenames

assembly="Just_Forge_Engine"
compilerFlags="-g -shared -fdeclspec -fPIC"
# -fms-extensions 
# -Wall -Werror
includeFlags="-Isrc -I$VULKAN_SDK/include"
linkerFlags="-lvulkan -lxcb -lX11 -lX11-xcb -lxkbcommon -L$VULKAN_SDK/lib -L/usr/X11R6/lib"
defines="-DFORGE_EXPORT -D_DEBUG"

echo "Building $assembly..."
clang $cFilenames $compilerFlags -o ../build/lib$assembly.so $defines $includeFlags $linkerFlags
