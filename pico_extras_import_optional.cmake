# Optional import helper for pico_extras
# If the pico_extras import CMake file exists inside the SDK, include it.
# Otherwise do nothing so projects remain buildable without pico_extras installed.

if (DEFINED ENV{PICO_SDK_PATH} AND (NOT PICO_SDK_PATH))
    set(PICO_SDK_PATH $ENV{PICO_SDK_PATH})
endif ()

if (PICO_SDK_PATH)
    if (EXISTS "${PICO_SDK_PATH}/external/pico_extras/pico_extras_import.cmake")
        include("${PICO_SDK_PATH}/external/pico_extras/pico_extras_import.cmake")
    elseif (EXISTS "${PICO_SDK_PATH}/lib/pico_extras/pico_extras_import.cmake")
        include("${PICO_SDK_PATH}/lib/pico_extras/pico_extras_import.cmake")
    else ()
        # No pico_extras available; this is optional so we silently continue.
        message(STATUS "pico_extras not found; continuing without extras")
    endif ()
endif ()
