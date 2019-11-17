add_definitions("-DXP_UNIX")

add_library(cmd-common STATIC "cmd.cpp")

add_executable(cmd "main.cpp")
target_link_libraries(cmd cmd-common utils binder)
