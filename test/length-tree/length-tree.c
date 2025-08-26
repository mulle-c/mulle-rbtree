//
//  length-tree.c
//  mulle-a_tree
//
//  Test implementation of length tree using dirty callback
//

#include "include.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define NO_TRACE_FUZZ
//
// Length tree node structure stored in extra data
//
struct length_info {
    char   *text;        // Text content
    size_t length;       // Length of this text
    size_t total;        // Total length of subtree (left + right + self)
};



//
// Custom value callback functions for length_info management
// The length_info structure itself is part of the node extra data,
// so we only need to manage the text string within it
//
void *length_info_retain(struct mulle_container_valuecallback *callback,
                        void *p,
                        struct mulle_allocator *allocator)
{
    struct length_info *info = (struct length_info *) p;

    if (!info || !info->text)
        return info;

    // Duplicate the text string and update the structure in place
    char *text_copy = mulle_allocator_strdup(allocator, info->text);
    if (!text_copy)
        return NULL;

    info->text = text_copy;
    return info;
}

void length_info_release( struct mulle_container_valuecallback *callback,
                          void *p,
                          struct mulle_allocator *allocator)
{
    struct length_info *info = (struct length_info *) p;

    if (!info)
        return;

    if (info->text) {
        mulle_allocator_free(allocator, info->text);
        info->text = NULL;
    }
}

char *length_info_describe(struct mulle_container_valuecallback *callback,
                          void *p,
                          struct mulle_allocator **p_allocator)
{
    struct length_info *info = (struct length_info *) p;
    char buf[256];

    if (!info) {
        *p_allocator = NULL;
        return "(null)";
    }

    snprintf(buf, sizeof(buf), "length_info{text=\"%s\", length=%zu, total=%zu}",
             info->text ? info->text : "(null)", info->length, info->total);

    return mulle_allocator_strdup(*p_allocator, buf);
}

// Custom value callback structure
struct mulle_container_valuecallback length_info_callback =
{
    .retain   = length_info_retain,
    .release  = length_info_release,
    .describe = length_info_describe,
    .userinfo = NULL
};


//
// Dirty callback function for maintaining subtree totals
//
void   length_tree_dirty_callback(void *value, void *left, void *right)
{
    struct length_info *n = (struct length_info *) value;
    struct length_info *l = (struct length_info *) left;
    struct length_info *r = (struct length_info *) right;
    
    // total = left_subtree_total + current_length + right_subtree_total
    n->total = (l ? l->total : 0) + 
               n->length +
               (r ? r->total : 0);
}

//
// Position-based comparison function for length nodes
// Orders nodes by their position (cumulative position)
//      •  0, if the s1 and s2 are equal;
//      •  a negative value if s1 is less than s2;
//      •  a positive value if s1 is greater than s2.

int length_node_compare(void *a, void *b)
{
    struct length_info *node_a = (struct length_info *) a;
    struct length_info *node_b = (struct length_info *) b;
    
    // Compare based on position in document for lookup
    // This is used for finding nodes at specific positions
    size_t pos_a = node_a->total + node_a->length;
    size_t pos_b = node_b->total + node_b->length;
    
    if (pos_a < pos_b) return -1;
    if (pos_a > pos_b) return 1;
    return 0;
}

//
// Initialize length tree with proper configuration
//
void  length_tree_init(struct mulle_rbtree *tree)
{
    struct mulle_rbtree_config config =
    {
        .comparison = length_node_compare,
        .dirty      = length_tree_dirty_callback,
        .node_extra = sizeof(struct length_info),
        .callback   = &length_info_callback,
        .options    = mulle_rbtree_option_use_extra | mulle_rbtree_option_use_dirty
    };
    
    mulle_rbtree_init_with_config(tree, &config, NULL);

    if( _mulle__rbtree_get_extra_size( (struct mulle__rbtree *) tree) < sizeof(struct length_info))
      abort();
}


