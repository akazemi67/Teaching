# Simple script to set environment variables for a system under investigation.

usage () { 
	echo "usage: $0 <forensics workstation IP> [log port] [filename port] [file transfer port]"
	echo "Simple script to set variables for communication to forensics workstation"
	exit 1
}

# did you specify a file?
if [ $# -lt 1 ] ; then
   usage
fi

export RHOST=$1

if [ $# -gt 1 ] ; then
   export RPORT=$2
else
   export RPORT=4444
fi
if [ $# -gt 2 ] ; then
   export RFPORT=$3
else
   export RFPORT=5555
fi
if [ $# -gt 3 ] ; then 
   export RFTPORT=$4
else
   export RFTPORT=5556
fi


