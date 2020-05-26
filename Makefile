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
$(BASE_DIR)/$(APP_NAME).app:
	@(make -C .)
	@mkdir -p $@
	@mkdir -p $@/Contents
	@mkdir -p $@/Contents/MacOS
	@mkdir -p $@/Contents/Resources
	@mkdir -p $@/Contents/PlugIns
	@mkdir -p $@/Contents/Frameworks
	cp $(BDIR)/$(EXE_NAME) $@/Contents/MacOS/
	cp $(BUILDDIR)/Info.plist $@/Contents/
	cp $(BUILDDIR)/tetris.icns $@/Contents/Resources
	@mkdir -p $@/Contents/Resources/main/
	cp -r $(GAMEDIR)/res $@/Contents/Resources/main/
	@mkdir -p $@/Contents/Resources/glib/
	cp -r $(LIBDIR)/res $@/Contents/Resources/glib/
	cp -r $(FONTS_DIR) $@/Contents/Resources/
	#cp -r $(GL_FRAMEWORK) $@/Contents/Frameworks/
	cp $(LIBGLEW) $@/Contents/PlugIns/
	cp $(LIBGLFW) $@/Contents/PlugIns/
	cp $(LIBFREETYPE) $@/Contents/PlugIns/


.PHONY: clean
clean:
	(make clean -C $(LIBDIR))
	(make clean -C $(GAMEDIR))
	rm -rf $(BDIR)
	rm -rf $(LDIR)
	rm -rf $(BASE_DIR)/$(APP_NAME).app

