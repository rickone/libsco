.PHONY: all clean src test 3rd

all: src test

clean:
	rm -rf bin
	rm -rf lib
	rm -rf obj
	rm -rf include

src: 3rd
	$(MAKE) -C src

test: src
	$(MAKE) -C test

3rd:
	$(MAKE) -C 3rd
