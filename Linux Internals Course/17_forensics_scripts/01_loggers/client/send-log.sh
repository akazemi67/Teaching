# Simple script to send a new log entry to listener on forensics workstation. 

# defaults primarily for testing
[ -z "$RHOST" ] && { export RHOST=localhost; }
[ -z "$RPORT" ] && { export RPORT=4444; }

usage () {
	echo "usage: $0 <command or script>"
	echo "Simple script to send a log entry to listener"
	exit 1
}

# did you specify a command?
if [ $# -lt 1 ] ; then
   usage
else
   echo -e "++++Sending log for $@ at $(date) ++++\n $($@) \n----end----\n"  | nc $RHOST $RPORT 
fi 




