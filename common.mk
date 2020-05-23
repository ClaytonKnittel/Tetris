CC=gcc
AR=ar

BASE_DIR=$(shell pwd)
GAMEDIR=$(BASE_DIR)/main
LIBDIR=$(BASE_DIR)/glib

LDIR=$(BASE_DIR)/lib
BDIR=$(BASE_DIR)/bin

# testing binary directory
TDIR=$(BDIR)

IFLAGS=-I$(GAMEDIR)/include -I$(LIBDIR)/include $(shell pkg-config --cflags freetype2)
LFLAGS=-L$(LDIR)

#opengl libraries
GLLIBS=-lglew.2.1 -lglfw.3.3 -framework OpenGL

DEBUG=0

ifeq ($(DEBUG), 0)
CFLAGS=-O3 -Wall -Wno-unused-function -MMD -MP
else
CFLAGS=-O0 -Wall -Wno-unused-function -MMD -MP -g3 -DDEBUG
endif

# -flto allows link-time optimization (like function inlining)
LDFLAGS=$(LFLAGS) $(GLLIBS) -flto -lfreetype

