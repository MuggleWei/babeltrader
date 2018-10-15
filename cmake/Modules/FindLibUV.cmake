set(LIBUV_ROOT_DIR CACHE PATH "The root directory of libuv")
if (WIN32)
    set(_LIBUV_ROOT_HINTS 
        ${LIBUV_ROOT_DIR}
        ENV LIBUV_ROOT_DIR
    )
else()
    set(_LIBUV_ROOT_HINTS 
        ${LIBUV_ROOT_DIR}
        /usr/local
        ENV LIBUV_ROOT_DIR
    )
endif()

find_path(LIBUV_INCLUDE_DIR
    NAMES uv.h ${_LIBUV_ROOT_HINTS}
    PATH_SUFFIXES include
)
find_library(LIBUV_LIBRARIES
    NAMES libuv ${_LIBUV_ROOT_HINTS}
    PATH_SUFFIXES lib
)

find_package_handle_standard_args(LibUV
    REQUIRED_VARS LIBUV_INCLUDE_DIR LIBUV_LIBRARIES
)
