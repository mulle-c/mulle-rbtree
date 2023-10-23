#include <mulle-rbtree/mulle-rbtree.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>


static int  callback( void *payload, void *userinfo)
{
   printf( "%s\n", (char *) payload);
   return( 1);
}


//
// https://stackoverflow.com/questions/12252103/read-line-from-stdin-blocking
//
int main( int argc, const char * argv[])
{
   char                   *line = NULL;
   size_t                 size = 0;
   size_t                 len;
   struct mulle_rbtree    tree;

   mulle_rbtree_init( &tree,
                      (int (*)( void *, void *)) strcmp,
                      &mulle_container_valuecallback_copied_cstring,
                      NULL);

   for(;;)
   {
      if( getline( &line, &size, stdin) == -1)
      {
         free( line);
         break;
      }

      len = strlen( line);
      assert( len);

      if( line[ len - 1] == '\n')
         line[ len - 1] = 0;

      mulle_rbtree_add( &tree, line);
      fprintf( stderr , "inserted: %s\n", line);
   }

   mulle_rbtree_walk( &tree, callback, NULL);
   mulle_rbtree_done( &tree);

   return( 0);
}
