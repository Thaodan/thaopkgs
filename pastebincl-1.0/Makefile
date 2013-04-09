# Makefile for PastebinCL by BASTIAN Théophile (aka. Tobast)

BINARY=pastebincl
MANSOURCE=$(BINARY).man.1
MANOUT=$(BINARY).1
prefix=/usr/

all: $(BINARY)

$(BINARY):
	cd src/ && make


clean:
	cd src/ && make clean
mrproper:
	cd src/ && make mrproper
.PHONY: clean


install: $(BINARY)
	mkdir -p "$(prefix)bin/"
	mkdir -p "$(prefix)share/man/man1/"
	cp $(BINARY) $(prefix)bin/
	chmod +x $(prefix)bin/$(BINARY)
	cp $(MANSOURCE) $(prefix)share/man/man1/$(MANOUT)
	gzip $(prefix)share/man/man1/$(MANOUT)

