//
//  dirty-extra-test.c
//  mulle-rbtree
//
//  Test for combined mulle_rbtree_option_use_extra and mulle_rbtree_option_use_dirty
//  functionality with cumulative totals. Each node stores a value and maintains
//  a total that is the sum of left child + right child + own value.
//

#include "include.h"
#include <stdio.h>
#include <string.h>

struct total_node
{
   int   value;
   int   total;
};

static int  compare_value( void *a, void *b)
{
   struct total_node *node_a = (struct total_node *) a;
   struct total_node *node_b = (struct total_node *) b;
   
   if( node_a->value < node_b->value)
      return( -1);
   if( node_a->value > node_b->value)
      return( 1);
   return( 0);
}

static void  dirty_callback( void *node, void *left, void *right)
{
   struct total_node *n = (struct total_node *) node;
   struct total_node *l = (struct total_node *) left;
   struct total_node *r = (struct total_node *) right;
   
   int left_total  = l ? l->total : 0;
   int right_total = r ? r->total : 0;
   
   fprintf( stderr, "DIRTY CALLBACK: node=%p(val=%d), left=%p(val=%d,tot=%d), right=%p(val=%d,tot=%d)\n",
            (void*)n, n ? n->value : -1,
            (void*)l, l ? l->value : -1, left_total,
            (void*)r, r ? r->value : -1, right_total);
   
   n->total = left_total + right_total + n->value;
   fprintf( stderr, "  -> new total for node %d: %d\n", n->value, n->total);
}

static void  print_tree( struct mulle_rbtree *tree)
{
   struct total_node *node;
   int count = 0;
   
   printf( "Tree contents (value:total):\n");
   mulle_rbtree_for( tree, node)
   {
      printf( "  Node %d: value=%d, total=%d\n", ++count, node->value, node->total);
   }
}

static void  test_dirty_extra_functionality( void)
{
   struct mulle_rbtree           tree;
   struct mulle_rbtree_config    config;
   struct total_node             nodes[5];
   struct total_node             *found;
   int                           i;
   
   printf( "Testing combined extra + dirty functionality with cumulative totals\n");
   printf( "===================================================================\n");
   
   // Initialize config for extra + dirty functionality
   config.comparison = compare_value;
   config.dirty      = dirty_callback;
   config.callback   = NULL;
   config.node_extra = sizeof( struct total_node);
   config.options    = mulle_rbtree_option_use_extra | mulle_rbtree_option_use_dirty;
   
   mulle_rbtree_init_with_config( &tree, &config, NULL);
   
   // Create test nodes with values
   int values[] = {30, 20, 40, 10, 50};
   for( i = 0; i < 5; i++)
   {
      nodes[i].value = values[i];
      nodes[i].total = values[i]; // Initial total is just the value
   }
   
   printf( "\nInserting nodes:\n");
   for( i = 0; i < 5; i++)
   {
      printf( "  Inserting value %d\n", nodes[i].value);
      mulle_rbtree_add( &tree, &nodes[i]);
   }
   
   printf( "\nTree count: %zu\n", _mulle__rbtree_get_count( (struct mulle__rbtree *) &tree));
   
   // Totals should be automatically updated by dirty mechanism
   
   // Print tree with totals
   print_tree( &tree);
   
   // Test removal and verify totals are updated
   printf( "\nTesting removal and total recalculation:\n");
   struct total_node search_key;
   search_key.value = 30; // Remove root
   if( mulle_rbtree_remove( &tree, &search_key) == 0)
   {
      printf( "  Successfully removed value 30 (root)\n");
   }
   
   printf( "\nTree after removal:\n");
   print_tree( &tree);
   
   printf( "\nFinal tree count: %zu\n", _mulle__rbtree_get_count( (struct mulle__rbtree *) &tree));
   
   mulle_rbtree_done( &tree);
   
   // Build tree again to demonstrate leaf deletion
   printf( "\n\nBuilding tree again for leaf deletion demonstration\n");
   printf( "====================================================\n");
   
   mulle_rbtree_init_with_config( &tree, &config, NULL);
   
   // Create test nodes with values for leaf deletion test
   struct total_node leaf_nodes[7];
   int leaf_values[] = {50, 30, 70, 20, 40, 60, 80};
   for( i = 0; i < 7; i++)
   {
      leaf_nodes[i].value = leaf_values[i];
      leaf_nodes[i].total = leaf_values[i];
   }
   
   printf( "\nInserting nodes for leaf deletion test:\n");
   for( i = 0; i < 7; i++)
   {
      printf( "  Inserting value %d\n", leaf_nodes[i].value);
      mulle_rbtree_add( &tree, &leaf_nodes[i]);
   }
   
   printf( "\nTree count: %zu\n", _mulle__rbtree_get_count( (struct mulle__rbtree *) &tree));
   
   // Totals should be automatically updated by dirty mechanism
   
   // Print tree with totals
   print_tree( &tree);
   
   // Test leaf node removals
   printf( "\nTesting leaf node removals:\n");
   
   // Remove leaf nodes (20, 40, 60, 80 are leaves)
   int leaf_removals[] = {20, 40, 60, 80};
   for( i = 0; i < 4; i++)
   {
      search_key.value = leaf_removals[i];
      if( mulle_rbtree_remove( &tree, &search_key) == 0)
      {
         printf( "  Successfully removed leaf value %d\n", leaf_removals[i]);
         printf( "  Tree after removing %d:\n", leaf_removals[i]);
         print_tree( &tree);
         printf( "\n");
      }
   }
   
   printf( "\nFinal tree count after leaf deletions: %zu\n", _mulle__rbtree_get_count( (struct mulle__rbtree *) &tree));
   
   mulle_rbtree_done( &tree);
   printf( "\nTest completed successfully!\n");
}

int  main( void)
{
   test_dirty_extra_functionality();
   return( 0);
}