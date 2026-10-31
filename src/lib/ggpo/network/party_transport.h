/* -----------------------------------------------------------------------
 * GGPO.net (http://ggpo.net)  -  Copyright 2009 GroundStorm Studios, LLC.
 *
 * Use of this software is governed by the MIT license that can be found
 * in the LICENSE file.
 */

#ifndef _PARTY_TRANSPORT_H
#define _PARTY_TRANSPORT_H

// The PartyNet library GGPO sends through. UnityGGPO's PlayFab_Init loads it and stores the
// handle here; until then every lookup returns NULL, so a missing library means "never connects".

#ifdef _WIN32
#  define PARTY_CALL __stdcall
#else
#  define PARTY_CALL
#endif

typedef void* PartyLibHandle;

extern PartyLibHandle hinstLib;

// GetProcAddress / dlsym on hinstLib; NULL when the library isn't loaded or lacks the symbol.
void* PartyLibSymbol(const char* name);

#endif
