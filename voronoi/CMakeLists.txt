#BOOST
find_package(Boost REQUIRED)
include_directories(${BOOST_INCLUDES})

#CGAL
find_package(CGAL REQUIRED)
include_directories(${CGAL_INCLUDES})
link_directories(${CGAL_LIBRARIES})

link_libraries(${CGAL_LIBRARIES})

# find source in this directory
aux_source_directory (. VORONOI_SOURCE)

# generate library
add_library (voronoi STATIC ${VORONOI_SOURCE})

# link CGAL
target_link_libraries(voronoi  ${CGAL_LIBRARIES})