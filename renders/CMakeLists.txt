# GLAD
include_directories(${NANOGUI_EXTRA_INCS})

# find source in this directory
aux_source_directory (. RENDERS_SOURCE)

# generate library
add_library (renders STATIC ${RENDERS_SOURCE})