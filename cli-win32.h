#ifndef LADE_CLI_WIN32_H
#define LADE_CLI_WIN32_H

#include <io.h>
#include <windows.h>
#include <direct.h>

// TODO: Replace these with Wide char counterparts
static inline int
access(const char *path, int mode)
{
    return _access(path, mode);
}

int __cdecl isatty(int fd);

static inline int
pause(void)
{
    Sleep(INFINITE);
    return -1;
}

static inline int
chdir(const char *dirname)
{
    return _chdir(dirname);
}

#endif
