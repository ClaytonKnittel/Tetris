#!/usr/bin/bash

# configure dylibs by resolving references between each other
# the dylib is expected to be in the frameworks directory along with the
# dependency
# usage: bash config_dylibs.sh <framework_dir> <dylib_loc dependency_loc>*

FRAMEWORK_DIR=$1
shift

while [ $# -gt 1 ]
do

    DYLIB=$1
    DYLIB_NAME=${DYLIB##*/}
    DEPENDENCY=$2
    DEP_NAME=${DEPENDENCY##*/}

    # resolve libfreetype dependency on libpng to local copy
    install_name_tool -change $DEPENDENCY "@loader_path/../Frameworks/$DEP_NAME" $FRAMEWORK_DIR/$DYLIB_NAME

    shift
    shift
done

