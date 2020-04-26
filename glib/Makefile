include ../common.mk

SDIR=src
ODIR=.obj

SRC=$(shell find $(SDIR) -type f -name '*.c')
OBJ=$(patsubst $(SDIR)/%.c,$(ODIR)/%.o,$(SRC))
DEP=$(wildcard $(SDIR)/*.h)

DIRS=$(shell find $(SDIR) -type d)
OBJDIRS=$(patsubst $(SDIR)/%,$(ODIR)/%,$(DIRS))

$(shell mkdir -p $(ODIR))
$(shell mkdir -p $(OBJDIRS))

DEPFILES=$(SRC:%.c=$(ODIR)/%.d)

.PHONY: all
all: $(LDIR)/libglib.a

$(LDIR)/libglib.a: $(OBJ)
	$(AR) -rcs $@ $^

$(ODIR)/%.o: $(SDIR)/%.c
	$(CC) $(CFLAGS) $^ -c -o $@ $(IFLAGS)

-include $(wildcard $(DEPFILES))

.PHONY: clean
clean:
	find $(ODIR) -type f -name '*.[od]' -delete
	rm -rf $(ODIR)

