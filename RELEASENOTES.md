### 0.2.1


feature: add configurable node pool capacity and embed tree base as real member

* new ``mulle_rbtree_config.capacity`` chooses the node pool bucket size (0 = default 32, clamped to `USHRT_MAX)` so callers can match allocation to expected live nodes
* new ``_mulle__rbtree_init_with_config()`` and `struct `mulle__rbtree_config`` for low-level initialization
* **BREAKING** `struct `mulle_rbtree`` now holds the internal tree as member `base` instead of repeating its fields; reach internal fields via `tree.base`
* **BREAKING** ``mulle_rbtree_config`` gained a leading `capacity` field; zero or fully initialize the config before use, otherwise capacity is garbage
* fix a strict-aliasing violation (C11 6.5p7) that GCC exploited at -O2/-O3 by handing inner functions `&tree->base` rather than casting between the two struct types
* ``_mulle__rbtree_init_with_options()`` is kept as an unchanged-signature convenience wrapper

## 0.2.0





* edge-case test callbacks now type as `const` and drop explicit casts




* fix over-allocation of node storage by computing the payload area from `offsetof(payload)` instead of `sizeof(node)` in ``_mulle__rbtree_init_with_options`` and ``_mulle__rbtree_get_extra_size``
* ``_mulle_rbtree_remove`` now correctly returns `ENOENT` for absent keys by checking for the nil node instead of a NULL pointer
* new ``mulle_rbtree_is_empty`` / ``_mulle_rbtree_is_empty`` helpers to test for an empty tree
* ``_mulle_rbtree_remove_node`` now asserts the node is non-NULL and not the nil node



* new comprehensive API reference documentation for AI consumption
* proper BSD license headers added to all source files
* API documentation link added to README
