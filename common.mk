CC=gcc
AR=ar

BASE_DIR=${CURDIR}
GAMEDIR=$(BASE_DIR)/main
LIBDIR=$(BASE_DIR)/glib

LDIR=$(BASE_DIR)/lib
BDIR=$(BASE_DIR)/bin

IFLAGS=-I$(GAMEDIR)/include -I$(LIBDIR)/include
LFLAGS=-L$(LDIR)

#opengl libraries
GLLIBS=-lglew.2.1 -lglfw.3.2 -framework OpenGL

DEBUG=1

ifeq ($(DEBUG), 0)
CFLAGS=-O3 -Wall -Wno-unused-function -MMD -MP
else
CFLAGS=-O0 -Wall -Wno-unused-function -MMD -MP -g3 -DDEBUG
endif

# -flto allows link-time optimization (like function inlining)
LDFLAGS=$(LFLAGS) $(GLLIBS) -flto

