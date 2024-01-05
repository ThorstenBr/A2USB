
function(a2vgaConfig BUILDTARGET)
    if(${CMAKE_CURRENT_BINARY_DIR} MATCHES "-4ns")
     if(${CMAKE_CURRENT_BINARY_DIR} MATCHES "-gs")
         pico_generate_pio_header(${BUILDTARGET}
            ${CMAKE_CURRENT_SOURCE_DIR}/lib/a2vga/common/abus-gs-4ns.pio)
     else()
        pico_generate_pio_header(${BUILDTARGET}
            ${CMAKE_CURRENT_SOURCE_DIR}/lib/a2vga/common/abus-4ns.pio)
     endif(${CMAKE_CURRENT_BINARY_DIR} MATCHES "-gs")
    elseif(${CMAKE_CURRENT_BINARY_DIR} MATCHES "-8ns")
     if(${CMAKE_CURRENT_BINARY_DIR} MATCHES "-gs")
        pico_generate_pio_header(${BUILDTARGET}
            ${CMAKE_CURRENT_SOURCE_DIR}/lib/a2vga/common/abus-gs-8ns.pio)
     else()
        pico_generate_pio_header(${BUILDTARGET}
            ${CMAKE_CURRENT_SOURCE_DIR}/lib/a2vga/common/abus-8ns.pio)
     endif(${CMAKE_CURRENT_BINARY_DIR} MATCHES "-gs")
    endif()

    message(STATUS "CMAKE_CURRENT_FUNCTION_LIST_DIR=${CMAKE_CURRENT_FUNCTION_LIST_DIR}")
    target_include_directories(${BUILDTARGET} PUBLIC ${CMAKE_CURRENT_FUNCTION_LIST_DIR})
    # source list
    target_sources(${BUILDTARGET} PUBLIC
        ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/common/abus.c
        ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/common/buffers.c
        #${CMAKE_CURRENT_FUNCTION_LIST_DIR}/common/config.c
        #${CMAKE_CURRENT_FUNCTION_LIST_DIR}/common/flash.c
        )

    # set libraries to be linked
    target_link_libraries(${BUILDTARGET} PUBLIC
            hardware_resets
            hardware_pio
            #hardware_flash
        )
endfunction()

