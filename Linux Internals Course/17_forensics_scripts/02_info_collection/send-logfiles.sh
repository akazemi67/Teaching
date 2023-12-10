# Simple script to send all logs as part of initial live incident response.
# Warning: This script might take a long time to run!

usage () {
	echo "usage: $0 "
	echo "Simple script to send log files to a log listener"
	exit 1
}

if [ $# -gt 0 ] ; then
   usage
fi

# find only files, exclude files with numbers as they are old logs 
# execute echo, cat, and echo for all files found 
send-log.sh find /var/log -type f -regextype posix-extended -regex '/var/log/[a-zA-Z\.]+(/[a-zA-Z\.]+)*' -exec echo -e "---dumping logfile {} ---\n" \; -exec cat {} \; -exec echo -e "---end of dump for logfile {} ---\n" \;
