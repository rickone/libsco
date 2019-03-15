#!/bin/bash

#mkdir -p pkg
if [ ! -d "pkg/cpkg" ]; then
	git clone https://github.com/rickone/cpkg.git pkg/cpkg
fi
pkg/cpkg/cpkg build
