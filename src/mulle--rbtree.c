//
//  mulle_rb.c
//  MulleXLSReader
//
//  Created by Nat! on 24.10.14.
//  Copyright (c) 2014 Mulle kybernetiK. All rights reserved.
//

#include "mulle--rbtree.h"

#include <errno.h>

/*-
 * Copyright (C) 2006 Jason Evans <jasone@FreeBSD.org>.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice(s), this list of conditions and the following disclaimer as
 *    the first lines of this file unmodified other than the possible
 *    addition of one or more copyright notices.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice(s), this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER(S) ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER(S) BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *******************************************************************************
 *
 * Red-black trees are difficult to explain without lots of diagrams, so little 
 * attempt is made to document this code.  However, an excellent discussion can 
 * be found in the following book, which was used as the reference for writing 
 * this implementation:
 *
 *   Introduction to Algorithms
 *   Thomas H. Cormen, Charles E. Leiserson, and Ronald L. Rivest
 *   MIT Press (1990)
 *   ISBN 0-07-013143-0
 *
 * Some functions use a comparison function pointer, which is expected to have the
 * following prototype:
 *
 *   int (compare *)( void *payload_a, void *payload_b);
 *
 * Interpretation of comparison function return values:
 *
 *   <0 : a_a < a_b
 *    0 : a_a == a_b
 *   >0 : a_a > a_b
 *
 *******************************************************************************
 */

// if in read only memory, cant be changed, convenient catcher of bugs
// ... or not our algorithm actually clobbers ._parent but its intended...
// const struct mulle_rbnode    mulle_rbnode_nil =
// {
//    ._color = mulle__rbtree_black
// };
//


void   _mulle__rbtree_init_with_options( struct mulle__rbtree *a_tree,
                                         size_t node_extra_size,
                                         unsigned int options,
                                         struct mulle_allocator *allocator)
{
   struct mulle_rbnode   dummy;
   size_t                extra;

   memset( a_tree, 0, sizeof( struct mulle__rbtree));

   a_tree->_nil._left   = &a_tree->_nil;
   a_tree->_nil._right  = &a_tree->_nil;
   a_tree->_nil._parent = &a_tree->_nil;

   a_tree->_root        = &a_tree->_nil;
   a_tree->_options     = options;

   // so possibly extend node_extra for alignment
   // assert( extra && (options & mulle_rbtree_option_use_extra))
   // we'll let it slide...
   extra = &((char *) _mulle_rbnode_get_extra( &dummy))[ node_extra_size] - (char *) &dummy;

   _mulle_storage_init( &a_tree->_nodes,
                        sizeof( struct mulle_rbnode) + extra,
                        alignof( struct mulle_rbnode),
                        32,
                        allocator);
}


// the tree won't be cleaned off _nodes though, you have to do this
void   _mulle__rbtree_done( struct mulle__rbtree *a_tree)
{
   assert( a_tree);
   _mulle_storage_done( &a_tree->_nodes);
#ifdef DEBUG
   mulle_memset_uint32( a_tree, 0xDEADDEAD, sizeof( struct mulle__rbtree));
#endif
}


struct mulle_rbnode  *_mulle__rbtree_new_node( struct mulle__rbtree *a_tree,
                                               void *value)
{
   struct mulle_rbnode   *node;

   node = _mulle_storage_malloc( &a_tree->_nodes);
   _mulle__rbtree_init_node( a_tree, node, value);
   return( node);
}



struct mulle_rbnode   *
   _mulle__rbtree_find_next_node( struct mulle__rbtree *a_tree,
                                  struct mulle_rbnode *a_node)
{
   struct mulle_rbnode   *p;
   struct mulle_rbnode   *t;
   struct mulle_rbnode   *nil;

   nil = _mulle__rbtree_get_nil_node( a_tree);
   if( a_node->_right != nil)
      return( _mulle__rbtree_find_leftmost_node( a_tree, a_node->_right));
   
