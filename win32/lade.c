#include "lade.h"

#include <stdio.h>
#include <stdlib.h>

#include <windows.h>

static inline void*
win_perror(const char *msg)
{
    char *err;
    FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER
            | FORMAT_MESSAGE_FROM_SYSTEM
            | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            GetLastError(),
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR)&err,
            0,
            NULL
    );
    if (msg && *msg) {
        fprintf(stderr, "%s: %s\n", (msg), err);
    } else {
        fputs(err, stderr);
    }
    LocalFree(err);
    return NULL;
}

struct lade_t {
    int placeholder;
};

lade_t *lade(pid_t pid, const char *dll, int flags)
{
    (void)flags;
    HANDLE proc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!proc)
        return win_perror("OpenProcess");
    // debug privs: http://stackoverflow.com/questions/865152/how-can-i-get-a-process-handle-by-its-name-in-c

    FARPROC LL = GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");
    if (!LL)
        return win_perror("GetProcAddress");

    SIZE_T dll_len = strlen(dll) + 1;

    LPVOID arg = VirtualAllocEx(proc, NULL, dll_len, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (arg == NULL)
        return win_perror("VirtualAllocEx");

    SIZE_T bytes_written;
    if (!WriteProcessMemory(proc, arg,  dll, dll_len, &bytes_written))
        return win_perror("WriteProcessMemory");

    HANDLE tid = CreateRemoteThread(proc, NULL, 0, (LPTHREAD_START_ROUTINE)LL, arg, 0, NULL);
    if (tid == NULL)
        return win_perror("CreateRemoteThread");

    return tid;
}

