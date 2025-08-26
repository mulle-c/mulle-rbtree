//
//  insert-case4.c
//  mulle-rbtree
//
// The parent P is red and the root. Because N is also red,
// requirement 3 is violated. But after switching P’s color the tree is in
// RB-shape. The black height of the tree increases by 1.


#include "include.h"

static char *print_node_name(void *value)
{
   char *name = (char *) value;
   char *result = malloc(8);
   snprintf(result, 8, "%s", name);
   return result;
}

// Custom comparator for case 4a: P < N < G < U (zig-zag left-right)
static int case4a_compare(void *a, void *b)
{
   char *str_a = (char *)a;
   char *str_b = (char *)b;
   
   // Define ordering: P=0, N=1, G=2, U=3
   int order_a = (*str_a == 'P') ? 0 : (*str_a == 'N') ? 1 : (*str_a == 'G') ? 2 : 3;
   int order_b = (*str_b == 'P') ? 0 : (*str_b == 'N') ? 1 : (*str_b == 'G') ? 2 : 3;
   
   return order_a - order_b;
}

// Custom comparator for case 4b: U < G < N < P (zig-zag right-left)  
static int case4b_compare(void *a, void *b)
{
   char *str_a = (char *)a;
   char *str_b = (char *)b;
   
   // Define ordering: U=0, G=1, N=2, P=3
   int order_a = (*str_a == 'U') ? 0 : (*str_a == 'G') ? 1 : (*str_a == 'N') ? 2 : 3;
   int order_b = (*str_b == 'U') ? 0 : (*str_b == 'G') ? 1 : (*str_b == 'N') ? 2 : 3;
   
   return order_a - order_b;
}

static void test_case4a_left_right(void)
{
   struct mulle_rbtree tree;
   char *err;

   printf("=== Insert Case 4a: Left-Right Configuration (Zig-Zag) ===\n");
   printf("Building tree by insertion: P, G, U, then N\n");
   printf("Expected: P(red) left child of G, N(red) right child of P\n");
   
   // Initialize tree with case 4a comparator
   mulle_rbtree_init(&tree, case4a_compare, &mulle_container_valuecallback_nonowned_cstring, NULL);

   printf("\n1. Insert P:\n");
   mulle_rbtree_add(&tree, "P");
   mulle__rbtree_node_ascii_fprintf(stdout, (struct mulle__rbtree *)&tree, print_node_name);
   
   printf("\n2. Insert G:\n");
   mulle_rbtree_add(&tree, "G");
   mulle__rbtree_node_ascii_fprintf(stdout, (struct mulle__rbtree *)&tree, print_node_name);
   
   printf("\n3. Insert U:\n");
   mulle_rbtree_add(&tree, "U");
   mulle__rbtree_node_ascii_fprintf(stdout, (struct mulle__rbtree *)&tree, print_node_name);
   
   printf("\n4. Insert N (should trigger case 4a -> case 5):\n");
   mulle_rbtree_add(&tree, "N");
   mulle__rbtree_node_ascii_fprintf(stdout, (struct mulle__rbtree *)&tree, print_node_name);
   
   // Validate final tree
   err = mulle__rbtree_validate((struct mulle__rbtree *)&tree);
   if (err)
      printf("\nValidation error: %s\n", err);
   else
      printf("\nTree is valid after insertion!\n");
   
   mulle_rbtree_done(&tree);
}

static void test_case4b_right_left(void)
{
   struct mulle_rbtree tree;
   char *err;

   printf("\n=== Insert Case 4b: Right-Left Configuration (Zig-Zag) ===\n");
   printf("Building tree by insertion: G, U, P, then N\n");
   printf("Expected: P(red) right child of G, N(red) left child of P\n");
   
   // Initialize tree with case 4b comparator
   mulle_rbtree_init(&tree, case4b_compare, &mulle_container_valuecallback_nonowned_cstring, NULL);

   printf("\n1. Insert G:\n");
   mulle_rbtree_add(&tree, "G");
   mulle__rbtree_node_ascii_fprintf(stdout, (struct mulle__rbtree *)&tree, print_node_name);
   
   printf("\n2. Insert U:\n");
   mulle_rbtree_add(&tree, "U");
   mulle__rbtree_node_ascii_fprintf(stdout, (struct mulle__rbtree *)&tree, print_node_name);
   
   printf("\n3. Insert P:\n");
   mulle_rbtree_add(&tree, "P");
   mulle__rbtree_node_ascii_fprintf(stdout, (struct mulle__rbtree *)&tree, print_node_name);
   
   printf("\n4. Insert N (should trigger case 4b -> case 5):\n");
   mulle_rbtree_add(&tree, "N");
   mulle__rbtree_node_ascii_fprintf(stdout, (struct mulle__rbtree *)&tree, print_node_name);
   
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
   test_case4a_left_right();
   test_case4b_right_left();
   return 0;
}
