# OpenHouseWarnings.cmake
#
# Provides openhouse_enable_warnings(<target>) to apply a strict, consistent
# warning set to a target, matching docs/CODING_STANDARD.md:
#   "Use clang-format." / "Treat warnings as errors in CI."
#
# Deliberately a PRIVATE compile option on the target it's applied to
# (never PUBLIC/INTERFACE): a consumer of OpenHouse::Foundation or
# OpenHouse::Geometry should not be forced to build their own code
# warnings-as-errors just because they depend on us. Only OpenHouseCAD's
# own executables/tests opt in.

function(openhouse_enable_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4)
        if(OPENHOUSE_WARNINGS_AS_ERRORS)
            target_compile_options(${target} PRIVATE /WX)
        endif()
    else()
        target_compile_options(${target} PRIVATE
            -Wall
            -Wextra
            -Wpedantic
            -Wshadow
            -Wconversion
            -Wsign-conversion
        )
        if(OPENHOUSE_WARNINGS_AS_ERRORS)
            target_compile_options(${target} PRIVATE -Werror)
        endif()
    endif()
endfunction()
