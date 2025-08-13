#include "include.h"

static char  *print_intptr_value( void *value)
{
   intptr_t i;
   char     *s;

   i = (intptr_t) value;
   asprintf( &s, "%d", (int) i);
   return( s);
}

static int compare_intptr(void *a, void *b)
{
    intptr_t ia = (intptr_t)a;
    intptr_t ib = (intptr_t)b;
    return (int) (ia - ib);
}

int main( void)
{
   struct mulle_rbtree   tree;
   int                   values[ 7] = { 50, 30, 70, 20, 40, 60, 80};
   int                   i;

   mulle_rbtree_init( &tree, compare_intptr, &mulle_container_valuecallback_intptr, NULL);

   printf( "ASCII Tree Printer Test\n");
   printf( "=======================\n\n");

   // Test 1: Empty tree
   printf( "1. Empty tree:\n");
   mulle__rbtree_node_ascii_fprintf( stdout, (struct mulle__rbtree *) &tree, print_intptr_value);
   printf( "\n");

   // Test 2: Single node
   mulle_rbtree_add( &tree, (void *) values[ 0]);
   printf( "2. Single node (50):\n");
   mulle__rbtree_node_ascii_fprintf( stdout, (struct mulle__rbtree *) &tree, print_intptr_value);
   printf( "\n");

   // Test 3: Multiple nodes
   for( i = 1; i < 7; i++)
      mulle_rbtree_add( &tree, (void *) values[ i]);

   printf( "3. Complete tree:\n");
   mulle__rbtree_node_ascii_fprintf( stdout, (struct mulle__rbtree *) &tree, print_intptr_value);
   printf( "\n");

   // Test 4: With dot output for comparison
   printf( "4. Same tree in DOT format:\n");
   mulle__rbtree_node_dot_fprintf( stdout, (struct mulle__rbtree *) &tree, print_intptr_value);
   printf( "\n");

   mulle_rbtree_done( &tree);
   return( 0);
}