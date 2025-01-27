# - Find a osgAudio installation or build tree.
# The following variables are set if osgWorks is found.  If osgAudio is not
# found, osgAudio_FOUND is set to false.
#  osgAudio_FOUND         - Set to true when osgAudio is found.
#  osgAudio_USE_FILE      - CMake file to use osgAudio.
#  osgAudio_MAJOR_VERSION - The osgAudio major version number.
#  osgAudio_MINOR_VERSION - The osgAudio minor version number 
#                       (odd non-release).
#  osgAudio_BUILD_VERSION - The osgAudio patch level 
#                       (meaningless for odd minor).
#  osgAudio_INCLUDE_DIRS  - Include directories for osgAudio
#  osgAudio_LIBRARY_DIRS  - Link directories for osgAudio libraries

# The following cache entries must be set by the user to locate osgWorks:
#  osgAudio_DIR  - The directory containing osgAudioConfig.cmake.  
#             This is either the root of the build tree,
#             or the lib directory.  This is the 
#             only cache entry.


# Assume not found.
SET(osgAudio_FOUND 0)

# Construct consitent error messages for use below.
SET(osgAudio_DIR_DESCRIPTION "directory containing osgAudioConfig.cmake.  This is either the root of the build tree, or PREFIX/lib for an installation.")
SET(osgAudio_DIR_MESSAGE "osgAudio not found.  Set the osgAudio_DIR cmake cache entry to the ${osgAudio_DIR_DESCRIPTION}")

# Use the Config mode of the find_package() command to find osgAudioConfig.
# If this succeeds (possibly because osgAudio_DIR is already set), the
# command will have already loaded osgAudioConfig.cmake and set osgWorks_FOUND.
IF(NOT osgAudio_FOUND)
  FIND_PACKAGE(osgAudio QUIET NO_MODULE)
ENDIF(NOT osgAudio_FOUND)

#-----------------------------------------------------------------------------
IF(NOT osgAudio_FOUND)
  # osgAudio not found, explain to the user how to specify its location.
  IF(osgAudio_FIND_REQUIRED)
    MESSAGE(FATAL_ERROR ${osgAudio_DIR_MESSAGE})
  ELSE(osgAudio_FIND_REQUIRED)
    IF(NOT osgAudio_FIND_QUIETLY)
      MESSAGE(STATUS ${osgAudio_DIR_MESSAGE})
    ENDIF(NOT osgAudio_FIND_QUIETLY)
  ENDIF(osgAudio_FIND_REQUIRED)
ENDIF(NOT osgAudio_FOUND)
