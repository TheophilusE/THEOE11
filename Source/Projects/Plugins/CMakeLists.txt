
add_subdirectory(103_GamePlugin)
add_subdirectory(STARPlugin)
add_subdirectory(PIDControllerPlugin)

vs_group_subdirectory_targets(${CMAKE_CURRENT_SOURCE_DIR} "Plugins")