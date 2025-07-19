
function(add_example name)
    add_executable(${name}
        ${monome_python_sources}
        ${CMAKE_CURRENT_SOURCE_DIR}/${name}.c
    )
    target_compile_definitions(${name} PRIVATE EMBED_PROTOS)

    target_include_directories(${name} PRIVATE ${CMAKE_SOURCE_DIR}/src/private)

    target_include_directories(
        ${name}
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/public>
    )
    target_link_libraries(
        ${name}
        PRIVATE 
        ${libmonome_libs}
        monome_static
    )
endfunction()


add_example(life)
add_example(simple)
add_example(test)
add_example(test_arc)
add_example(torture)
