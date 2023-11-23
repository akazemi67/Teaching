#LD_PRELOAD=./sandbox/libsandbox.so SECCOMP_SYSCALL_DENY=uname ./sec_test
LD_PRELOAD=./sandbox/libsandbox.so SECCOMP_SYSCALL_ALLOW=exit_group:brk:uname:write:mmap:close:getrandom:mprotect:rseq:set_robust_list:arch_prctl:pread64 ./sec_test


