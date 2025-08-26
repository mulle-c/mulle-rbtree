//
//  insert-case6.c
//  mulle-rbtree
//
// The current node N is now certain to be an "outer" grandchild of G (left of
// left child or right of right child). Now (1-dir)-rotate at G, putting P in
// place of G and making P the parent of N and G. G is black and its former
// child P is red, since requirement 3 was violated. After switching the colors
// of P and G the resulting tree satisfies requirement 3. Requirement 4 also
// remains satisfied, since all paths that went through the black G now go
// through the black P.
//

#include "include.h"

static char *print_node_name(void *value)
{
   char *name = (char *) value;
   char *result = malloc(8);
   snprintf(result, 8, "%s", name);
   return result;
}

// Custom comparator for case 6a: N < G < P < U (outer left-left case)
static int case6a_compare(void *a, void *b)
{
   char *str_a = (char *)a;
   char *str_b = (char *)b;
   
   // Define ordering for case 6a: N=0, G=1, P=2, U=3
   int order_a = (*str_a == 'N') ? 0 : (*str_a == 'G') ? 1 : (*str_a == 'P') ? 2 : 3;
   int order_b = (*str_b == 'N') ? 0 : (*str_b == 'G') ? 1 : (*str_b == 'P') ? 2 : 3;
   
   return order_a - order_b;
}

// Custom comparator for case 6b: U < P < G < N (outer right-right case)
static int case6b_compare(void *a, void *b)
{
   char *str_a = (char *)a;
   char *str_b = (char *)b;
   
   // Define ordering for case 6b: U=0, P=1, G=2, N=3
   int order_a = (*str_a == 'U') ? 0 : (*str_a == 'P') ? 1 : (*str_a == 'G') ? 2 : 3;
   int order_b = (*str_b == 'U') ? 0 : (*str_b == 'P') ? 1 : (*str_b == 'G') ? 2 : 3;
   
   return order_a - order_b;
}

static struct mulle_rbnode *create_node(struct mulle__rbtree *tree, char *name, int color)
{
   struct mulle_rbnode *node = _mulle__rbtree_new_node(tree, name);
   node->_color = color;
   return node;
}

static void test_case6a_left_left(void)
{
   struct mulle_rbtree tree;
   char *err;

   printf("=== Insert Case 6a: Left-Left Configuration ===\n");
   printf("Building tree: U, P, G, N (N < G < P < U ordering)\n");
   
   // Initialize tree with case 6a comparator
   mulle_rbtree_init(&tree, case6a_compare, &mulle_container_valuecallback_nonowned_cstring, NULL);

   // Build the base tree step by step
   printf("\n1. Insert U (becomes black root):\n");
   mulle_rbtree_add(&tree, "U");
   mulle__rbtree_node_ascii_fprintf(stdout, (struct mulle__rbtree *)&tree, print_node_name);
   
   printf("\n2. Insert P (red left child of U):\n");
   mulle_rbtree_add(&tree, "P");
   mulle__rbtree_node_ascii_fprintf(stdout, (struct mulle__rbtree *)&tree, print_node_name);
   
   printf("\n3. Insert G (red left child of P):\n");
   mulle_rbtree_add(&tree, "G");
   mulle__rbtree_node_ascii_fprintf(stdout, (struct mulle__rbtree *)&tree, print_node_name);
   
   printf("\n4. Insert N (should trigger case 6a - outer left grandchild):\n");
   mulle_rbtree_add(&tree, "N");
   mulle__rbtree_node_ascii_fprintf(stdout, (struct mulle__rbtree *)&tree, print_node_name);
   
   // Expected result: P(black) root with proper balancing
   printf("\nExpected after Case 6a: Tree rotated and recolored\n");
   
   // Validate final tree
   err = mulle__rbtree_validate((struct mulle__rbtree *)&tree);
   if (err)
      printf("\nValidation error: %s\n", err);
   else
      printf("\nTree is valid after insertion!\n");
   
   mulle_rbtree_done(&tree);
}

static void test_case6b_right_right(void)
{
   struct mulle_rbtree tree;
   char *err;

   printf("\n=== Insert Case 6b: Right-Right Configuration (Mirror) ===\n");
   printf("Building tree: U, P, G, N (U < P < G < N ordering)\n");
   
   // Initialize tree with case 6b comparator
   mulle_rbtree_init(&tree, case6b_compare, &mulle_container_valuecallback_nonowned_cstring, NULL);

   // Build the base tree step by step
   printf("\n1. Insert U (becomes black root):\n");
   mulle_rbtree_add(&tree, "U");
   mulle__rbtree_node_ascii_fprintf(stdout, (struct mulle__rbtree *)&tree, print_node_name);
   
   printf("\n2. Insert P (red right child of U):\n");
   mulle_rbtree_add(&tree, "P");
   mulle__rbtree_node_ascii_fprintf(stdout, (struct mulle__rbtree *)&tree, print_node_name);
   
   printf("\n3. Insert G (red right child of P):\n");
   mulle_rbtree_add(&tree, "G");
   mulle__rbtree_node_ascii_fprintf(stdout, (struct mulle__rbtree *)&tree, print_node_name);
   
   printf("\n4. Insert N (should trigger case 6b - outer right grandchild):\n");
   mulle_rbtree_add(&tree, "N");
   mulle__rbtree_node_ascii_fprintf(stdout, (struct mulle__rbtree *)&tree, print_node_name);
   
   // Expected result: Tree rotated and recolored
   printf("\nExpected after Case 6b: Tree rotated and recolored\n");
   
   // Validate final tree
   err = mulle__rbtree_validate((struct mulle__rbtree *)&tree);
   if (err)
      printf("\nValidation error: %s\n", err);
   else
      printf("\nTree is valid after insertion!\n");
   
   mulle_rbtree_done(&tree);
}

int main(void)
{
   test_case6a_left_left();
   test_case6b_right_right();
   return 0;
}
