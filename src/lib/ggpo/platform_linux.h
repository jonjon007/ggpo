/* -----------------------------------------------------------------------
 * GGPO.net (http://ggpo.net)  -  Copyright 2009 GroundStorm Studios, LLC.
 *
 * Use of this software is governed by the MIT license that can be found
 * in the LICENSE file.
 */

#ifndef _GGPO_LINUX_H_
#define _GGPO_LINUX_H_

// POSIX platform layer, used for macOS as well as Linux.

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#ifndef MAX_PATH
#  define MAX_PATH PATH_MAX
#endif

#ifndef INFINITE
#  define INFINITE (-1)
#endif

#ifndef FALSE
#  define FALSE 0
#endif

class Platform {
public:  // types
   typedef pid_t ProcessID;

public:  // functions
   static ProcessID GetProcessID() { return getpid(); }
   static void AssertFailed(char *msg) { fprintf(stderr, "GGPO Assertion Failed: %s\n", msg); }
   static uint32 GetCurrentTimeMS();
   static void SleepMS(int ms) { usleep((useconds_t)ms * 1000); }
};

#endif
