include common.mk

.PHONY: all
all:
	mkdir -p $(LDIR)
	mkdir -p $(BDIR)
	(make -C $(LIBDIR) BASE_DIR=${CURDIR} LDIR=$(LDIR))
	(make -C $(GAMEDIR) BASE_DIR=${CURDIR} BDIR=$(BDIR))

.PHONY: clean
clean:
	(make clean -C $(LIBDIR))
	(make clean -C $(GAMEDIR))
	rm -rf $(BDIR)
	rm -rf $(LDIR)

