#ifndef LADE_BOOTSTRAP_H_
#define LADE_BOOTSTRAP_H_

#include <stddef.h>

typedef void lade_bootstrap_t(ptrdiff_t offset, void *param, size_t psize, void *dummy);
lade_bootstrap_t bootstrap __attribute__((visibility("default")));

#endif
