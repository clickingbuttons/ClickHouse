option (USE_INTERNAL_ZSTD_LIBRARY "Set to FALSE to use system zstd library instead of bundled" ON)

if(NOT EXISTS "${ClickHouse_SOURCE_DIR}/contrib/zstd/lib/zstd.h")
    if(USE_INTERNAL_ZSTD_LIBRARY)
        message(WARNING "submodule contrib/zstd is missing. to fix try run: \n git submodule update --init")
        message (${RECONFIGURE_MESSAGE_LEVEL} "Can't find internal zstd library")
        set(USE_INTERNAL_ZSTD_LIBRARY 0)
    endif()
    set(MISSING_INTERNAL_ZSTD_LIBRARY 1)
endif()

if (NOT USE_INTERNAL_ZSTD_LIBRARY)
    find_library (ZSTD_LIBRARY zstd)
    find_path (ZSTD_INCLUDE_DIR NAMES zstd.h PATHS ${ZSTD_INCLUDE_PATHS})
    if (NOT ZSTD_LIBRARY OR NOT ZSTD_INCLUDE_DIR)
        message (${RECONFIGURE_MESSAGE_LEVEL} "Can't find system zstd library")
    endif ()
endif ()

if (ZSTD_LIBRARY AND ZSTD_INCLUDE_DIR)
elseif (NOT MISSING_INTERNAL_ZSTD_LIBRARY)
    set (USE_INTERNAL_ZSTD_LIBRARY 1)
    set (ZSTD_LIBRARY zstd)
    set (ZSTD_INCLUDE_DIR ${ClickHouse_SOURCE_DIR}/contrib/zstd/lib)
endif ()

message (STATUS "Using zstd: ${ZSTD_INCLUDE_DIR} : ${ZSTD_LIBRARY}")