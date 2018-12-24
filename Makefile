.PHONY: all clean src test

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
