function(LIBRARY TARGET)
    if(NOT DEFINED PLATFORM_PATH)
        message(FATAL_ERROR "PLATFORM_PATH is not defined. Set it before calling LIBRARY.")
    endif()

    foreach(LIB_NAME IN LISTS ARGN)
        # esp8266_library(${TARGET} ${lib})
        set(LIB_PATH "${PLATFORM_PATH}/libraries/${LIB_NAME}")
        if(NOT EXISTS "${LIB_PATH}")
            set(LIB_PATH "${CMAKE_SOURCE_DIR}/libraries/${LIB_NAME}")
            if(NOT EXISTS "${LIB_PATH}")
                message(FATAL_ERROR "Library ${LIB_NAME} not found in ${LIB_PATH}")
            else()
                message("User library ${LIB_NAME} found in ${LIB_PATH}")
            endif()
        else()
            message("System library ${LIB_NAME} found in ${LIB_PATH}")
        endif()

        file(GLOB_RECURSE LIB_SOURCES CONFIGURE_DEPENDS
            "${LIB_PATH}/*.c"
            "${LIB_PATH}/*.cpp"
            "${LIB_PATH}/*.S"
            "${LIB_PATH}/*.s"

        )

        # Получить все уникальные поддиректории, содержащие .h/.hpp
        file(GLOB_RECURSE LIB_INCLUDE_FILES CONFIGURE_DEPENDS
            "${LIB_PATH}/*.h"
            "${LIB_PATH}/*.hpp"
        )

        set(LIB_INCLUDE_DIRS "")
        foreach(_file IN LISTS LIB_INCLUDE_FILES)
        get_filename_component(_dir "${_file}" DIRECTORY)
        list(APPEND LIB_INCLUDE_DIRS "${_dir}")
        endforeach()
        list(REMOVE_DUPLICATES LIB_INCLUDE_DIRS)

        target_sources(${TARGET} PRIVATE ${LIB_SOURCES})
        target_include_directories(${TARGET} PRIVATE "${LIB_INCLUDE_DIRS}")
    endforeach()

    
endfunction()