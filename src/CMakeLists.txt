cmake_minimum_required(VERSION 3.15)
project(GeometryFigures)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Включить все предупреждения
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
endif()

# Поиск GoogleTest
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.15.0
  TLS_VERIFY false
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Добавить include директорию
include_directories(include)

# Основное приложение
add_executable(run
    src/main.cpp
)

enable_testing()

add_executable(pmr_stack_tests
	tests/test_memory_resource.cpp
	tests/test_stack.cpp
)

target_include_directories(pmr_stack_tests PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(pmr_stack_tests PRIVATE gtest_main)

add_test(NAME pmr_stack_tests COMMAND pmr_stack_tests)