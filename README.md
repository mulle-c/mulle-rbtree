# mulle-rbtree

#### 🍫 mulle-rbtree organizes data in a red/black tree

This is an implementation of a [red-black tree](https://en.wikipedia.org/wiki/Red%E2%80%93black_tree).
[mulle-allocator](//github.com/mulle-c/mulle-allocator) is used to simplify
memory management. It isn't thread-safe.



| Release Version                                       | Release Notes  | AI Documentation
|-------------------------------------------------------|----------------|---------------
| ![Mulle kybernetiK tag](https://img.shields.io/github/tag/mulle-c/mulle-rbtree.svg) [![Build Status](https://github.com/mulle-c/mulle-rbtree/workflows/CI/badge.svg)](//github.com/mulle-c/mulle-rbtree/actions) | [RELEASENOTES](RELEASENOTES.md) | [DeepWiki for mulle-rbtree](https://deepwiki.com/mulle-c/mulle-rbtree)




## Documentation & Guides

* [API Summary](asset/dox/api/toc)



## Info

Red-black trees are difficult to explain without lots of diagrams, so little
attempt is made to document this code.  However, an excellent discussion can
be found in the following book, which was used as the reference for writing
this implementation:

```
   Introduction to Algorithms
   Thomas H. Cormen, Charles E. Leiserson, and Ronald L. Rivest
   MIT Press (1990)
   ISBN 0-07-013143-0
```

Some functions use a comparison function pointer, which is expected to have the
following prototype:

``` c
   int (*compare)( void *payload_a, void *payload_b);
```

Interpretation of comparison function return values:

| Return value | Comparison
|--------------|-------------------
|  `< 0`       | `a_a < a_b`
|    `0`       | `a_a == a_b`
|  `> 0`       | `a_a > a_b`


### You are here

![Overview](overview.dot.svg)





## Add

### Add mulle-rbtree via mulle-core

mulle-rbtree is a component of the
[mulle-core](//github.com/mulle-core/mulle-core) library.
In your code include its umbrella header:

``` c
#include <mulle-core/mulle-core.h>
```

In your project add mulle-core as a git submodule and link against it:

``` bash
git submodule add -f "https://github.com/mulle-core/mulle-core.git" \
                     "stash/mulle-core"
git submodule update --init --recursive
```

then add these comands to your `CMakeLists.txt`:

``` cmake
include_directories( SYSTEM "stash/mulle-core/src")
add_subdirectory( stash/mulle-core)
target_link_libraries( ${PROJECT_NAME} PUBLIC mulle-core)
```

Or let [mulle-sde](//github.com/mulle-sde) do it:

``` sh
mulle-sde add mulle-core/mulle-core
```




### Add mulle-rbtree as a cmake subproject

In your code include its header:

``` c
#include <mulle-rbtree/mulle-rbtree.h>
```

Add mulle-rbtree and dependencies as git submodules and link against
them:

``` bash
git submodule add -f --name "mulle-storage" \
                            "https://github.com/mulle-c/mulle-storage.git" \
                            "stash/mulle-storage"
git submodule add -f --name "mulle-rbtree" \
                        "https://github.com/mulle-c/mulle-rbtree" \
                        "stash/mulle-rbtree"
git submodule update --init --recursive
```

``` cmake
include_directories( SYSTEM "${CMAKE_BINARY_DIR}/include")

add_subdirectory( stash/mulle-storage)
add_subdirectory( stash/mulle-rbtree)

target_link_libraries( ${PROJECT_NAME} PUBLIC mulle-storage)
target_link_libraries( ${PROJECT_NAME} PUBLIC mulle-rbtree)
```

Or let [mulle-sde](//github.com/mulle-sde) do it:

``` sh
mulle-sde add github:mulle-c/mulle-rbtree
```

## Individual Install

Install all the prerequisites.

``` bash
PREFIX=/tmp/usr/local
BUILD_DIR="mulle-rbtree/build"

cmake -S "mulle-rbtree" -B "${BUILD_DIR}"    \
  -DCMAKE_BUILD_TYPE="Release"                  \
  -DCMAKE_INSTALL_PREFIX="${PREFIX}"            \
  -DCMAKE_PREFIX_PATH="${PREFIX}"               \
  -DCMAKE_C_FLAGS="-I"${PREFIX}"/include"       \
&& cmake --build "${BUILD_DIR}" &&              \
&& cmake --install "${BUILD_DIR}"
```


## Author

[Nat!](https://mulle-kybernetik.com/weblog) for Mulle kybernetiK  



