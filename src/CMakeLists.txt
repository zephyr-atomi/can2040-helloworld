add_library(can2040 can2040.c)

target_link_libraries(can2040 PUBLIC
        cmsis_core
        hardware_pio
        pico_stdlib
)

#target_include_directories(can2040 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(can2040-helloworld main.c)

# Link with necessary libraries, e.g., hardware libs from Pico SDK.
target_link_libraries(can2040-helloworld PUBLIC
        can2040
)

# Generate hex / bin / uf2 files
pico_add_extra_outputs(can2040-helloworld)
