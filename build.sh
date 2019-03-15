#!/bin/bash

CPKG=`which cpkg`
if [ "$CPKG" != "" ]; then
    cpkg build
    exit 0
fi

if [ ! -d "pkg/cpkg" ]; then
	git clone https://github.com/rickone/cpkg.git pkg/cpkg
fi
pkg/cpkg/cpkg build
