/**
 * \file cli.c
 *
 * Copyright (C) 2017 Ahmad Fatoum <ahmad@a3f.at>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * This is the main file for the lade command-line utility
 */

#include "config.h"
#include "lade.h"
#include "cli.h"
#ifdef  _WIN32
#include "cli-win32.h"
#else
#include <unistd.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <getopt.h>

#include "ext/utarray.h"

#ifdef HAVE_LIBPROC_H
static pid_t getProcessByName(const char *name);
#endif

static const char *arg0;
static int
print_usage(bool verbose)
{
    int indent = printf("Usage: %s ", arg0);
    puts("[-av] ");
    printf("%*c", indent, ' ');
    puts("[-p pid] [-w title] [-c class] [-f exe]  [--] dll ...");
    
    printf("       %s -h\n", arg0);

    if (verbose) {
        puts("more...");
        return EXIT_SUCCESS;
    }
    return EXIT_FAILURE;
}

struct {
    unsigned verbose :1;
    unsigned failed  :1;
    unsigned await   :1;
    unsigned color   :1;
} opts = {0};

int
main(int argc, char *argv[])
{
    UT_array *dlls = NULL;
    pid_t pid = -1;
    int flags = 0;
    arg0 = argv[0];

    if (isatty(fileno(stderr)))
        opts.color = 1;

    utarray_new(dlls, &ut_str_icd);

    while (optind < argc) {
        int opt;
        if((opt = getopt(argc, argv, "ap:w:c:f:hv")) != -1) {
            switch(opt) {
                char *endptr;

                case 'f': /*   file name  */
#ifdef HAVE_LIBPROC_H
                    pid = getProcessByName(optarg);
                    if (pid < 0)
                        die("No matching process found.");
                break;
#endif
                case 'w': /* window title */
                case 'c': /* window class */
                    /*die("libpid support not compiled in. Please supply pid directly.");*/
                    die("Not yet supported");
                break;

                case 'p': /*  process id  */
                    if (*optarg == '-'
                    || !(pid = strtoul(optarg, &endptr, 0))
                    || optarg == endptr) {
                        warn("%s: argument must be process or job ID", optarg);
                    }
                    break;

                case 'a': /* await */
                    opts.await = 1;
                break;
                case 'h': return print_usage(true);
                case 'v': opts.verbose = 1; break;
                default:
                    break;
            }
        } else {
            char *dll = argv[optind++];
            if (access(dll, F_OK) == -1) {
                warn("File '%s' not found", dll);
                opts.failed = 1;
                continue;
            }

            utarray_push_back(dlls, &dll);
        }
    }

    if (!utarray_len(dlls))
        die("%s: No input files", arg0);

    for(const char **dll = utarray_front(dlls); dll; dll = utarray_next(dlls, dll)) {
        lade_t *h = lade(pid, *dll, flags);
        if (!h)
            die("Injecting %ld with %s failed.", (long)pid, *dll); 
    }

    if (opts.await && pid == -1) {
        puts("Waiting indefinitely... Press ^C to exit.");
        for (;;) pause();
    }

    return EXIT_SUCCESS;
}

#ifdef HAVE_LIBPROC_H

#include <libproc.h>

//FIXME: remove this, move it to libpid
// See xnu/ for LICENSE notice
pid_t getProcessByName(const char *name)
{
    int procCnt = proc_listpids(PROC_ALL_PIDS, 0, NULL, 0);
    pid_t pids[1024];
    memset(pids, 0, sizeof pids);
    proc_listpids(PROC_ALL_PIDS, 0, pids, sizeof(pids));

    for (int i = 0; i < procCnt; i++) {
        if (!pids[i]) continue;
        char curPath[PROC_PIDPATHINFO_MAXSIZE];
        char curName[PROC_PIDPATHINFO_MAXSIZE];
        memset(curPath, 0, sizeof curPath);
        proc_pidpath(pids[i], curPath, sizeof curPath);
        int len = strlen(curPath);
        if (len) {
            int pos = len;
            while (pos && curPath[pos] != '/')
                --pos;
            strcpy(curName, curPath + pos + 1);
            if (!strcmp(curName, name))
                return pids[i];
        }
    }
    return -1;
}

#endif

