.PHONY: all clean xbin

all: xbin

clean:
	$(MAKE) -C xbin clean

xbin: xbin/Makefile
	$(MAKE) -C xbin

xbin/Makefile:
	git submodule update --init
