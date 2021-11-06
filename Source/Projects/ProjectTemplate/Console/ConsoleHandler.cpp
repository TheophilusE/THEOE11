#include "ConsoleHandler.h"
#include "../Config/ConfigManager.h"
#include "../LevelManagerEvents.h"
#include "ConsoleHandlerEvents.h"
#include <Urho3D/Core/Object.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Engine/EngineEvents.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/UI/UI.h>
#if URHO3D_RMLUI
#include <Urho3D/RmlUI/RmlUI.h>
#endif
#if URHO3D_SYSTEMUI
#include <Urho3D/SystemUI/DebugHud.h>
#endif

using namespace ConsoleHandlerEvents;
using namespace LevelManagerEvents;

ConsoleHandler::ConsoleHandler(Context* context)
    : Object(context)
{
    Init();
}

ConsoleHandler::~ConsoleHandler() {}

void ConsoleHandler::Init() { SubscribeToEvents(); }

void ConsoleHandler::Create()
{
    Input* input = GetSubsystem<Input>();

    // Create console
    console_ = GetSubsystem<Engine>()->CreateConsole();

    /*
    // Hack to hide interpretator DropDownList
    PODVector<UIElement*> elements;
    console_->GetBackground()->GetChildren(elements, true);
    for (auto it = elements.begin(); it != elements.end(); ++it) {
        if ((*it)->GetType() == "DropDownList") {
            (*it)->SetVisible(false);
        }
    }

    for (auto it = registeredConsoleCommands_.begin(); it != registeredConsoleCommands_.end(); ++it) {
        console_->AddAutoComplete((*it).first);
    }
    */

    console_->SetVisible(false);
}

void ConsoleHandler::SubscribeToEvents()
{
    SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(ConsoleHandler, HandleKeyDown));
    SubscribeToEvent(E_CONSOLECOMMAND, URHO3D_HANDLER(ConsoleHandler, HandleConsoleCommand));
    SubscribeToEvent(E_CONSOLE_COMMAND_ADD, URHO3D_HANDLER(ConsoleHandler, HandleConsoleCommandAdd));

    VariantMap data;
    data["ConsoleCommandName"] = "help";
    data["ConsoleCommandEvent"] = "ConsoleHelp";
    data["ConsoleCommandDescription"] = "Displays all available commands";

    SendEvent("ConsoleCommandAdd", data);

    SubscribeToEvent("ConsoleHelp", URHO3D_HANDLER(ConsoleHandler, HandleConsoleCommandHelp));

    // How to use lambda (anonymous) functions
    SendEvent(E_CONSOLE_COMMAND_ADD, ConsoleCommandAdd::P_NAME, "map", ConsoleCommandAdd::P_EVENT, "#map",
              ConsoleCommandAdd::P_DESCRIPTION, "Change the map");
    SubscribeToEvent("#map",
                     [&](StringHash eventType, VariantMap& eventData)
                     {
                         StringVector params = eventData["Parameters"].GetStringVector();
                         if (params.size() != 2)
                         {
                             URHO3D_LOGERROR("Invalid number of params!");
                             return;
                         }
                         VariantMap& data = GetEventDataMap();
                         data["Name"] = "Loading";
                         data["Map"] = "Scenes/" + params[1];
                         SendEvent(E_SET_LEVEL, data);
                     });

    SendEvent(E_CONSOLE_COMMAND_ADD, ConsoleCommandAdd::P_NAME, "opacity", ConsoleCommandAdd::P_EVENT, "#opacity",
              ConsoleCommandAdd::P_DESCRIPTION, "Change the console opacity");
    SubscribeToEvent("#opacity",
                     [&](StringHash eventType, VariantMap& eventData)
                     {
                         StringVector params = eventData["Parameters"].GetStringVector();
                         if (params.size() != 2)
                         {
                             URHO3D_LOGERROR("Invalid number of params!");
                             return;
                         }
                         float value = ToFloat(params[1]);
                         // console_->GetBackground()->SetOpacity(value);
                     });
}

void ConsoleHandler::HandleKeyDown(StringHash eventType, VariantMap& eventData)
{
    //if (GetSubsystem<ConfigManager>()->GetBool("game", "DeveloperConsole", true))
    {
        using namespace KeyDown;
        int key = eventData[P_KEY].GetInt();

    // Toggle console with F1 or backquote
#if URHO3D_SYSTEMUI
        if (key == KEY_F1 || key == KEY_BACKQUOTE)
        {
#if URHO3D_RMLUI
            if (auto* ui = GetSubsystem<RmlUI>())
            {
                if (ui->IsInputCaptured())
                    return;
            }
#endif
            if (auto* ui = GetSubsystem<UI>())
            {
                if (UIElement* element = ui->GetFocusElement())
                {
                    if (element->IsEditable())
                        return;
                }
            }

            console_->Toggle();
            auto input = GetSubsystem<Input>();
            if (console_->IsVisible())
            {
                input->SetMouseVisible(true);
            }
            else
            {
                input->ResetMouseVisible();
            }
        }
        // Toggle debug HUD with F2
        else if (key == KEY_F2)
        {
            GetSubsystem<Engine>()->CreateDebugHud()->ToggleAll();
            return;
        }
#endif
    }
    //else
    {
        // If console is still visible when it was disabled via options, hide it
        if (console_->IsVisible())
        {
           //console_->Toggle();
        }
    }
}

