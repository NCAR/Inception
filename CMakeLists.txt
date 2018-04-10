project(inception)
cmake_minimum_required(VERSION 2.8)

find_package(PkgConfig)

set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")

if(PKG_CONFIG_FOUND)
	pkg_check_modules(JANSSONPKG "jansson")
endif(PKG_CONFIG_FOUND)
set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
#SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

option(NCAR_UNSAFE "Enable NCAR Specific features" OFF)
if(NCAR_UNSAFE)
	add_definitions(-DNCAR_UNSAFE)
endif()

if(JANSSON_STATIC_LIBRARY_DIRS)
	link_directories(${JANSSONPKG_STATIC_LIBRARY_DIRS})
	set(JANSSON_LIBS ${JANSSONPKG_STATIC_LIBRARIES})
	set(JANSSON_INCLUDE_DIRS ${JANSSONPKG_STATIC_INCLUDE_DIRS})
else()
	message(WARNING "pkg-config --static didn't work")
	link_directories(${JANSSONPKG_LIBRARY_DIRS})
	set(JANSSON_LIBS ${JANSSONPKG_LIBRARIES})
	set(JANSSON_INCLUDE_DIRS ${JANSSONPKG_INCLUDE_DIRS})
endif()

if(CMAKE_C_COMPILER_ID MATCHES GNU)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 ")
endif()

set(INCEPTION_CONFIG_PATH "./inception.json" CACHE STRING "location of inception config file")
add_definitions(-DINCEPTION_CONFIG_PATH="${INCEPTION_CONFIG_PATH}")

include_directories(${JANSSON_INCLUDE_DIRS})
add_executable(inceptioncli inception.c cli.c)
set_target_properties(inceptioncli PROPERTIES LINK_SEARCH_START_STATIC 1)
set_target_properties(inceptioncli PROPERTIES LINK_SEARCH_END_STATIC 1)
set_target_properties(inceptioncli PROPERTIES OUTPUT_NAME inception)
target_link_libraries(inceptioncli ${JANSSON_LIBS})

add_library(inception STATIC inception.c)
set_target_properties(inception PROPERTIES POSITION_INDEPENDENT_CODE 1)
target_link_libraries(inception ${JANSSON_LIBS})

set(INCEPTION_LIB_INSTALL_TARGETS inception)

option(BUILD_SHARED_LIBS "Build a shared library" ON)
if(BUILD_SHARED_LIBS)
	add_library(inceptionshared SHARED inception.c)
	set_target_properties(inceptionshared PROPERTIES OUTPUT_NAME inception)
	target_link_libraries(inceptionshared ${JANSSON_LIBS})
	set(INCEPTION_LIB_INSTALL_TARGETS ${INCEPTION_LIB_INSTALL_TARGETS} inceptionshared)
endif(BUILD_SHARED_LIBS)

install(TARGETS inceptioncli 
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
	PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE
	WORLD_READ WORLD_EXECUTE SETUID)

install(TARGETS ${INCEPTION_LIB_INSTALL_TARGETS}
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
	PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE
	WORLD_READ WORLD_EXECUTE)
