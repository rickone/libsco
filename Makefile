.PHONY: all clean src 3rd example

BuildPath ?= $(shell pwd)/build

all: src

clean:
	rm -rf $(BuildPath)

src: 3rd
	$(MAKE) -C src

3rd:
	$(MAKE) -C 3rd BuildPath=$(BuildPath)

example: src
	$(MAKE) -C example
