.PHONY: all clean src example

all: src example

clean:
	$(MAKE) clean -C src
	$(MAKE) clean -C example

src:
	$(MAKE) -C src

example: src
	$(MAKE) -C example