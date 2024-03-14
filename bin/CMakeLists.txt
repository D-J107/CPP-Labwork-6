add_executable(lab6 main.cpp)

target_link_libraries(lab6 ITMLparse)
target_include_directories(lab6 PRIVATE ${PROJECT_SOURCE_DIR})

set(CMAKE_EXE_LINKER_FLAGS "-static")