# Simple script to collect basic information as part of initial live incident response.

usage () {
	echo "usage: $0 [listening host]"
	echo "Simple script to send a log entry to listener"
	exit 1
}

# did you specify a listener IP?
if [ $# -gt 1 ] || [ "$1" == "--help" ] ; then
   usage
fi

# did you specify a listener IP?
if [ "$1" != "" ] ; then
   source setup-client.sh $1
fi

# now collect some info!
send-log.sh date 
send-log.sh uname -a
send-log.sh ifconfig -a
send-log.sh netstat -anp
send-log.sh lsof -V
send-log.sh ps -ef
send-log.sh netstat -rn
send-log.sh route
send-log.sh lsmod
send-log.sh df
send-log.sh mount
send-log.sh w
send-log.sh last 
send-log.sh cat /etc/passwd
send-log.sh cat /etc/shadow



