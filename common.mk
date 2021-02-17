CC=gcc
AR=ar
BASH=bash

BASE_DIR=$(shell pwd)
GAMEDIR=$(BASE_DIR)/main
LIBDIR=$(BASE_DIR)/glib
BUILDDIR=$(BASE_DIR)/build

LDIR=$(BASE_DIR)/lib
BDIR=$(BASE_DIR)/bin

EXE_NAME=game
APP_NAME=tetris

# testing binary directory
TDIR=$(BDIR)

IFLAGS=-I$(GAMEDIR)/include -I$(LIBDIR)/include $(shell pkg-config --cflags freetype2)
LFLAGS=-L$(LDIR)

# opengl libraries
GLLIBS=-lglew.2.2 -lglfw.3.3 -framework OpenGL

# set when in debug mode
DEBUG=0

# set when building for production
BUILD=1

ifeq ($(BUILD), 1)
BFLAGS=-DBUILD
endif

ifeq ($(DEBUG), 0)
CFLAGS=-O3 -Wall -Wno-unused-function -MMD -MP $(BFLAGS)
else
CFLAGS=-O0 -Wall -Wno-unused-function -MMD -MP -g3 -DDEBUG $(BFLAGS)
endif


# -flto allows link-time optimization (like function inlining)
LDFLAGS=$(LFLAGS) $(GLLIBS) -flto -lfreetype -framework CoreFoundation


GL_FRAMEWORK=/System/Library/Frameworks/OpenGL.framework

LIBPNG=/usr/local/opt/libpng/lib/libpng16.16.dylib
LIBFREETYPE=/usr/local/opt/freetype/lib/libfreetype.6.dylib
LIBGLEW=/usr/local/opt/glew/lib/libGLEW.2.1.dylib
LIBGLFW=/usr/local/opt/glfw/lib/libglfw.3.dylib

FONTS_DIR=$(BASE_DIR)/fonts

