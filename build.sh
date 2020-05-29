#!/usr/bin/bash

# Shell script for building system dependent application bundle

# usage: bash build.sh <base_dir> <app_name> <executable_loc> <dependencies ...>

if [ $# -lt 3 ]
then
    exit -1
fi

BASE_DIR=$1
APP_NAME=$2
APP_DIR=$BASE_DIR/$APP_NAME
EXE_LOC=$3
shift #skip past these three arguments
shift
shift

mkdir -p $APP_DIR
mkdir -p $APP_DIR/Contents
mkdir -p $APP_DIR/Contents/MacOS
mkdir -p $APP_DIR/Contents/Resources
mkdir -p $APP_DIR/Contents/Frameworks


# signal to OS (and git) to keep directories even if they are empty
touch $APP_DIR/Contents/Resources/.keep
touch $APP_DIR/Contents/Frameworks/.keep


EXE_NAME=${EXE_LOC##*/}
# move executable into MacOS folder
cp $EXE_LOC $APP_DIR/Contents/MacOS/


# move each dependency into the apropriate directory
while [[ $# -gt 0 ]]
do
    DEP=$1
    EXT=${DEP##*.}

    if [ $EXT == "dylib" ]
    then
        DYLIB_NAME=${DEP##*/}
        cp -r $DEP $APP_DIR/Contents/Frameworks/
        # need to change search path for this dyli
        install_name_tool -change "$DEP" "@loader_path/../Frameworks/$DYLIB_NAME" $APP_DIR/Contents/MacOS/$EXE_NAME
    elif [ $EXT == "framework" ]
    then
        cp -r $DEP $APP_DIR/Contents/Frameworks/
    elif [ $EXT == "icns" ]
    then
        cp -r $DEP $APP_DIR/Contents/Resources/
    elif [ $EXT == "plist" ]
    then
        cp -r $DEP $APP_DIR/Contents/
    else
        DIRS=${DEP%/*}
        REL_DIRS=${DIRS#$BASE_DIR}
        mkdir -p $APP_DIR/Contents/Resources/$REL_DIRS
        cp -r $DEP $APP_DIR/Contents/Resources/$REL_DIRS
    fi

    shift
done



