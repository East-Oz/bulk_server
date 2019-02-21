cmake_minimum_required( VERSION 3.2 )

project( bulk_server VERSION 0.0.$ENV{TRAVIS_BUILD_NUMBER} )

add_executable( bulk_server bulk_server.cpp bulk.h )


set_target_properties( bulk_server PROPERTIES
	CXX_STANDARD 14
	CXX_STANDARD_REQUIRED ON
	COMPILE_OPTIONS "-Wpedantic;-Wall;-Wextra"
)

install( TARGETS bulk_server RUNTIME DESTINATION bin)


set( CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}" )
set( CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}" )
set( CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}" )

set( CPACK_GENERATOR DEB )

set( CPACK_PACKAGE_CONTACT surdukov@gmail.com )

set( CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON )

include( CPack )
