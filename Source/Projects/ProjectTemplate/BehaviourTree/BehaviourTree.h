#pragma once

#include <Urho3D/Container/Str.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Input/Controls.h>
#include <Urho3D/Scene/LogicComponent.h>

#include "BehaviourTreeDefs.h"

using namespace Urho3D;

struct BTService
{
    eastl::string name;
};

struct BTDecorator
{
    eastl::string name;
};

struct BTNode
{
    BTNodeType nodeType;
    eastl::string name;
    eastl::vector<BTService> services;
    eastl::vector<BTNode> childNodes;
    eastl::vector<BTDecorator> decorators;
};

class BehaviourTree : public LogicComponent
{
    URHO3D_OBJECT(BehaviourTree, LogicComponent);

public:
    explicit BehaviourTree(Context* context);

    virtual ~BehaviourTree();

    static void RegisterFactory(Context* context);

    void Init(const eastl::string& config);

    const Controls& GetControls();

    void FixedUpdate(float timeStep) override;

private:
    void SubscribeToEvents();

    void LoadConfig(const eastl::string& config);

    Controls controls_;

    BTNode rootNode_;
};
