cmake_minimum_required(VERSION 3.20)

project(
    AvaLang
    VERSION 0.0.0
    LANGUAGES C
)

function(ava_configure_c_target target_name)
    target_compile_features(
        ${target_name}
        PRIVATE
            c_std_17
    )

    if(CMAKE_C_COMPILER_ID MATCHES "Clang|GNU")
        target_compile_options(
            ${target_name}
            PRIVATE
                -Wall
                -Wextra
                -Wpedantic
                -Wconversion
                -Wshadow
        )
    endif()
endfunction()

add_library(
    ava_compiler
    STATIC
        src/source.c
)

target_include_directories(
    ava_compiler
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

ava_configure_c_target(ava_compiler)

add_executable(
    ava
    src/main.c
    src/cli.c
)

target_link_libraries(
    ava
    PRIVATE
        ava_compiler
)

target_compile_definitions(
    ava
    PRIVATE
        AVA_VERSION="${PROJECT_VERSION}"
)

ava_configure_c_target(ava)

include(CTest)

if(BUILD_TESTING)
    add_executable(
        ava_source_test
        tests/source_test.c
    )

    target_link_libraries(
        ava_source_test
        PRIVATE
            ava_compiler
    )

    ava_configure_c_target(ava_source_test)

    add_test(
        NAME source_load
        COMMAND
            ava_source_test
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures/empty.ava"
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures/basic.ava"
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures/missing.ava"
    )

    add_test(
        NAME cli_help
        COMMAND ava --help
    )

    set_tests_properties(
        cli_help
        PROPERTIES
            PASS_REGULAR_EXPRESSION "AvaLang"
    )

    add_test(
        NAME cli_version
        COMMAND ava --version
    )

    set_tests_properties(
        cli_version
        PROPERTIES
            PASS_REGULAR_EXPRESSION "AvaLang 0.0.0"
    )

    add_test(
        NAME cli_unknown_option
        COMMAND ava --unknown-option
    )

    set_tests_properties(
        cli_unknown_option
        PROPERTIES
            WILL_FAIL TRUE
    )
endif()
