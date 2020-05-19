include ../common.mk

SDIR=src
ODIR=.obj
TEST_SRC_DIR=test

SRC=$(shell find $(SDIR) -type f -name '*.c')
OBJ=$(patsubst $(SDIR)/%.c,$(ODIR)/%.o,$(SRC))
DEP=$(wildcard $(SDIR)/*.h)

DIRS=$(shell find $(SDIR) -type d)
OBJDIRS=$(patsubst $(SDIR)/%,$(ODIR)/%,$(DIRS))

# test files (each source is compiled into own executable)
TEST_SRC=$(shell find $(TEST_SRC_DIR) -type f -name '*.c')
# obj files
TEST_OBJ=$(patsubst $(TEST_SRC_DIR)/%.c,$(ODIR)/$(TEST_SRC_DIR)/%.o,$(TEST_SRC))
# executables
TESTS=$(patsubst $(TEST_SRC_DIR)/%.c,$(TDIR)/%,$(TEST_SRC))

$(shell mkdir -p $(ODIR))
$(shell mkdir -p $(OBJDIRS))
$(shell mkdir -p $(ODIR)/$(TEST_SRC_DIR))

DEPFILES=$(SRC:$(SDIR)/%.c=$(ODIR)/%.d)

.PHONY: all
all: $(LDIR)/libglib.a $(TESTS)

$(LDIR)/libglib.a: $(OBJ)
	$(AR) -rcs $@ $^

$(ODIR)/%.o: $(SDIR)/%.c
	$(CC) $(CFLAGS) $< -c -o $@ $(IFLAGS)

$(TDIR)/%: $(ODIR)/$(TEST_SRC_DIR)/%.o $(LDIR)/libglib.a
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS) -lglib

$(ODIR)/$(TEST_SRC_DIR)/%.o: $(TEST_SRC_DIR)/%.c
	$(CC) $(CFLAGS) $< -c -o $@ $(IFLAGS)

-include $(wildcard $(DEPFILES))

.PHONY: clean
clean:
	find $(ODIR) -type f -name '*.[od]' -delete
	rm -rf $(ODIR)

