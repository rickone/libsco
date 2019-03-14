#!/bin/bash

CPKG=`which cpkg`

if [ "$CPKG" == "" ]; then
	mkdir -p pkg
	if [ ! -d "pkg/cpkg" ]; then
		git clone https://github.com/rickone/cpkg.git pkg/cpkg
	fi
	CPKG_PATH=`pwd`/pkg/cpkg pkg/cpkg/cpkg build
else
	cpkg build
fi
