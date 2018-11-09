#!/bin/bash

if [ ! -z "$1" ]; then
	unset EXTERNAL 
	unset EXTERNAL_PHONY 
	unset EXTERNAL_PHONY_CLEAN
	if [ "${HAVE_LIB_EXTERNAL}" = "1" ]; then
		export EXTERNAL="$1"
		export EXTERNAL_PHONY="EXTERNAL"
		export EXTERNAL_PHONY_CLEAN="EXTERNAL_CLEAN"
		echo "EXTERNAL=${EXTERNAL}"
		# load source.${PLATFORM}.sh
		source "${EXTERNAL}/platform/source.${PLATFORM}.sh"
	fi
else
	# load global env
	source mk/source.sh
	
fi