// Calculate total length of all text in tree
//
size_t   length_tree_get_total_length(struct mulle_rbtree *tree)
{
   struct length_info   *info;
   struct mulle_rbnode  *root;
   struct mulle_rbnode  *nil;

   nil  = _mulle__rbtree_get_nil_node( (struct mulle__rbtree *) tree);
   root = _mulle__rbtree_get_root_node( (struct mulle__rbtree *) tree);

   if( root == nil)
      return( 0);

   info = _mulle__rbtree_get_node_value( (struct mulle__rbtree *) tree, root);
   return( info->total);
}


char  *length_tree_validate_total_length( struct mulle_rbtree *tree)
{
   struct length_info   *info;
   size_t               total;

   total = 0;
   mulle_rbtree_for( tree, info)
   {
      total += info->length;
   }

   if( total != length_tree_get_total_length( tree))
      return( "tree total is wrong");
   return( NULL);
}


char  *length_tree_validate( struct mulle_rbtree *tree)
{
   char   *err;

   assert( ! _mulle__rbtree_is_dirty( (struct mulle__rbtree *) tree));

   err = mulle__rbtree_validate( (struct mulle__rbtree *) tree);
   if( err)
      return( err);

   return( length_tree_validate_total_length( tree));
}



void   _mulle__rbtree_insert_fixup( struct mulle__rbtree *a_tree,
                                    struct mulle_rbnode *x);

static int   always_right_comparison( void *a, void *b)
{
   return( +1);
}

//
// Add a text node to the length tree
//
char  *length_tree_add( struct mulle_rbtree *tree, const char *text)
{
   struct length_info       extra = { 0 };
   struct length_info       search = { 0 };
   struct mulle_rbnode      *node;
   struct mulle_rbnode      *root;
   struct mulle_rbnode      *nil;
   struct mulle_rbnode      *predecessor;
   struct mulle_allocator   *allocator;
   struct mulle__rbtree     *a_tree = (struct mulle__rbtree *) tree;
   void                     *value;
   int                      rval;
   char                     *err;

   root      = _mulle__rbtree_get_root_node( a_tree);
   nil       = _mulle__rbtree_get_nil_node( a_tree);
   allocator = _mulle_rbtree_get_allocator( tree);

   extra.text     = (char *) text;
   extra.length   = strlen( text);
   extra.total    = 0;  // Will be calculated by dirty callback

   // need payload for comparison before insertion, don't change it
   // afterwards
   value = (*tree->callback.retain)( &tree->callback,
                                     &extra,
                                     allocator);
   node  = _mulle__rbtree_new_node( (struct mulle__rbtree *) a_tree, value);
   rval  = _mulle__rbtree_insert_node( (struct mulle__rbtree *) a_tree,
                                       node,
                                       always_right_comparison);
   if( rval)
   {
      (*tree->callback.release)( &tree->callback,
                                 value,
                                 allocator);
      _mulle__rbtree_free_node( (struct mulle__rbtree *) a_tree, node);
      err = "did not worky";
   }
   else
   {
      _mulle_rbtree_walk_dirty( tree);
      err = length_tree_validate( tree);
   }
   return( err);
}


//
// Print tree structure for debugging
//
void   length_tree_print(struct mulle_rbtree *tree)
{
    struct mulle_rbtreeenumerator rover;
    struct length_info *info;

    printf("Length Tree Contents (in-order traversal):\n");
    printf("=========================================\n");
    
    mulle_rbtree_for( tree, info)
    {
         printf("Node: \"%s\" (length=%zu, total=%zu)\n",
                info->text, info->length, info->total);
    }
}



//
// Find node at a specific position using manual tree traversal
// Returns the node containing the position and sets offset to the position within that node
// Position is 0-based (first character is position 0)
//
struct length_find_result
{
   struct mulle_rbnode  *node;
   struct length_info   *info;
   size_t               offset;
};


