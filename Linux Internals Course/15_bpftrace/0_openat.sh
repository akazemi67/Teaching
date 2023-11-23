#   Syntax
#   probe[,probe,...] /filter/ { action }
bpftrace -e 'tracepoint:syscalls:sys_enter_openat /pid!=425/ { printf("%d %s => %s\n", pid, comm, str(args->filename)); }'

