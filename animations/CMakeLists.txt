CMAKE_MINIMUM_REQUIRED (VERSION 2.6)
SET (TARGET animation-video)
PROJECT (${TARGET})

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")
elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -Wall")
else()
        message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()


SET( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/../CMakeModules
    ~/marble/sources/
    ~/marble/sources/src/lib/geodata/
    ~/marble/sources/src/lib/marble/
    ~/marble/sources/src/lib/geodata/data/
    )


FIND_PACKAGE (Qt4 4.6.0 REQUIRED QtCore QtGui)
FIND_PACKAGE (Marble REQUIRED)
FIND_PACKAGE (OpenCV REQUIRED)


include_directories(
 ${CMAKE_CURRENT_SOURCE_DIR}
 ${CMAKE_CURRENT_BINARY_DIR}
 ${QT_INCLUDE_DIR}
)

set( ${TARGET}_SRC main.cpp)
add_executable( ${TARGET} ${${TARGET}_SRC} )

target_link_libraries( ${TARGET} ${OpenCV_LIBS} ) # marblewidget )
if (QT4_FOUND)
  target_link_libraries( ${TARGET} ${QT_QTCORE_LIBRARY} ${QT_QTMAIN_LIBRARY} )
else()
  target_link_libraries( ${TARGET} ${Qt5Core_LIBRARIES} )
endif()

INCLUDE (${QT_USE_FILE})
INCLUDE_DIRECTORIES (${MARBLE_INCLUDE_DIR})
SET (LIBS ${LIBS} ${MARBLE_LIBRARIES} ${QT_LIBRARIES})

TARGET_LINK_LIBRARIES (${TARGET} ${LIBS})
TARGET_LINK_LIBRARIES (${TARGET} marblewidget)
include ( ${QT_USE_FILE} )
set( LIBS ${LIBS} ${QT_LIBRARIES})
install( TARGETS ${TARGET} RUNTIME DESTINATION bin )


