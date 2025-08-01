pico_sdk_init()

include_directories(${CMAKE_CURRENT_LIST_DIR})
add_compile_options(-Wall -Werror -Wfatal-errors -O3)

function(make_firmware board board_def)
    add_executable(${board}
        main.c button.c hub75.c matrix.c save.c config.c ubthax.c
        font.c marker.c grid.c score.c resource.c
        cli.c commands.c usb_descriptors.c)
    target_compile_definitions(${board} PUBLIC ${board_def})
    pico_enable_stdio_usb(${board} 1)
    pico_enable_stdio_uart(${board} 0)

    pico_generate_pio_header(${board} ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio)
    pico_generate_pio_header(${board} ${CMAKE_CURRENT_LIST_DIR}/hub75.pio)
    
    target_link_libraries(${board} PRIVATE
        pico_multicore pico_stdlib hardware_pio hardware_pwm hardware_flash
        hardware_adc hardware_i2c hardware_dma hardware_watchdog pico_unique_id
        tinyusb_device tinyusb_board)

    pico_add_extra_outputs(${board})

    add_custom_command(TARGET ${board} PRE_BUILD
    COMMAND touch ${CMAKE_CURRENT_SOURCE_DIR}/cli.c)

    add_custom_command(TARGET ${board} POST_BUILD
                       COMMAND ${CMAKE_COMMAND}
                       ARGS -E copy ${board}.uf2 ${CMAKE_CURRENT_LIST_DIR}/..)
endfunction()

make_firmware(matrix_pico BOARD_MATRIX_PICO)

