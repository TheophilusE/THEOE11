
## Plugins
add_subdirectory(Plugins)

## Projects
add_subdirectory(STAR)
add_subdirectory(InputManagerTest)

vs_group_subdirectory_targets(${CMAKE_CURRENT_SOURCE_DIR} "Projects")