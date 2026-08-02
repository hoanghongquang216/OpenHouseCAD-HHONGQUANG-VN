# Centralized OpenHouseCAD compiler warning policy.
# Applied only to project-owned targets, not exported dependencies.

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
        )
        if(OPENHOUSE_WARNINGS_AS_ERRORS)
            target_compile_options(${target} PRIVATE -Werror)
        endif()
    endif()
endfunction()
