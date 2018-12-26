.PHONY: all clean src test 3rd

all: src test

clean:
	rm -rf bin
	rm -rf lib
	rm -rf obj
	rm -rf include

src:
	$(MAKE) -C src

test: src
	$(MAKE) -C test

3rd: 3rd/xbin/Makefile:
	$(MAKE) -C 3rd

3rd/xbin/Makefile:
	git submodule update --init
