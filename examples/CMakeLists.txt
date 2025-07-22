set(examples
    life
    simple
    test
    test_arc
    torture
)

foreach(example ${examples})
    add_executable(${example} ${CMAKE_CURRENT_SOURCE_DIR}/${example}.c)
    target_link_libraries(${example} PRIVATE monome)
endforeach()