   t = a_node;
   p = a_node->_parent;
   
   while( p != nil && t == p->_right)
   {
      t = p;
      p = p->_parent;
   }
   return( p);
}


struct mulle_rbnode    *
   _mulle__rbtree_find_previous_node( struct mulle__rbtree *a_tree,
                                      struct mulle_rbnode *a_node)
{
   struct mulle_rbnode   *p;
   struct mulle_rbnode   *t;
   struct mulle_rbnode   *nil;

   nil = _mulle__rbtree_get_nil_node( a_tree);

   if( a_node->_left != nil)
      return( _mulle__rbtree_find_rightmost_node( a_tree, a_node->_left));
			
   t = a_node;
   p = a_node->_parent;
   
   while( p != nil && t == p->_left)
   {
      t = p;
      p = p->_parent;
   }
   return( p);
}



MULLE__RBTREE_GLOBAL
MULLE_C_NONNULL_FIRST_THIRD
struct mulle_rbnode    *
   _mulle__rbtree_find_node( struct mulle__rbtree *a_tree,
                             void *a_key,
                             int (*a_comp)( void *, void *))
{
   if( a_tree->_options & mulle_rbtree_option_use_extra)
      return( _mulle__rbtree_find_node_with_extra( a_tree, a_key, a_comp));
   return( _mulle__rbtree_find_node_with_payload( a_tree, a_key, a_comp));
}


MULLE__RBTREE_GLOBAL
MULLE_C_NONNULL_FIRST_THIRD
struct mulle_rbnode    *
   _mulle__rbtree_find_node_equal_or_greater( struct mulle__rbtree *a_tree,
                                              void *a_key,
                                              int (*a_comp)( void *, void *))
{
   if( a_tree->_options & mulle_rbtree_option_use_extra)
      return( _mulle__rbtree_find_node_with_equal_or_greater_extra( a_tree, a_key, a_comp));
   return( _mulle__rbtree_find_node_with_equal_or_greater_payload( a_tree, a_key, a_comp));
}



static inline void  mulle_rb_node_set_left( struct mulle_rbnode *p,
                                            struct mulle_rbnode *o,
                                            struct mulle__rbtree *a_tree)
{
   struct mulle_rbnode   *nil;

   nil = _mulle__rbtree_get_nil_node( a_tree);

   if( o != nil)
      o->_parent = p;
   p->_left = o;
}


static inline void  mulle_rb_node_set_right( struct mulle_rbnode *p,
                                             struct mulle_rbnode *o,
                                             struct mulle__rbtree *a_tree)
{
   struct mulle_rbnode   *nil;

   nil = _mulle__rbtree_get_nil_node( a_tree);

   if( o != nil)
      o->_parent = p;
   p->_right = o;
}


static inline void   mulle_rb_node_reparent( struct mulle_rbnode *p,
                                             struct mulle_rbnode *t,
                                             struct mulle__rbtree *a_tree)
{
   struct mulle_rbnode   *nil;

   nil = _mulle__rbtree_get_nil_node( a_tree);

   // move 't' above 'p'
   t->_parent = p->_parent;

   // If p was the root, then t becomes the new root of the tree.
   // Otherwise, reconnect t into p’s parent — replacing p in the tree.
   // Handles whether p was a left or right child.
   if( p->_parent == nil)
      a_tree->_root = t;
   else
      if( p == p->_parent->_right)
         p->_parent->_right = t;
      else
         p->_parent->_left = t;
}



//
// we propagate the dirty flag up
//
void   _mulle__rbtree_mark_node_as_dirty( struct mulle__rbtree *a_tree, void *node)
{
   struct mulle_rbnode   *nil;

   nil = _mulle__rbtree_get_nil_node( a_tree);

   while( node != nil)
   {
      // brute force to the top
      _mulle_rbnode_set_dirty( node);
      node = _mulle_rbnode_get_parent( node);
   }
}

