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
GLLIBS=-lglew.2.1 -lglfw.3.3 -framework OpenGL

# set when in debug mode
DEBUG=1

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

LIBFREETYPE=/usr/local/Cellar/freetype/2.10.1/lib/libfreetype.dylib
LIBGLEW=/usr/local/Cellar/glew/2.1.0_1/lib/libGLEW.2.1.dylib
LIBGLFW=/usr/local/Cellar/glfw/3.3.2/lib/libglfw.3.3.dylib

FONTS_DIR=$(BASE_DIR)/fonts