void ConsoleHandler::HandleConsoleCommandAdd(StringHash eventType, VariantMap& eventData)
{
    using namespace ConsoleCommandAdd;
    eastl::string command = eventData[P_NAME].GetString();
    eastl::string eventToCall = eventData[P_EVENT].GetString();
    eastl::string description = eventData[P_DESCRIPTION].GetString();
    bool overwrite = eventData[P_OVERWRITE].GetBool();
    if (registeredConsoleCommands_.contains(command) && !overwrite)
    {
        URHO3D_LOGWARNINGF("Console command '{%s}' already registered! Skipping console command registration!",
                           command.c_str());
        return;
    }

    // Add to autocomplete
    if (console_)
    {
        // console_->AddAutoComplete(command);
    }

    // Register new console command
    SingleConsoleCommand singleConsoleCommand;
    singleConsoleCommand.command = command;
    singleConsoleCommand.description = description;
    singleConsoleCommand.eventToCall = eventToCall;
    registeredConsoleCommands_[command] = singleConsoleCommand;
}

void ConsoleHandler::HandleConsoleCommand(StringHash eventType, VariantMap& eventData)
{
    using namespace ConsoleCommand;
    eastl::string input = eventData[P_COMMAND].GetString();
    URHO3D_LOGINFO(input);
    ParseCommand(input);
}

void ConsoleHandler::ParseCommand(eastl::string input)
{
    if (input.empty())
    {
        return;
    }
    StringVector params = input.split(' ', false);
    eastl::string command = params[0];
    if (registeredConsoleCommands_.contains(command))
    {
        VariantMap data;
        data["Parameters"] = params;

        // Call the actual event and pass all the parameters
        SendEvent(registeredConsoleCommands_[command].eventToCall, data);
    }
    else
    {
        if (GetGlobalVar(command) != Variant::EMPTY)
        {
            HandleConsoleGlobalVariableChange(params);
        }
        else
        {
            URHO3D_LOGERRORF("Command '{%s}' not registered", command.c_str());
        }
    }
}

void ConsoleHandler::HandleConsoleCommandHelp(StringHash eventType, VariantMap& eventData)
{
    URHO3D_LOGINFO("");
    URHO3D_LOGINFO("------- All available (registered) commands -------");
    URHO3D_LOGINFO("-");
    for (auto it = registeredConsoleCommands_.begin(); it != registeredConsoleCommands_.end(); ++it)
    {
        SingleConsoleCommand info = (*it).second;
        URHO3D_LOGINFOF("- '{%s}' => '{%s}': {%s}", info.command.c_str(), info.eventToCall.c_str(),
                        info.description.c_str());
    }
    URHO3D_LOGINFO("-");
    URHO3D_LOGINFO("---------------------------------------------------");
    URHO3D_LOGINFO("");
}

void ConsoleHandler::HandleConsoleGlobalVariableChange(StringVector params)
{
    const Variant value = GetSubsystem<Engine>()->GetGlobalVar(params[0]);

    // Only show variable
    if (params.size() == 1 && !value.IsEmpty())
    {
        eastl::string stringValue;
        if (value.GetType() == VAR_STRING)
        {
            stringValue = value.GetString();
        }
        if (value.GetType() == VAR_BOOL)
        {
            stringValue = value.GetBool() ? "1" : "0";
        }
        if (value.GetType() == VAR_INT)
        {
            stringValue = fmt::format("{}", value.GetInt());
        }
        if (value.GetType() == VAR_FLOAT)
        {
            stringValue = fmt::format("{}", value.GetFloat());
        }
        if (value.GetType() == VAR_DOUBLE)
        {
            stringValue = fmt::format("{}", value.GetDouble());
        }
        URHO3D_LOGINFO(params[0] + " = " + stringValue);
        return;
    }

    // Read console input parameters and change global variable
    if (params.size() == 2)
    {
        eastl::string oldValue;
        eastl::string newValue;
        if (value.GetType() == VAR_STRING)
        {
            oldValue = value.GetString();
            SetGlobalVar(params[0], params[1]);
            newValue = params[1];
        }
        if (value.GetType() == VAR_BOOL)
        {
            oldValue = value.GetBool() ? "1" : "0";
            if (params[1] == "1" || params[1] == "true")
            {
                SetGlobalVar(params[0], true);
                newValue = "1";
            }
            if (params[1] == "0" || params[1] == "false")
            {
                SetGlobalVar(params[0], false);
                newValue = "0";
            }
        }
        if (value.GetType() == VAR_INT)
        {
            oldValue = fmt::format("{}", value.GetInt());
            int newIntVal = ToInt(params[1].c_str());
            SetGlobalVar(params[0], newIntVal);
            newValue = fmt::format("{}", newIntVal);
        }
        if (value.GetType() == VAR_FLOAT)
        {
            oldValue = fmt::format("{}", value.GetFloat());
            float newFloatVal = ToFloat(params[1].c_str());
            SetGlobalVar(params[0], newFloatVal);
            newValue = fmt::format("{}", newFloatVal);
        }
        if (value.GetType() == VAR_DOUBLE)
        {
            oldValue = fmt::format("{}", value.GetDouble());
            float newFloatVal = ToFloat(params[1].c_str());
            SetGlobalVar(params[0], newFloatVal);
            newValue = fmt::format("{}", newFloatVal);
        }
        URHO3D_LOGINFO("Global variable changed'" + params[0] + "' from '" + oldValue + "' to '" + newValue + "'");

        // Let the system know that variable was changed
        using namespace ConsoleGlobalVariableChanged;
        VariantMap& data = GetEventDataMap();
        data[P_NAME] = params[0];
        data[P_VALUE] = newValue;

        SendEvent("global_variable_changed_" + params[0], data);
        SendEvent(E_CONSOLE_GLOBAL_VARIABLE_CHANGED, data);
    }
}
