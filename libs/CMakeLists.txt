
# ---------------------------------------- subdirectories --------------------------------------------------------------
# ======================================================================================================================

add_subdirectory(cxxopts EXCLUDE_FROM_ALL)
add_subdirectory(cxxsemaphore)
add_subdirectory(cxxshm)

# ---------------------------------------- link libraries --------------------------------------------------------------
# ======================================================================================================================

target_link_libraries(${Target} PRIVATE rt)
target_link_libraries(${Target} PRIVATE cxxopts)
target_link_libraries(${Target} PRIVATE cxxsemaphore)
target_link_libraries(${Target} PRIVATE cxxshm)
