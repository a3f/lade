#ifndef LADE_LIBPID_SHIM_H_
#define LADE_LIBPID_SHIM_H_

#include "config.h"

#ifdef HAVE_LIBPID
#include <libpid.h>
#define LADE_LIBPID_MSG(strliteral) \
    const char *msg = strliteral;
#else
#define LADE_LIBPID_MSG(strliteral) \
    const char *msg = "libpid support not compiled in. Please supply pid directly.";
#endif

#ifndef SHIM_ACTION
#define SHIM_ACTION return -1;
#endif

#ifndef pid_byname
static inline pid_t pid_byname(const char *arg) {
    LADE_LIBPID_MSG("Retrieveing PID by executable name not supported on this platform.")
    (void)msg; (void)arg;
    SHIM_RETURN
}
#endif

#ifndef pid_bywindow
static inline pid_t pid_bywindow(const char *arg1, const char *arg2) {
    LADE_LIBPID_MSG("Retrieveing PID by window not supported on this platform.")
    (void)msg; (void)arg1; (void)arg2;
    SHIM_RETURN
}
#endif

#endif