struct length_find_result  length_tree_find( struct mulle_rbtree *tree,
                                             size_t position)
{
   struct mulle__rbtree *a_tree = (struct mulle__rbtree *) tree;
   struct mulle_rbnode *nil = _mulle__rbtree_get_nil_node(a_tree);
   struct mulle_rbnode *node = _mulle__rbtree_get_root_node(a_tree);
   size_t search_position = position;
   struct length_find_result   result = { 0 };

   if( ! tree || node == nil)
       return( result);

   while( node != nil)
   {
       struct length_info  *info = _mulle__rbtree_get_node_value(a_tree, node);
       struct mulle_rbnode *left = node->_left;

       // Get left subtree total
       size_t left_total = 0;
       if (left != nil) {
           struct length_info *left_info = _mulle__rbtree_get_node_value(a_tree, left);
           left_total = left_info->total;
       }

       // Case 1: Go left (position is in left subtree)
       if (search_position < left_total)
       {
           node = left;
           continue;
       }

       // Case 2: Found it (position is in current node)
       if (search_position < left_total + info->length)
       {
          result.node   = node;
          result.info   = info;
          result.offset = search_position - left_total;
          return( result);
       }

       // Case 3: Go right (position is in right subtree)
       search_position -= (left_total + info->length);
       node             = node->_right;
   }

    return( result);
}

struct length_info  *length_tree_find_info( struct mulle_rbtree *tree,
                                            size_t position,
                                            size_t *offset)
{
   struct length_find_result   result;

   result = length_tree_find( tree, position);
   if( ! result.node)
      return( NULL);

   *offset = result.offset;
   return( result.info);
}


// not "API"
struct mulle_rbnode   *_length_tree_attach( struct mulle_rbtree *tree,
                                            char *text,
                                            struct mulle_rbnode *parent,
                                            int direction)
{
   struct length_info   extra;
   void                 *value;
   struct mulle_rbnode  *node;
   struct mulle__rbtree *a_tree = (struct mulle__rbtree *) tree;

   extra.text   = (char *) text;
   extra.length = strlen( text);
   extra.total  = 0;  // Will be calculated by dirty callback

   // need payload for comparison before insertion, don't change it
   // afterwards
   value = (*tree->callback.retain)( &tree->callback,
                                     &extra,
                                     mulle_rbtree_get_allocator( tree));
   node  = _mulle__rbtree_new_node( (struct mulle__rbtree *) tree, value);

   // Use the library's attach function
   if( direction < 0)
      _mulle__rbtree_insert_node_before_node( a_tree, node, parent);
   else
      _mulle__rbtree_insert_node_after_node( a_tree, node, parent);

   // Mark as dirty for callback processing
   if( a_tree->_options & mulle_rbtree_option_use_dirty )
      _mulle__rbtree_mark_node_as_dirty(a_tree, node);

   return( node);
}

