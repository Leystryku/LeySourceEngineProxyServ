# GeoLite2++ (C) 2016 Stephane Charette <stephanecharette@gmail.com>
# $Id: CMakeLists.txt 1990 2016-10-16 10:03:16Z stephane $

CONFIGURE_FILE ( ${CMAKE_CURRENT_SOURCE_DIR}/GeoLite2PP_version.hpp.in ${CMAKE_CURRENT_SOURCE_DIR}/GeoLite2PP_version.hpp )

FILE ( GLOB LIB_SOURCE	*.cpp	)
FILE ( GLOB LIB_HEADERS	*.hpp	)
LIST ( SORT LIB_SOURCE			)
LIST ( SORT LIB_HEADERS			)

# this creates the static link library (.a)
ADD_LIBRARY				( geolite2++ STATIC ${LIB_SOURCE} )
TARGET_LINK_LIBRARIES	( geolite2++ ${CMAKE_THREAD_LIBS_INIT} ${GeoLite2PP_EXTERNAL_LIBS} )

# this creates the shared library (.so)
ADD_LIBRARY				( geolite2++s SHARED ${LIB_SOURCE} )
SET_TARGET_PROPERTIES	( geolite2++s PROPERTIES OUTPUT_NAME "geolite2++" )
SET_TARGET_PROPERTIES	( geolite2++s PROPERTIES PREFIX "lib" )

INSTALL ( TARGETS geolite2++ geolite2++s ARCHIVE DESTINATION lib LIBRARY DESTINATION lib )
INSTALL ( FILES ${LIB_HEADERS} DESTINATION include/ )
