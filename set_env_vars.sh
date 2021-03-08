#!/bin/sh

if [ -z $CHAOS_TOOLS_PATH_SET ]; then
    if [ -z $BASH_SOURCE ]; then
        PATH="$(dirname "$(readlink -f "$0")")/tools:$(dirname "$(readlink -f "$0")")/tools/i686-elf-tools-linux/bin:$(dirname "$(readlink -f "$0")")/tools/python27-linux:$PATH"
        LD_LIBRARY_PATH="$(dirname "$(readlink -f "$0")")/tools:$(dirname "$(readlink -f "$0")")/tools/python27-linux"
        TCL_LIBRARY="$(dirname "$(readlink -f "$0")")/tools/python27-linux/Lib/tcl8.6"
        TK_LIBRARY="$(dirname "$(readlink -f "$0")")/tools/python27-linux/Lib/tk8.6"
    else
        PATH="$(dirname "$(readlink -f "$BASH_SOURCE")")/tools:$(dirname "$(readlink -f "$BASH_SOURCE")")/tools/i686-elf-tools-linux/bin:$(dirname "$(readlink -f "$BASH_SOURCE")")/tools/python27-linux:$PATH"
        LD_LIBRARY_PATH="$(dirname "$(readlink -f "$BASH_SOURCE")")/tools:$(dirname "$(readlink -f "$BASH_SOURCE")")/tools/python27-linux"
        TCL_LIBRARY="$(dirname "$(readlink -f "$BASH_SOURCE")")/tools/python27-linux/Lib/tcl8.6"
        TK_LIBRARY="$(dirname "$(readlink -f "$BASH_SOURCE")")/tools/python27-linux/Lib/tk8.6"
    fi
    
    export PATH
    export LD_LIBRARY_PATH
    export TCL_LIBRARY
    export TK_LIBRARY
    export CHAOS_TOOLS_PATH_SET=1
    echo "Environment variables have been set"
else
    echo "Environment variables are already set"
fi
