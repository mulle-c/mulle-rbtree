#
# Reset to empty
#
export MULLE_SDE_REFLECT_CALLBACKS="sourcetree:filesystem"


#
# Used by `mulle-match find` to speed up the search.
#
export MULLE_MATCH_FILENAMES="config*:*.h:*.inc:*.c:CMakeLists.txt:*.cmake"


#
# Used by `mulle-match find` to locate files
#
export MULLE_MATCH_PATH=".mulle/etc/sourcetree:${PROJECT_SOURCE_DIR}:CMakeLists.txt:cmake"


#
# Used by `mulle-match find` to ignore boring subdirectories like .git
#
export MULLE_MATCH_IGNORE_PATH=""


#
# mulle-c and mulle-objc projects have an actual latest tag, so don't resolve
#
export MULLE_SOURCETREE_RESOLVE_TAG="NO"


#
# tell mulle-sde to keep files protected from read/write changes
#
export MULLE_SDE_PROTECT_PATH="cmake/share"


