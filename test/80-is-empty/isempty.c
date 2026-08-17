#include <mulle-rbtree/mulle-rbtree.h>

#include <string.h>
#include <assert.h>
#include <stdio.h>


static int  string_compare( void *a, void *b)
{
   return( strcmp( (char *) a, (char *) b));
}


int  main( void)
{
   struct mulle_rbtree   tree;

   printf( "=== Test: mulle_rbtree_is_empty ===\n");

   mulle_rbtree_init( &tree,
                      string_compare,
                      &mulle_container_valuecallback_nonowned_cstring,
                      NULL);

   // a freshly initialized tree is empty
   printf( "empty after init: %d (expect 1)\n", mulle_rbtree_is_empty( &tree));
   assert( mulle_rbtree_is_empty( &tree));

   // adding values makes it non-empty
   mulle_rbtree_add( &tree, "bravo");
   mulle_rbtree_add( &tree, "alpha");
   mulle_rbtree_add( &tree, "charlie");
   printf( "empty after 3 adds: %d (expect 0)\n", mulle_rbtree_is_empty( &tree));
   assert( ! mulle_rbtree_is_empty( &tree));

   // removing a missing key leaves it non-empty
   assert( mulle_rbtree_remove( &tree, "zulu") == ENOENT);
   printf( "empty after failed remove: %d (expect 0)\n", mulle_rbtree_is_empty( &tree));
   assert( ! mulle_rbtree_is_empty( &tree));

   // removing all values empties it again
   assert( mulle_rbtree_remove( &tree, "alpha") == 0);
   assert( mulle_rbtree_remove( &tree, "bravo") == 0);
   assert( mulle_rbtree_remove( &tree, "charlie") == 0);
   printf( "empty after removing all: %d (expect 1)\n", mulle_rbtree_is_empty( &tree));
   assert( mulle_rbtree_is_empty( &tree));

   // a NULL tree is treated as empty (graceful)
   printf( "empty for NULL tree: %d (expect 1)\n", mulle_rbtree_is_empty( NULL));
   assert( mulle_rbtree_is_empty( NULL));

   mulle_rbtree_done( &tree);
   printf( "PASSED\n");

   return( 0);
}
