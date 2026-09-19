cmake_minimum_required(VERSION 3.20)

project(
    AvaLang
    VERSION 0.0.0
    LANGUAGES C
)

add_executable(
    ava
    src/main.c
)

target_compile_features(
    ava
    PRIVATE
        c_std_17
)

target_compile_definitions(
    ava
    PRIVATE
        AVA_VERSION="${PROJECT_VERSION}"
)

if(CMAKE_C_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(
        ava
        PRIVATE
            -Wall
            -Wextra
            -Wpedantic
            -Wconversion
            -Wshadow
    )
endif()

include(CTest)

if(BUILD_TESTING)
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
