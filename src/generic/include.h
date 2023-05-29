#ifndef mulle__rbtree_include_h__
#define mulle__rbtree_include_h__

// this is a global symbol that is exposed, which can be used to detect
// if this library is available

#define HAVE_INCLUDE_MULLE__RBTREE

/* DO:    #include this files in public headers.

   DONT:  #include this files in private headers.
          #include this file directly in sources.
          #import this file anywhere (except in import.h)
 */


/* This is a central include file to keep dependencies out of the library
   C files. It is usually included by .h files only.

   As the name "include.h" is standardized across mulle-sde projects,
   .c and .h files are now  motile. They can be moved to other projects and
   don't need to be edited.

   If you delete it, it will not be regenerated.
*/

/*
 * Include the header file generated by mulle-sde reflect.
 */

#include "_mulle-rbtree-include.h"

#ifndef MULLE__RBTREE_GLOBAL
# ifdef MULLE__RBTREE_BUILD
#  define MULLE__RBTREE_GLOBAL    MULLE_C_GLOBAL
# else
#  if defined( MULLE__RBTREE_INCLUDE_DYNAMIC) || (defined( MULLE_INCLUDE_DYNAMIC) && ! defined( MULLE__RBTREE_INCLUDE_STATIC))
#   define MULLE__RBTREE_GLOBAL   MULLE_C_GLOBAL
#  else
#   define MULLE__RBTREE_GLOBAL   extern
#  endif
# endif
#endif

/* You can add some more include statements here */

#endif
