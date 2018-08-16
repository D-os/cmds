add_executable(bctest
    "bctest.c"
    "binder.c"
)
target_link_libraries(bctest cutils)

add_definitions("-DEXCLUDE_FS_CONFIG_STRUCTURES")
add_executable(servicemanager
    "service_manager.c"
    "binder.c"
)
target_link_libraries(servicemanager cutils selinux)
