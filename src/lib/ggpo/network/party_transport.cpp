/* -----------------------------------------------------------------------
 * GGPO.net (http://ggpo.net)  -  Copyright 2009 GroundStorm Studios, LLC.
 *
 * Use of this software is governed by the MIT license that can be found
 * in the LICENSE file.
 */

#include "party_transport.h"

#ifdef _WIN32
#  include <windows.h>
#else
#  include <dlfcn.h>
#endif

#include <stddef.h>

PartyLibHandle hinstLib = NULL;

void*
PartyLibSymbol(const char* name)
{
   if (hinstLib == NULL) {
      return NULL;
   }
#ifdef _WIN32
   return (void*)GetProcAddress((HMODULE)hinstLib, name);
#else
   return dlsym(hinstLib, name);
#endif
}
