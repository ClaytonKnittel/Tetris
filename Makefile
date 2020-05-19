include common.mk

.PHONY: all
all: dirs glib main

.PHONY: dirs
dirs:
	@mkdir -p $(LDIR)
	@mkdir -p $(BDIR)

.PHONY: main
main:
	@(make -C $(GAMEDIR) BASE_DIR=${CURDIR} BDIR=$(BDIR) LDIR=$(LDIR))

.PHONY: glib
glib:
	@(make -C $(LIBDIR) BASE_DIR=${CURDIR} TDIR=$(TDIR) LDIR=$(LDIR))

.PHONY: clean
clean:
	(make clean -C $(LIBDIR))
	(make clean -C $(GAMEDIR))
	rm -rf $(BDIR)
	rm -rf $(LDIR)