// T as right child of P moves to the top. P becomes left child.
// Former left child of T becomes right child of P. The number of nodes on
// the left side is now
//
//          P                                 T
//         / \                               / \
//        A  [T]              ->           <P>  C
//           / \                           / \
//         <B>   C                        A  [B]
//
// P is the node being rotated.    T has taken P's place.
// T = P->_right                   P is now the left child of T.
// A is P->_left                   B, which was T->_left, is now P->_right.
// B = T->_left                    All parent pointers are updated accordingly.
// C = T->_right
//
MULLE_C_NONNULL_FIRST_SECOND
static void   _mulle__rbtree_left_rotate_node( struct mulle__rbtree *a_tree,
                                               struct mulle_rbnode *p)
{
   struct mulle_rbnode   *t;
   struct mulle_rbnode   *nil;

   nil = _mulle__rbtree_get_nil_node( a_tree);

   // t is the node to the right
   t   = p->_right;

   assert( p != nil);
   assert( t != nil);

   // Move t's left subtree into p's right subtree (we still have t..)
   mulle_rb_node_set_right( p, t->_left, a_tree);

   // move 't' above 'p'
   mulle_rb_node_reparent( p, t, a_tree);

   // p becomes the left child of t
   // t becomes the new parent of p
   t->_left   = p;
   p->_parent = t;

   // MEMO: need to redo 'p' and then 't' (and propagate up)
   if( a_tree->_options & mulle_rbtree_option_use_dirty)
      _mulle__rbtree_mark_node_as_dirty( a_tree, p);
}


//         P                                   T
//        / \                                 / \
//       T   C                 ->            A   P
//      / \                                     / \
//     A   B                                   B   C
//
// P is the node being rotated.     T has taken P's place.
// T = P->_left                     P is now the right child of T.
// C is P->_right                   B, which was T->_right, is now P->_left.
// A = T->_left                     All parent pointers are updated accordingly.
// B = T->_right
//
// just the inverse of left rotate
MULLE_C_NONNULL_FIRST_SECOND
static void   _mulle__rbtree_right_rotate_node( struct mulle__rbtree *a_tree,
                                                struct mulle_rbnode *p)
{
   struct mulle_rbnode    *t;
   struct mulle_rbnode   *nil;

   nil = _mulle__rbtree_get_nil_node( a_tree);
   
   t   = p->_left;

   assert( p != nil);
   assert( t != nil);

   mulle_rb_node_set_left( p, t->_right, a_tree);
   
   mulle_rb_node_reparent( p, t, a_tree);
   
   t->_right  = p;
   p->_parent = t;

   // MEMO: need to redo 'p' and then 't' (and propagate up)
   if( a_tree->_options & mulle_rbtree_option_use_dirty)
      _mulle__rbtree_mark_node_as_dirty( a_tree, p);
}


void   _mulle__rbtree_red_red_fixup( struct mulle__rbtree *a_tree,
                                     struct mulle_rbnode *x)
{
   struct mulle_rbnode   *y;

