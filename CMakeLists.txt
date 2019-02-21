cmake_minimum_required( VERSION 3.2 )

project( bulk_server VERSION 0.0.$ENV{TRAVIS_BUILD_NUMBER} )

find_package(Boost COMPONENTS system filesystem REQUIRED)

add_executable( bulk_server bulk_server.cpp bulk.h )


set_target_properties( bulk_server PROPERTIES
	CXX_STANDARD 14
	CXX_STANDARD_REQUIRED ON
	COMPILE_OPTIONS "-Wpedantic;-Wall;-Wextra"
)

set_target_properties(bulk_server PROPERTIES
  INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIR}
)

target_link_libraries(bulk_server
  ${Boost_LIBRARIES}
)

install( TARGETS bulk_server RUNTIME DESTINATION bin)


set( CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}" )
set( CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}" )
set( CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}" )

set( CPACK_GENERATOR DEB )

set( CPACK_PACKAGE_CONTACT surdukov@gmail.com )

set( CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON )

include( CPack )

