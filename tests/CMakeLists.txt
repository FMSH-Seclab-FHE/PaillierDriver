
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

add_executable(FMSH_PAILLIER_TEST
    main.c
    test__fmsh_paillierDriver.c
    gen.c
)

target_include_directories(FMSH_PAILLIER_TEST PRIVATE /usr/local/include)
target_include_directories(FMSH_PAILLIER_TEST PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(FMSH_PAILLIER_TEST
    PRIVATE
        FMSH_PAILLIER_DRIVER 
        gmp                  
)
