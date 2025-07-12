find_package(
    Python
    COMPONENTS Interpreter Development.Module
    REQUIRED)

include(UseCython)

cython_transpile(
    ${CMAKE_SOURCE_DIR}/bindings/python/monome.pyx 
    LANGUAGE C
    OUTPUT_VARIABLE monome_c
)

python_add_library(pymonome MODULE "${monome_c}" WITH_SOABI)

# target_include_directories(
#     pymonome
#     PRIVATE
#     src/private
# )

target_include_directories(
    pymonome
    PUBLIC
    public
)

target_compile_options(
    pymonome
    PUBLIC
    -fPIC
)

target_link_libraries(
    pymonome
    PUBLIC
    monome_static
    # $<$<PLATFORM_ID:Linux>:-ludev>
)

set_target_properties(pymonome PROPERTIES OUTPUT_NAME "monome")

install(TARGETS pymonome DESTINATION .)
