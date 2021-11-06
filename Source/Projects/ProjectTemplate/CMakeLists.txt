cmake_minimum_required (VERSION 3.4.3)

# Name your project. Can be anything.
project (STAR)

# Minimal supported C++ version.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set binary output directories. Changing these paths will require you adjusting your resource path configuration!
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

set(SOURCE
    BaseApplication.h
	BaseApplication.cpp
	BaseLevel.h
	BaseLevel.cpp
	CustomEvents.h
	LevelManager.h
	LevelManager.cpp
	LevelManagerEvents.h
	SceneManager.h
	SceneManager.cpp
	SceneManagerEvents.h
    STAR.h
    STAR.cpp
	
	AndroidEvents/AndroidDefines.h
	AndroidEvents/NSObject.h
	AndroidEvents/ServiceCmd.h
	AndroidEvents/ServiceCmd.cpp
	AndroidEvents/ServiceEvents.h
	AndroidEvents/Wrapper.h
	
	Audio/AudioEvents.h
	Audio/AudioManager.h
	Audio/AudioManager.cpp
	Audio/AudioManagerDefs.h
	
	BehaviourTree/BehaviourTree.h
	BehaviourTree/BehaviourTree.cpp
	BehaviourTree/BehaviourTreeDefs.h
	
	Config/ConfigFile.h
	Config/ConfigFile.cpp
	Config/ConfigManager.h
	Config/ConfigManager.cpp
	
	Console/ConsoleHandler.h
	Console/ConsoleHandler.cpp
	Console/ConsoleHandlerEvents.h
	
	Generator/Generator.h
	Generator/Generator.cpp
	Generator/PerlinNoise.h
	Generator/SimplexNoise.h
	Generator/SimplexNoise.cpp
	
	Globals/CollisionLayers.h
	Globals/GUIDefines.h
	Globals/Settings.h
	Globals/ViewLayers.h
	
	Input/ControlDefines.h
	Input/ControllerEvents.h
	Input/ControllerInput.h
	Input/ControllerInput.cpp
	Input/Controllers/BaseInput.h
	Input/Controllers/JoystickInput.h
	Input/Controllers/JoystickInput.cpp
	Input/Controllers/MouseInput.h
	Input/Controllers/MouseInput.cpp
	Input/Controllers/ScreenJoystickInput.h
	Input/Controllers/ScreenJoystickInput.cpp
	
	Levels/Credits.h
	Levels/Credits.cpp
	Levels/ExitGame.h
	Levels/ExitGame.cpp
	Levels/Level.h
	Levels/Level.cpp
	Levels/Loading.h
	Levels/Loading.cpp
	Levels/MainMenu.h
	Levels/MainMenu.cpp
	Levels/Splash.h
	Levels/Splash.cpp
	Levels/Voxel/Chunk.h
	Levels/Voxel/Chunk.cpp
	Levels/Voxel/ChunkGenerator.h
	Levels/Voxel/ChunkGenerator.cpp
	Levels/Voxel/ChunkMesh.h
	Levels/Voxel/ChunkMesh.cpp
	Levels/Voxel/LightManager.h
	Levels/Voxel/LightManager.cpp
	Levels/Voxel/TreeGenerator.h
	Levels/Voxel/TreeGenerator.cpp
	Levels/Voxel/VoxelDefs.h
	Levels/Voxel/VoxelEvents.h
	Levels/Voxel/VoxelWorld.h
	Levels/Voxel/VoxelWorld.cpp
	
	Messages/Achievements.h
	Messages/Achievements.cpp
	Messages/MessageEvents.h
	Messages/Notifications.h
	Messages/Notifications.cpp
	Messages/SingleAchievement.h
	Messages/SingleAchievement.cpp
	
	Mods/ModLoader.h
	Mods/ModLoader.cpp
	
	Nakama/NakamaEvents.h
	Nakama/NakamaManager.h
	Nakama/NakamaManager.cpp
	Nakama/NetworkProtocol.h
	Nakama/PacketHandler.h
	Nakama/PacketHandler.cpp
	
	Network/NetworkEvents.h
	
	PackageManager/PackageManager.h
	PackageManager/PackageManager.cpp
	PackageManager/PackageManagerEvents.h
	
	Player/Player.h
	Player/Player.cpp
	Player/PlayerEvents.h
	Player/PlayerState.h
	Player/PlayerState.cpp
	
	State/State.h
	State/State.cpp
	State/StateEvents.h
	
	UI/BaseWindow.h
	UI/BaseWindow.cpp
	UI/WindowEvents.h
	UI/WindowManager.h
	UI/WindowManager.cpp
	UI/Achievements/AchievementsWindow.h
	UI/Achievements/AchievementsWindow.cpp
	UI/NewGameSettings/NewGameSettingsWindow.h
	UI/NewGameSettings/NewGameSettingsWindow.cpp
	UI/Pause/PauseWindow.h
	UI/Pause/PauseWindow.cpp
	UI/PopupMessage/PopupMessageWindow.h
	UI/PopupMessage/PopupMessageWindow.cpp
	UI/QuitConfirmation/QuitConfirmationWindow.h
	UI/QuitConfirmation/QuitConfirmationWindow.cpp
	UI/Scoreboard/ScoreboardWindow.h
	UI/Scoreboard/ScoreboardWindow.cpp
	UI/Settings/SettingsWindow.h
	UI/Settings/SettingsWindow.cpp
	UI/Settings/UIOption.h
	UI/Settings/UIOption.cpp
)

# Define executable name.
#add_library(STAR STATIC ${SOURCE})
add_executable(STAR WIN32 ${SOURCE})

# Link to game engine library.
target_link_libraries(STAR
    Urho3D
)
