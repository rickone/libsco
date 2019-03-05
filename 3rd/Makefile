.PHONY: all clean box

all: box

clean:
	$(MAKE) -C box clean

box: box/Makefile
	$(MAKE) -C box

box/Makefile:
	git clone https://github.com/rickone/box.git