   // Fixup: while parent is red, we need to maintain red-black properties
   while( x != a_tree->_root && _mulle_rbnode_is_red( x->_parent))
   {
      // Case: parent is a left child
      if( x->_parent == x->_parent->_parent->_left)
      {
         y = x->_parent->_parent->_right;  // uncle

         // Case 1: Uncle is red – recolor
         //
         //        G                             G
         //       / \                           / \
         //      P   U     ->                  B   B
         //     /                             /
         //    x                             R
         //
         // G = grandparent, P = parent, U = uncle
         // Recolor P and U to black, G to red, and move up to G
         if( _mulle_rbnode_is_red( y))
         {
            _mulle__rbtree_set_node_black( a_tree, x->_parent);
            _mulle__rbtree_set_node_red( a_tree, x->_parent->_parent);
            _mulle__rbtree_set_node_black( a_tree, y);

            x = _mulle_rbnode_get_grandparent( x);
         }
         else
         {
            // Case 2: Uncle is black, node is right child – rotate left
            //
            //       G                           G
            //      / \                         / \
            //     P   U      -->              x   U
            //      \                         /
            //       x                       P
            //
            if( x == x->_parent->_right)
            {
               x = x->_parent;
               _mulle__rbtree_left_rotate_node( a_tree, x);
            }

            // Case 3: Uncle is black, node is left child – rotate right
            //
            //         G                          P
            //        / \       -->              / \
            //       P   U                      x   G
            //      /                                \
            //     x                                  U
            //
            _mulle__rbtree_set_node_black( a_tree, x->_parent);
            _mulle__rbtree_set_node_red( a_tree, x->_parent->_parent);
            x = _mulle_rbnode_get_grandparent( x);

            _mulle__rbtree_right_rotate_node( a_tree, x);
         }
      }
      else
      {
         // Mirror case: parent is right child of grandparent
         y = x->_parent->_parent->_left;  // uncle

         // Case 1: Uncle is red – recolor
         //
         //         G                           G
         //        / \                         / \
         //       U   P     ->                B   B
         //            \                           \
         //             x                           R
         if( _mulle_rbnode_is_red( y))
         {
            _mulle__rbtree_set_node_black( a_tree, x->_parent);
            _mulle__rbtree_set_node_red( a_tree, x->_parent->_parent);
            _mulle__rbtree_set_node_black( a_tree, y);

            x = _mulle_rbnode_get_grandparent( x);
         }
         else
         {
            // Case 2: Uncle is black, node is left child – rotate right
            //
            //         G                           G
            //        / \                         / \
            //       U   P        -->            U   x
            //          /                               \
            //         x                                 P
            if( x == x->_parent->_left)
            {
               x = x->_parent;
               _mulle__rbtree_right_rotate_node( a_tree, x);
            }

            // Case 3: Uncle is black, node is right child – rotate left
            //
            //         G                             P
            //        / \                           / \
            //       U   P         -->             G   x
            //            \                       /
            //             x                     U
            _mulle__rbtree_set_node_black( a_tree, x->_parent);
            _mulle__rbtree_set_node_red( a_tree, x->_parent->_parent);
            x = _mulle_rbnode_get_grandparent( x);

            _mulle__rbtree_left_rotate_node( a_tree, x);
         }
      }
   }

   // fix this up
   _mulle__rbtree_set_node_black( a_tree, a_tree->_root);
}



