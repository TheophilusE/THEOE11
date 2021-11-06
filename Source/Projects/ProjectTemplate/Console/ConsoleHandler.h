#pragma once

#include <Urho3D/Container/Str.h>
#include <Urho3D/SystemUI/Console.h>

using namespace Urho3D;

struct SingleConsoleCommand
{
    eastl::string command;
    eastl::string eventToCall;
    eastl::string description;
};

class ConsoleHandler : public Object
{
    URHO3D_OBJECT(ConsoleHandler, Object);

public:
    ConsoleHandler(Context* context);

    virtual ~ConsoleHandler();

    virtual void Init();

    /**
     * Create the console
     */
    virtual void Create();

private:
    /**
     * Subscribe console related events
     */
    void SubscribeToEvents();

    /**
     * Toggle console
     */
    void HandleKeyDown(StringHash eventType, VariantMap& eventData);

    /**
     * Add new console command
     */
    void HandleConsoleCommandAdd(StringHash eventType, VariantMap& eventData);

    /**
     * Process incomming console commands
     */
    void HandleConsoleCommand(StringHash eventType, VariantMap& eventData);

    /**
     * Parse incomming command input
     */
    void ParseCommand(eastl::string input);

    /**
     * Display help
     */
    void HandleConsoleCommandHelp(StringHash eventType, VariantMap& eventData);

    /**
     * Handle configuration change via console
     */
    void HandleConsoleGlobalVariableChange(StringVector params);

    /**
     * Registered console commands
     */
    eastl::hash_map<eastl::string, SingleConsoleCommand> registeredConsoleCommands_;

    /**
     * Console handler in the engine
     */
    SharedPtr<Console> console_;
};