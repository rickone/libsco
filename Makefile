.PHONY: all clean src test

all: src test

clean:
	rm -rf bin lib obj

src:
	cd src && Dep=0 cmk

test: src
	cd test && Dep=0 cmk
