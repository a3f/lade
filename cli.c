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
#include "cli-utils.h"
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


struct {
    unsigned verbose :1;
    unsigned failed  :1;
    unsigned await   :1;
    unsigned noarg   :1;
    unsigned color   :1;
} opts = {0};

#define SHIM_RETURN die("%s", msg);
#include "libpid-shim.h"

static const char *arg0;
static int
print_usage(bool verbose)
{
    int indent = printf("Usage: %s ", arg0);
    puts("[-av] ");
    printf("%*c", indent, ' ');
    puts("[-p pid] [-w title] [-c class] [-f exe]  [-Ldir] [-ldll] [--] [dll] ...");
    
    printf("       %s [exe] ... [-Ldir] -ldll \n" , arg0);
    printf("       %s -h\n", arg0);

    if (verbose) {
        puts("more...");
        return EXIT_SUCCESS;
    }
    return EXIT_FAILURE;
}

int
main(int argc, char *argv[])
{
    UT_array *dlls = NULL;
    pid_t pid = -1;
    char *wndtitle = NULL, *wndclass = NULL;
    int flags = 0;
    arg0 = argv[0];

    if (isatty(fileno(stderr)))
        opts.color = 1;

    utarray_new(dlls, &ut_str_icd);

    while (optind < argc) {
        int opt;
        if((opt = getopt(argc, argv, "ap:w:c:f:hvl:L:x")) != -1) {
            switch(opt) {
                char *endptr;

                case 'L':
                    if (chdir(optarg) == -1) 
                        die("Invalid path -L%s", optarg);
                    break;
                case 'l': {
                              char *dll = concat(DLL_PREFIX, optarg, DLL_SUFFIX, 0);
                              if (access(dll, F_OK) == -1) {
                                  warn("File '%s' not found", dll);
                                  opts.failed = 1;
                                  continue;
                              }

                              utarray_push_back(dlls, &dll);
                          }
                          break;
                case 'f': /*   file name  */
                    // FIXME: doesn't work on windows
                    pid = pid_byname(optarg);
                    if (pid < 0)
                        die("No matching process found.");

                break;
                case 'w': /* window title */
                    wndtitle = optarg;
                    break;
                case 'c': /* window class */
                    wndclass = optarg;
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
                case 'x':
                    flags = LADE_CRASH;
                    opts.noarg = 1;
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

    if (wndclass || wndtitle)
        pid = pid_bywindow(wndclass, wndtitle);

    if (opts.noarg)
        return lade(pid, NULL, flags) != NULL;
    else if (!utarray_len(dlls))
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


