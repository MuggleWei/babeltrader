set(UWS_ROOT_DIR CACHE PATH "The root directory of uWebsockets")
if (WIN32)
    set(_UWS_ROOT_HINTS 
        ${UWS_ROOT_DIR}
        ENV UWS_ROOT_DIR
    )
else()
    set(_UWS_ROOT_HINTS 
        ${UWS_ROOT_DIR}
        /usr/local
        ENV UWS_ROOT_DIR
    )
endif()

find_path(UWS_INCLUDE_DIR
    NAMES uWS.h ${_UWS_ROOT_HINTS}
    PATH_SUFFIXES include/uWS
)
find_library(UWS_LIBRARIES
    NAMES uWS ${_UWS_ROOT_HINTS}
    PATH_SUFFIXES lib
)

find_package_handle_standard_args(uWS
    REQUIRED_VARS UWS_INCLUDE_DIR UWS_LIBRARIES
)
