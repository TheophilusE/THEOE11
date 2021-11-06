#pragma once

#include "Config/ConfigManager.h"
#include "LevelManager.h"
#include "Messages/Achievements.h"
#include "Messages/Notifications.h"
#include "UI/WindowManager.h"
#include <Urho3D/Engine/Application.h>

using namespace Urho3D;

class BaseApplication : public Application
{
    URHO3D_OBJECT(BaseApplication, Application);

public:
    BaseApplication(Context* context);

    /**
     * Setup before engine initialization
     */
    virtual void Setup() override;
    /**
     * Setup after engine initialization
     */
    virtual void Start() override;

    /**
     * Cleanup after the main loop
     */
    virtual void Stop() override;

    void Exit();

#if defined(__EMSCRIPTEN__)
    /**
     * HTML canvas size was changed, we must resize our rendered
     */
    static void JSCanvasSize(int width, int height, bool fullscreen, float scale);

    /**
     * HTML canvas has gained focus, update our mouse handling properly
     */
    static void JSMouseFocus();
#endif

private:
    /**
     * Apply specific graphics settings which are not configurable automatically
     */
    void ApplyGraphicsSettings();

    /**
     * Load configuration files
     */
    void LoadINIConfig(eastl::string filename);

    /**
     * Load custom configuration file
     */
    void LoadConfig(eastl::string filename, eastl::string prefix = "", bool isMain = false);

    /**
     * Load all translation files
     */
    void LoadTranslationFiles();

    /**
     * Handle event for config loading
     */
    void HandleLoadConfig(StringHash eventType, VariantMap& eventData);

    /**
     * Add global config
     */
    void HandleAddConfig(StringHash eventType, VariantMap& eventData);

    /**
     * Subscribe to console commands
     */
    void RegisterConsoleCommands();

    /**
     * Handle key down event to process key controls common to all Apps
     */
    void HandleKeyDown(StringHash eventType, VariantMap& eventData);
    /**
     * Handle key up event to process key controls common to all Apps.
     */
    void HandleKeyUp(StringHash eventType, VariantMap& eventData);

    /**
     * Subscribe to config events
     */
    void SubscribeToEvents();

    /**
     * Set engine parameter and make it globally available
     */
    void SetEngineParameter(eastl::string parameter, Variant value);

    /**
     * Handle exit via events
     */
    void HandleExit(StringHash eventType, VariantMap& eventData);

    /**
     * Global configuration
     */
    VariantMap globalSettings_;

    /**
     * Main configuration file
     */
    eastl::string configurationFile_;
};
