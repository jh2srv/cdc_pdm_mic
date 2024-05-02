cmake_minimum_required(VERSION 3.5)

# set the path of the tinyusb add the family_support.cmake
set(TINYUSB_DIR $ENV{PICO_SDK_PATH}/lib/tinyusb)
include(${TINYUSB_DIR}/hw/bsp/family_support.cmake)

# gets PROJECT name for the example (e.g. <BOARD>-<DIR_NAME>)
family_get_project_name(PROJECT ${CMAKE_CURRENT_LIST_DIR})
include(pico_sdk_import.cmake)

project(${PROJECT})

# Checks this example is valid for the family and initializes the project
family_initialize_project(${PROJECT} ${CMAKE_CURRENT_LIST_DIR})

add_executable(${PROJECT})

# source
target_sources(${PROJECT} PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src/main.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/usb_descriptors.c
        )

# include
target_include_directories(${PROJECT} PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        )

pico_generate_pio_header(${PROJECT} ${CMAKE_CURRENT_LIST_DIR}/src/pdm_microphone.pio)
target_link_libraries(${PROJECT} tinyusb_device tinyusb_board pico_stdlib hardware_dma hardware_pio)
pico_add_extra_outputs(${PROJECT})
