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

.PHONY: app

app: $(BASE_DIR)/$(APP_NAME).app

.PHONY: $(BASE_DIR)/$(APP_NAME).app
$(BASE_DIR)/$(APP_NAME).app: all
	bash build.sh $(BASE_DIR) $(APP_NAME).app $(BDIR)/$(EXE_NAME) \
		$(BUILDDIR)/Info.plist $(BUILDDIR)/tetris.icns $(GAMEDIR)/res \
		$(LIBDIR)/res $(FONTS_DIR) $(LIBGLEW) $(LIBGLFW) $(LIBFREETYPE) $(LIBPNG)
	bash build/config_dylibs.sh $@/Contents/Frameworks $(LIBFREETYPE) $(LIBPNG)


.PHONY: clean
clean:
	(make clean -C $(LIBDIR))
	(make clean -C $(GAMEDIR))
	rm -rf $(BDIR)
	rm -rf $(LDIR)
	rm -rf $(BASE_DIR)/$(APP_NAME).app