//
// Fixup differences:
//
// | Aspect              | Insert Fixup                   | Delete Fixup                               |
// | ------------------- | ------------------------------ | ------------------------------------------ |
// | Triggered by        | Red-red violation              | Black-height violation                     |
// | Node color involved | Always starts with red node    | Usually starts after removing black node   |
// | Key violation       | Parent and child both red      | Double-black / missing black height        |
// | Fixup method        | Recolor and/or 1–2 rotations   | Recolor, 0–3 rotations, upward propagation |
// | Complexity          | Simpler, localized             | More complex, can propagate up to root     |
// | Symmetry            | Symmetric for left/right cases | Also symmetric, but more intricate         |
//
void   _mulle__rbtree_black_black_fixup( struct mulle__rbtree *a_tree, struct mulle_rbnode *x)
{
   struct mulle_rbnode   *w, *v;

   // While x is not root and is black, fix double-black issue
   while( x != a_tree->_root && _mulle_rbnode_is_black( x))
   {
      if( x == x->_parent->_left)
      {
         w = x->_parent->_right; // sibling

         // Case 1: sibling is red
         if( _mulle_rbnode_is_red( w))
         {
            _mulle__rbtree_set_node_black( a_tree, w);
            v = x->_parent;
            _mulle__rbtree_set_node_red( a_tree, v);

            _mulle__rbtree_left_rotate_node( a_tree, v);
            w = x->_parent->_right;
         }

         // Case 2: sibling and its children are black
         if( _mulle_rbnode_is_black( w->_left) &&
             _mulle_rbnode_is_black( w->_right))
         {
            if( _mulle_rbnode_is_red( x->_parent) )
            {
               // Delete case 4: parent is red, sibling and its children are black
               // Swap colors of sibling and parent, then done
               _mulle__rbtree_set_node_black( a_tree, x->_parent );
               _mulle__rbtree_set_node_red( a_tree, w );
               break; // We're done
            }
            else
            {
               // Delete case 3: parent, sibling and its children are all black
               // Make sibling red and recurse on parent
               _mulle__rbtree_set_node_red( a_tree, w);
               x = x->_parent;
            }
         }
         else
         {
            // Case 3: sibling's right child is black and left is red
            if( _mulle_rbnode_is_black( w->_right))
            {
               _mulle__rbtree_set_node_black( a_tree, w->_left);
               _mulle__rbtree_set_node_red( a_tree, w);

               _mulle__rbtree_left_rotate_node( a_tree, w);
               w = x->_parent->_right;
            }

            // Case 4: sibling's right child is red
            _mulle__rbtree_set_node_color( a_tree, w, _mulle_rbnode_get_color( x->_parent));
            _mulle__rbtree_set_node_black( a_tree, x->_parent);
            _mulle__rbtree_set_node_black( a_tree, w->_right);

            v = x->_parent;
            _mulle__rbtree_right_rotate_node( a_tree, v);
            break;
         }
      }
      else
      {
         // Symmetric cases for when x is the right child
         w = x->_parent->_left;

         // Case 1: sibling is red
         if( _mulle_rbnode_is_red( w))
         {
            _mulle__rbtree_set_node_black( a_tree, w);
            v = x->_parent;
            _mulle__rbtree_set_node_red( a_tree, v);

            _mulle__rbtree_left_rotate_node( a_tree, v);
            w = x->_parent->_left;
         }

         // Case 2: sibling and both children are black
         if( _mulle_rbnode_is_black( w->_right) &&
             _mulle_rbnode_is_black( w->_left))
         {
            if( _mulle_rbnode_is_red( x->_parent) )
            {
               // Delete case 4: parent is red, sibling and its children are black
               // Swap colors of sibling and parent, then done
               _mulle__rbtree_set_node_black( a_tree, x->_parent );
               _mulle__rbtree_set_node_red( a_tree, w );
               break; // We're done
            }
            else
            {
               // Delete case 3: parent, sibling and its children are all black
               // Make sibling red and recurse on parent
               _mulle__rbtree_set_node_red( a_tree, w);
               x = x->_parent;
            }
         }
         else
         {
            // Case 3: sibling's left child is black
            if( _mulle_rbnode_is_black( w->_left))
            {
               _mulle__rbtree_set_node_black( a_tree, w->_right);
               _mulle__rbtree_set_node_red( a_tree, w);

               _mulle__rbtree_right_rotate_node( a_tree, w);
               w = x->_parent->_left;
            }

            // Case 4: sibling's left child is red
            _mulle__rbtree_set_node_color( a_tree, w, _mulle_rbnode_get_color( x->_parent));
            _mulle__rbtree_set_node_black( a_tree, x->_parent);
            _mulle__rbtree_set_node_black( a_tree, w->_left);

            v = x->_parent;
            _mulle__rbtree_left_rotate_node( a_tree, v);
            break;
         }
      }
   }

   _mulle__rbtree_set_node_black( a_tree, a_tree->_root);
}



