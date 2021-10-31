
## Plugins
add_subdirectory(Plugins)

## Projects
add_subdirectory(STAR)
add_subdirectory(InputManagerTest)
add_subdirectory(JobSystemTest)

vs_group_subdirectory_targets(${CMAKE_CURRENT_SOURCE_DIR} "Projects")