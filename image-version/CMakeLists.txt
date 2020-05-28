
### nanogui
include_directories(../external/nanogui/include)
include_directories(${NANOGUI_EXTRA_INCS})

### stb_image
include_directories(../external/stb)

### glm
include_directories(../external/glm)

include_directories(../voronoi)

include_directories(../renders)

aux_source_directory(. SOURCE_FILES)
add_executable(image-demo ${SOURCE_FILES})

target_link_libraries(image-demo nanogui ${NANOGUI_EXTRA_LIBS})
target_link_libraries(image-demo voronoi)
target_link_libraries(image-demo renders)