static void   _mulle__rbtree_attach_node( struct mulle__rbtree *a_tree,
                                         struct mulle_rbnode *a_node,
                                         struct mulle_rbnode *parent,
                                         int direction)
{
   struct mulle_rbnode   *nil;

   nil = _mulle__rbtree_get_nil_node( a_tree);

   assert( a_node != nil);

   // must be singular unattached note
   assert( a_node->_parent == nil);
   assert( a_node->_left == nil);
   assert( a_node->_right == nil);
   assert( _mulle_rbnode_is_red( a_node));

   // Attach the new node as a child of parent
   a_node->_parent = parent;

   if( parent == nil)
   {
      a_tree->_root = a_node;
      _mulle__rbtree_set_node_black( a_tree, a_tree->_root);
   }
   else
   {
      assert( a_tree->_root != nil);
      if( direction < 0)
      {
         assert( parent->_left == nil);
         parent->_left = a_node;
      }
      else
      {
         assert( parent->_right == nil);
         parent->_right = a_node;
      }

      if( _mulle_rbnode_is_red( parent))
      {
         // Case A: parent is red, node is red -> red-red violation
         // black height not affected
         _mulle__rbtree_red_red_fixup( a_tree, a_node);
      }
   }

   assert( a_tree->_root != nil);

   // Mark node and ancestors as dirty after
   if( a_tree->_options & mulle_rbtree_option_use_dirty)
      _mulle__rbtree_mark_node_as_dirty( a_tree, a_node);
}



/* a_node is always the first argument to a_comp. */
int _mulle__rbtree_insert_node( struct mulle__rbtree *a_tree,
                                struct mulle_rbnode *a_node,
                                int (*a_comp)( void *, void *))
{
   struct mulle_rbnode  *x;
   struct mulle_rbnode  *y;
   int                   c;
   struct mulle_rbnode  *nil;
   void                 *a_value;
   void                 *y_value;

   nil = _mulle__rbtree_get_nil_node( a_tree);

   assert( a_node != nil);

   x = nil;
   y = a_tree->_root;
   c = 0;

   // Traverse the tree to find the appropriate insertion point.
   // 'y' is the current node being inspected.
   // 'x' trails behind 'y' and becomes the parent of the new node.
   if( y != nil)
   {
      a_value = _mulle__rbtree_get_node_value( a_tree, a_node);

      do
      {
         x       = y;
         y_value = _mulle__rbtree_get_node_value( a_tree, y);
         c       = (*a_comp)( a_value, y_value);
         if( ! c && ! (a_tree->_options & mulle_rbtree_option_allow_duplicates))
            return( EEXIST);  // Duplicate key not allowed

         if( c < 0)
            y = y->_left;
         else
            y = y->_right;
      }
      while( y != nil);
   }

   _mulle__rbtree_attach_node( a_tree, a_node, x, c);
   return( 0);
}



void   _mulle__rbtree_insert_node_before_node( struct mulle__rbtree *a_tree,
                                               struct mulle_rbnode *a_node,
                                               struct mulle_rbnode *successor)
{
   struct mulle_rbnode  *x;
   struct mulle_rbnode  *y;
   struct mulle_rbnode  *nil;


   nil = _mulle__rbtree_get_nil_node( a_tree);

   assert( a_node != nil);
   assert( a_node != successor);
   assert( successor != nil);

   // go successor left and then right until nil
   x = successor;
   y = x->_left;
   if( y == nil)
   {
      _mulle__rbtree_attach_node( a_tree, a_node, x, -1);
      return;
   }

   do
   {
      x = y;
      y = y->_right;
   }
   while( y != nil);

   _mulle__rbtree_attach_node( a_tree, a_node, x, +1);
}


void  _mulle__rbtree_insert_node_after_node( struct mulle__rbtree *a_tree,
                                             struct mulle_rbnode *a_node,
                                             struct mulle_rbnode *predecessor)
{
   struct mulle_rbnode  *x;
   struct mulle_rbnode  *y;
   struct mulle_rbnode  *nil;

   nil = _mulle__rbtree_get_nil_node( a_tree);

   assert( a_node != nil);
   assert( a_node != predecessor);
   assert( predecessor != nil);

   // go predecessor right and then left until nil
   x = predecessor;
   y = x->_right;
   if( y == nil)
   {
      _mulle__rbtree_attach_node( a_tree, a_node, x, +1);
      return;
   }

   do
   {
      x = y;
      y = y->_left;
   }
   while( y != nil);

