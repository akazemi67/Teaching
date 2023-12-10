# Simple script to collect file information as part of initial live incident response.
# Warning: This script might take a long time to run!

usage () {
	echo "usage: $0 <starting directory>"
	echo "Simple script to send file information to a log listener"
	exit 1
}

if [ $# -lt 1 ] ; then
   usage
fi

# semicolon delimited file which makes import to spreadsheet easier
# printf is access date, access time, modify date, modify time,
#           create date, create time, permissions, user id, user name,
#           group id, group name, file size, filename and then line feed
send-log.sh find $1 -printf "%Ax;%AT;%Tx;%TT;%Cx;%CT;%m;%U;%u;%G;%g;%s;%p\n"

