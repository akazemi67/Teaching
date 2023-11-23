# “hrtimer_wakeup” is responsible for waking up a process and move it to the set of runnable processes.
bpftrace -e 'kfunc:hrtimer_wakeup { printf("name:%s  pid:%d\n",curtask->comm,curtask->pid); }'