   _mulle__rbtree_attach_node( a_tree, a_node, x, -1);
}



//
// Here is a tree where we want to delete 2
//
//           10(B)
//          /     \
//       5(R)     15(R)
//      /   \     /   \
//   2(B)  7(B) 12(B) 17(B)
//
// Root (10) is black (B).
// Its children (5 and 15) are red (R).
// Their children (2,7,12,17) are black (B).
//
// This satisfies:
//     No two reds in a row,
//     Same number of black nodes on all root-to-leaf paths (3 blacks),
//     Root is black.
//
// All paths:
//    10(B) -> 5(R)  -> 2(B)  -> NIL(B)
//    10(B) -> 5(R)  -> 7(B)  -> NIL(B)
//    10(B) -> 15(R) -> 12(B) -> NIL(B)
//    10(B) -> 15(R) -> 17(B) -> NIL(B)
//
// Step 2: Remove node 2(B) → Before fixup
//           10(B)
//          /     \
//       5(R)     15(R)
//         \      /   \
//         7(B)  12(B) 17(B)
//
// Node 2 (black) removed, replaced with nil (black leaf).
// Path through left side lost a black node → black-height imbalance.
//
// All paths:
//    10(B) -> 5(R) -> NIL(B)             → only 2 black nodes
//    10(B) -> 5(R) -> 7(B) -> NIL(B)
//    10(B) -> 15(R) -> 12(B) -> NIL(B)
//    10(B) -> 15(R) -> 17(B) -> NIL(B)
//
// We recolor:
//           10(B)
//          /     \
//       5(B)     15(R)
//         \      /   \
//        7(R)  12(B) 17(B)
//
// All paths:
//    10(B) → 5(B)
//    10(B) → 5(B) → 7(R)
//    10(B) → 15(R) → 12(B)
//    10(B) → 15(R) → 17(B)
//
// -------------
//
// Here is a tree where we want to delete node 5
//
//           10(B)
//          /     \
//       5(R)     15(R)
//      /   \     /   \
//   2(B)  7(B) 12(B) 17(B)
//
//
//           10(B)
//          /     \
//       7(R)     15(R)
//      /         /   \
//   2(B)      12(B) 17(B)
//
//
//           10(B)
//          /     \
//       7(B)     15(R)
//      /         /   \
//   2(R)      12(B) 17(B)
//
void _mulle__rbtree_remove_node( struct mulle__rbtree *a_tree,
                                 struct mulle_rbnode *a_node)
{
   int                   fixup;
   struct mulle_rbnode   *x, *y;
   struct mulle_rbnode   *nil;

   nil = _mulle__rbtree_get_nil_node( a_tree);

   assert( a_node != nil);

   // IMPORTANT: Save original children before ANY modifications
   struct mulle_rbnode *orig_left = a_node->_left;
   struct mulle_rbnode *orig_right = a_node->_right;

   // If a_node has at most one non-nil child, it's the node to be removed.
   // Otherwise, we find its in-order successor to replace it.
   //
   //        a_node                       y (successor)
   //        /    \                      /      \
   //      ...   ...         ->        ...      ...
   //
   // y is guaranteed to have at most one child (makes removal easier)
   if( a_node->_left == nil || a_node->_right == nil)
      y = a_node;
   else
      y = _mulle__rbtree_find_next_node( a_tree, a_node);

   // x is y's only child (or nil), and will be spliced into y's place.
   //
   //         y                        y
   //        / \                      / \
   //      x   nil        ->        x   nil
   //
   // x replaces y in the tree.

   if( y->_left != nil)
      x = y->_left;
   else
      x = y->_right;

   // Splice x into y's parent
   //
   //     y_parent                     y_parent
   //     /      \        ->           /      \
   //    y      ...                  x       ...
   //
   // y is removed and x takes its place in the parent's child pointer

   // fun fact: x can be the nil node, here clobbering its parent is
   // considered harmless
   x->_parent = y->_parent;

