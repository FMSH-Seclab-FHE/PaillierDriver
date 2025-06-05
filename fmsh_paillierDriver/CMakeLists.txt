
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

add_library(FMSH_PAILLIER_DRIVER STATIC
    conf.c
    data_process.c
    fpga_interface.c
    init.c
    paillier_intf.c
)

target_include_directories(FMSH_PAILLIER_DRIVER PUBLIC /usr/local/include)
target_link_libraries(FMSH_PAILLIER_DRIVER PUBLIC /usr/local/lib/libgmp.a)
