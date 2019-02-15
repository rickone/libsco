.PHONY: all clean src 3rd example

all: src

clean:
	rm -rf bin
	rm -rf lib
	rm -rf obj
	rm -rf include

src: 3rd
	$(MAKE) -C src

3rd:
	$(MAKE) -C 3rd

example: src
	$(MAKE) -C example
