#include "signal.h"
#include "errno.h"
#include "stdlib.h"
#include "syscall.h"

typedef struct {
    unsigned long sa_handler;
    unsigned long sa_mask;
    int sa_flags;
} k_sigaction_t;

__attribute__((weak)) sighandler_t signal(int sig, sighandler_t handler) {
    struct sigaction act;
    struct sigaction old;

    if (sig <= 0 || sig >= 32) {
        errno = EINVAL;
        return SIG_ERR;
    }

    act.sa_handler = handler;
    act.sa_mask = 0;
    act.sa_flags = 0;
    if (sigaction(sig, &act, &old) != 0) {
        return SIG_ERR;
    }
    return old.sa_handler;
}

__attribute__((weak)) int sigaction(int sig, const struct sigaction *act, struct sigaction *oldact) {
    k_sigaction_t kact;
    k_sigaction_t kold;
    int rc;

    if (sig <= 0 || sig >= 32) {
        errno = EINVAL;
        return -1;
    }

    if (act) {
        kact.sa_handler = (unsigned long)act->sa_handler;
        kact.sa_mask = (unsigned long)act->sa_mask;
        kact.sa_flags = act->sa_flags;
    }

    rc = sys_sigaction(sig, act ? &kact : NULL, oldact ? &kold : NULL);
    if (rc < 0) {
        errno = EINVAL;
        return -1;
    }

    if (oldact) {
        oldact->sa_handler = (sighandler_t)kold.sa_handler;
        oldact->sa_mask = (sigset_t)kold.sa_mask;
        oldact->sa_flags = kold.sa_flags;
    }

    return 0;
}

__attribute__((weak)) int sigprocmask(int how, const sigset_t *set, sigset_t *oldset) {
    if (sys_sigprocmask(how,
                        (const unsigned long *)set,
                        (unsigned long *)oldset) < 0) {
        errno = EINVAL;
        return -1;
    }
    return 0;
}

__attribute__((weak)) int sigpending(sigset_t *set) {
    if (!set) {
        errno = EINVAL;
        return -1;
    }
    if (sys_sigpending((unsigned long *)set) < 0) {
        errno = EINVAL;
        return -1;
    }
    return 0;
}

__attribute__((weak)) int raise(int sig) {
    if (sig <= 0 || sig >= 32) {
        errno = EINVAL;
        return -1;
    }

    if (sys_kill_signal(-1, sig) < 0) {
        errno = EINVAL;
        return -1;
    }
    return 0;
}

__attribute__((weak)) int kill(pid_t pid, int sig) {
    if (pid <= 0) {
        errno = EINVAL;
        return -1;
    }

    if (sys_kill_signal((int)pid, sig) < 0) {
        errno = ENOTSUP;
        return -1;
    }

    return 0;
}
