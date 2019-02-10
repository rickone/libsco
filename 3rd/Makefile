.PHONY: all clean box hiredis

all: box hiredis

clean:
	$(MAKE) -C box clean

box: box/Makefile
	$(MAKE) -C box

box/Makefile:
	git submodule update --init

hiredis:
	cd hiredis && make && sudo make install
