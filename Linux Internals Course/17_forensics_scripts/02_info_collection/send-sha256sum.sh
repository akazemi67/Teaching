# Simple script to calculate sha256 sum as part of initial live incident response.
# Warning: This script might take a long time to run!


usage () {
	echo "usage: $0 <starting directory>"
	echo "Simple script to send SHA256 hash to a log listener"
	exit 1
}

if [ $# -lt 1 ] ; then
   usage
fi

# find only files, don't descend to other filesystems, 
# execute command sha256sum -b <filename> for all files found 
send-log.sh find $1 -type f -xdev -exec sha256sum -b {} \;
