/* HSpace (Hemlock Space) is an exclusive product of the Hemlock MUSH series.
 * All code contained within the space and ship files are the sole ownership
 * of Gepht (Kyle M Forbes).  Any distribution of the source code shall be
 * under the permission of Gepht and must contain this header.
 *
 * Contained within space.h are the necessary definitions for planets, ships,
 * sensor contacts, and any other type of object that might be included with
 * space.
 */

#ifndef __SPACE_INCLUDED
#define __SPACE_INCLUDED

#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include "config.h"

#ifdef I_STRING
#include <string.h>
#else
#include <strings.h>
#endif

#ifdef I_SYS_TYPES
#include <sys/types.h>
#endif

#ifdef I_TIME
#include <time.h>
#endif

#ifdef I_SYS_TIME
#include <sys/time.h>
#endif

#ifdef I_STDLIB
#include <stdlib.h>
#endif

#include <locale.h>


/* lua headers */
//#include "lua.h"
//#include "lauxlib.h"
//#include "lualib.h"

/* pennmush headers */
#include "conf.h"
#include "flags.h"
#include "externs.h"
#include "match.h"
#include "dbdefs.h"
#include "ansi.h"
#include "attrib.h"
#include "lock.h"
#include "function.h"
#include "command.h"
#include "switches.h"
#include "parse.h"
#include "notify.h"
#include "strutil.h"

/* hspace headers */
#include "hsdefines.h"
#include "hstypes.h"
#include "hsexterns.h"

#endif


