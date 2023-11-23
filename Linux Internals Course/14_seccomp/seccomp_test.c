/*
    gcc seccomp_test.c -o sec_test -lseccomp
Code from:
     https://blog.cloudflare.com/sandboxing-in-linux-with-zero-lines-of-code/
*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/utsname.h>
#include <seccomp.h>
#include <err.h>

static void sandbox(void) {
    /* allow all syscalls by default */
    scmp_filter_ctx seccomp_ctx = seccomp_init(SCMP_ACT_ALLOW);
    if (!seccomp_ctx)
        err(1, "seccomp_init failed");

    /* kill the process, if it tries to use "uname" syscall */
    if (seccomp_rule_add_exact(seccomp_ctx, SCMP_ACT_LOG, seccomp_syscall_resolve_name("uname"), 0)) {
        perror("seccomp_rule_add_exact failed");
        exit(1);
    }

    /* apply the composed filter */
    if (seccomp_load(seccomp_ctx)) {
        perror("seccomp_load failed");
        exit(1);
    }

    /* release allocated context */
    seccomp_release(seccomp_ctx);
}

int main(void) {
    struct utsname name;

    sandbox();

    if (uname(&name)) {
        perror("uname failed: ");
        return 1;
    }

    printf("My OS is %s!\n", name.sysname);
    return 0;
}

