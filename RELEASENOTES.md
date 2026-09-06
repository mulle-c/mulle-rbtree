## 0.2.0





* edge-case test callbacks now type as `const` and drop explicit casts




* fix over-allocation of node storage by computing the payload area from `offsetof(payload)` instead of `sizeof(node)` in ``_mulle__rbtree_init_with_options`` and ``_mulle__rbtree_get_extra_size``
* ``_mulle_rbtree_remove`` now correctly returns `ENOENT` for absent keys by checking for the nil node instead of a NULL pointer
* new ``mulle_rbtree_is_empty`` / ``_mulle_rbtree_is_empty`` helpers to test for an empty tree
* ``_mulle_rbtree_remove_node`` now asserts the node is non-NULL and not the nil node



* new comprehensive API reference documentation for AI consumption
* proper BSD license headers added to all source files
* API documentation link added to README
