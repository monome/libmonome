find_package(Python COMPONENTS Interpreter Development.Module REQUIRED)

set(monome_python_c ${CMAKE_CURRENT_BINARY_DIR}/monome_python.c)

add_custom_command(OUTPUT ${monome_python_c}
    COMMAND ${Python_EXECUTABLE} -m cython -o ${monome_python_c} ${CMAKE_CURRENT_SOURCE_DIR}/monome.pyx 
    DEPENDS monome.pyx
        ${CMAKE_SOURCE_DIR}/public/monome.h)

set(monome_python_sources)
list(APPEND monome_python_sources ${monome_python_c})
list(APPEND monome_python_sources $<PATH:ABSOLUTE_PATH,NORMALIZE,${libmonome_sources},${CMAKE_SOURCE_DIR}>)

python_add_library(monome_python MODULE ${monome_python_sources})

target_compile_definitions(monome_python PRIVATE EMBED_PROTOS)
target_include_directories(monome_python PRIVATE ${CMAKE_SOURCE_DIR}/src/private)
target_include_directories(monome_python PUBLIC $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/public>)
target_link_libraries(monome_python PRIVATE ${libmonome_libs})

set_target_properties(
    monome_python
    PROPERTIES
    OUTPUT_NAME "monome"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)