   if( y->_parent == nil)
      a_tree->_root = x;
   else
   {
      if( y == y->_parent->_left)
         y->_parent->_left = x;
      else
         y->_parent->_right = x;
   }

   // Determine whether we need a fixup based on y's color.
   // If y was black, removing it may have violated the red-black properties.
   fixup = _mulle_rbnode_is_black( y);

   // If y is not the same as the original node to remove,
   // transplant y into a_node's position.
   //
   //       a_node                        y
   //       /    \                       / \
   //     L       R         ->         L   R
   //                                  (from a_node)
   //
   // y takes a_node's position and adopts its children.
   
   if( y != a_node)
   {
      y->_parent = a_node->_parent;
      y->_left   = orig_left;
      
      // Set right child appropriately
      if( y != orig_right)
      {
         y->_right  = orig_right;
      }
      else
      {
         // y was the original right child, so it keeps its own right subtree
         // Don't change y->_right - it already has the correct subtree
      }

      _mulle__rbtree_set_node_color( a_tree, y, _mulle_rbnode_get_color( a_node));

      if( y->_parent != nil)
      {
         if (y->_parent->_left == a_node)
            y->_parent->_left = y;
         else
            y->_parent->_right = y;
      }
      else
         a_tree->_root = y;

      // MFING AI: Don't change this, doing this to nil is harmless
      y->_right->_parent = y;
      y->_left->_parent  = y;
   }

   // Mark affected ancestors as dirty for cumulative total recalculation
   // The parent of the removed node needs recalculation since it lost a child
   if( a_tree->_options & mulle_rbtree_option_use_dirty)
   {
      // Always mark from the parent of the original removed node upwards
      // since removing a_node changes the structure for its parent
      if( a_node->_parent != nil)
         _mulle__rbtree_mark_node_as_dirty( a_tree, a_node->_parent);
      
      // If we transplanted y into a_node's position, also mark y's ancestors
      if( y != a_node && y != nil)
         _mulle__rbtree_mark_node_as_dirty( a_tree, y);

      // Mark the replacement node x if it's not nil
      if( x != nil)
         _mulle__rbtree_mark_node_as_dirty( a_tree, x);
   }

   // Free memory of the removed node
   _mulle__rbtree_free_node( a_tree, a_node);

   // ### Delete Fixup: Fixing Black-Height Imbalance
   //
   // Deletion is more complex because of how Red-Black Trees handle the
   // removal of black nodes, which can affect the **black-height property**:
   // *every path from root to a leaf must contain the same number of black
   // nodes*.
   //
   // There are two main deletion cases:
   //
   // 1. A red node is removed → **no violation** (just remove it).
   // 2. A black node is removed → **potential black-height violation**.
   //
   // When a black node is removed, it may cause a black deficit in one of
   // the tree's paths — meaning that subtree now has one fewer black node
   // than required.
   // This imbalance is tracked by the node x (which replaces the deleted node),
   // and the deletion fixup algorithm works to eliminate the black deficit by
   // adjusting colors and performing rotations as necessary.
   // Unlike insertion, which only ever introduces a red-red violation,
   // deletion can require upward propagation of this black deficit,
   // making the fixup procedure more complex.
   if( fixup)
   {
      _mulle__rbtree_black_black_fixup( a_tree, x);
   }
}


void   _mulle__rbtree_walk_dirty( struct mulle__rbtree *a_tree,
                                  void (*callback)( void *node,
                                                    void *left,
                                                    void *right))
{
   struct mulle_rbnode   *root;

   root = _mulle__rbtree_get_root_node( a_tree);
   __mulle__rbtree_walk_dirty( a_tree, root, callback);
}


size_t   _mulle__rbtree_get_count( struct mulle__rbtree *a_tree)
{
   struct mulle_rbnode   *root;

   root = _mulle__rbtree_get_root_node( a_tree);
   __mulle__rbtree_walk_count( a_tree, root);
}


