#!/bin/bash

type=$1

if [[ $type == 'q' ]]
then
    echo "Looking for $2"
    /opt/metasploit-framework/embedded/framework/tools/exploit/pattern_offset.rb -q $2 2>/dev/null
else
    /opt/metasploit-framework/embedded/framework/tools/exploit/pattern_create.rb -l $2 2>/dev/null
fi

