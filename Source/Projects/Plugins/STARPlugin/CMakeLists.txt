
if (NOT URHO3D_PLUGINS OR NOT URHO3D_PLAYER)
    return ()
endif ()

set(SOURCE
	STARPlugin.h
	STARPlugin.cpp
	
	Locomotion/Character/BaseCharacter.h
	Locomotion/Character/BaseCharacter.cpp

    Locomotion/Camera/PlayerCameraManager.h
    Locomotion/Camera/PlayerCameraManager.cpp

    Locomotion/Character/CharacterController.h
    Locomotion/Character/CharacterController.cpp
)

add_library (STARPlugin ${SOURCE})
target_link_libraries (STARPlugin PUBLIC Urho3D)
target_include_directories(STARPlugin PRIVATE ..)
install(TARGETS STARPlugin
    RUNTIME DESTINATION ${DEST_BIN_DIR_CONFIG}
    LIBRARY DESTINATION ${DEST_LIBRARY_DIR_CONFIG}
    ARCHIVE DESTINATION ${DEST_ARCHIVE_DIR_CONFIG}
    BUNDLE  DESTINATION ${DEST_BIN_DIR_CONFIG}
)
