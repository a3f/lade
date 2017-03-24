#include "lade.h"
#include "bootstrap.h"
#include "mach_inject.h"

#include <mach/mach.h>
#include <dlfcn.h>
#include <unistd.h>

#include <stdlib.h>
#include <stdio.h>

static lade_bootstrap_t *bootstrapfn;

static struct lade_t {
    int placeholder;
} lade_inject_success;

#define warning(...) do { \
    fputs("\033[31m", stdout); \
    fprintf(stderr, __VA_ARGS__); \
    fputs("\033[m", stdout); \
} while (0)

lade_t *
lade(pid_t pid, const char *dll, int flags)
{
    if (pid < 0 && !dll)
        return NULL; /* can't self inject self */

    if ((pid < 0 || pid == getpid()) && !(flags & LADE_SELF_INJECT))
        return dlopen(dll, RTLD_LAZY);

    if (!dll) {
        return NULL; /* Inferring self name not yet supported */
    }

    // FIXME: turn relative to absolute path

    if (!bootstrapfn) {
        if (flags & LADE_VERBOSE)
            warning("Failed to inject: bootstrapfn:%p\n", (void*)bootstrapfn);
        return NULL;
    }

    mach_error_t err = mach_inject(bootstrapfn, dll, strlen(dll) + 1, pid, 0);
    
    return err == ERR_SUCCESS ? &lade_inject_success : NULL;
}

/* User should never have to call those; linker will */
void lade_init(void) __attribute((constructor));
void lade_finish(void) __attribute((destructor));

static void *module;
void
lade_init(void)
{
    if ((module = dlopen("liblade-bootstrap.dylib", RTLD_NOW | RTLD_LOCAL)))
        bootstrapfn = (lade_bootstrap_t*)dlsym(module, "lade_bootstrap");
}
void
lade_finish(void)
{
    dlclose(module);
}


