#include "bootstrap.h"
#include <stdio.h>

#include <mach/thread_act.h>
#include <mach/mach_init.h>
#include <pthread.h>
#include <dlfcn.h>

extern void _pthread_set_self(void*);

static void *
loaderThread(void *patch_bundle)
{
    void *bundle = dlopen(patch_bundle, RTLD_NOW);
    if (!bundle)
        fprintf(stderr, "Could not load patch bundle: %s\n", dlerror());
    return NULL;
}

void
lade_bootstrap(ptrdiff_t offset, void *param, size_t psize, void *dummy)
{
    (void)offset;
    (void)psize;

    _pthread_set_self(dummy);

    pthread_attr_t attr;
    pthread_attr_init(&attr); 
    
    int policy;
    pthread_attr_getschedpolicy(&attr, &policy);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    
    struct sched_param sched;
    sched.sched_priority = sched_get_priority_max(policy);
    pthread_attr_setschedparam(&attr, &sched);

    pthread_t thread;
    pthread_create(&thread, &attr, loaderThread, param);
    pthread_attr_destroy(&attr);
    
    thread_suspend(mach_thread_self());
}

