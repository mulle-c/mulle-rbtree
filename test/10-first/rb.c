#include <mulle-rbtree/mulle-rbtree.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>


static int  callback( struct mulle_rbnode *node, void *userinfo)
{
   char   *payload;

   payload = mulle_rbnode_extract_payload( node);
   printf( "%s\n", payload);
   mulle_free( payload);

   return( 1);
}


//
// https://stackoverflow.com/questions/12252103/read-line-from-stdin-blocking
//
int main( int argc, const char * argv[])
{
   char                  *line = NULL;
   size_t                size;
   size_t                len;
   struct mulle__rbtree   tree;
   struct mulle_rbnode   *node;
   void                  *payload;

   _mulle__rbtree_init( &tree, NULL);

   for(;;)
   {
      if( getline( &line, &size, stdin) == -1)
      {
         if( getline( &line, 0, stdin) == -1)
         {
            free( line);
            break;
         }
      }

      len = strlen( line);
      assert( len);

      if( line[ len - 1] == '\n')
         line[ len - 1] = 0;

      node = _mulle__rbtree_new_node( &tree);
      mulle_rbnode_set_payload( node, mulle_strdup( line));
      _mulle__rbtree_insert_node( &tree, node, (void *) strcmp);
      fprintf( stderr , "inserted: %s\n", line);

      line = NULL;
   }

   _mulle__rbtree_walk( &tree, callback, NULL);

   _mulle__rbtree_done( &tree);

   return( 0);
}
