add_executable(main
        hc06.c
        main.c
)

target_link_libraries(main pico_stdlib hardware_i2c freertos hardware_adc Fusion)
pico_add_extra_outputs(main)