//
// Split a word at a specific position into two separate nodes
// Returns 0 on success, -1 on failure
// Position is 0-based (first character is position 0)
//
char  *length_tree_insert(struct mulle_rbtree *tree, char *text, size_t position)
{
   struct mulle__rbtree        *a_tree = (struct mulle__rbtree *) tree;
   struct mulle_allocator      *allocator;
   struct length_info          *info;
   struct mulle_rbnode         *node;
   struct mulle_rbnode         *nil;
   char                        *tail_text;
   char                        *err;
   struct length_find_result   result;
   size_t                      total;

   if( ! tree)
       return( "NULL tree");

   nil       = _mulle__rbtree_get_nil_node(a_tree);
   allocator = _mulle_rbtree_get_allocator(tree);

   // Find the node containing the position
   result = length_tree_find( tree, position);
   if( ! result.node)
   {
      // check if we append
      total = length_tree_get_total_length( tree);
      if( position != total)
         return( "position outta bounds");
      return( length_tree_add( tree, text));
   }

   if( result.offset == 0)
   {
      // insert ahead ez
      node = _length_tree_attach( tree, text, result.node, -1);
      if( ! node)
         return( "failed to attach leading node");

      _mulle_rbtree_walk_dirty(tree);
      err = length_tree_validate( tree);
      return( err);
   }


   // split original text according to position:  head|tail
   // we
   // a) add new text behind the node
   // b) add tail text behind new text node
   // c) shrink original to head and mark it as dirty
   // --> tree should be balanced here
   // Store original values

   /*
    * a)
    */
   node = _length_tree_attach( tree, text, result.node, +1);
   if( ! node)
      return( "failed to attach trailing node");
   // Validate after first attachment
   _mulle_rbtree_walk_dirty(tree);
   err = length_tree_validate( tree);
   if( err )
      return( err );
   // we are already done: the "just append" case
   if( result.offset >= result.info->length)
   {
      return( NULL );  // Success, no error
   }

   /*
    * b)
    */
   tail_text = &result.info->text[ result.offset];
   node      = _length_tree_attach( tree, tail_text, node, +1);
   if( ! node)
      return( "failed to hack original node");
   // Validate after second attachment
   _mulle_rbtree_walk_dirty(tree);
   err = length_tree_validate( tree);
   if( err )
      return( err );

   /*
    * c)
    */
   result.info->text[ result.offset] = 0;
   result.info->length               = result.offset;
   _mulle__rbtree_mark_node_as_dirty( a_tree, result.node);

   _mulle_rbtree_walk_dirty( tree);
   err = length_tree_validate( tree);
   return( err);
}


static char   *print_ascii_tree_value( void *value)
{
   struct length_info *info = value;
   char               *s;

   asprintf( &s, "%.3s %d", info->text, (int) info->total);
   return( s);
}


static void  add( struct mulle_rbtree *tree, char *s)
{
   char  *err;

   err = length_tree_add( tree, s);
   if( err)
   {
      printf("Failed to add \"%s\": %s\n", s, err);
      printf("\nFailed tree:\n");
      length_tree_print( tree);
      mulle__rbtree_node_ascii_fprintf(stdout, (struct mulle__rbtree *) tree, print_ascii_tree_value);
      abort();
   }
}

static void  insert( struct mulle_rbtree *tree, char *s, size_t position)
{
   char  *err;

//   printf("\nTree before insert \"%s\" at %td:\n", s, position);
//   length_tree_print( tree);
//   mulle__rbtree_node_ascii_fprintf(stdout, (struct mulle__rbtree *) tree, print_ascii_tree_value);

   err = length_tree_insert( tree, s, position);
   if( err)
   {
      printf("Failed to insert \"%s\" at %td: %s\n", s, position, err);
      printf("\nFailed tree:\n");
      length_tree_print( tree);
      mulle__rbtree_node_ascii_fprintf(stdout, (struct mulle__rbtree *) tree, print_ascii_tree_value);
      abort();
   }
}



/* Fuzz test runner without any reference vector/oracle.
 * Generates random strings, inserts them into the tree and
 * validates RB invariants after every operation.
 */


void
fuzz_test_run( int ops )
{
   int               i;
   size_t            pos;
   size_t            tot;
   char              s[32];
   const char       *err;
   struct mulle_rbtree tree;

   /* init */
   srand( 1848 );
   memset( &tree, 0, sizeof( tree ) );
   length_tree_init( &tree );

   /* build initial list */
   for( i = 0; i < 20; i++ )
   {
      int  k;
      int  len;

      len = 1 + rand() % 6;
      for( k = 0; k < len; k++ ) s[k] = 'a' + (rand() % 26);
      s[len] = '\0';

#ifndef NO_TRACE_FUZZ
      printf( "Add \"%s\"\n", s );
#endif
      add( &tree, s );

      err = length_tree_validate( &tree );
      if( err != NULL )
      {
         fprintf( stderr, "Initial build validation failed after Add \"%s\": %s\n",
                  s, err );
         length_tree_print( &tree );
         mulle__rbtree_node_ascii_fprintf( stdout, (struct mulle__rbtree *) &tree, print_ascii_tree_value );
         abort();
      }
   }

   for( i = 0; i < ops; i++ )
   {
      int  k;
      int  len;

      len = 1 + rand() % 4;
      for( k = 0; k < len; k++ ) s[k] = 'A' + (rand() % 26);
      s[len] = '\0';

      tot = length_tree_get_total_length( &tree );
      pos = tot ? (rand() % (tot + 1)) : 0;

#ifndef NO_TRACE_FUZZ
      printf( "Insert \"%s\" at %zu\n", s, pos );
#endif
      insert( &tree, s, pos );

      /* immediate structural sanity check */
      err = length_tree_validate( &tree );
      if( err != NULL )
      {
         fprintf( stderr, "Validation failed after Insert \"%s\" at %zu: %s\n", s, pos, err );
         length_tree_print( &tree );
         mulle__rbtree_node_ascii_fprintf( stdout, (struct mulle__rbtree *) &tree, print_ascii_tree_value );
         abort();
      }
   }

   /* cleanup */
   mulle_rbtree_done( &tree );
   return;
}

//
// Main test function
//
int main(void)
{
   struct mulle_rbtree tree;

   printf("=== Length Tree Test ===\n\n");

   // Initialize tree
   length_tree_init(&tree);

#if 1
    // Disable other tests to focus on failing case
   // Test 1: Basic insertion and position calculation
   printf("Test 1: Basic insertion\n");

   add(&tree, "I");
   add(&tree, "am");
   add(&tree, "a");
   add(&tree, "very");
   add(&tree, "fine");
   add(&tree, "quick");
   add(&tree, "and");
   add(&tree, "brown");
   add(&tree, "fox");

   length_tree_print(&tree);

   printf("\nTree structure:\n");
   mulle__rbtree_node_ascii_fprintf(stdout, (struct mulle__rbtree *) &tree, print_ascii_tree_value);

   // Test 2: Position lookup demo
   printf("\nTest 2: Position lookup demo\n");
   printf("============================\n");

   size_t total_length = length_tree_get_total_length(&tree);
   printf("Total document length: %zu characters\n", total_length);
   printf("\nCharacter-by-character breakdown:\n");

   for (size_t pos = 0; pos < total_length; pos++)
   {
       size_t offset;
       struct length_info *info = length_tree_find_info(&tree, pos, &offset);

       if (info && info->text)
       {
           printf("Position %zu: '%c' (word: \"%s\", offset: %zu)\n",
                  pos, info->text[offset], info->text, offset);
       }
       else
       {
           printf("Position %zu: <not found>\n", pos);
       }
   }
#endif

#if 1
   // Test 3: Split functionality
   printf("\nTest 3: Split functionality\n");
   printf("==========================\n");

   mulle_rbtree_done(&tree);
   length_tree_init(&tree);

   add(&tree, "Hello");
   add(&tree, "World");
   add(&tree, "Test");

   printf("Before split:\n");
   length_tree_print(&tree);

   printf("\nSplitting at position 2...\n");
   insert(&tree, "-", 2);
   printf("Split successful!\n");
   length_tree_print(&tree);
   printf("\nTree structure after split:\n");
   mulle__rbtree_node_ascii_fprintf(stdout, (struct mulle__rbtree *) &tree, print_ascii_tree_value);

   // Test 4: Split first word at various positions
   printf("\nTest 4: Split first word at various positions\n");
   printf("=============================================\n");

   mulle_rbtree_done(&tree);
   length_tree_init(&tree);

   add(&tree, "FirstWord");
   add(&tree, "SecondWord");
   add(&tree, "ThirdWord");

   printf("Before splitting first word:\n");
   length_tree_print(&tree);

   printf("\nSplitting first word at position 3...\n");
   insert(&tree, "INSERT", 3);
   printf("Split first word successful!\n");
   length_tree_print(&tree);

   printf("\nSplitting first word at position 1...\n");
   insert(&tree, "X", 1);
   printf("Split first word at position 1 successful!\n");
   length_tree_print(&tree);
   printf("\nTree structure after second split:\n");
   mulle__rbtree_node_ascii_fprintf(stdout, (struct mulle__rbtree *) &tree, print_ascii_tree_value);
#endif

#if 1
   // Test 5: Split second word
   printf("\nTest 5: Split second word at various positions\n");
   printf("==============================================\n");

   mulle_rbtree_done(&tree);
   length_tree_init(&tree);

   add(&tree, "Alpha");
   add(&tree, "BetaGamma");
   add(&tree, "Delta");

   printf("Before splitting second word:\n");
   length_tree_print(&tree);
   mulle__rbtree_node_ascii_fprintf(stdout, (struct mulle__rbtree *) &tree, print_ascii_tree_value);

   size_t second_word_start = 5;

   printf("\nSplitting second word at position 2 within word...\n");
   insert(&tree, "SPLIT", second_word_start + 2);
   printf("Split second word successful!\n");
   length_tree_print(&tree);
   mulle__rbtree_node_ascii_fprintf(stdout, (struct mulle__rbtree *) &tree, print_ascii_tree_value);

   printf("\nSplitting second word at position 0 within word...\n");
   insert(&tree, "START", second_word_start);
   printf("Split second word at start successful!\n");
   length_tree_print(&tree);
   mulle__rbtree_node_ascii_fprintf(stdout, (struct mulle__rbtree *) &tree, print_ascii_tree_value);
#endif

#if 1
   // Disable remaining tests to focus on failing case
   // Test 6: Multiple splits
   printf("\nTest 6: Multiple splits on same word\n");
   printf("====================================\n");

   mulle_rbtree_done(&tree);
   length_tree_init(&tree);

   add(&tree, "ABCDEFGHIJ");
   add(&tree, "After");

   printf("Before multiple splits:\n");
   length_tree_print(&tree);

   printf("\nFirst split at position 3...\n");
   insert(&tree, "X1", 3);
   printf("First split successful!\n");
   length_tree_print(&tree);

   printf("\nSecond split at position 5 (within original word)...\n");
   insert(&tree, "X2", 5);
   printf("Second split successful!\n");
   length_tree_print(&tree);

   // Edge cases
   printf("\nTesting edge cases:\n");
   printf("Split at position 0 (boundary):\n");
   insert(&tree, ">>>", 0);

   size_t total_len = length_tree_get_total_length(&tree);
   printf("Split at end position %zu (boundary):\n", total_len);
   insert(&tree, "<<<", total_len);

   printf("\nFinal tree state:\n");
   length_tree_print(&tree);
   printf("\nFinal tree structure:\n");
   mulle__rbtree_node_ascii_fprintf(stdout, (struct mulle__rbtree *) &tree, print_ascii_tree_value);
#endif

#if 1
   // Disable other tests to focus on failing case
   // Test 7:
   printf("Test 1: Basic insertion\n");
   mulle_rbtree_done(&tree);
   length_tree_init(&tree);
   add(&tree, "AB");
   add(&tree, "CD");
   add(&tree, "EF");
   add(&tree, "GH");
   add(&tree, "IJ");
   add(&tree, "KL");
   add(&tree, "MN");
   add(&tree, "OP");

   int len = length_tree_get_total_length( &tree);

   for( int i = 0; i <= len; i+=2)
   {
      insert( &tree, "-", i);
      ++len;
      length_tree_print(&tree);
      mulle__rbtree_node_ascii_fprintf(stdout, (struct mulle__rbtree *) &tree, print_ascii_tree_value);
   }

   printf("\nTree structure:\n");
   mulle__rbtree_node_ascii_fprintf(stdout, (struct mulle__rbtree *) &tree, print_ascii_tree_value);
#endif
   mulle_rbtree_done(&tree);

#if 1  // Disable other tests to focus on failing case
   fuzz_test_run( 1000);
#endif

   return 0;
}
